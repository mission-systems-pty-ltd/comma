import numpy as np
import sys
from StringIO import StringIO
import argparse
import itertools
import warnings
import operator
import re
import comma.csv.format
import comma.csv.time


class struct_error(Exception):
    pass


class stream_error(Exception):
    pass


def custom_formatwarning(msg, *args):
    return " comma.csv warning: " + str(msg) + '\n'
warnings.formatwarning = custom_formatwarning


def strip_prefix(string, prefix_chars='<>|='):
    if string.startswith(tuple(prefix_chars)):
        return string[1:]


def merge_arrays(first, second):
    """
    merge two arrays faster than np.lib.recfunctions.merge_arrays
    """
    if first.size != second.size:
        msg = "array sizes not equal, {} != {}".format(first.size, second.size)
        raise ValueError(msg)
    dtype = np.dtype([('first', first.dtype), ('second', second.dtype)])
    merged = np.empty(first.size, dtype=dtype)
    for name in first.dtype.names:
        merged['first'][name] = first[name]
    for name in second.dtype.names:
        merged['second'][name] = second[name]
    return merged


def unrolled_types_of_flat_dtype(dtype):
    """
    transform a dtype into unrolled types
    e.g. np.dtype('u4,(2,3)f8') becomes ('u4', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8')
    """
    shape_unrolled_types = []
    for descr in dtype.descr:
        type = strip_prefix(descr[1])
        shape = descr[2] if len(descr) > 2 else ()
        shape_unrolled_types.extend([type] * reduce(operator.mul, shape, 1))
    return tuple(shape_unrolled_types)


def structured_dtype(numpy_format):
    """
    return structured array even for a format string containing a single type
    note: passing a single type format string to numpy dtype returns a scalar,
        e.g., np.dtype('f8') yields dtype('float64'),
        whereas np.dtype('f8,f8') yields dtype([('f0', '<f8'), ('f1', '<f8')])
    """
    number_of_types = len(re.sub(r'\([^\)]*\)', '', numpy_format).split(','))
    if number_of_types == 1:
        return np.dtype([('f0', numpy_format)])
    else:
        return np.dtype(numpy_format)


def format_from_types(types):
    """
    return the format string corresponding to a sequence of types,
    which can contain types as strings (including arrays) and numpy types,
        e.g., ( 'f8', '(2,3)u4', np.uint32 )
    NOTE: add checks to ensure type strings are valid types (use descr, but make sure it works for arrays)
    """
    types_as_strings = []
    for type in types:
        if isinstance(type, basestring):
            type_as_string = type
        else:
            type_as_string = strip_prefix(np.dtype(type).str)
        types_as_strings.append(type_as_string)
    return ','.join(types_as_strings)


def readlines(source, size):
    """
    read lines from source, such as stdin, without buffering
    builtin readlines() buffers the input and hence prevents flushing every line
    """
    if size >= 0:
        lines = ''
        number_of_lines = 0
        while number_of_lines < size:
            line = source.readline()
            if not line:
                break
            lines += line
            number_of_lines += 1
    else:
        lines = source.read()
    return lines


def add_standard_options( parser, defaults={} ):

    def help_text( help, default=None ):
        return "%s (default: %s)" % ( help, default ) if default else help

    option_defaults = {
        'fields': '',
        'binary': '',
        'delimiter': ',',
        'precision': 12
    }
    option_defaults.update( defaults )

    parser.add_argument( "-f", "--fields", default=option_defaults["fields"], metavar="<names>",
                         help=help_text( "field names of input stream", option_defaults["fields"] ))

    parser.add_argument( "-b", "--binary", default=option_defaults["binary"], metavar="<format>",
                         help="format for binary stream (default: ascii)" )

    parser.add_argument( "-d", "--delimiter", default=option_defaults["delimiter"], metavar="<char>",
                         help=help_text( "csv delimiter of ascii stream", option_defaults["delimiter"] ))

    parser.add_argument( "--precision", default=option_defaults["precision"], metavar="<precision>",
                         help=help_text( "floating point precision of ascii output", option_defaults["precision"] ))

    parser.add_argument( "--flush", "--unbuffered", action="store_true",
                         help="flush stdout after each record (stream is unbuffered)" )


class struct:
    default_field_name = 'comma_struct_default_field_name_'

    def __init__(self, concise_fields, *concise_types):
        self.concise_types = concise_types
        self.concise_fields = self._fill_blanks(concise_fields)
        self._check_fields_conciseness()
        self.dtype = np.dtype(zip(self.concise_fields, self.concise_types))
        self.fields = self._full_xpath_fields()
        self.types = self._basic_types()
        self.shorthand = self._shorthand()
        self.format = format_from_types(self.types)
        self.flat_dtype = np.dtype(zip(self.fields, self.types))
        unrolled_types = unrolled_types_of_flat_dtype(self.flat_dtype)
        self.unrolled_flat_dtype = structured_dtype(','.join(unrolled_types))
        self.type_of_field = dict(zip(self.fields, self.types))
        leaves = tuple(xpath.split('/')[-1] for xpath in self.fields)
        self.ambiguous_leaves = set(leaf for leaf in leaves if leaves.count(leaf) > 1)
        self.xpath_of_leaf = self._xpath_of_leaf(leaves)

    def __call__(self, size=1):
        return np.empty(size, dtype=self)

    def to_tuple(self, s):
        if s.dtype != self.dtype:
            msg = "expected {}, got {}".format(repr(self.dtype), repr(s.dtype))
            raise struct_error(msg)
        if s.shape != (1,):
            msg = "expected shape=(1,), got {}".format(s.shape)
            raise struct_error(msg)
        return s.view(self.unrolled_flat_dtype).item()

    def expand_shorthand(self, compressed_fields):
        if isinstance(compressed_fields, basestring):
            compressed_fields = compressed_fields.split(',')
        expand = self.shorthand.get
        field_tuples = map(lambda name: expand(name) or (name,), compressed_fields)
        return sum(field_tuples, ())

    def _fill_blanks(self, fields):
        if isinstance(fields, basestring):
            fields = fields.split(',')
        ntypes = len(self.concise_types)
        if len(fields) > ntypes:
            fields_without_type = ','.join(fields.split(',')[ntypes:])
            msg = "missing types for fields '{}'".format(fields_without_type)
            raise struct_error(msg)
        omitted_fields = [''] * (ntypes - len(fields))
        fields_without_blanks = []
        for index, field in enumerate(fields + omitted_fields):
            if field:
                nonblank_field = field
            else:
                nonblank_field = '{}{}'.format(struct.default_field_name, index)
            fields_without_blanks.append(nonblank_field)
        return fields_without_blanks

    def _check_fields_conciseness(self):
        for field in self.concise_fields:
            if '/' in field:
                msg = "expected fields without '/', got '{}'".format(self.concise_fields)
                raise struct_error(msg)

    def _full_xpath_fields(self):
        fields = []
        for name, type in zip(self.concise_fields, self.concise_types):
            if isinstance(type, struct):
                fields_of_type = [name + '/' + field for field in type.fields]
                fields.extend(fields_of_type)
            else:
                fields.append(name)
        return tuple(fields)

    def _basic_types(self):
        types = []
        for type in self.concise_types:
            if isinstance(type, struct):
                types.extend(type.types)
            else:
                types.append(type)
        return tuple(types)

    def _shorthand(self):
        shorthand = {}
        for name, type in zip(self.concise_fields, self.concise_types):
            if not isinstance(type, struct):
                continue
            fields_of_type = [name + '/' + field for field in type.fields]
            shorthand[name] = tuple(fields_of_type)
            for subname, subfields in type.shorthand.iteritems():
                xpath = name + '/' + subname
                shorthand[xpath] = tuple(name + '/' + field for field in subfields)
        return shorthand

    def _xpath_of_leaf(self, leaves):
        xpath_of_leaf = dict(zip(leaves, self.fields))
        for ambiguous_leaf in self.ambiguous_leaves:
            del xpath_of_leaf[ambiguous_leaf]
        return xpath_of_leaf


class stream:
    buffer_size_in_bytes = 65536

    def __init__(self,
                 s,
                 fields='',
                 format='',
                 binary=None,
                 delimiter=',',
                 precision=12,
                 flush=False,
                 source=sys.stdin,
                 target=sys.stdout,
                 tied=None,
                 full_xpath=True,
                 verbose=False,
                 default_values=None):
        self.struct = self._struct(s)
        self.delimiter = delimiter
        self.precision = precision
        self.flush = flush
        self.source = source
        self.target = target
        self.tied = tied
        self.full_xpath = full_xpath
        self.verbose = verbose
        self.fields = self._fields(fields)
        self._check_fields_uniqueness()
        self.format = self._format(binary, format)
        self.binary = self.format != ''
        self._check_consistency_with_tied()
        self.input_dtype = self._input_dtype()
        self.size = self._default_buffer_size()
        if not self.binary:
            unrolled_types = unrolled_types_of_flat_dtype(self.input_dtype)
            self.ascii_converters = comma.csv.time.ascii_converters(unrolled_types)
            num_utypes = len(unrolled_types)
            self.usecols = tuple(range(num_utypes))
            self.filling_values = '' if num_utypes == 1 else ('',) * num_utypes
        self.missing_fields = self._missing_fields()
        self.missing_dtype = self._missing_dtype()
        self.complete_fields = self.fields + self.missing_fields
        self.complete_dtype = self._complete_dtype()
        self.default_values = self._default_values(default_values)
        self.data_extraction_dtype = self._data_extraction_dtype()
        self._input_data = None
        self._missing_data = None
        self._ascii_buffer = None

    def iter(self, size=None):
        while True:
            s = self.read(size)
            if s is None:
                break
            yield s

    def read(self, size=None):
        if size is None:
            size = self.size
        if size < 0:
            if self.source == sys.stdin:
                msg = "stdin requires positive size, got {}".format(size)
                raise stream_error(msg)
            size = -1  # read entire file
        if self.binary:
            self._input_data = np.fromfile(self.source, dtype=self.input_dtype, count=size)
        else:
            with warnings.catch_warnings():
                warnings.simplefilter('ignore')
                self._ascii_buffer = readlines(self.source, size)
                if not self._ascii_buffer:
                    return
                self._input_data = np.atleast_1d(np.genfromtxt(
                    StringIO(self._ascii_buffer),
                    dtype=self.input_dtype,
                    delimiter=self.delimiter,
                    converters=self.ascii_converters,
                    usecols=self.usecols,
                    filling_values=self.filling_values,
                    comments=None))
        if self._input_data.size == 0:
            return
        if not self.data_extraction_dtype:
            return self._input_data.copy().view(self.struct)
        if self.missing_fields:
            missing_data = self.missing_data(self._input_data.size)
            complete_data = merge_arrays(self._input_data, missing_data)
        else:
            complete_data = self._input_data
        raw_extracted_data = np.ndarray(
            shape=complete_data.shape,
            dtype=self.data_extraction_dtype,
            buffer=complete_data,
            strides=complete_data.itemsize).tolist()
        extracted_data = np.array(raw_extracted_data, dtype=self.struct.flat_dtype)
        return extracted_data.view(self.struct)

    def missing_data(self, size):
        if self._missing_data is not None and size <= self._missing_data.size:
            return self._missing_data[:size]
        self._missing_data = np.zeros(size, dtype=self.missing_dtype)
        if not self.default_values:
            return self._missing_data
        dtype_name_of = dict(zip(self.missing_fields, self.missing_dtype.names))
        for field, value in self.default_values.iteritems():
            name = dtype_name_of[field]
            if self.missing_dtype[name] == np.dtype(comma.csv.time.NUMPY_TYPE):
                self._missing_data[name] = comma.csv.time.to_numpy(value)
            else:
                self._missing_data[name] = value
        return self._missing_data

    def numpy_scalar_to_string(self, scalar):
        if scalar.dtype.char in np.typecodes['AllInteger']:
            return str(scalar)
        elif scalar.dtype.char in np.typecodes['Float']:
            return "{scalar:.{precision}g}".format(scalar=scalar, precision=self.precision)
        elif scalar.dtype.char in np.typecodes['Datetime']:
            return comma.csv.time.from_numpy(scalar)
        elif scalar.dtype.char in 'S':
            return scalar
        msg = "converting {} to string is not implemented".format(repr(scalar.dtype))
        raise stream_error(msg)

    def write(self, s):
        if s.dtype != self.struct.dtype:
            msg = "expected {}, got {}".format(repr(self.struct.dtype), repr(s.dtype))
            raise stream_error(msg)
        if s.shape != (s.size,):
            msg = "expected shape=({},), got {}".format(s.size, s.shape)
            raise stream_error(msg)
        if self.tied and s.size != self.tied._input_data.size:
            msg = "size {} not equal to tied size {}".format(s.size, self.tied.size)
            raise stream_error(msg)
        if self.binary:
            (merge_arrays(self.tied._input_data, s) if self.tied else s).tofile(self.target)
        else:
            tied_lines = self.tied._ascii_buffer.splitlines() if self.tied else []
            unrolled_s = s.view(self.struct.unrolled_flat_dtype)
            for tied_line, scalars in itertools.izip_longest(tied_lines, unrolled_s):
                tied_line_with_separator = tied_line + self.delimiter if self.tied else ''
                output_line = self.delimiter.join(map(self.numpy_scalar_to_string, scalars))
                print >> self.target, tied_line_with_separator + output_line
        self.target.flush()

    def _struct(self, s):
        if not isinstance(s, struct):
            msg = "expected '{}', got '{}'".format(repr(struct), repr(s))
            raise stream_error(msg)
        return s

    def _fields(self, fields):
        if fields == '':
            return self.struct.fields
        if self.full_xpath:
            return self.struct.expand_shorthand(fields)
        if '/' in fields:
            msg = "expected fields without '/', got '{}'".format(fields)
            raise stream_error(msg)
        ambiguous_leaves = self.struct.ambiguous_leaves.intersection(fields.split(','))
        if ambiguous_leaves:
            msg = "fields '{}' are ambiguous in '{}', use full xpath" \
                .format(','.join(ambiguous_leaves), fields)
            raise stream_error(msg)
        xpath = self.struct.xpath_of_leaf.get
        full_xpath_fields = tuple(xpath(name) or name for name in fields.split(','))
        if not set(full_xpath_fields).intersection(self.struct.fields):
            msg = "fields '{}' do not match any of expected fields '{}'" \
                .format(fields, ','.join(self.struct.fields))
            raise stream_error(msg)
        return full_xpath_fields

    def _format(self, binary, format):
        if isinstance(binary, basestring):
            if self.verbose and binary and format and binary != format:
                msg = "ignoring '{}' and using '{}' since binary keyword has priority" \
                    .format(format, binary)
                warnings.warn(msg)
            return binary
        if binary is True and not format:
            if not set(self.struct.fields).issuperset(self.fields):
                msg = "failed to infer type of every field in '{}', specify format" \
                    .format(','.join(self.fields))
                raise stream_error(msg)
            type_of = self.struct.type_of_field.get
            return format_from_types(type_of(field) for field in self.fields)
        elif binary is False and format:
            return ''
        return format

    def _check_fields_uniqueness(self):
        duplicates = tuple(field for field in self.struct.fields
                           if field in self.fields and self.fields.count(field) > 1)
        if duplicates:
            msg = "fields '{}' have duplicates in '{}'" \
                "".format(','.join(duplicates), ','.join(self.fields))
            raise stream_error(msg)

    def _check_consistency_with_tied(self):
        if not self.tied:
            return
        if not isinstance(self.tied, stream):
            msg = "expected tied stream of type '{}', got '{}'" \
                "".format(str(stream), repr(self.tied))
            raise stream_error(msg)
        if self.tied.binary != self.binary:
            msg = "expected tied stream to be {}, got {}" \
                "".format("binary" if self.binary else "ascii",
                          "binary" if self.tied.binary else "ascii")
            raise stream_error(msg)
        if not self.binary and self.tied.delimiter != self.delimiter:
            msg = "expected tied stream to have the same delimiter '{}', got '{}'" \
                "".format(self.delimiter, self.tied.delimiter)
            raise stream_error(msg)

    def _input_dtype(self):
        if self.binary:
            input_dtype = structured_dtype(self.format)
            if len(self.fields) != len(input_dtype.names):
                msg = "expected same number of fields and format types, got '{}' and '{}'" \
                    "".format(','.join(self.fields), self.format)
                raise stream_error(msg)
        else:
            if self.fields == self.struct.fields:
                input_dtype = self.struct.flat_dtype
            else:
                type_of = self.struct.type_of_field.get
                types = [type_of(name) or 'S' for name in self.fields]
                format = format_from_types(types)
                input_dtype = structured_dtype(format)
        return input_dtype

    def _default_buffer_size(self):
        if self.tied:
            return self.tied.size
        elif self.flush:
            return 1
        else:
            return max(1, stream.buffer_size_in_bytes / self.input_dtype.itemsize)

    def _missing_fields(self):
        missing_fields = [field for field in self.struct.fields if field not in self.fields]
        if not missing_fields:
            return ()
        if self.verbose:
            msg = "expected fields '{}' are not found in supplied fields '{}'" \
                .format(','.join(missing_fields), ','.join(self.fields))
            warnings.warn(msg)
        return tuple(missing_fields)

    def _missing_dtype(self):
        if not self.missing_fields:
            return
        n = len(self.input_dtype.names)
        missing_names = ['f{}'.format(n + i) for i in xrange(len(self.missing_fields))]
        type_of = self.struct.type_of_field.get
        missing_types = [type_of(name) for name in self.missing_fields]
        return np.dtype(zip(missing_names, missing_types))

    def _complete_dtype(self):
        if self.missing_dtype:
            return np.dtype(self.input_dtype.descr + self.missing_dtype.descr)
        else:
            return self.input_dtype

    def _default_values(self, default_values):
        if not (self.missing_fields and default_values):
            return
        if self.full_xpath:
            default_fields_ = default_values.keys()
            default_values_ = default_values.copy()
        else:
            leaves = default_values.keys()
            xpath_of = self.struct.xpath_of_leaf.get
            default_fields_ = tuple(xpath_of(leaf) or leaf for leaf in leaves)
            default_values_ = dict(zip(default_fields_, default_values.values()))
        default_fields_in_stream = set(default_fields_).intersection(self.fields)
        unknown_default_fields = set(default_fields_).difference(self.struct.fields)
        if default_fields_in_stream:
            if self.verbose:
                msg = "default values for fields in stream are ignored: '{}'" \
                    "".format(','.join(default_fields_in_stream))
                warnings.warn(msg)
        if unknown_default_fields:
            if self.verbose:
                msg = "found default values for fields not in struct: '{}'" \
                    "".format(','.join(unknown_default_fields))
                warnings.warn(msg)
        for field in default_fields_in_stream.union(unknown_default_fields):
            del default_values_[field]
        return default_values_

    def _data_extraction_dtype(self):
        if self.fields == self.struct.fields:
            return
        index_of = self.complete_fields.index
        names = ['f{}'.format(index_of(field)) for field in self.struct.fields]
        formats = [self.complete_dtype.fields[name][0] for name in names]
        offsets = [self.complete_dtype.fields[name][1] for name in names]
        return np.dtype(dict(names=names, formats=formats, offsets=offsets))

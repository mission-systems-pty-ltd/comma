import numpy as np
import sys
from StringIO import StringIO
import itertools
import warnings
import operator
import re
import comma.csv.format
import comma.csv.options
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


class struct:
    default_field_name = 'comma_struct_default_field_name_'

    def __init__(self, concise_fields, *concise_types):
        if '/' in concise_fields:
            msg = "expected fields without '/', got '{}'".format(concise_fields)
            raise struct_error(msg)
        given_fields = concise_fields.split(',')
        if len(given_fields) > len(concise_types):
            fields_without_type = ','.join(concise_fields.split(',')[len(concise_types):])
            msg = "missing types for fields '{}'".format(fields_without_type)
            raise struct_error(msg)
        omitted_fields = [''] * (len(concise_types) - len(given_fields))
        concise_fields_no_blanks = []
        for index, field in enumerate(given_fields + omitted_fields):
            if field:
                nonblank_field = field
            else:
                nonblank_field = '{}{}'.format(struct.default_field_name, index)
            concise_fields_no_blanks.append(nonblank_field)
        self.dtype = np.dtype(zip(concise_fields_no_blanks, concise_types))
        fields, types = [], []
        self.shorthand = {}
        for name, type in zip(concise_fields_no_blanks, concise_types):
            if isinstance(type, struct):
                fields_of_type = [name + '/' + field for field in type.fields]
                fields.extend(fields_of_type)
                types.extend(type.types)
                self.shorthand[name] = tuple(fields_of_type)
                for subname, subfields in type.shorthand.iteritems():
                    xpath = name + '/' + subname
                    self.shorthand[xpath] = tuple(name + '/' + field for field in subfields)
            else:
                fields.append(name)
                types.append(type)
        self.fields = tuple(fields)
        self.types = tuple(types)
        self.format = format_from_types(self.types)
        self.flat_dtype = np.dtype(zip(self.fields, self.types))
        unrolled_types = unrolled_types_of_flat_dtype(self.flat_dtype)
        self.unrolled_flat_dtype = structured_dtype(','.join(unrolled_types))
        self.type_of_field = dict(zip(self.fields, self.types))
        leaves = tuple(xpath.split('/')[-1] for xpath in self.fields)
        self.ambiguous_leaves = set(leaf for leaf in leaves if leaves.count(leaf) > 1)
        d = dict(zip(leaves, self.fields))
        for ambiguous_leaf in self.ambiguous_leaves:
            del d[ambiguous_leaf]
        self.xpath_of_leaf = d


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
                 verbose=False):
        if not isinstance(s, struct):
            raise stream_error("expected '{}', got '{}'".format(str(struct), repr(s)))
        self.struct = s
        self.full_xpath = full_xpath
        if fields == '':
            self.fields = self.struct.fields
        else:
            if self.full_xpath:
                shorthand = self.struct.shorthand.get
                fields_ = map(lambda name: shorthand(name) or (name,), fields.split(','))
                self.fields = sum(fields_, ())
            else:
                if '/' in fields:
                    msg = "expected fields without '/', got '{}'".format(fields)
                    raise stream_error(msg)
                ambiguous_leaves = self.struct.ambiguous_leaves.intersection(fields.split(','))
                if ambiguous_leaves:
                    msg = "fields '{}' are ambiguous in '{}', use full xpath" \
                        "".format(','.join(ambiguous_leaves), fields)
                    raise stream_error(msg)
                self.fields = tuple(self.struct.xpath_of_leaf.get(name) or name for name in fields.split(','))
            if not set(self.fields).intersection(self.struct.fields):
                msg = "fields '{}' do not match any of expected fields '{}'" \
                    "".format(fields, ','.join(self.struct.fields))
                raise stream_error(msg)
        if binary is True and not format:
            if not set(self.struct.fields).issuperset(self.fields):
                msg = "failed to infer type of every field in '{}', specify format" \
                    "".format(','.join(self.fields))
                raise stream_error(msg)
            format = format_from_types(self.struct.type_of_field[name] for name in self.fields)
        elif binary is False and format:
            format = ''
        self.binary = format != ''
        self.delimiter = delimiter
        self.flush = flush
        self.precision = precision
        self.source = source
        self.target = target
        self.tied = tied
        self.verbose = verbose
        duplicates = tuple(field for field in self.struct.fields if field in self.fields and self.fields.count(field) > 1)
        if duplicates:
            msg = "fields '{}' have duplicates in '{}'" \
                "".format(','.join(duplicates), ','.join(self.fields))
            raise stream_error(msg)
        if self.tied:
            if not isinstance(tied, stream):
                msg = "expected tied stream of type '{}', got '{}'" \
                    "".format(str(stream), repr(tied))
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
        if self.binary:
            self.input_dtype = structured_dtype(format)
            if len(self.fields) != len(self.input_dtype.names):
                msg = "expected same number of fields and format types, got '{}' and '{}'" \
                    "".format(','.join(self.fields), format)
                raise stream_error(msg)
        else:
            if self.fields == self.struct.fields:
                self.input_dtype = self.struct.flat_dtype
            else:
                types_ = [self.struct.type_of_field.get(name) or 'S' for name in self.fields]
                format_ = format_from_types(types_)
                self.input_dtype = structured_dtype(format_)
            unrolled_types = unrolled_types_of_flat_dtype(self.input_dtype)
            self.ascii_converters = comma.csv.time.ascii_converters(unrolled_types)
            self.usecols = tuple(range(len(unrolled_types)))
            self.filling_values = '' if len(unrolled_types) == 1 else ('',) * len(unrolled_types)
        if self.tied:
            self.size = self.tied.size
        else:
            if self.flush:
                self.size = 1
            else:
                self.size = max(1, stream.buffer_size_in_bytes / self.input_dtype.itemsize)
        if set(self.fields).issuperset(self.struct.fields):
            self.missing_fields = ()
        else:
            self.missing_fields = tuple(field for field in self.struct.fields if field not in self.fields)
            if self.verbose:
                warning_msg = "expected fields '{}' are not found in supplied fields '{}'" \
                    "".format(','.join(self.missing_fields), ','.join(self.fields))
                warnings.warn(warning_msg)
        if self.missing_fields:
            self.complete_fields = self.fields + self.missing_fields
            missing_names = ['f' + str(i + len(self.input_dtype.names)) for i in xrange(len(self.missing_fields))]
            missing_types = [self.struct.type_of_field.get(name) for name in self.missing_fields]
            self.missing_fields_dtype = np.dtype(zip(missing_names, missing_types))
            self.complete_dtype = np.dtype(self.input_dtype.descr + zip(missing_names, missing_types))
        else:
            self.complete_fields = self.fields
            self.missing_fields_dtype = None
            self.complete_dtype = self.input_dtype
        if self.fields == self.struct.fields:
            self.data_extraction_dtype = None
        else:
            names = ['f' + str(self.complete_fields.index(name)) for name in self.struct.fields]
            formats = [self.complete_dtype.fields[name][0] for name in names]
            offsets = [self.complete_dtype.fields[name][1] for name in names]
            self.data_extraction_dtype = np.dtype(dict(names=names, formats=formats, offsets=offsets))

    def iter(self, size=None):
        while True:
            s = self.read(size)
            if s is None:
                break
            yield s

    def read(self, size=None):
        size = self.size if size is None else size
        if size < 0:
            if self.source == sys.stdin:
                msg = "stdin requires positive size, got {}".format(size)
                raise stream_error(msg)
            size = -1  # read entire file
        if self.binary:
            self.input_data = np.fromfile(self.source, dtype=self.input_dtype, count=size)
        else:
            with warnings.catch_warnings():
                warnings.simplefilter('ignore')
                self.ascii_buffer = readlines(self.source, size)
                if not self.ascii_buffer:
                    return None
                self.input_data = np.atleast_1d(
                    np.genfromtxt(
                        StringIO(self.ascii_buffer),
                        dtype=self.input_dtype,
                        delimiter=self.delimiter,
                        converters=self.ascii_converters,
                        usecols=self.usecols,
                        filling_values=self.filling_values,
                        comments=None))
        if self.input_data.size == 0:
            return None
        if self.data_extraction_dtype:
            if self.missing_fields:
                missing_data = np.zeros(self.input_data.size, dtype=self.missing_fields_dtype)
                complete_data = merge_arrays(self.input_data, missing_data)
            else:
                complete_data = self.input_data
            raw_extracted_data = np.ndarray(
                complete_data.shape,
                self.data_extraction_dtype,
                complete_data,
                strides=complete_data.itemsize).tolist()
            extracted_data = np.array(raw_extracted_data, dtype=self.struct.flat_dtype)
            return extracted_data.view(self.struct)
        else:
            return self.input_data.copy().view(self.struct)

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
        if self.tied and s.size != self.tied.input_data.size:
            msg = "size {} not equal to tied size {}".format(s.size, self.tied.size)
            raise stream_error(msg)
        if self.binary:
            (merge_arrays(self.tied.input_data, s) if self.tied else s).tofile(self.target)
        else:
            tied_lines = self.tied.ascii_buffer.splitlines() if self.tied else []
            unrolled_s = s.view(self.struct.unrolled_flat_dtype)
            for tied_line, scalars in itertools.izip_longest(tied_lines, unrolled_s):
                tied_line_with_separator = tied_line + self.delimiter if self.tied else ''
                output_line = self.delimiter.join(map(self.numpy_scalar_to_string, scalars))
                print >> self.target, tied_line_with_separator + output_line
        self.target.flush()

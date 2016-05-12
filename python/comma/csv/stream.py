import numpy as np
import sys
from cStringIO import StringIO
import itertools
import warnings
from . import time as csv_time
from ..util.warning import warning
from .struct import struct
from .common import *


def custom_formatwarning(msg, *args):
    return __name__ + " warning: " + str(msg) + '\n'


class stream(object):
    """
    see github.com/acfr/comma/wiki/python-csv-module for details
    """
    buffer_size_in_bytes = 65536

    class error(Exception):
        pass

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
        self._check_fields()
        self.format = self._format(binary, format)
        self.binary = self.format != ''
        self._check_consistency_with_tied()
        self.input_dtype = self._input_dtype()
        self.size = self._default_buffer_size()
        if not self.binary:
            unrolled_types = unrolled_types_of_flat_dtype(self.input_dtype)
            self.ascii_converters = csv_time.ascii_converters(unrolled_types)
            num_utypes = len(unrolled_types)
            self.usecols = tuple(range(num_utypes))
            self.filling_values = None if num_utypes == 1 else ('',) * num_utypes
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
        """
        a generator that calls read() repeatedly until there is no data in the stream

        size has the same meaning as in read()
        """
        while True:
            s = self.read(size)
            if s is None:
                break
            yield s

    def read(self, size=None):
        """
        read the specified number of records from stream, extract data,
        put it into appropriate numpy array with the dtype defined by struct, and return it

        if size is None, default size is used
        if size is -1, all records from file are read (stdin is not allowed)
        """
        if size is None:
            size = self.size
        if size < 0:
            if self.source == sys.stdin:
                msg = "stdin requires positive size, got {}".format(size)
                raise stream.error(msg)
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
        """
        return numpy array (of the given size) representing missings data, consisting of
        fields that are not found in the stream

        use zero values to define the array unless default_values is given
        """
        if self._missing_data is not None and size <= self._missing_data.size:
            return self._missing_data[:size]
        self._missing_data = np.zeros(size, dtype=self.missing_dtype)
        if not self.default_values:
            return self._missing_data
        dtype_name_of = dict(zip(self.missing_fields, self.missing_dtype.names))
        for field, value in self.default_values.iteritems():
            name = dtype_name_of[field]
            if self.missing_dtype[name] == np.dtype(csv_time.NUMPY_TYPE):
                try:
                    self._missing_data[name] = csv_time.to_numpy(value)
                except:
                    self._missing_data[name] = value
            else:
                self._missing_data[name] = value
        return self._missing_data

    def numpy_scalar_to_string(self, scalar):
        """
        convert numpy scalar to a string
        """
        if scalar.dtype.char in np.typecodes['AllInteger']:
            return str(scalar)
        elif scalar.dtype.char in np.typecodes['Float']:
            return "{scalar:.{precision}g}".format(scalar=scalar, precision=self.precision)
        elif scalar.dtype.char in np.typecodes['Datetime']:
            return csv_time.from_numpy(scalar)
        elif scalar.dtype.char in 'S':
            return scalar
        msg = "converting {} to string is not implemented".format(repr(scalar.dtype))
        raise stream.error(msg)

    def write(self, s):
        """
        serialize the given numpy array of dtype defined by struct and write the result to
        the output
        """
        if s.dtype != self.struct.dtype:
            msg = "expected {}, got {}".format(repr(self.struct.dtype), repr(s.dtype))
            raise stream.error(msg)
        if s.shape != (s.size,):
            msg = "expected shape=({},), got {}".format(s.size, s.shape)
            raise stream.error(msg)
        if self.tied and s.size != self.tied._input_data.size:
            msg = "size {} not equal to tied size {}".format(s.size, self.tied.size)
            raise stream.error(msg)
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

    def dump(self, mask=None):
        """
        dump the data in the stream buffer to the output
        """
        if mask is None:
            self._dump()
        else:
            self._dump_with_mask(mask)

    def _dump(self):
        if self.binary:
            self._input_data.tofile(self.target)
        else:
            self.target.write(self._ascii_buffer)
        self.target.flush()

    def _dump_with_mask(self, mask):
        if mask.dtype != bool:
            msg = "expected mask type to be {}, got {}" \
                .format(repr(np.dtype(bool)), repr(mask.dtype))
            raise stream.error(msg)
        if mask.shape != (mask.size,):
            msg = "expected mask shape=({},), got {}".format(mask.size, mask.shape)
            raise stream.error(msg)
        if mask.size != self._input_data.size:
            msg = "mask size {} not equal to data size {}" \
                .format(mask.size, self._input_data.size)
            raise stream.error(msg)
        if self.binary:
            self._input_data[mask].tofile(self.target)
        else:
            it = itertools.izip(StringIO(self._ascii_buffer), mask)
            self.target.write(''.join(line for line, allowed in it if allowed))
        self.target.flush()

    def _warn(self, msg, verbose=True):
        if verbose:
            with warning(custom_formatwarning) as warn:
                warn(msg)

    def _struct(self, s):
        if not isinstance(s, struct):
            msg = "expected '{}', got '{}'".format(repr(struct), repr(s))
            raise stream.error(msg)
        return s

    def _fields(self, fields):
        if fields == '':
            return self.struct.fields
        if self.full_xpath:
            return self.struct.expand_shorthand(fields)
        if '/' in fields:
            msg = "expected fields without '/', got '{}'".format(fields)
            raise stream.error(msg)
        ambiguous_leaves = self.struct.ambiguous_leaves.intersection(fields.split(','))
        if ambiguous_leaves:
            msg = "fields '{}' are ambiguous in '{}', use full xpath" \
                .format(','.join(ambiguous_leaves), fields)
            raise stream.error(msg)
        xpath = self.struct.xpath_of_leaf.get
        return tuple(xpath(name) or name for name in fields.split(','))

    def _format(self, binary, format):
        if isinstance(binary, basestring):
            if self.verbose and binary and format and binary != format:
                msg = "ignoring '{}' and using '{}' since binary keyword has priority" \
                    .format(format, binary)
                self._warn(msg)
            return binary
        if binary is True and not format:
            if not set(self.struct.fields).issuperset(self.fields):
                msg = "failed to infer type of every field in '{}', specify format" \
                    .format(','.join(self.fields))
                raise stream.error(msg)
            type_of = self.struct.type_of_field.get
            return format_from_types(type_of(field) for field in self.fields)
        if binary is False:
            return ''
        return format

    def _check_fields(self):
        fields_in_struct = set(self.fields).intersection(self.struct.fields)
        if not fields_in_struct:
            msg = "fields '{}' do not match any of expected fields '{}'" \
                .format(','.join(self.fields), ','.join(self.struct.fields))
            raise stream.error(msg)
        duplicates = tuple(field for field in self.struct.fields
                           if field in self.fields and self.fields.count(field) > 1)
        if duplicates:
            msg = "fields '{}' have duplicates in '{}'" \
                "".format(','.join(duplicates), ','.join(self.fields))
            raise stream.error(msg)

    def _check_consistency_with_tied(self):
        if not self.tied:
            return
        if not isinstance(self.tied, stream):
            msg = "expected tied stream of type '{}', got '{}'" \
                "".format(str(stream), repr(self.tied))
            raise stream.error(msg)
        if self.tied.binary != self.binary:
            msg = "expected tied stream to be {}, got {}" \
                "".format("binary" if self.binary else "ascii",
                          "binary" if self.tied.binary else "ascii")
            raise stream.error(msg)
        if not self.binary and self.tied.delimiter != self.delimiter:
            msg = "expected tied stream to have the same delimiter '{}', got '{}'" \
                "".format(self.delimiter, self.tied.delimiter)
            raise stream.error(msg)

    def _input_dtype(self):
        if self.binary:
            input_dtype = structured_dtype(self.format)
            if len(self.fields) != len(input_dtype.names):
                msg = "expected same number of fields and format types, got '{}' and '{}'" \
                    "".format(','.join(self.fields), self.format)
                raise stream.error(msg)
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
            self._warn(msg)
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
                self._warn(msg)
        if unknown_default_fields:
            if self.verbose:
                msg = "found default values for fields not in struct: '{}'" \
                    "".format(','.join(unknown_default_fields))
                self._warn(msg)
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

import numpy as np
import operator
import re
from .time import from_numpy as from_numpy_time


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


def normalise_type(maybe_array_type):
    """
    >>> from comma.csv.common import normalise_type
    >>> normalise_type('<f8')
    'f8'
    >>> normalise_type('f8')
    'f8'
    >>> normalise_type('2f8')
    '2f8'
    >>> normalise_type('(2,)f8')
    '2f8'
    >>> normalise_type('(2, 3)f8')
    '(2,3)f8'
    """
    def strip_byte_order_prefix(string, prefix_chars='<>|='):
        return string[1:] if string.startswith(tuple(prefix_chars)) else string

    maybe_array_type = strip_byte_order_prefix(maybe_array_type).replace(' ', '')
    m = re.match(r'^\((\d+),\)(.*)', maybe_array_type)
    if m and len(m.groups()) == 2:
        size = m.group(1)
        type = m.group(2)
        return size + type
    else:
        return maybe_array_type


def types_of_flat_dtype(dtype, unroll=False):
    """
    transform a dtype into types

    >>> import numpy as np
    >>> from comma.csv.common import types_of_flat_dtype
    >>> types_of_flat_dtype(np.dtype('u4,(2, 3)f8'))
    ('u4', '(2,3)f8')
    >>> types_of_flat_dtype(np.dtype('u4,(2, 3)f8'), unroll=True)
    ('u4', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8')
    """
    shape_unrolled_types = []
    for descr in dtype.descr:
        if isinstance(descr[1], list):
            msg = "expected flat dtype, got '{}'".format(repr(dtype))
            raise ValueError(msg)
        single_type = normalise_type(descr[1])
        shape = descr[2] if len(descr) > 2 else ()
        if unroll:
            shape_unrolled_types.extend([single_type] * reduce(operator.mul, shape, 1))
        else:
            type = normalise_type(str(shape) + single_type) if shape else single_type
            shape_unrolled_types.append(type)
    return tuple(shape_unrolled_types)


def unrolled_types_of_flat_dtype(dtype):
    return types_of_flat_dtype(dtype, unroll=True)


def structured_dtype(numpy_format_or_type):
    """
    return structured array even for a format string containing a single type
    note: passing a single type format string to numpy dtype returns a scalar

    >>> import numpy as np
    >>> from comma.csv.common import structured_dtype
    >>> structured_dtype(np.float64).names
    ('f0',)
    >>> structured_dtype('f8').names
    ('f0',)
    >>> structured_dtype('f8,u2').names
    ('f0', 'f1')
    >>> np.dtype('f8').names
    """
    dtype = np.dtype(numpy_format_or_type)
    if len(dtype) != 0:
        return dtype
    return np.dtype([('f0', numpy_format_or_type)])


def format_from_types(types):
    """
    return the format string corresponding to a sequence of types,
    which can contain types as strings (including arrays) and numpy types,
        e.g., ( 'f8', '(2,3)u4', np.uint32 )

    >>> import numpy as np
    >>> from comma.csv.common import format_from_types
    >>> format_from_types(('f8', '(2,3)u4', np.uint32))
    'f8,(2,3)u4,u4'
    """
    types_as_strings = []
    for type in types:
        dtype = structured_dtype(type) if isinstance(type, basestring) else np.dtype(type)
        if len(dtype) > 1:
            msg = "expected single type, got {}".format(type)
            raise ValueError(msg)
        type_as_string = normalise_type(types_of_flat_dtype(dtype)[0])
        types_as_strings.append(type_as_string)
    return ','.join(types_as_strings)


DEFAULT_PRECISION = 12


def numpy_scalar_to_string(scalar, precision=DEFAULT_PRECISION):
    """
    convert numpy scalar to a string

    >>> from comma.csv.common import numpy_scalar_to_string
    >>> numpy_scalar_to_string(np.int32(-123))
    '-123'
    >>> numpy_scalar_to_string(np.float64(-12.3499), precision=4)
    '-12.35'
    >>> numpy_scalar_to_string(np.float64(0.1234567890123456))
    '0.123456789012'
    >>> numpy_scalar_to_string(np.string_('abc'))
    'abc'
    >>> numpy_scalar_to_string(np.datetime64('2015-01-02T12:34:56', 'us'))
    '20150102T123456'
    >>> numpy_scalar_to_string(np.datetime64('2015-01-02T12:34:56.000000', 'us'))
    '20150102T123456'
    >>> numpy_scalar_to_string(np.datetime64('2015-01-02T12:34:56.123456', 'us'))
    '20150102T123456.123456'
    >>> numpy_scalar_to_string(np.timedelta64(-123, 's'))
    '-123'
    """
    if scalar.dtype.char in np.typecodes['AllInteger']:
        return str(scalar)
    elif scalar.dtype.char in np.typecodes['Float']:
        return "{scalar:.{precision}g}".format(scalar=scalar, precision=precision)
    elif scalar.dtype.char in np.typecodes['Datetime']:
        return from_numpy_time(scalar)
    elif scalar.dtype.char in 'S':
        return scalar
    msg = "converting {} to string is not implemented".format(repr(scalar.dtype))
    raise NotImplementedError(msg)

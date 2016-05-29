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


def readlines_unbuffered(source, size):
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


def strip_byte_order_prefix(string, prefix_chars='<>|='):
    """
    >>> from comma.csv.common import strip_byte_order_prefix
    >>> strip_byte_order_prefix('<f8')
    'f8'
    """
    return string[1:] if string.startswith(tuple(prefix_chars)) else string


def shape_to_string(shape):
    """
    >>> from comma.csv.common import shape_to_string
    >>> shape_to_string((2,))
    '2'
    >>> shape_to_string((2, 3))
    '(2,3)'
    """
    s = str(shape).replace(' ', '')
    m = re.match(r'^\((\d+),\)', s)
    if m:
        return m.group(1)
    return s


def types_of_dtype(dtype, unroll=False):
    """
    return a tuple of numpy type strings for a given dtype

    >>> import numpy as np
    >>> from comma.csv.common import types_of_dtype
    >>> types_of_dtype(np.dtype('u4,(2, 3)f8'))
    ('u4', '(2,3)f8')
    >>> types_of_dtype(np.dtype('(2, 3)f8'))
    ('(2,3)f8',)
    >>> types_of_dtype(np.dtype('u4,(2, 3)f8'), unroll=True)
    ('u4', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8')
    >>> types_of_dtype(np.dtype([('a', 'S2'), ('b', [('c', '2f8'), ('d', 'u2')])]))
    ('S2', '2f8', 'u2')
    """
    if len(dtype) == 0:
        dtype = np.dtype([('', dtype)])
    types = []
    for descr in dtype.descr:
        if isinstance(descr[1], list):
            types.extend(types_of_dtype(np.dtype(descr[1]), unroll))
            continue
        single_type = strip_byte_order_prefix(descr[1])
        shape = descr[2] if len(descr) > 2 else ()
        if unroll:
            unrolled_types = [single_type] * reduce(operator.mul, shape, 1)
            types.extend(unrolled_types)
        else:
            type = shape_to_string(shape) + single_type if shape else single_type
            types.append(type)
    return tuple(types)


def structured_dtype(numpy_format_or_type):
    """
    return structured dtype even for a format string containing a single type
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
    return np.dtype([('', numpy_format_or_type)])


def numpy_type_to_string(type):
    """
    >>> import numpy as np
    >>> from comma.csv.common import numpy_type_to_string
    >>> numpy_type_to_string(np.uint32)
    'u4'
    >>> numpy_type_to_string('u4')
    'u4'
    >>> numpy_type_to_string('2u4')
    '2u4'
    >>> numpy_type_to_string('(2,3)u4')
    '(2,3)u4'
    """
    dtype = np.dtype(type)
    if len(dtype) != 0:
        msg = "expected single type, got {}".format(type)
        raise ValueError(msg)
    return types_of_dtype(dtype)[0]


DEFAULT_PRECISION = 12


def numpy_scalar_to_string(scalar, precision=DEFAULT_PRECISION):
    """
    convert numpy scalar to a string suitable to comma csv stream

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

import numpy as np
import operator
import re


def strip_prefix(string, prefix_chars='<>|='):
    """
    strip data storage prefix from numpy type, e.g. '<f8' becomes 'f8'
    """
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


def types_of_flat_dtype(dtype, unroll=False):
    """
    transform a dtype into types

    >>> import comma
    >>> import numpy as np
    >>> comma.csv.common.types_of_flat_dtype(np.dtype('u4,(2, 3)f8'))
    ('u4', '(2, 3)f8')
    >>> comma.csv.common.types_of_flat_dtype(np.dtype('u4,(2, 3)f8'), unroll=True)
    ('u4', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8')
    """
    shape_unrolled_types = []
    for descr in dtype.descr:
        type = strip_prefix(descr[1])
        shape = descr[2] if len(descr) > 2 else ()
        if unroll:
            shape_unrolled_types.extend([type] * reduce(operator.mul, shape, 1))
        else:
            shape_unrolled_types.append(str(shape) + type if shape else type)
    return tuple(shape_unrolled_types)


def unrolled_types_of_flat_dtype(dtype):
    return types_of_flat_dtype(dtype, unroll=True)


def structured_dtype(numpy_format):
    """
    return structured array even for a format string containing a single type
    note: passing a single type format string to numpy dtype returns a scalar

    >>> import comma
    >>> import numpy as np
    >>> np.dtype('f8')
    dtype('float64')
    >>> comma.csv.common.structured_dtype('f8')
    dtype([('f0', '<f8')])
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

from __future__ import absolute_import
import numpy as np
import operator
import re


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


def strip_byte_order_prefix(string, prefix_chars='<>|='):
    """
    >>> from comma.numpy import strip_byte_order_prefix
    >>> strip_byte_order_prefix('<f8')
    'f8'
    """
    return string[1:] if string.startswith(tuple(prefix_chars)) else string


def shape_to_string(shape):
    """
    >>> from comma.numpy import shape_to_string
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
    >>> from comma.numpy import types_of_dtype
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


def structured_dtype(format_or_type):
    """
    return structured dtype even for a format string containing a single type
    note: passing a single type format string to numpy dtype returns a scalar

    >>> import numpy as np
    >>> from comma.numpy import structured_dtype
    >>> structured_dtype(np.float64).names
    ('f0',)
    >>> structured_dtype('f8').names
    ('f0',)
    >>> structured_dtype('f8,u2').names
    ('f0', 'f1')
    >>> np.dtype('f8').names
    """
    dtype = np.dtype(format_or_type)
    if len(dtype) != 0:
        return dtype
    return np.dtype([('', format_or_type)])


def type_to_string(type):
    """
    >>> import numpy as np
    >>> from comma.numpy import type_to_string
    >>> type_to_string(np.uint32)
    'u4'
    >>> type_to_string('u4')
    'u4'
    >>> type_to_string('2u4')
    '2u4'
    >>> type_to_string('(2,3)u4')
    '(2,3)u4'
    """
    dtype = np.dtype(type)
    if len(dtype) != 0:
        msg = "expected single type, got {}".format(type)
        raise ValueError(msg)
    return types_of_dtype(dtype)[0]

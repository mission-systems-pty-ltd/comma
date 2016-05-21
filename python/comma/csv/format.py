import re
import numpy as np
from collections import OrderedDict
from itertools import groupby
from .time import TYPE as numpy_datetime_type
from .common import types_of_flat_dtype, structured_dtype


COMMA_TO_NUMPY_TYPE = OrderedDict([
    ('b', 'i1'),
    ('ub', 'u1'),
    ('w', 'i2'),
    ('uw', 'u2'),
    ('i', 'i4'),
    ('ui', 'u4'),
    ('l', 'i8'),
    ('ul', 'u8'),
    ('f', 'f4'),
    ('d', 'f8'),
    ('t', numpy_datetime_type)])

COMMA_TYPES = tuple(COMMA_TO_NUMPY_TYPE.keys())
NUMPY_TYPES = tuple(COMMA_TO_NUMPY_TYPE.values())
NUMPY_TO_COMMA_TYPE = OrderedDict(zip(NUMPY_TYPES, COMMA_TYPES))


def to_numpy_type(comma_type):
    """
    convert a single comma type to a numpy type

    >>> from comma.csv.format import *
    >>> to_numpy_type('d')
    'f8'
    >>> to_numpy_type('s[12]')
    'S12'
    """
    def to_numpy_string_type(comma_string_type):
        m = re.match(r'^s\[(\d+)\]$', comma_string_type)
        if m:
            length = m.group(1)
            return 'S' + length

    numpy_type = COMMA_TO_NUMPY_TYPE.get(comma_type) or to_numpy_string_type(comma_type)
    if numpy_type is None:
        known_types = ', '.join(COMMA_TYPES)
        msg = "'{}' is not among known comma types: {}".format(comma_type, known_types)
        raise ValueError(msg)
    return numpy_type


def to_comma_type(numpy_type):
    """
    convert a single numpy type to comma type

    numpy arrays are unrolled and converted to a prefixed comma type

    >>> from comma.csv.format import *
    >>> to_comma_type('f8')
    'd'
    >>> to_comma_type('S12')
    's[12]'
    >>> to_comma_type('3u4')
    '3ui'
    >>> to_comma_type('(2,3)u4')
    '6ui'
    """
    def to_comma_string_type(numpy_string_type):
        m = re.match(r'^S(\d+)$', numpy_string_type)
        if m:
            length = m.group(1)
            return 's[' + length + ']'

    dtype = structured_dtype(numpy_type)
    if len(dtype) != 1:
        msg = "expected single numpy type, got {}".format(numpy_type)
        raise ValueError(msg)
    unrolled = types_of_flat_dtype(structured_dtype(numpy_type), unroll=True)
    single_type = unrolled[0]
    numtypes = len(unrolled)
    comma_type = NUMPY_TO_COMMA_TYPE.get(single_type) or to_comma_string_type(single_type)
    if comma_type is None:
        known_types = ', '.join(NUMPY_TYPES)
        msg = "'{}' is not among known numpy types: {}".format(numpy_type, known_types)
        raise ValueError(msg)
    return str(numtypes) + comma_type if numtypes != 1 else comma_type


def expand_prefixed_comma_type(maybe_prefixed_comma_type):
    """
    expand a prefixed comma type into a list

    >>> from comma.csv.format import *
    >>> expand_prefixed_comma_type('3d')
    ['d', 'd', 'd']
    >>> expand_prefixed_comma_type('d')
    ['d']
    """
    m = re.match(r'^(\d+)(.+)$', maybe_prefixed_comma_type)
    if m and len(m.groups()) == 2:
        numerical_prefix = int(m.group(1))
        comma_type = m.group(2)
        return [comma_type] * numerical_prefix
    return [maybe_prefixed_comma_type]


def expand(comma_format):
    """
    expand comma format

    >>> from comma.csv.format import *
    >>> expand('3d,2ub,ui')
    'd,d,d,ub,ub,ui'
    """
    types = []
    for type in comma_format.split(','):
        types += expand_prefixed_comma_type(type)
    return ','.join(types)


def compress(comma_format):
    """
    compress comma format

    >>> from comma.csv.format import *
    >>> compress('d,2d,d,s[12],ub,ub,ub,ub,ub,ub,3ui,ub,ub,ul')
    '4d,s[12],6ub,3ui,2ub,ul'
    """
    types = expand(comma_format).split(',')
    counted_types = ((len(list(group)), type) for type, group in groupby(types))
    return ','.join(str(n) + type if n != 1 else type for n, type in counted_types)


def to_numpy(comma_format):
    """
    return a tuple of numpy types corresponsing to the provided comma format

    >>> from comma.csv.format import *
    >>> to_numpy('2d,6ub,ui')
    ('f8', 'f8', 'u1', 'u1', 'u1', 'u1', 'u1', 'u1', 'u4')
    """
    numpy_types = []
    for comma_type in expand(comma_format).split(','):
        numpy_types.append(to_numpy_type(comma_type))
    return tuple(numpy_types)


def from_numpy(*numpy_types_or_format):
    """
    return comma format corresponsing to the provided numpy types or format

    numpy arrays are unrolled and converted to the corresponding number of comma types

    >>> import numpy as np
    >>> from comma.csv.format import *
    >>> from_numpy(np.float64)
    'd'
    >>> from_numpy(np.float64, np.uint32)
    'd,ui'
    >>> from_numpy('f8', 'u4')
    'd,ui'
    >>> from_numpy('f8,f8,u1,u1,u1,u1,u1,u1,u4')
    '2d,6ub,ui'
    >>> from_numpy('2f8,(2,3)u1,u4')
    '2d,6ub,ui'
    """
    comma_types = []
    for numpy_type_or_format in numpy_types_or_format:
        for numpy_type in types_of_flat_dtype(np.dtype(numpy_type_or_format)):
            comma_types.append(to_comma_type(numpy_type))
    return compress(','.join(comma_types))

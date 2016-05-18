import re
from collections import OrderedDict
from .time import NUMPY_TYPE as numpy_datetime_type


class format_error(Exception):
    pass


NUMPY_TYPE_FROM_COMMA_TYPE = OrderedDict([
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

TYPES = tuple(NUMPY_TYPE_FROM_COMMA_TYPE.keys())
NUMPY_TYPES = tuple(NUMPY_TYPE_FROM_COMMA_TYPE.values())


def numpy_string_type(comma_string_type):
    m = re.match(r'^s\[(\d+)\]$', comma_string_type)
    if m:
        length = m.group(1)
        return 'S' + length


def numpy_type(comma_type, strict=False):
    type = NUMPY_TYPE_FROM_COMMA_TYPE.get(comma_type) or numpy_string_type(comma_type)
    if strict and type is None:
        known_types = ', '.join(TYPES)
        message = "'{}' is not among known types: {}".format(comma_type, known_types)
        raise format_error(message)
    return type


def expand_prefixed_type(maybe_prefixed_comma_type):
    m = re.match(r'^(\d+)(.+)$', maybe_prefixed_comma_type)
    if m and len(m.groups()) == 2:
        numerical_prefix = int(m.group(1))
        comma_type = m.group(2)
        return [comma_type] * numerical_prefix
    return [maybe_prefixed_comma_type]


def expand_format(comma_format):
    types = []
    for type in comma_format.split(','):
        types += expand_prefixed_type(type)
    return ','.join(types)


def numpy_format(comma_format):
    numpy_types = []
    for comma_type in expand_format(comma_format).split(','):
        numpy_types.append(numpy_type(comma_type, strict=True))
    return tuple(numpy_types)

# synonyms
expand = expand_format
to_numpy = numpy_format


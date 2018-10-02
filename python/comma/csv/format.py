# This file is part of comma, a generic and flexible library
# Copyright (c) 2011 The University of Sydney
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University of Sydney nor the
#    names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
# GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
# HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import re
from collections import OrderedDict
from itertools import groupby
from csv import reader
import numpy as np
from ..numpy import types_of_dtype
from .time import TYPE as numpy_datetime_type
from .time import to_numpy as parse_time

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

NUMPY_TO_COMMA_TYPE = OrderedDict([
    ('b1', 'b'),
    ('i1', 'b'),
    ('u1', 'ub'),
    ('i2', 'w'),
    ('u2', 'uw'),
    ('i4', 'i'),
    ('u4', 'ui'),
    ('i8', 'l'),
    ('u8', 'ul'),
    ('f4', 'f'),
    ('f8', 'd'),
    (numpy_datetime_type, 't')])

COMMA_TYPES = tuple(COMMA_TO_NUMPY_TYPE.keys())
NUMPY_TYPES = tuple(COMMA_TO_NUMPY_TYPE.values())
#NUMPY_TO_COMMA_TYPE = OrderedDict( zip( NUMPY_TYPES_FULL, COMMA_TYPES_FULL ) )

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
    dtype = np.dtype(numpy_type)
    if len(dtype) != 0:
        msg = "expected single numpy type, got {}".format(numpy_type)
        raise ValueError(msg)
    unrolled = types_of_dtype(dtype, unroll=True)
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


def to_numpy(comma_format, compress=False):
    """
    return a tuple of numpy types corresponsing to the provided comma format
    if the second argument is True, compress numpy types

    >>> from comma.csv.format import *
    >>> to_numpy('2d,6ub,ui')
    ('f8', 'f8', 'u1', 'u1', 'u1', 'u1', 'u1', 'u1', 'u4')
    >>> to_numpy('2d,6ub,ui', True)
    ('2f8', '6u1', 'u4')
    """
    numpy_types = []
    for comma_type in comma_format.split(','):
        expanded_comma_type = expand( comma_type ).split(',')
        count = len( expanded_comma_type )
        numpy_type = to_numpy_type( expanded_comma_type[0] )
        if compress:
            numpy_types.append( count > 1 and str( count ) + numpy_type or numpy_type )
        else:
            numpy_types.extend( [ numpy_type ] * count )
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
        for numpy_type in types_of_dtype(np.dtype(numpy_type_or_format)):
            comma_types.append(to_comma_type(numpy_type))
    return compress(','.join(comma_types))

def guess_type(element):
    """
    guess the type of one element
    """
    if element in ['']:
        return "s[1024]"

    try:
        d = float(element)
        return "d"
    except ValueError:
        try:
            t = parse_time(element)
            return "t"
        except TypeError:
            return "s[1024]"


def guess_format(record):
    """
    guess format given an example input record
    """
    comma_types = []
    for split in reader(record):
        for element in split:
            comma_types.append(guess_type(element))
    return ','.join(comma_types)

# Copyright (c) 2011 The University of Sydney

from __future__ import absolute_import
import numpy as np
import re
import os
import sys
import time

UNIT = 'us'
TYPE = 'M8[' + UNIT + ']'
DTYPE = np.dtype(TYPE)

NOT_A_DATE_TIME = np.datetime64('NaT')
POSITIVE_INFINITY = np.datetime64('294247-01-09T04:00:54.775807')
NEGATIVE_INFINITY = np.datetime64('-290308-12-22T19:59:05.224191')
BASESTRING = basestring if sys.version_info.major < 3 else str # sigh...
NUMPY_VERSION_MAJOR_, NUMPY_VERSION_MINOR_, NUMPY_VERSION_PATCH_ = ( int( _ ) for _ in np.__version__.split( '.' ) )

def is_undefined(numpy_time): return str(numpy_time) == str(NOT_A_DATE_TIME)

def is_positive_infinity(numpy_time): return numpy_time == POSITIVE_INFINITY

def is_negative_infinity(numpy_time): return numpy_time == NEGATIVE_INFINITY

def to_numpy(t):
    """
    return numpy datetime64 scalar corresponding to the given comma time string
    if t has nanoseconds, it will be trunkated (rather than rounded) to microseconds

    >>> import numpy as np
    >>> from comma.csv.time import to_numpy
    >>> to_numpy('20150102T123456') == np.datetime64('2015-01-02T12:34:56.000000', 'us')
    True
    >>> to_numpy('20150102T123456.010203') == np.datetime64('2015-01-02T12:34:56.010203', 'us')
    True
    >>> to_numpy('not-a-date-time')
    numpy.datetime64('NaT')
    >>> to_numpy('')
    numpy.datetime64('NaT')
    """
    if NUMPY_VERSION_MAJOR_ == 1 and NUMPY_VERSION_MAJOR_ < 14 and isinstance( t, bytes ): t = t.decode( 'utf-8' ) # quick and dirty, since ubuntu 18.04 python3-numpy still install numpy 1.13; remove once move on with the version since it's waste of cpu cycles
    if t in ['', 'not-a-date-time']: return NOT_A_DATE_TIME
    if t in ['+infinity', '+inf', 'infinity', 'inf']: return POSITIVE_INFINITY
    if t in ['-infinity', '-inf']: return NEGATIVE_INFINITY
    if not (isinstance(t, BASESTRING) and re.match(r'^(\d{8}T\d{6}(\.\d{0,12})?)$', t)):
        msg = "expected comma time, got '{}'".format(repr(t))
        raise TypeError(msg)
    v = list(t)
    for i in [13, 11]: v.insert(i, ':')
    for i in [6, 4]: v.insert(i, '-')
    return np.datetime64(''.join(v), UNIT)

def from_numpy(t):
    """
    return comma time string if datetime64 scalar is given or
    return time delta integer as a string if timedelta64 scalar is given

    >>> import numpy as np
    >>> from comma.csv.time import from_numpy
    >>> from_numpy(np.datetime64('2015-01-02T12:34:56', 'us'))
    '20150102T123456'
    >>> from_numpy(np.datetime64('2015-01-02T12:34:56.010203', 'us'))
    '20150102T123456.010203'
    >>> from_numpy(np.datetime64('NaT'))
    'not-a-date-time'
    >>> from_numpy(np.timedelta64(123, 'us'))
    '123'
    >>> from_numpy(np.timedelta64(-123, 's'))
    '-123'
    """
    if not ((isinstance(t, np.datetime64) and t.dtype == DTYPE) or
            is_undefined(t) or
            isinstance(t, np.timedelta64)):
        msg = "expected numpy time or timedelta of type '{}' or '{}', got '{}'".format(repr(DTYPE), repr(np.timedelta64), repr(t))
        raise TypeError(msg)
    if isinstance(t, np.timedelta64): return str(t.astype('i8'))
    if is_undefined(t): return 'not-a-date-time'
    if is_negative_infinity(t): return '-infinity'
    if is_positive_infinity(t): return '+infinity'
    s = re.sub(r'(\.0{6})?([-+]\d{4}|Z)?$', '', str(t))
    #return re.sub(r'(\.0{6})?([-+]\d{4}|Z)?$', '', str(t)).translate(None, ':-')
    return s.translate(str.maketrans('', '', ':-')) if sys.version_info.major > 2 else s.translate(None, ':-') # sigh... cannot believe i am going this...

def ascii_converters(types):
    converters = {}
    for i, type in enumerate(types):
        if np.dtype(type) == DTYPE: converters[i] = to_numpy
    return converters

def get_time_zone(): return os.environ.get('TZ')

def set_time_zone(name):
    if name:
        os.environ['TZ'] = name
        time.tzset()
    elif 'TZ' in os.environ:
        del os.environ['TZ']
        time.tzset()

zone = set_time_zone

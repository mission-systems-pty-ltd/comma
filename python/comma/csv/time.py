from __future__ import absolute_import
import numpy as np
import re
import os
import time

UNIT = 'us'
TYPE = 'M8[' + UNIT + ']'
DTYPE = np.dtype(TYPE)


def undefined_time():
    return np.datetime64('NaT')


def is_undefined(numpy_time):
    return str(numpy_time) == 'NaT'


def to_numpy(t):
    """
    return numpy datetime64 scalar corresponding to the given comma time string

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
    if not (isinstance(t, basestring) and
            re.match(r'^(\d{8}T\d{6}(\.\d{0,6})?|not-a-date-time|)$', t)):
        msg = "expected comma time, got '{}'".format(repr(t))
        raise TypeError(msg)
    if t == 'not-a-date-time' or t == '':
        return undefined_time()
    v = list(t)
    for i in [13, 11]:
        v.insert(i, ':')
    for i in [6, 4]:
        v.insert(i, '-')
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
        msg = "expected numpy time or timedelta of type '{}' or '{}', got '{}'" \
            .format(repr(DTYPE), repr(np.timedelta64), repr(t))
        raise TypeError(msg)
    if isinstance(t, np.timedelta64):
        return str(t.astype('i8'))
    if is_undefined(t):
        return 'not-a-date-time'
    return re.sub(r'(\.0{6})?([-+]\d{4}|Z)?$', '', str(t)).translate(None, ':-')


def ascii_converters(types):
    converters = {}
    for i, type in enumerate(types):
        if np.dtype(type) == np.dtype(TYPE):
            converters[i] = to_numpy
    return converters


def get_time_zone():
    return os.environ.get('TZ')


def set_time_zone(name):
    if name:
        os.environ['TZ'] = name
        time.tzset()
    elif 'TZ' in os.environ:
        del os.environ['TZ']
        time.tzset()

zone = set_time_zone

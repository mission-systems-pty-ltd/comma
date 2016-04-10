from __future__ import absolute_import
import numpy
import re
import os
import time


class time_error(Exception):
    pass

NUMPY_TYPE_UNIT = 'us'
NUMPY_TYPE = 'datetime64[' + NUMPY_TYPE_UNIT + ']'
NUMPY_INTEGER_TYPE_OF_TIMEDELATA = 'int64'


def undefined_time():
    return numpy.datetime64()


def get_time_zone():
    return os.environ.get('TZ')


def set_time_zone(name):
    if name:
        os.environ['TZ'] = name
        time.tzset()
    elif 'TZ' in os.environ:
        del os.environ['TZ']
        time.tzset()


def numpy_time(comma_time):
    if comma_time in ['', 'not-a-date-time']:
        return undefined_time()
    v = list(comma_time)
    if len(v) < 15:
        message = "'{}' is not a valid time".format(comma_time)
        raise time_error(message)
    for i in [13, 11]:
        v.insert(i, ':')
    for i in [6, 4]:
        v.insert(i, '-')
    return numpy.datetime64(''.join(v), NUMPY_TYPE_UNIT)


def comma_time(numpy_time):
    if isinstance(numpy_time, numpy.timedelta64):
        return str(numpy_time.astype(NUMPY_INTEGER_TYPE_OF_TIMEDELATA))
    if numpy_time == undefined_time():
        return 'not-a-date-time'
    if numpy_time.dtype != numpy.dtype(NUMPY_TYPE):
        message = "'{}' is not of expected type '{}'".format(repr(numpy_time), NUMPY_TYPE)
        raise time_error(message)
    return re.sub(r'(\.0{6})?([-+]\d{4}|Z)$', '', str(numpy_time)).translate(None, ':-')


def ascii_converters(types):
    converters = {}
    for i, type in enumerate(types):
        if numpy.dtype(type) == numpy.dtype(NUMPY_TYPE):
            converters[i] = numpy_time
    return converters


# synonyms
to_numpy = numpy_time
from_numpy = comma_time
zone = set_time_zone

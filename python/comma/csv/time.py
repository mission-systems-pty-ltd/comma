import numpy
import re

NUMPY_TYPE = 'datetime64[us]'

def to_numpy( comma_time_string ):
  if comma_time_string == 'not-a-date-time': return numpy.datetime64()
  v = list( comma_time_string )
  if len( v ) < 15: raise Exception( "expected time string in comma format, e.g. 20150101T000000, got '{}'".format( comma_time_string ) )
  for i in [13,11]: v.insert( i, ':' )
  for i in [6,4]: v.insert( i, '-' )
  return numpy.datetime64( ''.join( v ), 'us' )

def from_numpy( numpy_time ):
  if numpy_time == numpy.datetime64(): return 'not-a-date-time'
  if numpy_time.dtype != numpy.dtype( NUMPY_TYPE ):
    raise Exception( "expected time of type '{}', got '{}'".format( NUMPY_TYPE, repr( numpy_time ) ) )
  else: return re.sub( r'(\.0{6})?[-+]\d{4}$', '',  str( numpy_time ) ).translate( None, ':-' )

def ascii_converters( types ):
  return { i: to_numpy for i in numpy.where( numpy.array( types ) == numpy.dtype( NUMPY_TYPE ) )[0] }
  
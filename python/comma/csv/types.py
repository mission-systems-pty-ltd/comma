import numpy
import re

def dictionary( processed=False ):
  d = dict( b='int8', ub='uint8', w='int16', uw='uint16', i='int32', ui='uint32', l='int64', ul='uint64', f='float32', d='float64', t='datetime64[us]' )
  if processed: return dict( zip( d.keys(), map( lambda _: numpy.dtype( _ ).str, d.values() ) ) )
  else: return d

def expand( compressed_comma_format ):
  d = dictionary( processed=True )
  types = []
  for type in compressed_comma_format.split(','):
    m = re.match( r'^(\d+)(.*)', type )
    if m and len( m.groups() ) == 2 and m.group(2) in d.keys(): types += [ m.group(2) ] * int( m.group(1) )
    else: types.append( type )
  return ','.join( types )

def string_to_numpy( comma_string_type ):
  m = re.match( r'^s\[(\d+)\]$', comma_string_type )
  if m: return 'S' + m.group(1)

def string_from_numpy( numpy_string_type ):
  m = re.match( r'^S(\d+)$', numpy_string_type )
  if m: return 's[' + m.group(1) + ']'

def format_to_numpy( comma_format ):
  d = dictionary( processed=True )
  numpy_types = []
  for comma_type in expand( comma_format ).split(','):
    numpy_type = d.get( comma_type ) or string_to_numpy( comma_type )
    if numpy_type is None: raise Exception( "format string '{}' contains unrecongnised type '{}'".format( comma_format, comma_type ) )
    numpy_types.append( numpy_type )
  return ','.join( numpy_types )

def format_from_numpy( numpy_format ):
  d = dict( reversed(_) for _ in dictionary( processed=True ).items() )
  comma_types = []
  for numpy_type in numpy_format.split(','):
    comma_type = d.get( numpy_type ) or string_from_numpy( numpy_type )
    if comma_type is None: raise Exception( "format string '{}' contains unrecongnised type '{}'".format( numpy_format, numpy_type ) )
    comma_types.append( comma_type )
  return ','.join( comma_types )

def time_to_numpy( comma_time_string ):
  v = list( comma_time_string )
  if len( v ) < 15: raise Exception( "expected time string in comma iso format, got '{}'".format( comma_time_string ) )
  for i in [13,11]: v.insert( i, ':' )
  for i in [6,4]: v.insert( i, '-' )
  return numpy.datetime64( ''.join( v ), 'us' )

def time_from_numpy( numpy_time_object ):
  if not isinstance( numpy_time_object, numpy.datetime64 ):
    raise Exception( "expected '{}', got '{}'".format( numpy.dtype('datetime64[us]').type, repr( numpy_time_object ) ) )
  return re.sub( r'(\.0{6})?\+\d{4}$', '',  str( numpy_time_object ) ).translate( None, ':-' )

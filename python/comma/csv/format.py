import numpy
import re
import comma.csv.time

def dictionary( processed=False ):
  d = dict( b='int8', ub='uint8', w='int16', uw='uint16', i='int32', ui='uint32', l='int64', ul='uint64', f='float32', d='float64', t=comma.csv.time.NUMPY_TYPE )
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

def to_numpy( comma_format ):
  d = dictionary( processed=True )
  numpy_types = []
  for comma_type in expand( comma_format ).split(','):
    m = re.match( r'^s\[(\d+)\]$', comma_type )
    numpy_type = d.get( comma_type ) or ( 'S' + m.group(1) if m else None )
    if numpy_type is None: raise Exception( "format string '{}' contains unrecongnised type '{}'".format( comma_format, comma_type ) )
    numpy_types.append( numpy_type )
  return ','.join( numpy_types )

def from_numpy( numpy_format ):
  d = dict( reversed( _ ) for _ in dictionary( processed=True ).items() )
  comma_types = []
  for numpy_type in numpy_format.split(','):
    m = re.match( r'^S(\d+)$', numpy_type )
    comma_type = d.get( numpy_type ) or ( 's[' + m.group(1) + ']' if m else None )
    if comma_type is None: raise Exception( "format string '{}' contains unrecongnised type '{}'".format( numpy_format, numpy_type ) )
    comma_types.append( comma_type )
  return ','.join( comma_types )
  
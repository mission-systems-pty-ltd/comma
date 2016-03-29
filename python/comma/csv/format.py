import re

def dictionary():
  from comma.csv.time import NUMPY_TYPE as numpy_datetime_type
  return dict( b='i1', ub='u1', w='i2', uw='u2', i='i4', ui='u4', l='i8', ul='u8', f='f4', d='f8', t=numpy_datetime_type, bool='b1' )

def expand( compressed_comma_format ):
  d = dictionary()
  types = []
  for type in compressed_comma_format.split(','):
    m = re.match( r'^(\d+)(.*)', type )
    if m and len( m.groups() ) == 2 and ( m.group(2) in d.keys() or re.match( r'^s\[\d+\]$', m.group(2) ) ): types += [ m.group(2) ] * int( m.group(1) )
    else: types.append( type )
  return ','.join( types )

def to_numpy( comma_format ):
  d = dictionary()
  numpy_types = []
  for comma_type in expand( comma_format ).split(','):
    m = re.match( r'^s\[(\d+)\]$', comma_type )
    numpy_type = d.get( comma_type ) or ( 'S' + m.group(1) if m else None )
    if numpy_type is None: raise Exception( "format string '{}' contains unrecongnised type '{}'".format( comma_format, comma_type ) )
    numpy_types.append( numpy_type )
  return tuple( numpy_types )

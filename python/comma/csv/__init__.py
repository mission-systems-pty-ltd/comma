import numpy
import sys
from StringIO import StringIO
import itertools
import io
import warnings
import operator
import re

class struct:
  def __init__( self, fields, *types ):
    if len( fields.split(',') ) != len( types ): raise Exception( "expected {} types for fields '{}', got {} type(s)".format( len( fields.split(',') ), fields, len( types )) )
    self.dtype = numpy.dtype( zip( fields.split(','), types ) )
    self.fields = ','.join( struct.get( 'fields', self.dtype ) )
    self.format = ','.join( struct.get( 'format', self.dtype ) )

  @staticmethod
  def get( what, nested_dtype, path='' ):
    items = []
    for name in nested_dtype.names:
      dtype = nested_dtype.fields[name][0]
      if dtype.type == numpy.void:
        items.extend( struct.get( what, dtype, path + name + '/' ) )
      else:
        if what == 'fields': items.append( path + name )
        elif what == 'format': items.append( dtype.str )
    return items

def numpy_format_from_comma( comma_format ):
  numpy_types_dict = dict( b='int8', ub='uint8', w='int16', uw='uint16', i='int32', ui='uint32', l='int64', ul='uint64', f='float32', d='float64', t='datetime64[us]' )
  def numpy_string_type( comma_string_type ):
    match = re.match( '^s\[(\d+)\]$', comma_string_type )
    if match: return 'S' + match.group(1)
  numpy_types = []
  for comma_type in comma_format.split(','):
    numpy_type = numpy_types_dict.get( comma_type ) or numpy_string_type( comma_type )
    if numpy_type is None: raise Exception( "format string '{}' contains unrecongnised type '{}'".format( comma_format, comma_type ) )
    numpy_types.append( numpy_type )
  return ','.join( numpy_types )

def numpy_time_from_comma( comma_time_string ):
  v = list( comma_time_string )
  if len( v ) < 15: raise Exception( "ascii time string '{}' is not recognised".format( comma_time_string ) )
  for i in [13,11]: v.insert( i, ':' )
  for i in [6,4]: v.insert( i, '-' )
  return numpy.datetime64( ''.join( v ), 'us' )

def numpy_time_to_comma( numpy_time ):
  return numpy_time.item().isoformat().translate( None, '-:' )

class stream:
  buffer_size_in_bytes = 65536
  def __init__( self, struct, fields=None, format=None, binary=False, delimiter=',', flush=False ):
    self.struct = struct
    self.fields = fields if fields else self.struct.fields
    self.binary = binary or format is not None
    self.delimiter = delimiter if not self.binary else None
    self.flush = flush
    if format is None:
      if self.fields == self.struct.fields:
        self.format = self.struct.format
      else:
        struct_format_of_field = dict( zip( self.struct.fields.split(','), self.struct.format.split(',') ) )
        self.format = ','.join( struct_format_of_field.get( name ) or 'S' for name in self.fields.split(',') )
    else:
      try:
        numpy.dtype( format )
        self.format = format
      except TypeError:
        self.format = numpy_format_from_comma( format )
    self.dtype = numpy.dtype( self.format )
    self.default_size = max( 1, stream.buffer_size_in_bytes / self.dtype.itemsize )
    if not self.binary:
      self.converters = { i:numpy_time_from_comma for i in numpy.where( numpy.array( self.format.split(',') ) == numpy.dtype('datetime64[us]').str )[0] }
    self.struct_flat_dtype = numpy.dtype( self.struct.format )
    if self.fields == self.struct.fields:
        self.reshaped_dtype = None
    else:
      names = map( lambda _: 'f' + str( self.fields.split(',').index( _ ) ), self.struct.fields.split(',') )
      formats = [ self.dtype.fields[name][0] for name in names ]
      offsets = [ self.dtype.fields[name][1] for name in names ]
      self.reshaped_dtype = numpy.dtype( dict( names=names, formats=formats, offsets=offsets ) )

  def iter( self, size=None, recarray=True  ):
    size = self.default_size if size is None else size
    while True:
      s = self.read( size, recarray )
      if s is None: break
      yield s

  def read( self, size=None, recarray=True ):
    if self.binary:
      data = numpy.fromfile( sys.stdin, dtype=self.dtype, count=self.default_size if size is None else size )
    else:
      with warnings.catch_warnings():
        warnings.simplefilter( 'ignore' )
        data = numpy.loadtxt( StringIO( ''.join( itertools.islice( sys.stdin, size ) ) ), dtype=self.dtype , delimiter=self.delimiter, converters=self.converters, ndmin=1 )
    if data.size == 0: return None
    s = numpy.array( map( tuple, numpy.ndarray( data.shape, self.reshaped_dtype, data )[:] ), dtype=self.struct_flat_dtype ).view( self.struct ) if self.reshaped_dtype else data.view( self.struct )
    return s.view( numpy.recarray ) if recarray else s

  def write( self, s ):
    if self.binary:
      s.tofile( sys.stdout )
    else:
      to_string = lambda _: numpy_time_to_comma( _ ) if isinstance( _, numpy.datetime64 ) else str( _ )
      for _ in s.view( self.struct_flat_dtype ):
        print self.delimiter.join( map( to_string, _ ) )
    if self.flush: sys.stdout.flush()



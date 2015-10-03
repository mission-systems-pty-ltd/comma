import numpy
import sys
from StringIO import StringIO
import itertools
import io
import warnings
import operator
import types

class struct:
  def __init__( self, fields, *types ):
    if len( fields.split(',') ) != len( types ):
      raise Exception( "expected {} types for fields '{}', got {} type(s)".format( len( fields.split(',') ), fields, len( types )) )
    self.dtype = numpy.dtype( zip( fields.split(','), types ) )
    self.fields = ','.join( struct.get( 'fields', self.dtype ) )
    self.format = ','.join( struct.get( 'format', self.dtype ) )

  @staticmethod
  def get( what, nested_dtype, path='' ):
    items = []
    for name in nested_dtype.names:
      dtype = nested_dtype.fields[name][0]
      if dtype.type == numpy.void and not dtype.subdtype:
        items.extend( struct.get( what, dtype, path + name + '/' ) )
      else:
        if what == 'fields': items.append( path + name )
        elif what == 'format': items.append( dtype.str )
    return items

class stream:
  buffer_size_in_bytes = 65536
  def __init__( self, s, fields=None, format=None, binary=False, delimiter=',', flush=False ):
    if not isinstance( s, struct ): raise Exception( "expected '{}', got '{}'".format( str( struct ), repr( s ) ) )
    self.struct = s
    if fields is None:
      self.fields = self.struct.fields
    else:
      for field in self.struct.fields.split(','):
        if not field in fields.split(','): raise Exception( "expected field '{}' is not found in fields '{}'".format( field, fields ) )
      self.fields = fields
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
      if len( self.fields.split(',') ) != len( format.split(',') ):
        raise Exception( "expected same number of fields and format types, got '{}' and '{}'".format( self.fields, format ) )
      self.format = format
    self.dtype = numpy.dtype( self.format )
    self.size = max( 1, stream.buffer_size_in_bytes / self.dtype.itemsize )
    if not self.binary:
      self.converters = { i: types.time_to_numpy for i in numpy.where( numpy.array( self.format.split(',') ) == numpy.dtype('datetime64[us]').str )[0] }
    self.struct_flat_dtype = numpy.dtype( self.struct.format )
    if self.fields == self.struct.fields:
        self.reshaped_dtype = None
    else:
      names = map( lambda _: 'f' + str( self.fields.split(',').index( _ ) ), self.struct.fields.split(',') )
      formats = [ self.dtype.fields[name][0] for name in names ]
      offsets = [ self.dtype.fields[name][1] for name in names ]
      self.reshaped_dtype = numpy.dtype( dict( names=names, formats=formats, offsets=offsets ) )

  def iter( self, size=None, recarray=False  ):
    size = self.size if size is None else size
    while True:
      s = self.read( size, recarray )
      if s is None: break
      yield s

  def read( self, size=None, recarray=True ):
    if self.binary:
      data = numpy.fromfile( sys.stdin, dtype=self.dtype, count=self.size if size is None else size )
    else:
      with warnings.catch_warnings():
        warnings.simplefilter( 'ignore' )
        data = numpy.loadtxt( StringIO( ''.join( itertools.islice( sys.stdin, size ) ) ), dtype=self.dtype , delimiter=self.delimiter, converters=self.converters, ndmin=1 )
    if data.size == 0: return None
    if self.reshaped_dtype:
      s = numpy.array( map( tuple, numpy.ndarray( data.shape, self.reshaped_dtype, data, strides=data.itemsize ) ), dtype=self.struct_flat_dtype ).view( self.struct )
    else:
      s = data.view( self.struct )
    return s.view( numpy.recarray ) if recarray else s

  def write( self, s, flush=None ):
    if self.binary:
      s.tofile( sys.stdout )
    else:
      to_string = lambda _: types.time_from_numpy( _ ) if isinstance( _, numpy.datetime64 ) else str( _ )
      for _ in s.view( self.struct_flat_dtype ):
        print self.delimiter.join( map( to_string, _ ) )
    flush = self.flush if flush is None else flush
    if flush: sys.stdout.flush()

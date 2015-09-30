#!/bin/python

import numpy
import sys
from StringIO import StringIO
import itertools
import io
import warnings

class struct:
  def __init__( self, fields, *types ):
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

class stream:
  def __init__( self, struct, fields=None, format=None, binary=False, delimiter=',', flush=False ):
    self.struct = struct
    self.fields = fields if fields else self.struct.fields
    self.binary = binary or format is not None
    self.delimiter= delimiter
    self.flush = flush
    if format is None:
      if self.fields == self.struct.fields:
        format = self.struct.format
      else:
        struct_format_of_field = dict( zip( self.struct.fields.split(','), self.struct.format.split(',') ) )
        format = ','.join( struct_format_of_field.get( name ) or 'S' for name in self.fields.split(',') )
    self.dtype = numpy.dtype( format )
    self.default_size = max( 1, 65536 / self.dtype.itemsize )
    self.struct_dtype = numpy.dtype( self.struct.format )
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
        data = numpy.loadtxt( StringIO( ''.join( itertools.islice( sys.stdin, size ) ) ), dtype=self.dtype , delimiter=self.delimiter )
      names = map( lambda _: 'f' + str( self.fields.split(',').index( _ ) ), self.struct.fields.split(',') )
      formats = [ data.dtype.fields[name][0] for name in names ]
      offsets = [ data.dtype.fields[name][1] for name in names ]
      self.reshaped_dtype = numpy.dtype( dict( names=names, formats=formats, offsets=offsets ) )
    if data.size == 0: return None
    s = numpy.array( map( tuple, numpy.ndarray( data.shape, self.reshaped_dtype, data )[:] ), dtype=self.struct_dtype ).view( self.struct ) if self.reshaped_dtype else data.view( self.struct )
    return s.view( numpy.recarray ) if recarray else s

  def write( self, s ):
    if self.binary:
      s.tofile( sys.stdout )
    else:
      for _ in s.view( self.struct_dtype ):
        print ','.join( map( str, _ ) )
    if self.flush: sys.stdout.flush()

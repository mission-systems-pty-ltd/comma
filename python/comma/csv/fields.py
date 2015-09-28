#!/bin/python

import numpy
import sys

def from_dictionary( p, d ):
    for n, v in d.iteritems() : exec( "p." + n + " = " + str( v ) )

def to_dictionary( p, d ):
    for n, v in d.iteritems() : exec( "d[n] = p." + n )


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
        elif what == 'format': items.append( dtype.name )
    return items

class stream:
  def __init__( self, struct, fields=None, format=None, flush=False ):
    self.struct = struct
    self.fields = fields if fields else struct.fields
    self.format = format if format else struct.format
    self.dtype = numpy.dtype( self.format )
    self.flush = flush
    if self.fields == struct.fields:
      self.reshaped_dtype = None
    else:
      names = map( lambda _: 'f' + str( self.fields.split(',').index( _ ) ), struct.fields.split(',') )
      formats = [ self.dtype.fields[name][0] for name in names ]
      offsets = [ self.dtype.fields[name][1] for name in names ]
      self.reshaped_dtype = numpy.dtype( dict( names=names, formats=formats, offsets=offsets ) )

  def iter( self, size=1, recarray=True  ):
    while True:
      s = self.read( size, recarray )
      if s is None: break
      yield s

  def read( self, size=1, recarray=True ):
    data = numpy.fromfile( sys.stdin, dtype=self.format, count=size )
    if data.size == 0: return None
    s = numpy.array( map( tuple, numpy.ndarray( data.shape, self.reshaped_dtype, data )[:] ), dtype=self.struct.format ).view( self.struct ) if self.reshaped_dtype else data.view( self.struct )
    return s.view( numpy.recarray ) if recarray else s

  def write( self, s ):
    if s.dtype == self.struct: s.tofile( sys.stdout )
    if self.flush: sys.stdout.flush()

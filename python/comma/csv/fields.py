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
  def __init__( self, struct, fields=None, format=None, delimiter=',', flush=False ):
    self.struct = struct
    self.fields = fields if fields else struct.fields
    self.format = format if format else struct.format
    self.flush = flush
  
  def read( self, size=1, recarray=True ):
    while True:
      data = numpy.fromfile( sys.stdin, dtype=self.format, count=size )
      if data.size == 0: break
      s = data.view( self.struct ) # TODO: implement visiting for when format has more fields or they are in different order
      yield s if not recarray else s.view( numpy.recarray )
    
  def write( self, s ):
    if s.dtype == self.struct: s.tofile( sys.stdout )
    if self.flush: sys.stdout.flush()
    
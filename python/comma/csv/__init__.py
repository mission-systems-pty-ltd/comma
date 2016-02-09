import numpy
import sys
from StringIO import StringIO
import itertools
import warnings
import operator
import re
import comma.csv.format
import comma.csv.time

def merge_arrays( a1, a2 ):
  if a1.size != a2.size: raise Exception( "expected arrays of same size, got {} and {}".format( a1.size, a2.size ) )
  merged = numpy.empty( a1.size, dtype=numpy.dtype( [ ( 'a1', a1.dtype ), ( 'a2', a2.dtype ) ] ) )
  for name in a1.dtype.names: merged['a1'][name] = a1[name]
  for name in a2.dtype.names: merged['a2'][name] = a2[name]
  return merged

def unrolled_types_of_flat_dtype( dtype ):
  shape_unrolled_types = []
  for descr in dtype.descr:
    type = descr[1]
    shape = descr[2] if len( descr ) > 2 else ()
    shape_unrolled_types.extend( [ type ] * reduce( operator.mul, shape, 1 ) )
  return tuple( shape_unrolled_types )

def structured_dtype( numpy_format ):
  number_of_types = len( re.sub( r'\([^\)]*\)', '', numpy_format ).split(',') )
  return numpy.dtype( [ ( 'f0', numpy_format ) ] ) if number_of_types == 1 else numpy.dtype( numpy_format )

def format_from_types( types ):
  return ','.join( type if isinstance( type, basestring ) else numpy.dtype( type ).str for type in types )

class struct:
  default_field_name = 'comma_struct_default_field_name_'
  def __init__( self, concise_fields, *concise_types ):
    if '/' in concise_fields: raise Exception( "expected fields without '/', got '{}'".format( concise_fields ) )
    given_fields = concise_fields.split(',')
    if len( given_fields ) > len( concise_types ): raise Exception( "expected no more than {} fields for {} types, got {}".format( len( concise_types ), len( concise_types ), concise_fields ) )
    omitted_fields = [''] * ( len( concise_types ) - len( given_fields ) )
    concise_fields_no_blanks = [ field if field else '{}{}'.format( struct.default_field_name, index ) for index, field in enumerate( given_fields + omitted_fields ) ]
    self.dtype = numpy.dtype( zip( concise_fields_no_blanks, concise_types ) )
    fields, types = [], []
    self.shorthand = {}
    for name, type in zip( concise_fields_no_blanks, concise_types ):
      if isinstance( type, struct ):
        fields_of_type = [ name + '/' + field for field in type.fields ]
        fields.extend( fields_of_type )
        types.extend( type.types )
        self.shorthand[name] = tuple( fields_of_type )
        for subname,subfields in type.shorthand.iteritems():
          self.shorthand[ name + '/' + subname ] = tuple( name + '/' + field for field in subfields )
      else:
        fields.append( name )
        types.append( type )
    self.fields = tuple( fields )
    self.types = tuple( types )
    self.format = format_from_types( self.types )
    self.flat_dtype = numpy.dtype( zip( self.fields, self.types ) )
    self.unrolled_flat_dtype = structured_dtype( ','.join( unrolled_types_of_flat_dtype( self.flat_dtype ) ) )
    self.type_of_field = dict( zip( self.fields, self.types ) )
    self.leaves = tuple( xpath.split('/')[-1] for xpath in self.fields )
    self.ambiguous_leaves = set( leaf for leaf in self.leaves if self.leaves.count( leaf ) > 1 )
    d = dict( zip( self.leaves, self.fields ) )
    for ambiguous_leaf in self.ambiguous_leaves: del d[ ambiguous_leaf ]
    self.xpath_of_leaf = d

class stream:
  buffer_size_in_bytes = 65536
  def __init__( self, s, fields='', format='', binary=None, delimiter=',', precision=12, flush=False, source=sys.stdin, target=sys.stdout, tied=None, full_xpath=True ):
    if not isinstance( s, struct ): raise Exception( "expected '{}', got '{}'".format( str( struct ), repr( s ) ) )
    self.struct = s
    if fields == '':
      self.fields = self.struct.fields
    else:
      if full_xpath:
        self.fields = sum( map( lambda name: self.struct.shorthand.get( name ) or ( name, ), fields.split(',') ), () )
      else:
        if '/' in fields: raise Exception( "expected fields without '/', got '{}'".format( fields ) )
        ambiguous_leaves = self.struct.ambiguous_leaves.intersection( fields.split(',') )
        if ambiguous_leaves: raise Exception( "fields '{}' are ambiguous in '{}', use full xpath".format( ','.join( ambiguous_leaves ), fields ) )
        self.fields = tuple( self.struct.xpath_of_leaf.get( name ) or name for name in fields.split(',') )
      if not set( self.fields ).intersection( self.struct.fields ): raise Exception( "provided fields '{}' do not match any of the expected fields '{}'".format( fields, ','.join( self.struct.fields ) ) )
    if binary == True and not format:
      if not set( self.struct.fields ).issuperset( self.fields ): raise Exception( "failed to infer type of every field in '{}', specify format".format( ','.join( self.fields ) ) )
      format = format_from_types( self.struct.type_of_field[name] for name in self.fields )
    elif binary == False and format:
      format = ''
    self.binary = format != ''
    self.delimiter = delimiter
    self.flush = flush
    self.precision = precision
    self.source = source
    self.target = target
    self.tied = tied
    duplicates = tuple( field for field in self.struct.fields if field in self.fields and self.fields.count( field ) > 1 )
    if duplicates: raise Exception( "fields '{}' have duplicates in '{}'".format( ','.join( duplicates ), ','.join( self.fields ) ) )
    if self.tied:
      if not isinstance( tied, stream ): raise Exception( "expected tied stream of type '{}', got '{}'".format( str( stream ), repr( tied ) ) )
      if self.tied.binary != self.binary: raise Exception( "expected tied stream to be {}, got {}".format( "binary" if self.binary else "ascii", "binary" if self.tied.binary else "ascii" ) )
      if not self.binary and self.tied.delimiter != self.delimiter: raise Exception( "expected tied stream to have the same delimiter '{}', got '{}'".format( self.delimiter, self.tied.delimiter ) )
    if self.binary:
      self.input_dtype = structured_dtype( format )
      if len( self.fields ) != len( self.input_dtype.names ): raise Exception( "expected same number of fields and format types, got '{}' and '{}'".format( ','.join( self.fields ), format ) )
    else:
      self.input_dtype = structured_dtype( format_from_types( self.struct.type_of_field.get( name ) or 'S' for name in self.fields ) ) if self.fields != self.struct.fields else self.struct.flat_dtype
      self.usecols = tuple( range( len( unrolled_types_of_flat_dtype( self.input_dtype ) ) ) )
    self.size = self.tied.size if self.tied else ( 1 if self.flush else max( 1, stream.buffer_size_in_bytes / self.input_dtype.itemsize ) )
    self.ascii_converters = comma.csv.time.ascii_converters( unrolled_types_of_flat_dtype( self.input_dtype ) )
    if set( self.fields ).issuperset( self.struct.fields ):
      self.missing_fields = ()
    else:
      self.missing_fields = tuple( field for field in self.struct.fields if field not in self.fields )
      warnings.warn( "expected fields '{}' are not found in supplied fields '{}'".format( ','.join( self.missing_fields ), ','.join( self.fields ) ) )
    if self.missing_fields:
      self.complete_fields = self.fields + self.missing_fields
      missing_names = [ 'f' + str( i + len( self.input_dtype.names ) ) for i in xrange( len( self.missing_fields ) ) ]
      missing_types = [ self.struct.type_of_field.get( name ) for name in self.missing_fields ]
      self.missing_fields_dtype = numpy.dtype( zip( missing_names, missing_types ) )
      self.complete_dtype = numpy.dtype( self.input_dtype.descr + zip( missing_names, missing_types ) )
    else:
      self.complete_fields = self.fields
      self.complete_dtype = self.input_dtype
    if self.fields == self.struct.fields:
        self.data_extraction_dtype = None
    else:
      names = [ 'f' + str( self.complete_fields.index( name ) ) for name in self.struct.fields ]
      formats = [ self.complete_dtype.fields[name][0] for name in names ]
      offsets = [ self.complete_dtype.fields[name][1] for name in names ]
      self.data_extraction_dtype = numpy.dtype( dict( names=names, formats=formats, offsets=offsets ) )

  def iter( self, size=None ):
    while True:
      s = self.read( size )
      if s is None: break
      yield s

  def read( self, size=None ):
    size = self.size if size is None else size
    if size < 0:
      if self.source == sys.stdin: raise Exception( "expected positive size when stream source is stdin, got {}".format( size ) )
      size = -1 if self.binary else None
    if self.binary:
      self.input_data = numpy.fromfile( self.source, dtype=self.input_dtype, count=size )
    else:
      with warnings.catch_warnings():
        warnings.simplefilter( 'ignore' )
        self.ascii_buffer = ','.join( self.source.readline() for _ in xrange( size ) )
        if not self.ascii_buffer: return None
        self.input_data = numpy.loadtxt( StringIO( self.ascii_buffer ), dtype=self.input_dtype , delimiter=self.delimiter, converters=self.ascii_converters, ndmin=1, usecols=self.usecols )
    if self.input_data.size == 0: return None
    if self.data_extraction_dtype:
      complete_data = merge_arrays( self.input_data, numpy.zeros( self.input_data.size, dtype=self.missing_fields_dtype ) ) if self.missing_fields else self.input_data
      return numpy.array( numpy.ndarray( complete_data.shape, self.data_extraction_dtype, complete_data, strides=complete_data.itemsize ).tolist(), dtype=self.struct.flat_dtype ).view( self.struct )
    else:
      return self.input_data.copy().view( self.struct )

  def numpy_scalar_to_string( self, scalar ):
    if scalar.dtype.char in numpy.typecodes['AllInteger']: return str( scalar )
    elif scalar.dtype.char in numpy.typecodes['Float']: return "{scalar:.{precision}g}".format( scalar=scalar, precision=self.precision )
    elif scalar.dtype.char in numpy.typecodes['Datetime'] : return comma.csv.time.from_numpy( scalar )
    elif scalar.dtype.char in 'S': return scalar
    else: raise Exception( "conversion to string for numpy type '{}' is not implemented".format( repr( scalar.dtype ) ) )

  def write( self, s ):
    if s.dtype != self.struct.dtype: raise Exception( "expected object of dtype '{}', got '{}'".format( str( self.struct.dtype ), repr( s.dtype ) ) )
    if self.tied and s.size != self.tied.input_data.size: raise Exception( "expected size {} to equal tied size {}".format( s.size, self.tied.size ) )
    if self.binary:
      ( merge_arrays( self.tied.input_data, s ) if self.tied else s ).tofile( self.target )
    else:
      for tied_line, scalars in itertools.izip_longest( self.tied.ascii_buffer.splitlines() if self.tied else [], s.view( self.struct.unrolled_flat_dtype ) ):
        print >> self.target, ( tied_line + self.delimiter if self.tied else '' ) + self.delimiter.join( map( self.numpy_scalar_to_string, scalars ) )
    self.target.flush()

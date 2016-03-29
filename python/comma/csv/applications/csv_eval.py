#!/usr/bin/python

import sys
import argparse
import numpy
import re
import comma.csv
import comma.signal

class stream:
  def __init__( self, args ):
    self.args = args
    self.csv_options = dict( full_xpath=False, flush=self.args.flush, delimiter=self.args.delimiter, precision=self.args.precision )
    self.initialize_input()
    self.initialize_output()

  def initialize_input( self ):
    self.nonblank_input_fields = filter( None, self.args.fields.split(',') )
    if not self.nonblank_input_fields: raise Exception( "specify input stream fields, e.g. --fields=x,y" )
    check_fields( self.nonblank_input_fields )
    types = comma.csv.format.to_numpy( self.args.binary ) if self.args.binary else tuple( 'float64' if field else 'S' for field in self.args.fields.split(',') )
    input_t = comma.csv.struct( self.args.fields, *types )
    self.input = comma.csv.stream( input_t, binary=bool( self.args.binary ), **self.csv_options )

  def initialize_output( self ):
    if self.args.append_fields:
      fields = self.args.append_fields
    else:
      expressions = sum( [ line.split(';') for line in self.args.expressions.splitlines() if line.strip() ], [] )
      fields = ','.join( e.split('=', 1)[0].strip() for e in expressions )
    check_fields( fields.split(','), input_fields=self.nonblank_input_fields )
    format = self.args.append_binary if self.args.append_binary else ','.join( ('d',) * len( fields.split(',') ) )
    if self.args.verbose:
      print >> sys.stderr, "append fields: '{}'".format( fields )
      print >> sys.stderr, "append format: '{}'".format( format )
      print >> sys.stderr, "numpy format: '{}'".format( ','.join( comma.csv.format.to_numpy( format ) ) )
      print >> sys.stderr, "expressions: '{}'".format( self.args.expressions )
    output_t = comma.csv.struct( fields, *comma.csv.format.to_numpy( format ) )
    self.output = comma.csv.stream( output_t, tied=self.input, binary=bool( self.args.binary ), **self.csv_options )

def get_dict( module, update={}, delete=[] ):
  d = module.__dict__.copy()
  d.update( update )
  for k in set( delete ).intersection( d.keys() ): del d[k]
  return d

def check_fields( fields, input_fields=(), env=get_dict( numpy ) ):
  for name in fields:
    if not re.match( r'^[a-z_]\w*$', name, re.I ): raise Exception( "'{}' is not a valid field name".format( name ) )
    if name == '__input' or name == '__output' or name in env: raise Exception( "'{}' is a reserved name".format( name ) )
    if name in input_fields: raise Exception( "'{}' is an input field name".format( name ) )

def evaluate( expressions, stream, dangerous=False ):
  initialize_input = '\n'.join( "{name} = __input['{name}']".format( name=name ) for name in stream.nonblank_input_fields )
  initialize_output = '\n'.join( "__output['{name}'] = {name}".format( name=name ) for name in stream.output.struct.fields )
  code = compile( initialize_input + '\n' + expressions + '\n' + initialize_output + '\n', '<string>', 'exec' )
  restricted_numpy = get_dict( numpy ) if dangerous else get_dict( numpy, update=dict(__builtins__={}), delete=['sys'] )
  output = numpy.empty( stream.input.size, dtype=stream.output.struct )
  is_shutdown = comma.signal.is_shutdown()
  while not is_shutdown:
    i = stream.input.read()
    if i is None: break
    if output.size != i.size: output = numpy.empty( i.size, dtype=stream.output.struct )
    exec code in restricted_numpy, { '__input': i, '__output': output }
    stream.output.write( output )

def add_csv_options( parser ):
  comma.csv.options.standard_csv_options( parser, { 'fields': 'x,y,z' } )

  parser.add_argument( "--append-fields", "-F", help="fields appended to input stream (by default, inferred from expressions)", metavar='<names>' )
  parser.add_argument( "--append-binary", "-B", help="for binary stream, binary format of appended fields (by default, 'd' for each)", metavar='<format>' )

def main():
  description = """
evaluate numerical expressions and append computed values to csv stream
"""
  epilog = """
notes:
    1) in ascii mode, input fields are treated as floating point numbers
    2) fields appended to input stream are inferred from expressions (by default) or specified by --append-fields
    3) if --append-fields is omitted, only simple assignment statements are allowed in expressions
    4) in binary mode, appended fields are assigned comma type 'd' (by default) or format is specified by --append-binary
    5) full xpath input fields are not allowed

examples:
    ( echo 1,2; echo 3,4 ) | {script_name} --fields=x,y --precision=2 'a=2/(x+y);b=x-sin(y)*a**2'
    ( echo 1,2; echo 3,4 ) | csv-to-bin 2d | {script_name} --binary=2d --fields=x,y 'a=2/(x+y);b=x-sin(y)*a**2' | csv-from-bin 4d

    # define intermediate variable
    ( echo 1; echo 2 ) | csv-to-bin d | {script_name} --binary=d --fields=x 'a=2;y=a*x' --append-fields=y | csv-from-bin 2d

    # take minimum
    ( echo 1,2; echo 4,3 ) | csv-to-bin 2d | {script_name} --binary=2d --fields=x,y 'c=minimum(x,y)' | csv-from-bin 3d

    # clip index
    ( echo a,2; echo b,5 ) | csv-to-bin s[1],ui | {script_name} --binary=s[1],ui --fields=,id 'i=clip(id,3,inf)' --append-binary=ui | csv-from-bin s[1],ui,ui

    # compare fields
    ( echo 1,2; echo 4,3 ) | csv-to-bin 2i | {script_name} --binary=2i --fields=i,j 'flag=i+1==j' --append-binary=b | csv-from-bin 2i,b
    ( echo 1,2; echo 4,3 ) | csv-to-bin 2d | {script_name} --binary=2d --fields=x,y 'flag=x<y' --append-binary=b | csv-from-bin 2d,b
    ( echo 0,1; echo 1,2; echo 4,3 ) | csv-to-bin 2d | {script_name} --binary=2d --fields=x,y 'flag=logical_and(x<y,y<2)' --append-binary=b | csv-from-bin 2d,b

    # negate boolean
    ( echo 0; echo 1 ) | csv-to-bin b | {script_name} --binary=b --fields=flag 'a=logical_not(flag)' --append-binary=b | csv-from-bin 2b

    # select operation based on condition
    ( echo 1,2; echo 2,1 ) | csv-to-bin 2d | {script_name} --fields=x,y --binary=2d 'a=where(x<y,x+y,x-y)' | csv-from-bin 3d

    # count number of occurances of "/" in a string
    ( echo 'a'; echo 'a/b' ) | csv-to-bin s[36] | {script_name} --fields=path --binary=s[36] 'n=char.count(path,"/")' --append-binary=ui | csv-from-bin s[36],ui

    # add and subtract a microsecond
    ( echo 20150101T000000.000000; echo 20150101T000000.000010 ) | csv-to-bin t | {script_name} --fields=t --binary=t 'a=t+1;b=t-1' --append-binary=2t | csv-from-bin 3t
 
""".format( script_name=sys.argv[0].split('/')[-1] )
  parser = argparse.ArgumentParser( description=description, epilog=epilog, formatter_class=lambda prog: argparse.RawTextHelpFormatter( prog, max_help_position=50 ) )
  parser.add_argument( "expressions", help="semicolon-separated numerical expressions to evaluate (see examples)" )
  parser.add_argument( "--verbose", "-v", action="store_true", help="output expressions and appended field names/types to stderr" )
  parser.add_argument( "--dangerous", action="store_true", help=argparse.SUPPRESS )
  add_csv_options( parser )
  args = parser.parse_args()
  if not args.expressions: raise Exception( "no expressions are given" )
  evaluate( args.expressions.strip(';'), stream( args ), dangerous=args.dangerous )

if __name__ == '__main__':
  main()

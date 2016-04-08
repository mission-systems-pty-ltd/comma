from __future__ import print_function
import sys
import argparse
import numpy
import re
import comma.csv
import comma.signal
import itertools
import warnings
import os


def custom_formatwarning(message, *args):
    return os.path.basename(sys.argv[0]) + ': warning: ' + str(message) + '\n'
warnings.formatwarning = custom_formatwarning


class csv_eval_error(Exception):
    pass


class stream(object):
    def __init__(self, args):
        self.args = args
        self.csv_options = dict(
            full_xpath=False,
            flush=self.args.flush,
            delimiter=self.args.delimiter,
            precision=self.args.precision)
        self.initialize_input()
        self.initialize_output()
        if self.args.verbose:
            self.output_info()

    @staticmethod
    def output_fields_from_expressions(expressions_string):
        lines = expressions_string.splitlines()
        expressions = sum([line.split(';') for line in lines if line.strip()], [])
        return ','.join(e.split('=', 1)[0].strip() for e in expressions)

    def initialize_input(self):
        fields = self.args.fields
        self.nonblank_input_fields = filter(None, fields.split(','))
        if not self.nonblank_input_fields:
            raise csv_eval_error("specify input stream fields, e.g. --fields=x,y")
        check_fields(self.nonblank_input_fields)
        types = comma.csv.format.to_numpy(self.args.format)
        input_t = comma.csv.struct(fields, *types)
        options = dict(binary=self.args.binary, **self.csv_options)
        self.input = comma.csv.stream(input_t, **options)

    def initialize_output(self):
        fields = self.args.output_fields
        check_fields(fields.split(','), input_fields=self.nonblank_input_fields)
        types = comma.csv.format.to_numpy(self.args.output_format)
        output_t = comma.csv.struct(fields, *types)
        options = dict(tied=self.input, binary=self.args.binary, **self.csv_options)
        self.output = comma.csv.stream(output_t, **options)
        self.output_fields = self.output.struct.fields

    def output_info(self, file=sys.stderr):
        fields = ','.join(self.input.struct.fields)
        format = self.input.struct.format
        output_fields = ','.join(self.output.struct.fields)
        output_format = self.output.struct.format
        print("input fields: '{}'".format(fields), file=file)
        print("input format: '{}'".format(format), file=file)
        print("output fields: '{}'".format(output_fields), file=file)
        print("output format: '{}'".format(output_format), file=file)
        print("expressions: '{}'".format(self.args.expressions), file=file)


def get_dict(module, update={}, delete=[]):
    d = module.__dict__.copy()
    d.update(update)
    for k in set(delete).intersection(d.keys()):
        del d[k]
    return d


def check_fields(fields, input_fields=(), env=get_dict(numpy)):
    for field in fields:
        if not re.match(r'^[a-z_]\w*$', field, re.I):
            raise csv_eval_error("'{}' is not a valid field name".format(field))
        if field == '_input' or field == '_output' or field in env:
            raise csv_eval_error("'{}' is a reserved name".format(field))
        if field in input_fields:
            raise csv_eval_error("'{}' is an input field name".format(field))


def evaluate(expressions, stream, dangerous=False):
    input_initializer = ''
    for field in stream.nonblank_input_fields:
        input_initializer += "{field} = _input['{field}']\n".format(field=field)
    output_initializer = ''
    for field in stream.output_fields:
        output_initializer += "_output['{field}'] = {field}\n".format(field=field)
    code_string = input_initializer + '\n' + expressions + '\n' + output_initializer
    code = compile(code_string, '<string>', 'exec')
    kwds = {} if dangerous else {'update': dict(__builtins__={}), 'delete': ['sys']}
    restricted_numpy = get_dict(numpy, **kwds)
    output = numpy.empty(stream.input.size, dtype=stream.output.struct)
    is_shutdown = comma.signal.is_shutdown()
    while not is_shutdown:
        i = stream.input.read()
        if i is None:
            break
        if output.size != i.size:
            output = numpy.empty(i.size, dtype=stream.output.struct)
        exec code in restricted_numpy, {'_input': i, '_output': output}
        stream.output.write(output)


description = """
evaluate numerical expressions and append computed values to csv stream
"""


notes_and_examples = """
input fields:
    1) full xpath input fields are not allowed
    2) for ascii streams, input fields are treated as floating point numbers, unless --format is given

output fields:
    1) output fields are inferred from expressions (by default) or specified by --output-fields
    2) output fields are always appended to unmodified input
    3) if --output-fields is omitted, only simple assignment statements are allowed in expressions
    4) output fields are treated as floating point numbers, unless --output-format is given

examples:
    ( echo 1,2; echo 3,4 ) | %(prog)s --fields=x,y --precision=2 'a=2/(x+y);b=x-sin(y)*a**2'
    ( echo 1,2; echo 3,4 ) | csv-to-bin 2d | %(prog)s --binary=2d --fields=x,y 'a=2/(x+y);b=x-sin(y)*a**2' | csv-from-bin 4d

    # define intermediate variable
    ( echo 1; echo 2 ) | csv-to-bin d | %(prog)s --binary=d --fields=x 'a=2;y=a*x' --output-fields=y | csv-from-bin 2d

    # take minimum
    ( echo 1,2; echo 4,3 ) | csv-to-bin 2d | %(prog)s --binary=2d --fields=x,y 'c=minimum(x,y)' | csv-from-bin 3d

    # clip index
    ( echo a,2; echo b,5 ) | csv-to-bin s[1],ui | %(prog)s --binary=s[1],ui --fields=,id 'i=clip(id,3,inf)' --output-format=ui | csv-from-bin s[1],ui,ui

    # compare fields
    ( echo 1,2; echo 4,3 ) | csv-to-bin 2i | %(prog)s --binary=2i --fields=i,j 'flag=i+1==j' --output-format=b | csv-from-bin 2i,b
    ( echo 1,2; echo 4,3 ) | csv-to-bin 2d | %(prog)s --binary=2d --fields=x,y 'flag=x<y' --output-format=b | csv-from-bin 2d,b
    ( echo 0,1; echo 1,2; echo 4,3 ) | csv-to-bin 2d | %(prog)s --binary=2d --fields=x,y 'flag=logical_and(x<y,y<2)' --output-format=b | csv-from-bin 2d,b

    # negate boolean
    ( echo 0; echo 1 ) | %(prog)s --format=b --fields=flag 'a=logical_not(flag)' --output-format=b
    ( echo 0; echo 1 ) | csv-to-bin b | %(prog)s --binary=b --fields=flag 'a=logical_not(flag)' --output-format=b | csv-from-bin 2b

    # select operation based on condition
    ( echo 1,2; echo 2,1 ) | csv-to-bin 2d | %(prog)s --fields=x,y --binary=2d 'a=where(x<y,x+y,x-y)' | csv-from-bin 3d

    # count number of occurances of "/" in a string
    ( echo 'a'; echo 'a/b' ) | %(prog)s --fields=path --format=s[36] 'n=char.count(path,"/")' --output-format=ui
    ( echo 'a'; echo 'a/b' ) | csv-to-bin s[36] | %(prog)s --fields=path --binary=s[36] 'n=char.count(path,"/")' --output-format=ui | csv-from-bin s[36],ui

    # add and subtract a microsecond
    ( echo 20150101T000000.000000; echo 20150101T000000.000010 ) | %(prog)s --fields=t --format=t 'a=t+1;b=t-1' --output-format=2t
    ( echo 20150101T000000.000000; echo 20150101T000000.000010 ) | csv-to-bin t | %(prog)s --fields=t --binary=t 'a=t+1;b=t-1' --output-format=2t | csv-from-bin 3t
 
"""


def add_csv_options(parser):
    comma.csv.options.standard_csv_options(parser, defaults={'fields': 'x,y,z'})
    parser.add_argument(
        '--format',
        default='',
        metavar='<format>',
        help="for ascii stream, format of named input fields (by default, 'd' for each)")
    parser.add_argument(
        '--output-fields',
        '-o',
        default='',
        metavar='<names>',
        help="field names of output stream (by default, inferred from expressions)")
    parser.add_argument(
        '--output-format',
        default='',
        metavar='<format>',
        help="format of output fields (by default, 'd' for each)")
    # the options defined below are left for compatibility
    # use --output-fields and --output-format instead
    parser.add_argument('--append-fields', '-F', help=argparse.SUPPRESS)
    parser.add_argument('--append-binary', '-B', help=argparse.SUPPRESS)


def argparse_fmt(prog):
    return argparse.RawTextHelpFormatter(prog, max_help_position=50)


def get_parser():
    parser = argparse.ArgumentParser(
        description=description,
        epilog=notes_and_examples,
        formatter_class=argparse_fmt)
    parser.add_argument(
        'expressions',
        help='numerical expressions to evaluate (see examples)')
    parser.add_argument(
        '--verbose',
        '-v',
        action='store_true',
        help='print input/output fields and formats on stderr')
    parser.add_argument(
        '--dangerous',
        action='store_true',
        help=argparse.SUPPRESS)
    add_csv_options(parser)
    return parser


def ingest_deprecated_options(args):
    if args.append_binary:
        if args.verbose:
            warnings.warn("--append-binary is deprecated, consider using --output-format")
        args.output_format = args.append_binary
        del args.append_binary
    if args.append_fields:
        if args.verbose:
            warnings.warn("--append-fields is deprecated, consider using --output-fields")
        args.output_fields = args.append_fields
        del args.append_fields


def check_options(args):
    if not args.expressions:
        raise csv_eval_error("no expressions are given")
    if args.binary and args.format:
        raise csv_eval_error("--binary and --format are mutually exclusive")


def comma_type(maybe_type, field, default_type='d', type_of_unnamed_field='s[0]'):
    return type_of_unnamed_field if not field else maybe_type or default_type


def format_without_blanks(format, fields):
    maybe_types = comma.csv.format.expand(format).split(',')
    maybe_typed_fields = itertools.izip_longest(maybe_types, fields.split(','))
    types = [comma_type(maybe_type, field) for maybe_type, field in maybe_typed_fields]
    return ','.join(types)


def prepare_options(args):
    ingest_deprecated_options(args)
    check_options(args)
    if args.binary:
        args.format = comma.csv.format.expand(args.binary)
        args.binary = True
    else:
        args.format = format_without_blanks(args.format, args.fields)
        args.binary = False
    if not args.output_fields:
        args.output_fields = stream.output_fields_from_expressions(args.expressions)
    args.output_format = format_without_blanks(args.output_format, args.output_fields)


def main():
    args = get_parser().parse_args()
    prepare_options(args)
    evaluate(args.expressions.strip(';'), stream(args), dangerous=args.dangerous)


if __name__ == '__main__':
    main()

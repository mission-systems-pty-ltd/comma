import sys
import os
import argparse
import numpy as np
import re
import itertools
import ast
import comma

description = """
evaluate numerical expressions and append computed values to csv stream
"""


notes_and_examples = """
input fields:
    1) full xpath input fields are not allowed
    2) for ascii streams, input fields are treated as floating point numbers, unless --format is given

output fields:
    1) inferred from expressions (by default) or specified by --output-fields
    2) always appended to unmodified input
    3) treated as floating point numbers, unless --output-format is given

examples:
    # basic
    ( echo 1; echo 2; echo 3 ) | %(prog)s --fields=x 'y = x**2'

    # using an intermediate variable
    ( echo 1; echo 2; echo 3 ) | %(prog)s --fields=x 'n = 2; y = x**n' --output-fields=y

    # ascii stream with non-default formats
    ( echo 0,1; echo 1,1 ) | %(prog)s --fields=x,y 'n = x<y' --output-format=ub
    ( echo 0,1; echo 1,1 ) | %(prog)s --fields=i,j --format=2ub 'n = i==j' --output-format=ub

    # binary stream
    ( echo 0.1,2; echo 0.1,3 ) | csv-to-bin d,i | %(prog)s --binary=d,i --fields=x,n 'y = x**n' | csv-from-bin d,i,d

    # evaluate one of two expressions based on condition
    ( echo 1,2; echo 2,1 ) | %(prog)s --fields=x,y 'a=where(x<y,x+y,x-y)'

    # time arithmetic
    echo 20150101T000000.000000 | %(prog)s --fields=t --format=t 'a=t+1;b=t-1' --output-format=2t

    # select output based on condition
    ( echo 1,2 ; echo 1,3; echo 1,4 ) | %(prog)s --fields=a,b --format=2i --select="(a < b - 1) & (b < 4)"
"""

numpy_functions = """
functions:
    any function documented at
    http://docs.scipy.org/doc/numpy/reference/routines.html
    can be used in expressions provided that it is compatible with streaming, that is:
        - it performs element-wise operations only
        - it returns an array of the same shape and size as the input
    some examples are given below

math functions:
    http://docs.scipy.org/doc/numpy/reference/routines.math.html

    ( echo 1,2; echo 3,4 ) | %(prog)s --fields=x,y --precision=2 'a = 2/(x+y); b = a*sin(x-y)'
    ( echo 1,2; echo 4,3 ) | %(prog)s --fields=x,y 'm = minimum(x,y)'
    ( echo 1; echo 2; echo 3; echo 4 ) | %(prog)s --format=ui --fields=id 'c = clip(id,3,inf)' --output-format=ui

math constants: pi, e
    echo pi | %(prog)s --fields name --format=s[2] 'value=pi' --precision=16 --delimiter='='
    echo e | %(prog)s --fields name --format=s[2] 'value=e' --precision=16 --delimiter='='

logical functions:
    http://docs.scipy.org/doc/numpy/reference/routines.logic.html

    ( echo 0,1; echo 1,2; echo 4,3 ) | %(prog)s --fields=x,y 'flag=logical_and(x<y,y<2)' --output-format=b
    ( echo 0,1; echo 1,2; echo 4,3 ) | %(prog)s --fields=x,y 'flag=logical_or(x>y,y<2)' --output-format=b
    ( echo 0; echo 1 ) | %(prog)s --format=b --fields=flag 'a=logical_not(flag)' --output-format=b

bitwise functions:
    http://docs.scipy.org/doc/numpy/reference/routines.bitwise.html

    ( echo 0; echo 1 ) | %(prog)s --fields i --format=ub 'n = ~i'
    ( echo 0; echo 1 ) | %(prog)s --fields i --format=ub 'n = ~i.astype(bool)'
    ( echo 0,0; echo 0,1; echo 1,1 ) | %(prog)s --fields i,j --format=2ub 'm = i & j'
    ( echo 0,0; echo 0,1; echo 1,1 ) | %(prog)s --fields i,j --format=2ub 'm = i | j'

string functions:
    http://docs.scipy.org/doc/numpy/reference/routines.char.html

    ( echo 'a'; echo 'a/b' ) | %(prog)s --fields=path --format=s[36] 'n=char.count(path,"/")' --output-format=ui
    ( echo 'a'; echo 'a/b' ) | %(prog)s --fields=path --format=s[36] 'r=char.replace(path,"/","_")' --output-format=s[36]
"""


class csv_eval_error(Exception):
    pass


def custom_formatwarning(msg, *args):
    return __name__ + " warning: " + str(msg) + '\n'


def add_csv_options(parser):
    comma.csv.add_options(parser, defaults={'fields': 'x,y,z'})
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
        help="do not infer output fields from expressions; use specified fields instead")
    parser.add_argument(
        '--output-format',
        default='',
        metavar='<format>',
        help="format of output fields (by default, 'd' for each)")
    # the options defined below are left for compatibility
    # use --output-fields and --output-format instead
    parser.add_argument('--append-fields', '-F', help=argparse.SUPPRESS)
    parser.add_argument('--append-binary', '-B', help=argparse.SUPPRESS)


def get_args():
    parser = argparse.ArgumentParser(
        description=description,
        epilog=notes_and_examples,
        formatter_class=comma.util.argparse_fmt,
        add_help=False)
    parser.add_argument(
        'expressions',
        help='numerical expressions to evaluate (see examples)',
        nargs='?')
    parser.add_argument(
        '--help',
        '-h',
        action='store_true',
        help='show this help message and exit')
    parser.add_argument(
        '--verbose',
        '-v',
        action='store_true',
        help='more output to stderr')
    parser.add_argument(
        '--permissive',
        action='store_true',
        help='leave python builtins in the exec environment (use with care)')
    add_csv_options(parser)
    parser.add_argument(
        '--select',
        '--output-if',
        '--if',
        metavar='<cond>',
        help='select and output records of input stream that satisfy the condition')
    args = parser.parse_args()
    if args.help:
        if args.verbose:
            parser.epilog += numpy_functions
        else:
            parser.epilog += "\nfor more help run '%(prog)s -h -v'"
        parser.print_help()
        parser.exit(0)
    return args


def ingest_deprecated_options(args):
    if args.append_binary:
        args.output_format = args.append_binary
        del args.append_binary
        if args.verbose:
            with comma.util.warning(custom_formatwarning) as warn:
                msg = "--append-binary is deprecated, consider using --output-format"
                warn(msg)
    if args.append_fields:
        args.output_fields = args.append_fields
        del args.append_fields
        if args.verbose:
            with comma.util.warning(custom_formatwarning) as warn:
                msg = "--append-fields is deprecated, consider using --output-fields"
                warn(msg)


def check_options(args):
    if not (args.expressions or args.select):
        raise csv_eval_error("no expressions are given")
    if args.binary and args.format:
        raise csv_eval_error("--binary and --format are mutually exclusive")
    if args.select and args.expressions:
        msg = "--select <cond> cannot be used with expressions"
        raise csv_eval_error(msg)
    if args.select and (args.output_fields or args.output_format):
        msg = "--select cannot be used with --output-fields or --output-format"
        raise csv_eval_error(msg)


def format_without_blanks(format, fields=''):
    """
    >>> from comma.csv.applications.csv_eval import format_without_blanks
    >>> format_without_blanks('3ui', fields='a,b,c')
    'ui,ui,ui'
    >>> format_without_blanks('ui', fields='a,b,c')
    'ui,d,d'
    >>> format_without_blanks('ui', fields='a,,c')
    'ui,s[0],d'
    >>> format_without_blanks('4ui', fields='a,,c')
    'ui,s[0],ui,s[0]'
    >>> format_without_blanks('3ui')
    's[0],s[0],s[0]'
    """
    def comma_type(maybe_type, field, default_type='d', type_of_unnamed_field='s[0]'):
        return type_of_unnamed_field if not field else maybe_type or default_type

    maybe_types = comma.csv.format.expand(format).split(',')
    maybe_typed_fields = itertools.izip_longest(maybe_types, fields.split(','))
    types = [comma_type(maybe_type, field) for maybe_type, field in maybe_typed_fields]
    return ','.join(types)


def output_fields_from_expressions(expressions):
    """
    >>> from comma.csv.applications.csv_eval import output_fields_from_expressions
    >>> output_fields_from_expressions("a = 1; b = x + y; c = 'x = 1; y = 2'; d = (b == z)")
    'a,b,c,d'
    """
    tree = ast.parse(expressions, '<string>', mode='exec')
    fields = []
    for child in ast.iter_child_nodes(tree):
        if type(child) != ast.Assign:
            continue
        for target in child.targets:
            fields.extend(node.id for node in ast.walk(target) if type(node) == ast.Name)
    if not fields:
        msg = "failed to infer output fields from '{}'".format(expressions)
        raise csv_eval_error(msg)
    return ','.join(fields)


def prepare_options(args):
    ingest_deprecated_options(args)
    check_options(args)
    if args.binary:
        args.format = comma.csv.format.expand(args.binary)
        args.binary = True
    else:
        args.format = format_without_blanks(args.format, args.fields)
        args.binary = False
    if not args.select:
        if not args.output_fields:
            args.output_fields = output_fields_from_expressions(args.expressions)
        args.output_format = format_without_blanks(args.output_format, args.output_fields)


def restricted_numpy_env():
    d = np.__dict__.copy()
    d.update(__builtins__={})
    d.pop('sys', None)
    return d


class stream(object):
    def __init__(self, args):
        self.args = args
        self.csv_options = dict(
            full_xpath=False,
            binary=self.args.binary,
            flush=self.args.flush,
            delimiter=self.args.delimiter,
            precision=self.args.precision,
            verbose=self.args.verbose)
        self.initialize_input()
        self.initialize_output()
        if self.args.verbose:
            self.print_info()

    def initialize_input(self):
        fields = self.args.fields
        self.nonblank_input_fields = filter(None, fields.split(','))
        if not self.nonblank_input_fields:
            raise csv_eval_error("specify input stream fields, e.g. --fields=x,y")
        check_fields(self.nonblank_input_fields)
        types = comma.csv.format.to_numpy(self.args.format)
        input_t = comma.csv.struct(fields, *types)
        self.input = comma.csv.stream(input_t, **self.csv_options)

    def initialize_output(self):
        fields = self.args.output_fields
        if not fields:
            self.output = None
            return
        check_fields(fields.split(','), input_fields=self.nonblank_input_fields)
        types = comma.csv.format.to_numpy(self.args.output_format)
        output_t = comma.csv.struct(fields, *types)
        self.output = comma.csv.stream(output_t, tied=self.input, **self.csv_options)

    def print_info(self, file=sys.stderr):
        fields = ','.join(self.input.struct.fields)
        format = self.input.struct.format
        print >> file, "expressions: '{}'".format(self.args.expressions)
        print >> file, "select: '{}'".format(self.args.select)
        print >> file, "input fields: '{}'".format(fields)
        print >> file, "input format: '{}'".format(format)
        if self.output:
            output_fields = ','.join(self.output.struct.fields)
            output_format = self.output.struct.format
            print >> file, "output fields: '{}'".format(output_fields)
            print >> file, "output format: '{}'".format(output_format)


def check_fields(fields, input_fields=None):
    for field in fields:
        if not re.match(r'^[a-z_]\w*$', field, re.I):
            raise csv_eval_error("'{}' is not a valid field name".format(field))
        if field in ['_input', '_output']:
            raise csv_eval_error("'{}' is a reserved name".format(field))
        if field in np.__dict__:
            raise csv_eval_error("'{}' is a reserved numpy name".format(field))
        if input_fields and field in input_fields:
            raise csv_eval_error("'{}' is an input field name".format(field))


def evaluate(expressions, stream, permissive=False):
    input_initializer = ''
    for field in stream.nonblank_input_fields:
        input_initializer += "{field} = _input['{field}']\n".format(field=field)
    output_initializer = ''
    for field in stream.output.struct.fields:
        output_initializer += "_output['{field}'] = {field}\n".format(field=field)
    code_string = input_initializer + '\n' + expressions + '\n' + output_initializer
    code = compile(code_string, '<string>', 'exec')
    env = np.__dict__ if permissive else restricted_numpy_env()
    output = stream.output.struct(stream.input.size)
    is_shutdown = comma.signal.is_shutdown()
    while not is_shutdown:
        i = stream.input.read()
        if i is None:
            break
        if output.size != i.size:
            output = stream.output.struct(i.size)
        exec code in env, {'_input': i, '_output': output}
        stream.output.write(output)


def select(condition, stream):
    code = compile(condition, '<string>', 'eval')
    env = restricted_numpy_env()
    is_shutdown = comma.signal.is_shutdown()
    while not is_shutdown:
        i = stream.input.read()
        if i is None:
            break
        input_initializer = {field: i[field] for field in i.dtype.names}
        mask = eval(code, env, input_initializer)
        stream.input.dump(mask=mask)


def main():
    try:
        args = get_args()
        prepare_options(args)
        if args.select:
            select(args.select, stream(args))
        else:
            evaluate(args.expressions.strip(';'), stream(args), permissive=args.permissive)
    except csv_eval_error as e:
        name = os.path.basename(sys.argv[0])
        print >> sys.stderr, "{} error: {}".format(name, e)
        sys.exit(1)
    except StandardError as e:
        import traceback
        traceback.print_exc(file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()

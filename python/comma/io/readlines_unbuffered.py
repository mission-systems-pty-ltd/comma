import sys


def readlines_unbuffered(size, source=sys.stdin):
    """
    read the given number of lines from source, such as stdin, without buffering
    note: builtin readlines() buffers the input and hence prevents flushing every line
    """
    if size >= 0:
        lines = ''
        number_of_lines = 0
        while number_of_lines < size:
            line = source.readline()
            if not line:
                break
            lines += line
            number_of_lines += 1
    else:
        lines = source.read()
    return lines

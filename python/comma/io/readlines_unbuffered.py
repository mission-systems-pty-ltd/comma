import sys


def readlines_unbuffered(size, source=sys.stdin, skip_blank_lines=True):
    """
    read the given number of lines from source, such as stdin, without buffering
    note: builtin readlines() buffers the input and hence prevents flushing every line

    blank lines are skipped unless skip_blank_lines is False
    (a blank line is a line that has only whitespace characters or no characters)
    """
    if size >= 0:
        lines = ''
        number_of_lines = 0
        while number_of_lines < size:
            line = source.readline()
            if not line:
                break
            if skip_blank_lines and not line.strip():
                continue
            lines += line
            number_of_lines += 1
    else:
        if skip_blank_lines:
            lines = '\n'.join(filter(lambda line: line.strip(), source.read().strip().splitlines()))
        else:
            lines = source.read()
    return lines

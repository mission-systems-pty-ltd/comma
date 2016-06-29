import sys
import itertools


def readlines_unbuffered(size, source=sys.stdin, skip_blank_lines=True):
    """
    read the given number of lines from source, such as stdin, without buffering
    return a list of lines with the newline symbols stripped
    note: builtin readlines() buffers the input and hence prevents flushing every line

    blank lines are skipped unless skip_blank_lines is False
        - a blank line is a line that has only whitespace characters or no characters
        - blank lines are not counted towards size
    """
    if size >= 0:
        lines = []
        number_of_lines = 0
        while number_of_lines < size:
            line = source.readline()
            if not line:
                break
            if skip_blank_lines and not line.strip():
                continue
            lines.append(line.rstrip('\n'))
            number_of_lines += 1
        return lines
    if skip_blank_lines:
        source_ = itertools.ifilter(lambda line: line.strip(), source)
    else:
        source_ = source
    return [line.rstrip('\n') for line in source_]

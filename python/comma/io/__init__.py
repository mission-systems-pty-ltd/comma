from .readlines_unbuffered import readlines_unbuffered

try:
    from . import stream
except ImportError:
    pass

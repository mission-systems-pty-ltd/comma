# Copyright (c) 2011 The University of Sydney

__author__ = 'j.underwood'
'''
Import to enable binary stdout in windows. No effect if imported on another OS.
'''
import sys

# todo check this still allows ascii
# todo if not, provide a function instead
if sys.platform == "win32":
    import os, msvcrt
    msvcrt.setmode(sys.stdout.fileno(), os.O_BINARY)

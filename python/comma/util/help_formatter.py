# This file is part of comma, a generic and flexible library
# Copyright (c) 2011 The University of Sydney
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University of Sydney nor the
#    names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
# GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
# HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import argparse

MAX_HELP_POSITION = 50
BASE_FORMATTER = argparse.RawTextHelpFormatter


class patched_formatter(BASE_FORMATTER):
    def __init__(self, prog):
        super(patched_formatter, self).__init__(prog, max_help_position=MAX_HELP_POSITION)

    def _format_action_invocation(self, action):
        if not action.option_strings or action.nargs == 0:
            return super(patched_formatter, self)._format_action_invocation(action)
        default = action.dest.upper()
        args_string = self._format_args(action, default)
        return ', '.join(action.option_strings) + ' ' + args_string

    def _format_action(self, action):
        return ''.join([' '*4,
                        self._format_action_invocation(action),
                        ': ',
                        self._expand_help(action),
                        '\n'])


def can_be_patched(base_formatter):
    try:
        getattr(base_formatter, '_format_action_invocation')
        getattr(base_formatter, '_format_args')
        getattr(base_formatter, '_format_action')
        getattr(base_formatter, '_expand_help')
        return True
    except AttributeError:
        return False


def argparse_fmt(prog):
    """
    use this funciton as formatter_class in argparse.ArgumentParser
    """
    if can_be_patched(BASE_FORMATTER):
        return patched_formatter(prog)
    else:
        return BASE_FORMATTER(prog, max_help_position=MAX_HELP_POSITION)

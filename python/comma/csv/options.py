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

def add( parser, defaults={} ):
    """
    add csv options to parser
    """
    def help_text( help, default=None ):
        return "%s (default: %s)" % ( help, default ) if default else help

    option_defaults = {
        'fields': '',
        'binary': '',
        'delimiter': ',',
        'precision': 12
    }
    option_defaults.update( defaults )

    parser.add_argument( "--fields", "-f", default=option_defaults["fields"], metavar="<names>",
                         help=help_text( "field names of input stream", option_defaults["fields"] ))

    parser.add_argument( "--binary", "-b", default=option_defaults["binary"], metavar="<format>",
                         help="format for binary stream (default: ascii)" )

    parser.add_argument( "--delimiter", "-d", default=option_defaults["delimiter"], metavar="<char>",
                         help=help_text( "csv delimiter of ascii stream", option_defaults["delimiter"] ))

    parser.add_argument( "--precision", default=option_defaults["precision"], metavar="<precision>",
                         help=help_text( "floating point precision of ascii output", option_defaults["precision"] ))

    parser.add_argument( "--flush", "--unbuffered", action="store_true",
                         help="flush stdout after each record (stream is unbuffered)" )

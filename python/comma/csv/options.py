def standard_csv_options( parser, defaults={} ):

    def help_text( help, default=None ):
        return "%s (default: %s)" % ( help, default ) if default else help

    option_defaults = {
        'fields': '',
        'binary': '',
        'delimiter': ',',
        'precision': 12
    }
    option_defaults.update( defaults )

    parser.add_argument( "-f", "--fields", default=option_defaults["fields"], metavar="<names>",
                         help=help_text( "field names of input stream", option_defaults["fields"] ))

    parser.add_argument( "-b", "--binary", default=option_defaults["binary"], metavar="<format>",
                         help="format for binary stream (default: ascii)" )

    parser.add_argument( "-d", "--delimiter", default=option_defaults["delimiter"], metavar="<char>",
                         help=help_text( "csv delimiter of ascii stream", option_defaults["delimiter"] ))

    parser.add_argument( "--precision", default=option_defaults["precision"], metavar="<precision>",
                         help=help_text( "floating point precision of ascii output", option_defaults["precision"] ))

    parser.add_argument( "--flush", "--unbuffered", action="store_true",
                         help="flush stdout after each record (stream is unbuffered)" )

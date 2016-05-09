import argparse

MAX_HELP_POSITION = 50


class help_formatter(argparse.RawTextHelpFormatter):
    def __init__(self, prog):
        super(help_formatter, self).__init__(prog, max_help_position=MAX_HELP_POSITION)

    def _format_action_invocation(self, action):
        if not action.option_strings or action.nargs == 0:
            return super(help_formatter, self)._format_action_invocation(action)
        default = action.dest.upper()
        args_string = self._format_args(action, default)
        return ', '.join(action.option_strings) + ' ' + args_string


def argparse_fmt(prog):
    try:
        return help_formatter(prog)
    except:
        return argparse.RawTextHelpFormatter(prog, max_help_position=MAX_HELP_POSITION)

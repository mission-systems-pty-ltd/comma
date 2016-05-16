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


def can_be_patched(base_formatter):
    try:
        getattr(base_formatter, '_format_action_invocation')
        getattr(base_formatter, '_format_args')
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

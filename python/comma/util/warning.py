import warnings


class warning(object):
    default_formatwarning = warnings.formatwarning

    def __init__(self, formatwarning):
        self._formatwarning = formatwarning

    def __enter__(self):
        warnings.formatwarning = self._formatwarning
        return warnings.warn

    def __exit__(self, *args):
        warnings.formatwarning = warning.default_formatwarning

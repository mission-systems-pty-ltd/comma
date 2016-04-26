import warnings


class warning(object):
    default_formatwarning = warnings.formatwarning

    @staticmethod
    def formatwarning(msg, *args):
        return __name__ + " warning: " + str(msg) + '\n'

    def __enter__(self):
        warnings.formatwarning = warning.formatwarning
        return warnings.warn

    def __exit__(self, *args):
        warnings.formatwarning = warning.default_formatwarning


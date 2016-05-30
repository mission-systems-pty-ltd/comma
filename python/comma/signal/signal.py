from __future__ import absolute_import
import signal
import sys
import os

MANAGED_SIGNALS = (signal.SIGINT, signal.SIGTERM, signal.SIGHUP)


class is_shutdown(object):
    def __init__(self):
        self.state = False
        for s in MANAGED_SIGNALS:
            signal.signal(s, self.switch_on)

    def switch_on(self, signum, frame):
        print >> sys.stderr, os.path.basename(sys.argv[0]), "caught signal:", signum
        self.state = True

    def __nonzero__(self):
        return self.state

signal.signal(signal.SIGPIPE, signal.SIG_DFL)

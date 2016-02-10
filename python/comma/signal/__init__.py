from __future__ import absolute_import
import signal
import sys

MANAGED_SIGNALS = tuple( [ signal.SIGINT, signal.SIGTERM, signal.SIGHUP ] )

class is_shutdown:
  def __init__( self ):
    self.state = False
    for s in MANAGED_SIGNALS:
      signal.signal( s, self.switch_on )

  def switch_on( self, signum, frame ):
    print >> sys.stderr, "caught signal: ", signum
    self.state = True

  def __nonzero__( self ):
    return self.state

signal.signal( signal.SIGPIPE, signal.SIG_DFL )

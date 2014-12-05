#!/bin/python

def from_dictionary( p, d ):
    for n, v in d.iteritems() : exec( "p." + n + " = " + str( v ) )

def to_dictionary( p, d ):
    for n, v in d.iteritems() : exec( "d[n] = p." + n )

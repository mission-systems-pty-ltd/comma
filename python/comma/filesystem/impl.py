# Copyright (c) 2024 Vsevolod Vlaskine

import os

# same as os.walk, but with followmounts flag
def walk( dir, followlinks=False, followmounts=False, excluded=None ):
    if excluded is None: excluded = []
    real_root = os.path.realpath( dir )
    def _valid( i ): # quick and dirty, excessive and inefficient for now
        p = i[0]
        if os.path.basename( p ) in excluded: return False
        if followmounts: return True
        p = os.path.realpath( i[0] )
        while True:
            if p == '/': return True
            if os.path.ismount( p ): return False
            if p == '': return True
            p = os.path.dirname( p )
    return filter( _valid, os.walk( dir, followlinks=followlinks ) )

def find( what, dirs, find_children=False ):
    if not isinstance( what, list ): what = [ what ]
    def _valid( i ): # quick and dirty, inefficient for now
        p = i[0]
        if find_children:
            while True:
                for q in what:
                    if os.path.exists( f'{p}/{q}' ): return True # if os.path.isfile( f'{p}/{what}' ) or os.path.isfile( f'{p}/test' ): return True
                if p in ['', '/']: return False
                p = os.path.dirname( p )
        for q in what:
            if os.path.exists( f'{p}/{q}' ): return True
        return False
    return filter( _valid, dirs )


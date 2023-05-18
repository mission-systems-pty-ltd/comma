'''
operations on dict and dict-like objects with string keys
made for convenience, not for performance
'''

import copy, functools, typing

def at( d, path, delimiter = '/', no_throw = False, full = False ): # todo: default=...
    '''
    return value at a given path in a dictionary
    
    params
    ------
        d: dictionary
        path: path in dictionary, e.g 'a/b/c/d'
        delimiter: path delimiter
        no_throw: if path not found, return None instead of throwing exception
        full: output dictionary, not just the key value, see example below
    
    examples
    --------
        >>> d = { "a": { "b": { "c": 5, "d": 6 } }, "e": [ 7, [ 8, 9, 10 ] ] }
        >>> comma.dictionary.at( d, "a/b/c" )
        5
        >>> comma.dictionary.at( d, "a/b/c", full = True )
        { "a": { "b": { "c": 5 } } }
        >>> comma.dictionary.at( d, 'e[0]' )
        7
        >>> comma.dictionary.at( d, 'e[1]' )
        [ 8, 9, 10 ]
        >>> comma.dictionary.at( d, 'e[2][1]' )
        10
        >>> comma.dictionary.at( d, 'e[1][1:]' )
        [ 9, 10 ]
        >>> e = [1, 2, {'a': 3} ]
        >>> comma.dictionary.at( e, '[2]/a' )
        3
    '''
    s = path.split( delimiter )
    def _value( d, k ):
        if not k:
            if no_throw: return None
            raise ValueError( f'"{path}" has an empty element; remove initial, trailing, or duplicated delimiters' )
        n = k.split( '[', 1 )
        if len( n ) == 1: return None if no_throw and ( not isinstance( d, dict ) or not k in d ) else d[k]
        if full:
            if no_throw: return None
            raise KeyError( f'on path "{path}": full=True not supported for array indices, since it cannot be done consistently' )
        if no_throw:
            try: return eval( f'd[{n[1]}' if n[0] == '' else f'd[n[0]][{n[1]}', { 'd': d, 'n': n } )
            except: return None
        return eval( f'd[{n[1]}' if n[0] == '' else f'd[n[0]][{n[1]}', {'d': d, 'n': n} )
    r = functools.reduce( lambda d, k: _value( d, k ), s, d )
    return None if r is None else functools.reduce( lambda d, k: { k: d }, [ r ] + s[::-1] ) if full else r

def has( d, path, delimiter = '/' ):
    '''
    return true if element at a given dictionary path exists

    todo: support list indices
    
    examples
    --------
        >>> d = { "a": { "b": { "c": 1, "d": 2, "e": [ 3, 4, { "f": 5 } ] } } }
        >>> comma.dictionary.has( d, "a/b/c" )
        True
        >>> comma.dictionary.has( d, [ "a", "b", "c" ] )
        True
        >>> comma.dictionary.has( d, [ "a", "b", "x" ] )
        False
    '''
    p = path.split( delimiter ) if isinstance( path, str ) else path
    return functools.reduce( lambda d, k: ( d[k[1]] if k[0] + 1 < len( p ) else True ) if isinstance( d, dict ) and k[1] in d else False, enumerate( p ), d )

def set( d, path, value, delimiter = '/' ):
    '''
    assign value to a nested dictionary/list element
    
    examples
    --------
        >>> d = { "a": { "b": 1, "c": [ 2, 3 ], "d": { "e": 4 } } }
        >>> comma.dictionary.set( d, 'a/b/c[1]', 5 )
        todo
    '''
    def _set( d, p ):
        s = p[0].split( '[', 1 )
        if len( p ) == 1:
            if len( s ) == 1: d[p[0]] = value
            else: exec( f'd[{s[1]} = value' if s[0] == '' else f'd["{s[0]}"][{s[1]} = value', { 'd': d, 'value': value } )
        else:
            if len( s ) == 1:
                if not p[0] in d: d[p[0]] = {}
                _set( d[p[0]], p[ 1: ] )
            else:
                if ( len( s ) == 1 or s[0] != '' ) and not s[0] in d: raise KeyError( f'on path {path}: {s[0]} not found' )
                _set( eval( f'd[{s[1]}' if s[0] == '' else f'd["{s[0]}"][{s[1]}', { 'd': d } ), p[1:])
    _set( d, path.split( delimiter ) )
    return d

def leaves( d, path=None ):
    '''
    iterator through the leaf items of a nested dictionary or list, yields path-value pairs

    example
    -------
        >>> list( comma.dictionary.leaves( { "x": { "y": [ { "z": 0 }, {"w": 2 } ], "v": "hello" } } ) )
        [('x/y[0]/z', 0), ('x/y[1]/w', 2), ('x/v', 'hello')]
    '''
    if path is None: path = ''
    if isinstance( d, dict ):
        for key, value in d.items(): yield from leaves( value, f'{path}/{key}' )
    elif isinstance( d, list ):
        for i, value in enumerate(d): yield from leaves( value, f'{path}[{i}]' )
    else:
        yield path[1:] if path and path[0] == '/' else path, d

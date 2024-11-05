import pytest
from comma import dictionary

def test_dictionary_at():
    d = { 'a': { 'b': { 'c': { 'd': 1, 'f': 2 }, 'g': 3 } }, 'p': [ 4, 5, { 'q': 6 } ], 'r': { 's': [ [ 7, 8 ] ] } }
    assert dictionary.at( d, 'a/b/c' ) == { 'd': 1, 'f': 2 }
    assert dictionary.at( d, 'a.b.c', delimiter = '.' ) == { 'd': 1, 'f': 2 }
    assert dictionary.at( d, 'a/b/c/d' ) == 1
    assert dictionary.at( d, 'a/b/g' ) == 3
    assert dictionary.at( d, 'a', full = True ) == { 'a': { 'b': { 'c': { 'd': 1, 'f': 2 }, 'g': 3 } } }
    assert dictionary.at( d, 'a/b', full = True ) == { 'a': { 'b': { 'c': { 'd': 1, 'f': 2 }, 'g': 3 } } }
    assert dictionary.at( d, 'a/b/c', full = True ) == { 'a': { 'b': { 'c': { 'd': 1, 'f': 2 } } } }
    assert dictionary.at( d, 'a/b/c/d', full = True ) == { 'a': { 'b': { 'c': { 'd': 1 } } } }
    assert dictionary.at( d, 'a/b/g', full = True ) == { 'a': { 'b': { 'g': 3 } } }
    with pytest.raises( KeyError ): dictionary.at( d, 'x' )
    with pytest.raises( KeyError ): dictionary.at( d, 'x/y' )
    with pytest.raises( KeyError ): dictionary.at( d, 'a/z' )
    with pytest.raises( KeyError ): dictionary.at( d, 'a/b/c/e' )
    with pytest.raises( TypeError ): dictionary.at( d, 'a/b/c/d/x' )
    assert dictionary.at( d, 'x', no_throw = True ) is None
    assert dictionary.at( d, 'a/x', no_throw = True ) is None
    assert dictionary.at( d, 'a/b/x', no_throw = True ) is None
    assert dictionary.at( d, 'a/b/c/x', no_throw = True ) is None
    assert dictionary.at( d, 'a/b/c/d/x', no_throw = True ) is None
    assert dictionary.at( d, 'p' ) == [ 4, 5, { 'q': 6 } ]
    assert dictionary.at( d, 'p[0]' ) == 4
    assert dictionary.at( d, 'p[1]' ) == 5
    assert dictionary.at( d, 'p[2]' ) == { 'q': 6 }
    assert dictionary.at( d, 'p[2]/q' ) == 6
    assert dictionary.at( d, 'r' ) == { 's': [ [ 7, 8 ] ] }
    assert dictionary.at( d, 'r/s' ) == [ [ 7, 8 ] ]
    assert dictionary.at( d, 'r/s[0][0]' ) == 7
    assert dictionary.at( d, 'r/s[0][1]' ) == 8
    with pytest.raises( IndexError ): dictionary.at( d, 'r/s[0][2]' )
    with pytest.raises( KeyError ): dictionary.at( d, 'r/s[0][2]', full = True )
    assert dictionary.at( d, 'r/s[0][2]', full = True, no_throw = True ) is None
    assert dictionary.at( { 'a': [11,22,33,44] }, 'a[1:3]' ) == [22, 33]
    assert dictionary.at( { 'a': [11,22,33,44] }, 'a[1:]' ) == [22, 33, 44]
    assert dictionary.at([11, 22, 33, 44], '[1:]') == [22, 33, 44]
    assert dictionary.at([11, {'a': 22}, 33, 44], '[1]/a') == 22
    assert dictionary.at({'e': [3, 4, {'g': 6}]}, 'e[1]') == 4
    assert dictionary.at({'e': [3, 4, {'g': 6}]}, 'e[2]') == {'g': 6}
    assert dictionary.at({'e': [3, 4, {'g': 6}]}, 'e[2]/g') == 6
    assert dictionary.at({'a': {}, 'e': [3, 4, {'g': 6}]}, 'e[2]/g') == 6
    assert dictionary.at({'e': [3, 4, {'g': 6}]}, 'e[0]', no_throw=True) == 3
    assert dictionary.at({'e': [3, 4, {'g': 6}]}, 'e[2]', no_throw=True) == {'g': 6}
    assert dictionary.at({'e': [3, 4, {'g': 6}]}, 'e[2]/g', no_throw=True) == 6

def test_dictionary_has():
    d = { 'a': { 'b': { 'c': { 'd': 1 } }, 'e': [ 2, { 'f': 3 } ] } }
    assert dictionary.has( d, 'a/b/c' )
    assert dictionary.has( d, 'a.b.c', delimiter = '.' )
    assert dictionary.has( d, 'a/b/c/d' )
    # assert dictionary.has( d, 'a/b/e' ) # todo: support arrays
    # assert dictionary.has( d, 'a/b/e[0]' ) # todo: support arrays
    # assert dictionary.has( d, 'a/b/e[1]' ) # todo: support arrays
    # assert dictionary.has( d, 'a/b/e[1]/f' ) # todo: support arrays
    assert not dictionary.has( d, 'x' )
    assert not dictionary.has( d, 'x/y' )
    assert not dictionary.has( d, 'a/z' )
    assert not dictionary.has( d, 'a/b/c/e' )
    assert not dictionary.has( d, 'a/b/c/d/x' )
    assert not dictionary.has( d, 'a/b/c/d/x/y' )

def test_dictionary_leaves():
    assert list( dictionary.leaves( { 'a': 1, 'b': 2, 'c': 3 } ) ) == [ ( 'a', 1 ), ( 'b', 2 ), ( 'c', 3 ) ]
    assert list( dictionary.leaves( ['a', 'b', 'c', 'd'] ) ) == [ ('[0]', 'a'), ('[1]', 'b'), ('[2]', 'c'), ('[3]', 'd') ]
    assert list( dictionary.leaves( {'a': {'b': [0, 1]}, 'c': 2} ) ) == [('a/b[0]', 0), ('a/b[1]', 1), ('c', 2)]
    assert list( dictionary.leaves( [ {'a': 0}, {'b': {'c':1, 'd': 2}} ] ) ) == [('[0]/a', 0), ('[1]/b/c', 1), ('[1]/b/d', 2)]
    assert list( dictionary.leaves( { 'a': [ {'b': 0}, {'c': 1} ] } ) ) == [('a[0]/b', 0), ('a[1]/c', 1)]
    assert list( dictionary.leaves( { 'a': { 'b': { 'c': 0 } } } ) ) == [ ( 'a/b/c', 0 ) ]
    assert list( dictionary.leaves( 'a' ) ) == [ ( '', 'a' ) ]
    assert list( dictionary.leaves( {} ) ) == []
    assert list( dictionary.leaves( [] ) ) == []

def test_dictionary_parents():
    assert list( dictionary.parents( { 'a': { 'b': { 'c': {} } } }, 'a/b/c' ) ) == ['a/b', 'a']
    assert list( dictionary.parents( { 'a': { 'b': [ 5, { 'c': 6 } ] } }, 'a/b[1]/c' ) ) == ['a/b[1]', 'a']
    assert list( dictionary.parents( { 'a': { 'b': [ 5, { 'c': 6 } ] } }, 'a/b[1]/c', parent='parent' ) ) == ['a/b[1]', 'a']
    with pytest.raises( KeyError ): dictionary.at( {}, 'a/b/c' )
    assert list( dictionary.parents( { 'a': { 'parent': 'd', 'b': [ 5, { 'c': { 'parent': '/a/b[0]' } } ], 'd': 3 } }, 'a/b[1]/c', parent='parent' ) ) == ['a/b[0]', 'a', 'd']

def test_dictionary_set():
    d = {}
    dictionary.set( d, 'a/b/c', 5 )
    assert dictionary.at( d, 'a/b/c' ) == 5
    dictionary.set( d, 'a/b/c', 10 )
    assert dictionary.at( d, 'a/b/c' ) == 10
    dictionary.set( d, 'a.b.c', 15, delimiter = '.' )
    assert dictionary.at( d, 'a/b/c' ) == 15
    assert dictionary.set( {}, 'a.b.c', 15, delimiter = '.' ) == { 'a': { 'b': { 'c': 15 } } }
    d = dictionary.set( {}, 'a.x.y', 3, delimiter = '.' )
    assert dictionary.set( d, 'a.b.c', 15, delimiter = '.' ) == { 'a': { 'b': { 'c': 15 }, 'x': { 'y': 3 } } }
    assert d == { 'a': { 'b': { 'c': 15 }, 'x': { 'y': 3 } } }
    e = { 'a': [0,[11,22,33],{'b': 4},5,6,7] }
    dictionary.set(e, 'a[0]', 5)
    dictionary.set(e, 'a[3]', 55)
    assert e['a'][0] == 5
    assert e['a'][3] == 55
    dictionary.set(e, 'a[1][1]', 777)
    assert e['a'][1][1] == 777
    dictionary.set(e, 'a[2]/b', 8)
    dictionary.set(e, 'a[3:5]', [88,99])
    assert e['a'][3:5] == [88,99]
    f = [0,1,2,3,{'a': 4}]
    dictionary.set(f, '[2:4]', [22,33])
    assert f[2:4] == [22,33]
    dictionary.set(f, '[4]/a', 44)
    dictionary.set(f, '[4]/b', 55)
    assert f[4] == {'a': 44, 'b': 55 }

def test_dictionary_update():
    assert dictionary.update( {}, {} ) == {}
    assert dictionary.update( { 'a': 1, 'b': 2 }, {} ) == { 'a': 1, 'b': 2 }
    assert dictionary.update( { 'a': 1, 'b': 2 }, { 'c': 3 } ) == { 'a': 1, 'b': 2, 'c': 3 }
    assert dictionary.update( { 'a': 1, 'b': 2 }, { 'c': 3, 'd': { 'e': 4 } } ) == { 'a': 1, 'b': 2, 'c': 3, 'd': { 'e': 4 } }
    assert dictionary.update( {}, { 'c': 3 } ) == { 'c': 3 }
    assert dictionary.update( {}, { 'c': 3, 'd': { 'e': 4 } } ) == { 'c': 3, 'd': { 'e': 4 } }
    assert dictionary.update( { 'a': 1, 'b': 2 }, { 'a': 4, 'c': 3 } ) == { 'a': 4, 'b': 2, 'c': 3 }
    assert dictionary.update( { 'a': 1, 'b': 2, 'd': { 'e': { 'f': 3 }, 'g': { 'h': 4 } } }, { 'd': { 'e': { 'f': 5 }, 'g': { 'h': 4 } } } ) == { 'a': 1, 'b': 2, 'd': { 'e': { 'f': 5 }, 'g': { 'h': 4 } } }
    assert dictionary.update( [], [] ) == []
    assert dictionary.update( [1, 2], [] ) == [1, 2]
    assert dictionary.update( [1, 2], [3, 4, 5] ) == [3, 4, 5]
    assert dictionary.update( [1, { 'a': 1, 'b': { 'c': 2 } }], [1, {}, 5] ) == [1, { 'a': 1, 'b': { 'c': 2 } }, 5]
    assert dictionary.update( [1, { 'a': 1, 'b': { 'c': 2 } }], [1, { 'a': 4 }, 5] ) == [1, { 'a': 4, 'b': { 'c': 2 } }, 5]
    assert dictionary.update( [1, { 'a': 1, 'b': { 'c': 2 } }], [1, { 'a': 4, 'b': { 'c': 6, 'd': 7 } }, 5] ) == [1, { 'a': 4, 'b': { 'c': 6, 'd': 7 } }, 5]
    assert dictionary.update( [1, 2, [3, 4, 5]], [] ) == [1, 2, [3, 4, 5]]
    assert dictionary.update( [], [1, 2, [3, 4, 5]] ) == [1, 2, [3, 4, 5]]
    assert dictionary.update( [1, 2, [3, 4, 5]], [1, 2, [3, 4, 6, 7]] ) == [1, 2, [3, 4, 6, 7]]

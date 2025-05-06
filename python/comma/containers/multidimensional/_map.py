import ctypes, ctypes.util, numpy

class Map:
    def __init__( self, resolution, origin=None, values=None ):
        self._dtype = resolution.dtype
        # todo: if resolution is not numpy array, try origin; if origin is not numpy array, try values
        self._bindings = ctypes.CDLL( ctypes.util.find_library( 'comma_python_bindings' ) )
        self._create = self._bindings.comma_containers_multidimensional_map_create
        self._create.argtypes = [ ctypes.c_int, ctypes.c_uint, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint ]
        self._create.restype = ctypes.c_void_p
        self._at = self._bindings.comma_containers_multidimensional_map_at
        self._at.argtypes = [ ctypes.c_void_p, ctypes.c_void_p ]
        self._at.restype = ctypes.c_void_p
        self._count = self._bindings.comma_containers_multidimensional_map_count
        self._count.argtypes = [ ctypes.c_void_p ]
        self._count.restype = ctypes.c_uint
        self._size = self._bindings.comma_containers_multidimensional_map_size
        self._size.argtypes = [ ctypes.c_void_p ]
        self._size.restype = ctypes.c_uint
        key_types = { numpy.dtype( 'int32' ): 0, numpy.dtype( 'int64' ): 1, numpy.dtype( 'float32' ): 2, numpy.dtype( 'float64' ): 3 }
        if origin is None: origin = numpy.zeros( resolution.shape[0] )
        assert self._dtype in key_types, TypeError( f'expected key type in {list(key_types.keys())}; got: {self._dtype}' )
        self._map = self._create( key_types[self._dtype]
                        , resolution.shape[0]
                        , numpy.array( origin, dtype=self._dtype ).ctypes.data_as( ctypes.c_void_p )
                        , numpy.array( resolution, dtype=self._dtype ).ctypes.data_as( ctypes.c_void_p )
                        , numpy.array( [] if values is None else values, dtype=self._dtype ).ctypes.data_as( ctypes.c_void_p )
                        , 0 if values is None else len( values ) )
        
    def __enter__( self ): return self

    def __exit__( self, type, value, traceback ): self._bindings.comma_containers_multidimensional_map_destroy( self._map )

    def at( self, points ):
        # todo: call method
        # todo: 'cast' pointer to int
        # todo: make numpy array
        ...

    def count( self ): return self._count( self._map )
    
    def size( self ): return self._size( self._map )

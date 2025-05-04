import ctypes, ctypes.util, numpy

class Map:
    def __init__( self, key_type, dim, resolution, origin=None, values=None ):
        self._bindings = ctypes.CDLL( ctypes.util.find_library( 'comma_python_bindings' ) )
        self._create = self._bindings.comma_containers_multidimensional_map_create
        self._create.argtypes = [ ctypes.c_in, ctypes.c_uint, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint ]
        self._create.restype = None # todo? make callable?
        key_types = { numpy.int32: 0, numpy.int64: 1, numpy.float32: 2, numpy.float64: 3 }
        if origin is None: origin = numpy.zero( dim )
        assert key_type in key_types, TypeError( f'expected key type in {list(key_types.keys())}; got: {key_type}' )
        self._map = self._create( key_types[key_type]
                        , dim
                        , numpy.array( origin, dtype=key_type ).ctypes.data_as( ctypes.c_void_p )
                        , numpy.array( resolution, dtype=key_type ).ctypes.data_as( ctypes.c_void_p )
                        , values if isinstance( values, numpy.array ) else numpy.array( [] if values is None else values ).ctypes.data_as( ctypes.c_void_p )
                        , 0 if value is None else len( values ) )
        
        def __enter__( self ): return self

        def __exit__( self, type, value, traceback ):
            self._bindings.comma_containers_multidimensional_map_destroy( self._map )

        # todo: at()

        # todo: size()
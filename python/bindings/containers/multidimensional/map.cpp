#include <cstdint>
#include <vector>
#include "../../../../base/exception.h"
#include "../../../../containers/multidimensional/map.h"
#include "map.h"

namespace comma { namespace python { namespace bindings { namespace containers { namespace multidimensional { namespace map { namespace impl {

class base
{
    public:
        base( void* m ): _m( m ) {}
        virtual ~base() = default;
        virtual void insert( const void* p, int v ) = 0;
        virtual const int* at( const void* p, unsigned int* size ) const = 0;
        virtual unsigned int size( const void* p ) const = 0;
    protected:
        void* _m{nullptr};
};

template < typename K, unsigned int Dim >
struct proxy: public base
{
    typedef comma::containers::multidimensional::map< K, std::vector< int >, Dim > map_t;

    typedef typename map_t::point_type key_t;

    proxy( const void* o, const void* r, const void* p, int size )
        : base( new map_t( *reinterpret_cast< const key_t* >( o ), *reinterpret_cast< const key_t* >( r ) ) )
    {
        if( !p || size == 0 ) { return; }
        const K* q = reinterpret_cast< const K* >( p );
        for( int i = 0; i < size; ++i, q += sizeof( K ) * Dim * size ) { insert( q, i ); }
    }

    ~proxy() { if( _m ) { delete reinterpret_cast< map_t* >( _m ); } }

    void insert( const void* k, int v ) { map().touch_at( key( k ) )->second.push_back( v ); }

    const int* at( const void* k, unsigned int* size ) const { const auto& i = map().at( key( k ) ); *size = i == map().end() ? 0 : int( i->second.size() ); return i == map().end() || i->second.empty() ? nullptr : &i->second[0]; }
    
    unsigned int size( const void* p ) const { return map().size(); }

    map_t& map() { return *reinterpret_cast< map_t* >( _m ); }

    const map_t& map() const { return *reinterpret_cast< const map_t* >( _m ); }

    const key_t& key( const void* p ) const { return *reinterpret_cast< const key_t* >( p ); }
};

void* make( int key_type, unsigned int dim, const void* o, const void* r, const void* v, unsigned int s )
{
    switch( key_type )
    {
        case _comma_int32:
            switch( dim )
            {
                case 2: return new proxy< std::int32_t, 2 >( o, r, v, s );
                case 3: return new proxy< std::int32_t, 3 >( o, r, v, s );
                case 4: return new proxy< std::int32_t, 4 >( o, r, v, s );
                case 5: return new proxy< std::int32_t, 5 >( o, r, v, s );
                case 6: return new proxy< std::int32_t, 6 >( o, r, v, s );
                default: COMMA_THROW_BRIEF( comma::exception, "multidimensional map with int32 keys supports 2 to 6 dimensions; got: " << dim << "; just ask for more" );
            }
            break;
        case _comma_int64:
            switch( dim )
            {
                case 2: return new proxy< std::int64_t, 2 >( o, r, v, s );
                case 3: return new proxy< std::int64_t, 3 >( o, r, v, s );
                case 4: return new proxy< std::int64_t, 4 >( o, r, v, s );
                case 5: return new proxy< std::int64_t, 5 >( o, r, v, s );
                case 6: return new proxy< std::int64_t, 6 >( o, r, v, s );
                default: COMMA_THROW_BRIEF( comma::exception, "multidimensional map with int64 keys supports 2 to 6 dimensions; got: " << dim << "; just ask for more" );
            }
            break;
        case _comma_float32:
            static_assert( sizeof( float ) == 4 );
            switch( dim )
            {
                case 2: return new proxy< float, 2 >( o, r, v, s );
                case 3: return new proxy< float, 3 >( o, r, v, s );
                case 4: return new proxy< float, 4 >( o, r, v, s );
                case 5: return new proxy< float, 5 >( o, r, v, s );
                case 6: return new proxy< float, 6 >( o, r, v, s );
                default: COMMA_THROW_BRIEF( comma::exception, "multidimensional map with float32 keys supports 2 to 6 dimensions; got: " << dim << "; just ask for more" );
            }
            break;
        case _comma_float64:
            static_assert( sizeof( double ) == 8 );
            switch( dim )
            {
                case 2: return new proxy< double, 2 >( o, r, v, s );
                case 3: return new proxy< double, 3 >( o, r, v, s );
                case 4: return new proxy< double, 4 >( o, r, v, s );
                case 5: return new proxy< double, 5 >( o, r, v, s );
                case 6: return new proxy< double, 6 >( o, r, v, s );
                default: COMMA_THROW_BRIEF( comma::exception, "multidimensional map with double keys supports 2 to 6 dimensions; got: " << dim << "; just ask for more" );
            }
            break;
        default:
            COMMA_THROW_BRIEF( comma::exception, "multidimensional map supports types int32 (0), int64 (1), float32 (2), and float64 (3); got: " << key_type );
    }
    return nullptr;
}

} } } } } } } // namespace comma { namespace python { namespace bindings { namespace containers { namespace multidimensional { namespace map { namespace impl {

static auto as_base( void* p ) { return reinterpret_cast< comma::python::bindings::containers::multidimensional::map::impl::base* >( p ); }

static auto as_base( const void* p ) { return reinterpret_cast< const comma::python::bindings::containers::multidimensional::map::impl::base* >( p ); }

DLL_EXPORT void* comma_containers_multidimensional_map_create( int key_type, unsigned int dim, const void* origin, const void* resolution, const void* values, unsigned int size )
{
    return comma::python::bindings::containers::multidimensional::map::impl::make( key_type, dim, origin, resolution, values, size );
}

DLL_EXPORT void comma_containers_multidimensional_map_destroy( void* p ) { delete as_base( p ); }

DLL_EXPORT const void* comma_containers_multidimensional_map_at( const void* p, void* size ) { return as_base( p )->at( p, reinterpret_cast< unsigned int* >( size ) ); }

DLL_EXPORT unsigned int comma_containers_multidimensional_map_size( const void* p ) { return as_base( p )->size( p ); }


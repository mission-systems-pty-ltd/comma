#include <iostream>

#include <cstdint>
#include <limits>
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
        virtual const int* nearest( const void* k, unsigned int neighbourhood ) const = 0;
        virtual unsigned int count() const = 0;
        virtual unsigned int size() const = 0;
    protected:
        void* _m{nullptr};
};

template < unsigned int D > struct _traits
{ 
    static unsigned int power( unsigned int b ) { return _traits< D - 1 >::power( b ) * b; }
    static std::array< int, D > index( unsigned int v, int b, unsigned int i = D )
    {
        std::array< int, D > a;
        for( int i = D - 1; i >= 0; a[i--] = v % b - b / 2, v /= b );
        return a;
    } 
};

template <> struct _traits< 0 > { static unsigned int power( unsigned int ) { return 1; } };

template < typename K, unsigned int Dim >
struct proxy: public base
{
    typedef std::array< K, Dim > key_t;
    
    typedef comma::containers::multidimensional::map< K, std::pair< std::vector< int >, std::vector< key_t > >, Dim > map_t;

    proxy( const void* o, const void* r, const void* p, int size )
        : base( new map_t( *reinterpret_cast< const key_t* >( o ), *reinterpret_cast< const key_t* >( r ) ) )
    {
        if( !p || size == 0 ) { return; }
        const K* q = reinterpret_cast< const K* >( p );
        for( int i = 0; i < size; ++i, q += Dim ) { insert( q, i ); }
    }

    ~proxy() { if( _m ) { delete reinterpret_cast< map_t* >( _m ); } }

    void insert( const void* k, int v )
    {
        auto i = map().touch_at( key( k ) );
        i->second.first.push_back( v );
        i->second.second.push_back( key( k ) );
    }

    const int* at( const void* k, unsigned int* size ) const
    {
        auto i = map().at( key( k ) );
        *size = i == map().end() ? 0 : int( i->second.first.size() );
        return *size == 0 ? nullptr : &i->second.first[0];
    }

    // todo: up to a given number of nearest points
    // todo: all points in radius
    // todo: multple input points
    const int* nearest( const void* k, unsigned int neighbourhood ) const
    {
        auto i = map().index_of( key( k ) );
        int b = neighbourhood * 2 + 1;
        double s = std::numeric_limits< double >::max();
        const int* si{nullptr};
        for( unsigned int p = 0; p < _traits< Dim >::power( b ); ++p )
        {
            typename map_t::index_type j = _traits< Dim >::index( p, b );
            for( unsigned int m = 0; m < Dim; ++m ) { j[m] += i[m]; }
            auto n = map().find( j );
            if( n == map().end() ) { continue; }
            for( unsigned int q = 0; q < n->second.first.size(); ++q )
            {
                double t = 0;
                for( unsigned int m = 0; m < Dim; ++m )
                { 
                    double d = key( k )[m] - n->second.second[q][m]; 
                    t += d * d;
                }
                if( t < s ) { s = t; si = &n->second.first[q]; }
            }
        }
        return si;
    }
    
    unsigned int size() const { return map().size(); }

    unsigned int count() const { unsigned int c{0}; for( auto i: map() ) { c += i.second.first.size(); } return c; }

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

DLL_EXPORT const void* comma_containers_multidimensional_map_at( const void* p, const void* k, void* size ) { return as_base( p )->at( k, reinterpret_cast< unsigned int* >( size ) ); }

DLL_EXPORT unsigned int comma_containers_multidimensional_map_size( const void* p ) { return as_base( p )->size(); }

DLL_EXPORT unsigned int comma_containers_multidimensional_map_count( const void* p ) { return as_base( p )->count(); }

DLL_EXPORT const void* comma_containers_multidimensional_map_nearest( const void* p, const void* k, unsigned int n ) { return as_base( p )->nearest( k, n ); }
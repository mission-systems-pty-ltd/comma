#include <queue>
#include <tuple>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace comma { namespace containers { namespace synchronized {

namespace impl {

template < typename K > struct traits { typedef K diff_type; };
template <> struct traits< boost::posix_time::ptime > { typedef boost::posix_time::time_duration diff_type; };

}

template < typename K, typename T, typename S >
class multiqueue
{
    public:
        std::tuple< std::queue< std::pair< K, T > >, std::queue< std::pair< K, S > > > queues;

        multiqueue( typename impl::traits< K >::diff_type max_diff ): _max_diff( max_diff ) {}
        bool ready() const;
        void purge();

    private:
        typename impl::traits< K >::diff_type _max_diff;
        static typename impl::traits< K >::diff_type _abs_diff(K lhs, K rhs) { return lhs < rhs ? (rhs - lhs) : (lhs - rhs); }
};

template < typename K, typename T, typename S >
inline bool multiqueue<K, T, S>::ready() const
{
    if( std::get<0>(queues).empty() || std::get<1>(queues).empty() ) { return false; }
    return _abs_diff( std::get<1>(queues).front().first, std::get<0>(queues).front().first ) <= _max_diff;
}

template < typename K, typename T, typename S >
inline void multiqueue<K, T, S>::purge()
{
    if( std::get<1>(queues).empty() || std::get<0>(queues).empty() ) { return; }
    while( std::get<0>(queues).front().first - std::get<1>(queues).front().first > _max_diff ) 
    { 
        if( std::get<1>(queues).empty() ) { return; }
        std::get<1>(queues).pop(); 
    }
    while( std::get<1>(queues).front().first - std::get<0>(queues).front().first > _max_diff ) 
    { 
        if( std::get<0>(queues).empty() ) { return; }
        std::get<0>(queues).pop(); 
    }
}

} } } // namespace comma { namespace containers { namespace synchronized {

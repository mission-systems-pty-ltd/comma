#include <comma/csv/stream.h>

namespace comma { namespace csv { namespace detail {

bool unsyncronize_with_stdio_impl()
{
    bool b = std::ios_base::sync_with_stdio( false ); // std::cin, std::cout access are thread-unsafe now (safe by default)
    std::cin.tie( NULL ); // std::cin is tied to std::cout by default, which is thread-unsafe now
    return b;
}

static const bool unsyncronize_with_stdio_dummy = unsyncronize_with_stdio_impl();

void unsyncronize_with_stdio()
{
    static bool dummy = unsyncronize_with_stdio_dummy;
    ( void )( dummy ); // necessary, otherwise linker would not link to dummy symbol
}

} } } // namespace comma { namespace csv { namespace detail {

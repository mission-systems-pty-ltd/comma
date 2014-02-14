#include <comma/csv/stream.h>

namespace comma { namespace csv { namespace detail {

static const bool unsyncronize_with_stdio_dummy = std::ios_base::sync_with_stdio( false );

void unsyncronize_with_stdio() { static bool dummy = unsyncronize_with_stdio_dummy; ( void )( dummy ); } // necessary, otherwise linker would not link to dummy symbol

} } } // namespace comma { namespace csv { namespace detail {

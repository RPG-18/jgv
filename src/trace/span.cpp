#include "span.h"

namespace trace {
bool Span::isEmpty() const noexcept
{
    return traceID.isEmpty() || spanID.isEmpty();
}

} // namespace trace
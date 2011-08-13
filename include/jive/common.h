#ifndef JIVE_COMMON_H
#define JIVE_COMMON_H

#include <assert.h>

#define JIVE_DEBUG_ASSERT(x) assert(x)

#ifndef JIVE_EXPORTED_INLINE
# define JIVE_EXPORTED_INLINE static inline
#endif

#endif

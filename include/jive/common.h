/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_COMMON_H
#define JIVE_COMMON_H

#include <assert.h>

#define JIVE_DEBUG_ASSERT(x) assert(x)

#ifndef JIVE_EXPORTED_INLINE
# define JIVE_EXPORTED_INLINE static inline
#endif

#endif

/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_COMMON_H
#define JIVE_COMMON_H

#include <assert.h>

#define JIVE_ASSERT(x) assert(x)

#ifdef JIVE_DEBUG 
#	define JIVE_DEBUG_ASSERT(x) assert(x)
#else
#	define JIVE_DEBUG_ASSERT(x) (void)(x)
#endif

#ifndef JIVE_EXPORTED_INLINE
# define JIVE_EXPORTED_INLINE static inline
#endif

#endif

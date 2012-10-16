/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_INTERNAL_COMPILER_H
#define JIVE_INTERNAL_COMPILER_H

#if defined(__GNUC__)
 #undef likely
 #undef unlikely
 #if (__GNUC__>=4)
  #define likely(x) __builtin_expect((x), 1)
  #define unlikely(x) __builtin_expect((x), 0)
 #endif
#endif

#ifndef likely
 #define likely(x) (x)
#endif
#ifndef unlikely
 #define unlikely(x) (x)
#endif


#endif

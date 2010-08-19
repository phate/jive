#ifndef JIVE_ARCH_STACKFRAME_PRIVATE_H
#define JIVE_ARCH_STACKFRAME_PRIVATE_H

#include <jive/arch/stackframe.h>
#include <jive/vsdg/region.h>
#include <jive/debug-private.h>

static inline void
_jive_stackframe_init(jive_stackframe * self, jive_region * region, struct jive_output * stackptr)
{
	DEBUG_ASSERT(region->stackframe == 0);
	self->region = region;
	self->stackptr = stackptr;
	region->stackframe = self;
}

void
_jive_stackframe_fini(jive_stackframe * self);

#endif

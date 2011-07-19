#ifndef JIVE_ARCH_STACKFRAME_PRIVATE_H
#define JIVE_ARCH_STACKFRAME_PRIVATE_H

#include <jive/common.h>

#include <jive/arch/stackframe.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>

static inline void
_jive_stackframe_init(jive_stackframe * self, jive_region * region, struct jive_output * stackptr)
{
	JIVE_DEBUG_ASSERT(region->stackframe == 0);
	self->context = region->graph->context;
	self->region = region;
	self->stackptr = stackptr;
	self->slots.first = self->slots.last = 0;
	self->stackslot_size_classes.first = self->stackslot_size_classes.last = 0;
	self->reserved_stackslot_classes.first = self->reserved_stackslot_classes.last = 0;
	region->stackframe = self;
}

void
_jive_stackframe_fini(jive_stackframe * self);

#endif

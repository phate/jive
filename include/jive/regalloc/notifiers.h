#ifndef JIVE_REGALLOC_NOTIFIERS_H
#define JIVE_REGALLOC_NOTIFIERS_H

#include <jive/common.h>

#include <jive/vsdg/notifiers.h>

struct jive_shaped_region;
struct jive_shaped_ssavar;

/* region/ssavar notifiers */

typedef void (*jive_shaped_region_ssavar_notifier_function)(void * closure, struct jive_shaped_region * shaped_region, struct jive_shaped_ssavar * shaped_ssavar);
typedef struct jive_shaped_region_ssavar_notifier jive_shaped_region_ssavar_notifier;
typedef struct jive_shaped_region_ssavar_notifier_slot jive_shaped_region_ssavar_notifier_slot;

struct jive_shaped_region_ssavar_notifier_slot {
	struct {
		jive_shaped_region_ssavar_notifier * first;
		jive_shaped_region_ssavar_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_shaped_region_ssavar_notifier_slot_init(jive_shaped_region_ssavar_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_shaped_region_ssavar_notifier_slot_fini(jive_shaped_region_ssavar_notifier_slot * self);

jive_notifier *
jive_shaped_region_ssavar_notifier_slot_connect(jive_shaped_region_ssavar_notifier_slot * self, jive_shaped_region_ssavar_notifier_function function, void * closure);

void
jive_shaped_region_ssavar_notifier_slot_call(const jive_shaped_region_ssavar_notifier_slot * self, struct jive_shaped_region * shaped_region, struct jive_shaped_ssavar * shaped_ssavar);

#endif

#ifndef JIVE_REGCLS_COUNT_PRIVATE_H
#define JIVE_REGCLS_COUNT_PRIVATE_H

#include <jive/vsdg/regcls-count.h>

void
jive_regcls_count_clear(jive_regcls_count * self, jive_context * context);

static inline void
jive_regcls_count_init(jive_regcls_count * self)
{
	self->nitems = self->nbuckets = 0;
	self->buckets = 0;
}

static inline void
jive_regcls_count_fini(jive_regcls_count * self, jive_context * context)
{
	if (self->buckets) {
		jive_regcls_count_clear(self, context);
		jive_context_free(context, self->buckets);
	}
}

const struct jive_regcls *
jive_regcls_count_add(jive_regcls_count * self, jive_context * context, const struct jive_regcls * regcls);

void
jive_regcls_count_sub(jive_regcls_count * self, jive_context * context, const struct jive_regcls * regcls);

const struct jive_regcls *
jive_regcls_count_change(jive_regcls_count * self, jive_context * context, const struct jive_regcls * old_regcls, const struct jive_regcls * new_regcls);

const struct jive_regcls *
jive_regcls_count_check_add(const jive_regcls_count * self, const struct jive_regcls * regcls);

const struct jive_regcls *
jive_regcls_count_check_change(const jive_regcls_count * self, const struct jive_regcls * old_regcls, const struct jive_regcls * new_regcls);

void
jive_regcls_count_copy(jive_regcls_count * self, jive_context * context, const jive_regcls_count * src);

/* TODO: iterator support */

#endif

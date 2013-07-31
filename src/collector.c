/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/collector.h>
#include <jive/context.h>
#include <jive/util/list.h>

typedef struct jive_collector_entry jive_collector_entry;
struct jive_collector_entry {
	void * ptr;
	void (*fini)(void * ptr);

	struct {
		struct jive_collector_entry * prev;
		struct jive_collector_entry * next;
	} collector_entry_list;
};

struct jive_collector {
	struct jive_context * context;

	struct {
		struct jive_collector_entry * first;
		struct jive_collector_entry * last;
	} collector_entries;
};

static void
jive_collector_init_(struct jive_collector * self, struct jive_context * context)
{
	self->context = context;
	self->collector_entries.first = 0;
	self->collector_entries.last = 0;
}

struct jive_collector *
jive_collector_create(struct jive_context * context)
{
	jive_collector * collector = jive_context_malloc(context, sizeof(*collector));
	jive_collector_init_(collector, context);
	return collector;
}

void
jive_collector_register(struct jive_collector * self, void * ptr, void * fini)
{
	jive_collector_entry * entry = jive_context_malloc(self->context, sizeof(*entry));
	entry->ptr = ptr;
	entry->fini = fini;
	JIVE_LIST_PUSH_BACK(self->collector_entries, entry, collector_entry_list);
}

void
jive_collector_reclaim(struct jive_collector * self)
{
	while (self->collector_entries.first) {
		jive_collector_entry * entry = self->collector_entries.first;
		JIVE_LIST_REMOVE(self->collector_entries, entry, collector_entry_list);
		if (entry->fini != NULL)
			entry->fini(entry->ptr);
		jive_context_free(self->context, entry->ptr);
		jive_context_free(self->context, entry);
	}
}

void
jive_collector_destroy(struct jive_collector * self)
{
	jive_collector_reclaim(self);
	jive_context_free(self->context, self);
}

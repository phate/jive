/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/basetype.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/variable.h>

#include <stdio.h>

void
jive_resource_class_count_dump(const jive_resource_class_count * count)
{
	jive_resource_class_count_item * item;
	JIVE_LIST_ITERATE(count->items, item, item_list) {
		fprintf(stderr, "%s:%zd ", item->resource_class->name, item->count);
	}
	fprintf(stderr, "\n");
}

void
jive_variable_dump(const jive_variable * self)
{
	jive_ssavar * ssavar;
	fprintf(stderr, "var %p: ", self);
	JIVE_LIST_ITERATE(self->ssavars, ssavar, variable_ssavar_list)
		fprintf(stderr, "%p:%zd ", ssavar->origin->node, ssavar->origin->index);
	fprintf(stderr, " use_count=%zd\n", self->use_count);
}

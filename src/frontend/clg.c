/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/context.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>
#include <jive/util/buffer.h>
#include <jive/util/list.h>

#include <stdio.h>

static void
jive_clg_init_(struct jive_clg * self, struct jive_context * context)
{
	self->context = context;
	self->nodes.first = 0;
	self->nodes.last = 0;
}

static void
jive_clg_fini_(struct jive_clg * self)
{
	while (self->nodes.first)
		jive_clg_node_destroy(self->nodes.first);
}

struct jive_clg *
jive_clg_create(struct jive_context * context)
{
	jive_clg * clg = jive_context_malloc(context, sizeof(*clg));
	jive_clg_init_(clg, context);
	return clg;
}

void
jive_clg_convert_dot(const struct jive_clg * self, struct jive_buffer * buffer)
{
	jive_buffer_putstr(buffer, "digraph clg {\n");

	char tmp[96];
	jive_clg_node * node;
	JIVE_LIST_ITERATE(self->nodes, node, clg_node_list) {
		snprintf(tmp, sizeof(tmp), "%zu", (size_t)node);
		jive_buffer_putstr(buffer, tmp);
		jive_buffer_putstr(buffer, "[label = \"");
		jive_clg_node_get_label(node, buffer);
		jive_buffer_putstr(buffer, "\"];\n");

		size_t n;
		for (n = 0; n < node->ncalls; n++) {
			snprintf(tmp, sizeof(tmp), "%zu -> %zu;\n", (size_t)node, (size_t)node->calls[n]);
			jive_buffer_putstr(buffer, tmp);
		}
	}

	jive_buffer_putstr(buffer, "}\n");
}

void
jive_clg_view(const struct jive_clg * self)
{
	jive_buffer buffer;
	jive_buffer_init(&buffer, self->context);

	FILE * file = popen("tee /tmp/clg.dot | dot -Tps > /tmp/clg.ps ; gv /tmp/clg.ps", "w");
	jive_clg_convert_dot(self, &buffer);
	fwrite(buffer.data, buffer.size, 1, file);
	pclose(file);

	jive_buffer_fini(&buffer);
}

void
jive_clg_destroy(struct jive_clg * self)
{
	jive_clg_fini_(self);
	jive_context_free(self->context, self);
}

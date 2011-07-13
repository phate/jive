#ifndef JIVE_VSDG_UNION_LAYOUT_H
#define JIVE_VSDG_UNION_LAYOUT_H

#include <jive/vsdg/valuetype.h>

typedef struct jive_union_layout jive_union_layout;
typedef struct jive_value_type jive_union_layout_element;

struct jive_union_layout {
	struct jive_context * context;
	size_t nelements;
	const jive_union_layout_element ** element;
	size_t alignment;
	size_t total_size;
};

jive_union_layout *
jive_union_layout_create(struct jive_context * context,
	size_t nelements, const jive_union_layout_element * element[],
	size_t alignment, size_t total_size);

void
jive_union_layout_init(
	jive_union_layout * layout,
	struct jive_context * context,
	size_t nelements, const jive_union_layout_element * element[],
	size_t alignment, size_t total_size);

void
jive_union_layout_fini(jive_union_layout * layout);

void
jive_union_layout_destroy(jive_union_layout * layout);

#endif

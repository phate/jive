#ifndef JIVE_VSDG_RECORD_LAYOUT_H
#define JIVE_VSDG_RECORD_LAYOUT_H

#include <jive/vsdg/valuetype.h>

typedef struct jive_record_layout jive_record_layout;
typedef struct jive_record_layout_element jive_record_layout_element;

struct jive_record_layout_element {
	const jive_value_type * type;
	size_t offset;
};

struct jive_record_layout {
	struct jive_context * context;
	size_t nelements;
	jive_record_layout_element * element;
	size_t alignment;
	size_t total_size;
};

jive_record_layout *
jive_record_layout_create(struct jive_context * context,
	size_t nelements, const jive_record_layout_element * element,
	size_t alignment, size_t total_size); 

void
jive_record_layout_init(
	jive_record_layout * layout,
	struct jive_context * context,
	size_t nelements, const jive_record_layout_element * element,
	size_t alignment, size_t total_size);

void
jive_record_layout_fini(jive_record_layout * layout);

void
jive_record_layout_destroy(jive_record_layout * layout);

#endif

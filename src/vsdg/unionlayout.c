#include <jive/vsdg/unionlayout.h>
#include <jive/context.h>

jive_union_layout *
jive_union_layout_create(struct jive_context * context,
	size_t nelements, const jive_union_layout_element * element[],
	size_t alignment, size_t total_size)
{
	jive_union_layout * layout = jive_context_malloc(context, sizeof(*layout));

	jive_union_layout_init(layout, context,
		nelements, element,
		alignment, total_size);

	return layout;
}

void
jive_union_layout_init(jive_union_layout * layout, struct jive_context * context,
	size_t nelements, const jive_union_layout_element * element[],
	size_t alignment, size_t total_size)
{
	layout->element = jive_context_malloc(context, sizeof(*(layout->element)) * nelements);

	size_t i;
	for(i = 0; i < nelements; i++){
		layout->element[i] = (const jive_value_type *)
			jive_type_copy(&element[i]->base, context);
	}

	layout->context = context;
	layout->nelements = nelements;
	layout->alignment = alignment;
	layout->total_size = total_size;
}

void
jive_union_layout_fini(jive_union_layout * layout)
{
	size_t i;
	for(i = 0; i < layout->nelements; i++){
		jive_type_fini((jive_type *)&layout->element[i]->base);
		jive_context_free(layout->context, (jive_type*)layout->element[i]);
	}
	jive_context_free(layout->context, layout->element); 
}

void
jive_union_layout_destroy(jive_union_layout * layout)
{
	jive_union_layout_fini(layout);
	jive_context_free(layout->context, layout);
}

#include <jive/context.h>
#include <jive/vsdg/recordlayout.h>

jive_record_layout *
jive_record_layout_create(struct jive_context * context,
	size_t nelements, const jive_record_layout_element * element,
	size_t alignment, size_t total_size)
{
	jive_record_layout * layout = jive_context_malloc(context, sizeof(*layout));
	
	jive_record_layout_init(layout, context,
		nelements, element,
		alignment, total_size);

	return layout;
}

void
jive_record_layout_init(jive_record_layout * layout, struct jive_context * context,
	size_t nelements, const jive_record_layout_element * element,
	size_t alignment, size_t total_size)
{
	layout->element = jive_context_malloc(context, sizeof(*(layout->element)) * nelements);

	size_t i;
	for(i = 0; i < nelements; i++){
		layout->element[i].offset = element[i].offset;
		layout->element[i].type = (const jive_value_type *)
			jive_type_copy(&element[i].type->base, context);
	}

	layout->context = context;
	layout->nelements = nelements;
	layout->alignment = alignment; 
	layout->total_size = total_size;
}

void
jive_record_layout_fini(jive_record_layout * layout)
{
	size_t i;
	for(i = 0; i < layout->nelements; i++){
		jive_type_fini((jive_type *)&layout->element[i].type->base);
		jive_context_free(layout->context, (jive_type*)layout->element[i].type);
	}
	jive_context_free(layout->context, layout->element);
}

void
jive_record_layout_destroy(jive_record_layout * layout)
{
	jive_record_layout_fini(layout);
	jive_context_free(layout->context, layout) ;
}

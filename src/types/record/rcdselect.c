#include <jive/types/record/rcdselect.h>

#include <stdio.h>
#include <string.h>

#include <jive/vsdg/node-private.h>
#include <jive/types/record/rcdtype.h>

static char *
jive_select_node_get_label_(const jive_node * self);

static const jive_node_attrs *
jive_select_node_get_attrs_(const jive_node * self);

static jive_node *
jive_select_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static bool
jive_select_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_unop_reduction_path_t
jive_select_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs,
	const jive_output * operand);

static jive_output *
jive_select_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output * operand);

const jive_unary_operation_class JIVE_SELECT_NODE_ = {
	.base = { /* jive_node_class */
		.parent = &JIVE_UNARY_OPERATION,
		.name = "SELECT",
		.fini = jive_node_fini_, /* inherit */
		.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
		.get_label = jive_select_node_get_label_, /* override */
		.get_attrs = jive_select_node_get_attrs_, /* override */
		.match_attrs = jive_select_node_match_attrs_, /* override */
		.create = jive_select_node_create_,	/* override */
		.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
	},

	.single_apply_over = NULL,
	.multi_apply_over = NULL,
	
	.can_reduce_operand = jive_select_can_reduce_operand_, /* override */
	.reduce_operand = jive_select_reduce_operand_ /* override */
};

static void
jive_select_node_init_(jive_select_node * self, struct jive_region * region,
	size_t element, jive_output * operand)
{
	jive_context * context = region->graph->context;
	if (operand->class_ != &JIVE_RECORD_OUTPUT) {
		jive_context_fatal_error(context, "Type mismatch: need 'record' type as input to 'select' node");
	}
	const jive_record_type * operand_type = (const jive_record_type *)
		jive_output_get_type(operand);
	
	if (element >= operand_type->decl->nelements) {
		char tmp[256];
		snprintf(tmp, sizeof(tmp), "Type mismatch: attempted to select element #%zd from record of %zd elements",
			element, operand_type->decl->nelements);
		jive_context_fatal_error(context, jive_context_strdup(context, tmp));
	}
	
	self->attrs.element = element;

	const jive_type * output_type = &operand_type->decl->elements[element]->base;
	jive_node_init_(&self->base, region,
		1, (const jive_type * []){&operand_type->base.base}, &operand,
		1, &output_type);
}

static char *
jive_select_node_get_label_(const jive_node * self_)
{
	const jive_select_node * self = (const jive_select_node *) self_;

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "SELECT(%zd)", self->attrs.element);
	return strdup(tmp);
}

static const jive_node_attrs *
jive_select_node_get_attrs_(const jive_node * self_)
{
	const jive_select_node * self = (const jive_select_node *)self_;

	return &self->attrs.base;
} 

static bool
jive_select_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_select_node_attrs * first = &((const jive_select_node *)self)->attrs;
	const jive_select_node_attrs * second = (const jive_select_node_attrs *) attrs;

	if(first->element != second->element)
		return false;

	return true;
}

static jive_node *
jive_select_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_select_node_attrs * attrs = (const jive_select_node_attrs *)attrs_;

	return &jive_select_node_create(region, attrs->element, operands[0])->base;	
}

static jive_unop_reduction_path_t
jive_select_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs,
	const jive_output * operand)
{
	return jive_unop_reduction_none;
}

static jive_output *
jive_select_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand)
{
	return NULL;
}

jive_select_node *
jive_select_node_create(struct jive_region * region, size_t member, jive_output * operand)
{
	jive_select_node * node = jive_context_malloc(region->graph->context, sizeof(*node));

	node->base.class_ = &JIVE_SELECT_NODE;
	jive_select_node_init_(node, region, member, operand);

	return node;	
}

jive_output *
jive_select_create(size_t member, jive_output * operand)
{
	jive_select_node * node = jive_select_node_create(operand->node->region, member, operand);

	return (jive_output *)node->base.outputs[0];
}


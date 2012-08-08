#include <jive/types/union/unnchoose.h>

#include <jive/vsdg/node-private.h>
#include <jive/types/union/unntype.h>

#include <stdio.h>
#include <string.h>

static char *
jive_choose_node_get_label_(const jive_node * self);

static const jive_node_attrs *
jive_choose_node_get_attrs_(const jive_node * self);

static jive_node *
jive_choose_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static bool
jive_choose_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

const jive_node_class JIVE_CHOOSE_NODE = {
	.parent = &JIVE_NODE,
	.name = "CHOOSE",
	.fini = jive_node_fini_, /* inherit */
	.get_label = jive_choose_node_get_label_, /* override */
	.get_attrs = jive_choose_node_get_attrs_, /* override */
	.match_attrs = jive_choose_node_match_attrs_, /* overrride */
	.create = jive_choose_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_choose_node_init_(jive_choose_node * self, struct jive_region * region,
	size_t element, jive_output * operand)
{
	jive_context * context = region->graph->context;
	if (operand->class_ != &JIVE_UNION_OUTPUT) {
		jive_context_fatal_error(context, "Type mismatch: need 'union' type as input to 'choose' node");
	}
	
	const jive_union_type * operand_type = (const jive_union_type *)
		operand->class_->get_type(operand);

	if (element > operand_type->decl->nelements) {
		char tmp[256];
		snprintf(tmp, sizeof(tmp), "Type mismatch: attempted to select element #%zd from union of %zd elements",
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
jive_choose_node_get_label_(const jive_node * self_)
{
	const jive_choose_node * self = (const jive_choose_node *) self_;

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "CHOOSE(%zd)", self->attrs.element);
	return strdup(tmp);
}

static const jive_node_attrs *
jive_choose_node_get_attrs_(const jive_node * self_)
{
	const jive_choose_node * self = (const jive_choose_node *)self_;

	return &self->attrs.base;
}

static bool
jive_choose_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_choose_node_attrs * first = &((const jive_choose_node *)self)->attrs;
	const jive_choose_node_attrs * second = (const jive_choose_node_attrs *) attrs;

	if(first->element != second->element)
		return false;

	return true;
}

static jive_node *
jive_choose_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_choose_node_attrs * attrs = (const jive_choose_node_attrs *)attrs_;

	return &jive_choose_node_create(region, attrs->element, operands[0])->base;
}

jive_choose_node *
jive_choose_node_create(struct jive_region * region, size_t member, jive_output * operand)
{
	jive_choose_node * node = jive_context_malloc(region->graph->context, sizeof(*node));

	node->base.class_ = &JIVE_CHOOSE_NODE;
	jive_choose_node_init_(node, region, member, operand);

	return node;
}

jive_output *
jive_choose_create(size_t member, jive_output * operand)
{
	jive_choose_node * node = jive_choose_node_create(operand->node->region, member, operand);

	return (jive_output *)node->base.outputs[0];
}


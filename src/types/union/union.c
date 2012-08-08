#include <jive/types/union/union.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/union/uniontype.h>
#include <jive/context.h>
#include <jive/common.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/graph.h>

#include <stdio.h>
#include <string.h>

static void
jive_unify_node_init_(jive_unify_node * self,
	struct jive_region * region, const jive_union_declaration * decl,
	size_t option, jive_output * const operand);

static jive_node *
jive_unify_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static char *
jive_unify_node_get_label_(const jive_node * self_);

static const jive_node_attrs *
jive_unify_node_get_attrs_(const jive_node * self);

static bool
jive_unify_node_match_attrs_(const jive_node * self, const jive_node_attrs * second);

const jive_node_class JIVE_UNIFY_NODE = {
	.parent = &JIVE_NODE,
	.name = "UNIFY",
	.fini = jive_node_fini_, /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_unify_node_get_label_, /* override */
	.get_attrs = jive_unify_node_get_attrs_, /* override */
	.match_attrs = jive_unify_node_match_attrs_, /* override */
	.create = jive_unify_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static char *
jive_unify_node_get_label_(const jive_node * self_)
{
	return strdup("UNIFY");
}

static const jive_node_attrs *
jive_unify_node_get_attrs_(const jive_node * self_)
{
	const jive_unify_node * self = (const jive_unify_node*)self_;
	
	return &self->attrs.base;
}

static bool
jive_unify_node_match_attrs_(const jive_node * self, const jive_node_attrs * second_)
{
	const jive_unify_node_attrs * first = (const jive_unify_node_attrs *)jive_node_get_attrs(self);
	const jive_unify_node_attrs * second = (const jive_unify_node_attrs *)second_;
	
	return (first->decl == second->decl) && (first->option == second->option);
}

static jive_node *
jive_unify_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_unify_node_attrs * attrs = (const jive_unify_node_attrs *)attrs_ ;
	
	return jive_unify_node_create(region, attrs->decl, attrs->option, operands[0]);
}

static void
jive_unify_node_init_(jive_unify_node * self,
	struct jive_region * region, const jive_union_declaration * decl,
	size_t option, jive_output * const operand)
{
	if (option >= decl->nelements) {
		jive_context_fatal_error(region->graph->context,
			"Type mismatch: invalid option for union type");
	}
	
	const jive_type * arg_type = &decl->elements[option]->base;
	
	JIVE_DECLARE_UNION_TYPE(type, decl);
	
	jive_node_init_(&self->base, region,
		1, &arg_type, &operand,
		1, &type);
	
	self->attrs.option = option;
	self->attrs.decl = decl;
}

jive_node *
jive_unify_node_create(struct jive_region * region, const jive_union_declaration * decl,
	size_t option, jive_output * const operand)
{
	jive_unify_node * node = jive_context_malloc(region->graph->context, sizeof(*node));

	node->base.class_ = &JIVE_UNIFY_NODE;
	jive_unify_node_init_(node, region, decl, option, operand);

	return &node->base;
}

jive_output *
jive_unify_create(const jive_union_declaration * decl,
	size_t option, jive_output * const operand)
{
	jive_node * node = jive_unify_node_create(operand->node->region, decl, option, operand);

	return node->outputs[0];
}

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


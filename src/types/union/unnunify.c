#include <jive/types/union/unnunify.h>

#include <jive/vsdg/node-private.h>
#include <jive/types/union/unntype.h>

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

const jive_unary_operation_class JIVE_UNIFY_NODE_ = {
	.base = { /* jive_node_class */
		.parent = &JIVE_UNARY_OPERATION,
		.name = "UNIFY",
		.fini = jive_node_fini_, /* inherit */
		.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
		.get_label = jive_unify_node_get_label_, /* override */
		.get_attrs = jive_unify_node_get_attrs_, /* override */
		.match_attrs = jive_unify_node_match_attrs_, /* override */
		.create = jive_unify_node_create_, /* override */
		.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
	},

	.single_apply_over = NULL,
	.multi_apply_over = NULL,

	.can_reduce_operand = jive_unary_operation_can_reduce_operand_, /* inherit */
	.reduce_operand = jive_unary_operation_reduce_operand_ /* inherit */
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
	jive_unify_node_attrs attrs;
	attrs.decl = decl;
	attrs.option = option;

	const jive_unary_operation_normal_form * nf = (const jive_unary_operation_normal_form *)
		jive_graph_get_nodeclass_form(operand->node->graph, &JIVE_UNIFY_NODE);

	return jive_unary_operation_normalized_create(nf, operand->node->region, &attrs.base, operand);
}

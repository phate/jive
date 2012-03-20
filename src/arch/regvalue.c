#include <jive/arch/regvalue.h>

#include <string.h>

#include <jive/types/bitstring/type.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static void
jive_regvalue_node_init_(
	jive_regvalue_node * self,
	jive_region * region,
	jive_output * ctl,
	const jive_register_class * regcls,
	jive_output * value);

static char *
jive_regvalue_node_get_label_(const jive_node * self);

static const jive_node_attrs *
jive_regvalue_node_get_attrs_(const jive_node * self);

static bool
jive_regvalue_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_regvalue_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_REGVALUE_NODE = {
	.parent = &JIVE_NODE,
	.fini = jive_node_fini_, /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_regvalue_node_get_label_, /* override */
	.get_attrs = jive_regvalue_node_get_attrs_, /* override */
	.match_attrs = jive_regvalue_node_match_attrs_, /* override */
	.create = jive_regvalue_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_regvalue_node_init_(
	jive_regvalue_node * self,
	jive_region * region,
	jive_output * ctl,
	const jive_register_class * regcls,
	jive_output * value)
{
	if (!jive_output_isinstance(value, &JIVE_BITSTRING_OUTPUT)) {
		jive_context_fatal_error(region->graph->context, "Type mismatch: regvalue node requires bitstring operand");
	}
	size_t nbits = ((jive_bitstring_output *)value)->type.nbits;
	
	if (nbits != regcls->nbits) {
		jive_context_fatal_error(region->graph->context, "Type mismatch: size of value must match register size");
	}
	
	JIVE_DECLARE_CONTROL_TYPE(ctl_type);
	JIVE_DECLARE_BITSTRING_TYPE(bitstring_type, nbits);
	const jive_type * operand_types[] = {ctl_type, bitstring_type};
	jive_output * operands[] = {ctl, value};
	
	jive_node_init_(&self->base, region,
		2, operand_types, operands,
		1, &bitstring_type);
	
	self->attrs.regcls = regcls;
	self->base.outputs[0]->required_rescls = &regcls->base;
}

static char *
jive_regvalue_node_get_label_(const jive_node * self_)
{
	const jive_regvalue_node * self = (const jive_regvalue_node *) self_;
	return strdup(self->attrs.regcls->base.name);
}

static const jive_node_attrs *
jive_regvalue_node_get_attrs_(const jive_node * self_)
{
	const jive_regvalue_node * self = (const jive_regvalue_node *) self_;
	return &self->attrs.base;
}

static bool
jive_regvalue_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_regvalue_node_attrs * first = &((const jive_regvalue_node *) self)->attrs;
	const jive_regvalue_node_attrs * second = (const jive_regvalue_node_attrs *) attrs;
	return (first->regcls == second->regcls);
}

static jive_node *
jive_regvalue_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_regvalue_node_attrs * attrs = (const jive_regvalue_node_attrs *) attrs_;
	
	jive_regvalue_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_REGVALUE_NODE;
	jive_regvalue_node_init_(node, region, operands[0], attrs->regcls, operands[1]);
	
	return &node->base;
}

jive_output *
jive_regvalue(jive_output * ctl, const jive_register_class * regcls, jive_output * value)
{
	jive_regvalue_node_attrs attrs;
	attrs.regcls = regcls;
	
	jive_output * operands[] = {ctl, value};
	jive_region * region = jive_region_innermost(2, operands);
	
	const jive_node_normal_form * nf =
		jive_graph_get_nodeclass_form(region->graph, &JIVE_REGVALUE_NODE);
	jive_node * node = jive_node_cse_create(nf, region, &attrs.base, 2, operands);
	return node->outputs[0];
}
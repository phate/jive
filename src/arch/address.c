#include <jive/arch/address.h>

#include <stdio.h>
#include <string.h>

#include <jive/context.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

/* memberof */

static char *
jive_memberof_node_get_label_(const jive_node * self_);

static const jive_node_attrs *
jive_memberof_node_get_attrs_(const jive_node * self_);

static bool
jive_memberof_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_memberof_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

static bool
jive_memberof_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output * operand);

static bool
jive_memberof_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output ** operand_);

const jive_unary_operation_class JIVE_MEMBEROF_NODE_ = {
	.base = {
		.parent = &JIVE_UNARY_OPERATION,
		.name = "MEMBEROF",
		.fini = _jive_node_fini, /* inherit */
		.get_label = jive_memberof_node_get_label_, /* override */
		.get_attrs = jive_memberof_node_get_attrs_, /* override */
		.match_attrs = jive_memberof_node_match_attrs_, /* override */
		.create = jive_memberof_node_create_, /* override */
		.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
	},
	
	.single_apply_over = NULL,
	.multi_apply_over = NULL,
	
	.can_reduce_operand = jive_memberof_can_reduce_operand_, /* override */
	.reduce_operand = jive_memberof_reduce_operand_ /* override */
};

static char *
jive_memberof_node_get_label_(const jive_node * self_)
{
	const jive_memberof_node * self = (const jive_memberof_node *) self_;
	char tmp[128];
	snprintf(tmp, sizeof(tmp), "MEMBEROF %p:%zd", self->attrs.record_layout, self->attrs.index);
	return strdup(tmp);
}

static const jive_node_attrs *
jive_memberof_node_get_attrs_(const jive_node * self_)
{
	const jive_memberof_node * self = (const jive_memberof_node *) self_;
	return &self->attrs.base;
}

static bool
jive_memberof_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_memberof_node * self = (const jive_memberof_node *) self_;
	const jive_memberof_node_attrs * attrs = (const jive_memberof_node_attrs *) attrs_;
	return (self->attrs.record_layout == attrs->record_layout) && (self->attrs.index == attrs->index);
}

static jive_node *
jive_memberof_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	const jive_memberof_node_attrs * attrs = (const jive_memberof_node_attrs *) attrs_;
	return jive_memberof_node_create(region, operands[0], attrs->record_layout, attrs->index);
}

static bool
jive_memberof_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output * operand)
{
	const jive_memberof_node_attrs * attrs = (const jive_memberof_node_attrs *) attrs_;
	
	const jive_containerof_node * node = jive_containerof_node_cast(operand->node);
	if (!node)
		return false;
	
	if (node->attrs.record_layout == attrs->record_layout && node->attrs.index == attrs->index)
		return true;
	
	return false;
}

static bool
jive_memberof_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output ** operand)
{
	const jive_memberof_node_attrs * attrs = (const jive_memberof_node_attrs *) attrs_;
	
	const jive_containerof_node * node = jive_containerof_node_cast((*operand)->node);
	if (!node)
		return false;
	
	if (node->attrs.record_layout == attrs->record_layout && node->attrs.index == attrs->index) {
		*operand = node->base.inputs[0]->origin;
		return true;
	}
	
	return false;
}

jive_node *
jive_memberof_node_create(jive_region * region,
	jive_output * address,
	const jive_record_layout * record_layout, size_t index)
{
	jive_context * context = region->graph->context;
	jive_memberof_node * node = jive_context_malloc(context, sizeof(*node));
	
	node->base.class_ = &JIVE_MEMBEROF_NODE;
	
	JIVE_DECLARE_ADDRESS_TYPE(address_type);
	
	_jive_node_init(&node->base, region,
		1, &address_type, &address,
		1, &address_type);
	node->attrs.record_layout = record_layout;
	node->attrs.index = index;
	
	return &node->base;
}

jive_node *
jive_memberof_create(jive_region * region,
	jive_output * address,
	const jive_record_layout * record_layout, size_t index)
{
	jive_memberof_node_attrs attrs;
	attrs.record_layout = record_layout;
	attrs.index = index;
	
	return jive_unary_operation_normalized_create(&JIVE_MEMBEROF_NODE, region, &attrs.base, address)->node;
}

jive_output *
jive_memberof(jive_output * address,
	const jive_record_layout * record_layout, size_t index)
{
	jive_memberof_node_attrs attrs;
	attrs.record_layout = record_layout;
	attrs.index = index;
	
	return jive_unary_operation_normalized_create(&JIVE_MEMBEROF_NODE, address->node->region, &attrs.base, address);
}

/* containerof */

static char *
jive_containerof_node_get_label_(const jive_node * self_);

static const jive_node_attrs *
jive_containerof_node_get_attrs_(const jive_node * self_);

static bool
jive_containerof_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_containerof_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

static bool
jive_containerof_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output * operand);

static bool
jive_containerof_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output ** operand_);

const jive_unary_operation_class JIVE_CONTAINEROF_NODE_ = {
	.base = {
		.parent = &JIVE_UNARY_OPERATION,
		.name = "CONTAINEROF",
		.fini = _jive_node_fini, /* inherit */
		.get_label = jive_containerof_node_get_label_, /* override */
		.get_attrs = jive_containerof_node_get_attrs_, /* override */
		.match_attrs = jive_containerof_node_match_attrs_, /* override */
		.create = jive_containerof_node_create_, /* override */
		.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
	},
	
	.single_apply_over = NULL,
	.multi_apply_over = NULL,
	
	.can_reduce_operand = jive_containerof_can_reduce_operand_, /* override */
	.reduce_operand = jive_containerof_reduce_operand_ /* override */
};

static char *
jive_containerof_node_get_label_(const jive_node * self_)
{
	const jive_containerof_node * self = (const jive_containerof_node *) self_;
	char tmp[128];
	snprintf(tmp, sizeof(tmp), "CONTAINEROF %p:%zd", self->attrs.record_layout, self->attrs.index);
	return strdup(tmp);
}

static const jive_node_attrs *
jive_containerof_node_get_attrs_(const jive_node * self_)
{
	const jive_containerof_node * self = (const jive_containerof_node *) self_;
	return &self->attrs.base;
}

static bool
jive_containerof_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_containerof_node * self = (const jive_containerof_node *) self_;
	const jive_containerof_node_attrs * attrs = (const jive_containerof_node_attrs *) attrs_;
	return (self->attrs.record_layout == attrs->record_layout) && (self->attrs.index == attrs->index);
}

static jive_node *
jive_containerof_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	const jive_containerof_node_attrs * attrs = (const jive_containerof_node_attrs *) attrs_;
	return jive_containerof_node_create(region, operands[0], attrs->record_layout, attrs->index);
}

static bool
jive_containerof_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output * operand)
{
	const jive_containerof_node_attrs * attrs = (const jive_containerof_node_attrs *) attrs_;
	
	const jive_memberof_node * node = jive_memberof_node_cast(operand->node);
	if (!node)
		return false;
	
	if (node->attrs.record_layout == attrs->record_layout && node->attrs.index == attrs->index)
		return true;
	
	return false;
}

static bool
jive_containerof_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output ** operand)
{
	const jive_containerof_node_attrs * attrs = (const jive_containerof_node_attrs *) attrs_;
	
	const jive_memberof_node * node = jive_memberof_node_cast((*operand)->node);
	if (!node)
		return false;
	
	if (node->attrs.record_layout == attrs->record_layout && node->attrs.index == attrs->index) {
		*operand = node->base.inputs[0]->origin;
		return true;
	}
	
	return false;
}

jive_node *
jive_containerof_node_create(jive_region * region,
	jive_output * address,
	const jive_record_layout * record_layout, size_t index)
{
	jive_context * context = region->graph->context;
	jive_containerof_node * node = jive_context_malloc(context, sizeof(*node));
	
	node->base.class_ = &JIVE_CONTAINEROF_NODE;
	
	JIVE_DECLARE_ADDRESS_TYPE(address_type);
	
	_jive_node_init(&node->base, region,
		1, &address_type, &address,
		1, &address_type);
	node->attrs.record_layout = record_layout;
	node->attrs.index = index;
	
	return &node->base;
}

jive_node *
jive_containerof_create(jive_region * region,
	jive_output * address,
	const jive_record_layout * record_layout, size_t index)
{
	jive_containerof_node_attrs attrs;
	attrs.record_layout = record_layout;
	attrs.index = index;
	
	return jive_unary_operation_normalized_create(&JIVE_CONTAINEROF_NODE, region, &attrs.base, address)->node;
}

jive_output *
jive_containerof(jive_output * address,
	const jive_record_layout * record_layout, size_t index)
{
	jive_containerof_node_attrs attrs;
	attrs.record_layout = record_layout;
	attrs.index = index;
	
	return jive_unary_operation_normalized_create(&JIVE_CONTAINEROF_NODE, address->node->region, &attrs.base, address);
}

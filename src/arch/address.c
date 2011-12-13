#include <jive/arch/address.h>

#include <stdio.h>
#include <string.h>

#include <jive/context.h>
#include <jive/arch/addresstype.h>
#include <jive/bitstring/arithmetic.h>
#include <jive/bitstring/constant.h>
#include <jive/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/label.h>
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
jive_memberof_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[]);

static jive_unop_reduction_path_t 
jive_memberof_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	const jive_output * operand);

static jive_output * 
jive_memberof_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand_);

const jive_unary_operation_class JIVE_MEMBEROF_NODE_ = {
	.base = {
		.parent = &JIVE_UNARY_OPERATION,
		.name = "MEMBEROF",
		.fini = jive_node_fini_, /* inherit */
		.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
		.get_label = jive_memberof_node_get_label_, /* override */
		.get_attrs = jive_memberof_node_get_attrs_, /* override */
		.match_attrs = jive_memberof_node_match_attrs_, /* override */
		.create = jive_memberof_node_create_, /* override */
		.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
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
	snprintf(tmp, sizeof(tmp), "MEMBEROF %p:%zd", self->attrs.record_decl, self->attrs.index);
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
	return (self->attrs.record_decl == attrs->record_decl) && (self->attrs.index == attrs->index);
}

static jive_node *
jive_memberof_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	const jive_memberof_node_attrs * attrs = (const jive_memberof_node_attrs *) attrs_;
	return jive_memberof_node_create(region, operands[0], attrs->record_decl, attrs->index);
}

static jive_unop_reduction_path_t
jive_memberof_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	const jive_output * operand)
{
	const jive_memberof_node_attrs * attrs = (const jive_memberof_node_attrs *) attrs_;
	
	const jive_containerof_node * node = jive_containerof_node_cast(operand->node);
	if (!node)
		return jive_unop_reduction_none;
	
	if (node->attrs.record_decl == attrs->record_decl && node->attrs.index == attrs->index)
		return jive_unop_reduction_inverse;
	
	return jive_unop_reduction_none;
}

static jive_output * 
jive_memberof_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand)
{
	if (path == jive_unop_reduction_inverse)
		return operand->node->inputs[0]->origin;
	
	return NULL;
}

jive_node *
jive_memberof_node_create(jive_region * region,
	jive_output * address,
	const jive_record_declaration * record_decl, size_t index)
{
	jive_context * context = region->graph->context;
	jive_memberof_node * node = jive_context_malloc(context, sizeof(*node));
	
	node->base.class_ = &JIVE_MEMBEROF_NODE;
	
	JIVE_DECLARE_ADDRESS_TYPE(address_type);
	
	jive_node_init_(&node->base, region,
		1, &address_type, &address,
		1, &address_type);
	node->attrs.record_decl = record_decl;
	node->attrs.index = index;
	
	return &node->base;
}

jive_node *
jive_memberof_create(jive_region * region,
	jive_output * address,
	const jive_record_declaration * record_decl, size_t index)
{
	jive_memberof_node_attrs attrs;
	attrs.record_decl = record_decl;
	attrs.index = index;
	
	const jive_unary_operation_normal_form * nf =
		(const jive_unary_operation_normal_form *)
		jive_graph_get_nodeclass_form(region->graph, &JIVE_MEMBEROF_NODE);

	return jive_unary_operation_normalized_create(nf, region, &attrs.base, address)->node;
}

jive_output *
jive_memberof(jive_output * address,
	const jive_record_declaration * record_decl, size_t index)
{
	jive_memberof_node_attrs attrs;
	attrs.record_decl = record_decl;
	attrs.index = index;
	
	const jive_unary_operation_normal_form * nf =
		(const jive_unary_operation_normal_form *)
		jive_graph_get_nodeclass_form(address->node->graph, &JIVE_MEMBEROF_NODE);

	return jive_unary_operation_normalized_create(nf, address->node->region, &attrs.base, address);
}

/* containerof */

static char *
jive_containerof_node_get_label_(const jive_node * self_);

static const jive_node_attrs *
jive_containerof_node_get_attrs_(const jive_node * self_);

static bool
jive_containerof_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_containerof_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[]);

static jive_unop_reduction_path_t 
jive_containerof_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	const jive_output * operand);

static jive_output * 
jive_containerof_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand_);

const jive_unary_operation_class JIVE_CONTAINEROF_NODE_ = {
	.base = {
		.parent = &JIVE_UNARY_OPERATION,
		.name = "CONTAINEROF",
		.fini = jive_node_fini_, /* inherit */
		.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
		.get_label = jive_containerof_node_get_label_, /* override */
		.get_attrs = jive_containerof_node_get_attrs_, /* override */
		.match_attrs = jive_containerof_node_match_attrs_, /* override */
		.create = jive_containerof_node_create_, /* override */
		.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
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
	snprintf(tmp, sizeof(tmp), "CONTAINEROF %p:%zd", self->attrs.record_decl, self->attrs.index);
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
	return (self->attrs.record_decl == attrs->record_decl) && (self->attrs.index == attrs->index);
}

static jive_node *
jive_containerof_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	const jive_containerof_node_attrs * attrs = (const jive_containerof_node_attrs *) attrs_;
	return jive_containerof_node_create(region, operands[0], attrs->record_decl, attrs->index);
}

static jive_unop_reduction_path_t 
jive_containerof_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	const jive_output * operand)
{
	const jive_containerof_node_attrs * attrs = (const jive_containerof_node_attrs *) attrs_;
	
	const jive_memberof_node * node = jive_memberof_node_cast(operand->node);
	if (!node)
		return jive_unop_reduction_none;
	
	if (node->attrs.record_decl == attrs->record_decl && node->attrs.index == attrs->index)
		return jive_unop_reduction_inverse;
	
	return jive_unop_reduction_none;
}

static jive_output * 
jive_containerof_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand)
{
	if (path == jive_unop_reduction_inverse)
		return operand->node->inputs[0]->origin;	
	
	return NULL;
}

jive_node *
jive_containerof_node_create(jive_region * region,
	jive_output * address,
	const jive_record_declaration * record_decl, size_t index)
{
	jive_context * context = region->graph->context;
	jive_containerof_node * node = jive_context_malloc(context, sizeof(*node));
	
	node->base.class_ = &JIVE_CONTAINEROF_NODE;
	
	JIVE_DECLARE_ADDRESS_TYPE(address_type);
	
	jive_node_init_(&node->base, region,
		1, &address_type, &address,
		1, &address_type);
	node->attrs.record_decl = record_decl;
	node->attrs.index = index;
	
	return &node->base;
}

jive_node *
jive_containerof_create(jive_region * region,
	jive_output * address,
	const jive_record_declaration * record_decl, size_t index)
{
	jive_containerof_node_attrs attrs;
	attrs.record_decl = record_decl;
	attrs.index = index;
	
	const jive_unary_operation_normal_form * nf =
		(const jive_unary_operation_normal_form *)
		jive_graph_get_nodeclass_form(region->graph, &JIVE_CONTAINEROF_NODE);

	return jive_unary_operation_normalized_create(nf, region, &attrs.base, address)->node;
}

jive_output *
jive_containerof(jive_output * address,
	const jive_record_declaration * record_decl, size_t index)
{
	jive_containerof_node_attrs attrs;
	attrs.record_decl = record_decl;
	attrs.index = index;
	
	const jive_unary_operation_normal_form * nf =
		(const jive_unary_operation_normal_form *)
		jive_graph_get_nodeclass_form(address->node->graph, &JIVE_CONTAINEROF_NODE);

	return jive_unary_operation_normalized_create(nf, address->node->region, &attrs.base, address);
}

/* arraysubscript */

static void
jive_arraysubscript_node_fini_(jive_node * self_);

static jive_node *
jive_arraysubscript_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[]);

static const jive_node_attrs *
jive_arraysubscript_node_get_attrs_(const jive_node * self_);

static bool
jive_arraysubscript_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_binop_reduction_path_t 
jive_arraysubscript_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs_, const jive_output * operand1, const jive_output * operand2);

static jive_output * 
jive_arraysubscript_reduce_operand_pair_(jive_binop_reduction_path_t path, const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output * operand1, jive_output * operand2);

const jive_binary_operation_class JIVE_ARRAYSUBSCRIPT_NODE_ = {
	.base = {
		.parent = &JIVE_BINARY_OPERATION,
		.name = "ARRAYSUBSCRIPT",
		.fini = jive_arraysubscript_node_fini_, /* override */
		.get_default_normal_form = jive_binary_operation_get_default_normal_form_, /* inherit */
		.get_label = jive_node_get_label_, /* inherit */
		.get_attrs = jive_arraysubscript_node_get_attrs_, /* override */
		.match_attrs = jive_arraysubscript_node_match_attrs_, /* override */
		.create = jive_arraysubscript_node_create_, /* override */
		.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
	},
	
	.flags = jive_binary_operation_none,
	.single_apply_under = NULL,
	.multi_apply_under = NULL,
	.distributive_over = NULL,
	.distributive_under = NULL,
	
	.can_reduce_operand_pair = jive_arraysubscript_can_reduce_operand_pair_, /* override */
	.reduce_operand_pair = jive_arraysubscript_reduce_operand_pair_ /* override */
};

static void
jive_arraysubscript_node_fini_(jive_node * self_)
{
	jive_arraysubscript_node * self = (jive_arraysubscript_node *) self_;
	jive_type_fini(&self->attrs.element_type->base);
	jive_context_free(self->base.graph->context, self->attrs.element_type);
	jive_node_fini_(&self->base);
}

static const jive_node_attrs *
jive_arraysubscript_node_get_attrs_(const jive_node * self_)
{
	const jive_arraysubscript_node * self = (const jive_arraysubscript_node *) self_;
	return &self->attrs.base;
}

static bool
jive_arraysubscript_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_arraysubscript_node * self = (const jive_arraysubscript_node *) self_;
	const jive_arraysubscript_node_attrs * attrs = (const jive_arraysubscript_node_attrs *) attrs_;
	
	return jive_type_equals(&self->attrs.element_type->base, &attrs->element_type->base);
}

static jive_node *
jive_arraysubscript_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);
	const jive_arraysubscript_node_attrs * attrs = (const jive_arraysubscript_node_attrs *) attrs_;
	return jive_arraysubscript_node_create(region, operands[0], attrs->element_type, operands[1]);
}

static jive_binop_reduction_path_t
jive_arraysubscript_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, const jive_output * operand1, const jive_output * operand2)
{
	const jive_arraysubscript_node_attrs * attrs = (const jive_arraysubscript_node_attrs *) attrs_;
	
	jive_bitconstant_node * offset_constant = jive_bitconstant_node_cast(operand2->node);
	if (offset_constant && jive_bitconstant_is_zero(offset_constant))
		return jive_binop_reduction_rneutral;
	
	const jive_arraysubscript_node * node = jive_arraysubscript_node_cast(operand1->node);
	if (node &&
		jive_type_equals(&attrs->element_type->base, &node->attrs.element_type->base) &&
		jive_type_equals(jive_output_get_type(operand2), jive_input_get_type(node->base.inputs[1])))
		return jive_binop_reduction_lfold;
	
	return jive_binop_reduction_none;
}

static jive_output *
jive_arraysubscript_reduce_operand_pair_(jive_binop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand1, jive_output * operand2)
{
	if (path == jive_binop_reduction_rneutral)
		return operand1;

	const jive_arraysubscript_node * node = jive_arraysubscript_node_cast(operand1->node);
	if (path == jive_binop_reduction_lfold) {
		jive_output * operands[2] = {node->base.inputs[1]->origin, operand2};
		jive_output * sum = jive_bitsum(2, operands);
		
		operands[0] = node->base.inputs[0]->origin;
		operands[1] = sum;
		jive_region * region = jive_region_innermost(2, (jive_output *[]){operands[0], operands[1]});
		const jive_binary_operation_normal_form * nf = (const jive_binary_operation_normal_form *)
			jive_graph_get_nodeclass_form(region->graph, &JIVE_ARRAYSUBSCRIPT_NODE);
		
		return jive_binary_operation_normalized_create_new(nf, region, attrs_, 2, operands);
	}
	
	return NULL;
}

jive_node *
jive_arraysubscript_node_create(jive_region * region,
	jive_output * address, const jive_value_type * element_type, jive_output * index)
{
	jive_context * context = region->graph->context;
	jive_arraysubscript_node * node = jive_context_malloc(context, sizeof(*node));
	
	node->base.class_ = &JIVE_ARRAYSUBSCRIPT_NODE;
	
	JIVE_DECLARE_ADDRESS_TYPE(address_type);
	const jive_type * index_type = jive_output_get_type(index);
	JIVE_DEBUG_ASSERT(index_type->class_ == &JIVE_BITSTRING_TYPE);
	
	const jive_type * operand_types[2] = {address_type, index_type};
	jive_output * operands[2] = {address, index};
	
	jive_node_init_(&node->base, region,
		2, operand_types, operands,
		1, &address_type);
	node->attrs.element_type = (jive_value_type *) jive_type_copy(&element_type->base, context);
	
	return &node->base;
}

jive_node *
jive_arraysubscript_create(jive_region * region,
	jive_output * address, const jive_value_type * element_type,jive_output * index)
{
	return jive_arraysubscript_node_create(region, address, element_type, index);
}

jive_output *
jive_arraysubscript(jive_output * address, const jive_value_type * element_type,
	jive_output * index)
{
	jive_region * region = jive_region_innermost(2, (jive_output *[]){address, index});
	
	jive_arraysubscript_node_attrs attrs;
	attrs.element_type = (jive_value_type *) element_type;
	
	jive_output * operands[2] = {address, index};
	const jive_binary_operation_normal_form * nf =
		(const jive_binary_operation_normal_form *)
		jive_graph_get_nodeclass_form(region->graph, &JIVE_ARRAYSUBSCRIPT_NODE);
	return jive_binary_operation_normalized_create_new(nf, region, &attrs.base, 2, operands);
}

/* arrayindex */

static void
jive_arrayindex_node_fini_(jive_node * self_);

static jive_node *
jive_arrayindex_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[]);

static const jive_node_attrs *
jive_arrayindex_node_get_attrs_(const jive_node * self_);

static bool
jive_arrayindex_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_binop_reduction_path_t 
jive_arrayindex_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, const jive_output * operand1, const jive_output * operand2);

static jive_output * 
jive_arrayindex_reduce_operand_pair_(jive_binop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand1, jive_output * operand2);

const jive_binary_operation_class JIVE_ARRAYINDEX_NODE_ = {
	.base = {
		.parent = &JIVE_BINARY_OPERATION,
		.name = "ARRAYINDEX",
		.fini = jive_arrayindex_node_fini_, /* override */
		.get_default_normal_form = jive_binary_operation_get_default_normal_form_, /* inherit */
		.get_label = jive_node_get_label_, /* inherit */
		.get_attrs = jive_arrayindex_node_get_attrs_, /* override */
		.match_attrs = jive_arrayindex_node_match_attrs_, /* override */
		.create = jive_arrayindex_node_create_, /* override */
		.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
	},
	
	.flags = jive_binary_operation_none,
	.single_apply_under = NULL,
	.multi_apply_under = NULL,
	.distributive_over = NULL,
	.distributive_under = NULL,
	
	.can_reduce_operand_pair = jive_arrayindex_can_reduce_operand_pair_, /* override */
	.reduce_operand_pair = jive_arrayindex_reduce_operand_pair_ /* override */
};

static void
jive_arrayindex_node_fini_(jive_node * self_)
{
	jive_arrayindex_node * self = (jive_arrayindex_node *) self_;
	jive_type_fini(&self->attrs.element_type->base);
	jive_context_free(self->base.graph->context, self->attrs.element_type);
	jive_node_fini_(&self->base);
}

static const jive_node_attrs *
jive_arrayindex_node_get_attrs_(const jive_node * self_)
{
	const jive_arrayindex_node * self = (const jive_arrayindex_node *) self_;
	return &self->attrs.base;
}

static bool
jive_arrayindex_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_arrayindex_node * self = (const jive_arrayindex_node *) self_;
	const jive_arrayindex_node_attrs * attrs = (const jive_arrayindex_node_attrs *) attrs_;
	
	return jive_type_equals(&self->attrs.element_type->base, &attrs->element_type->base);
}

static jive_node *
jive_arrayindex_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);
	const jive_arrayindex_node_attrs * attrs = (const jive_arrayindex_node_attrs *) attrs_;
	return jive_arrayindex_node_create(region, operands[0], operands[1], attrs->element_type, &attrs->difference_type.base.base);
}

static const jive_output *
get_array_base(const jive_output * addr, const jive_value_type * element_type)
{
	jive_arraysubscript_node * node = jive_arraysubscript_node_cast(addr->node);
	if (node && jive_type_equals(&element_type->base, &node->attrs.element_type->base))
		return addr->node->inputs[0]->origin;
	else return addr;
}

static jive_output *
get_array_index(jive_output * addr, const jive_output * base, const jive_value_type * element_type,
	const jive_bitstring_type * index_type)
{
	jive_output * index = NULL;
	jive_arraysubscript_node * node = jive_arraysubscript_node_cast(addr->node);
	if (node && jive_type_equals(&element_type->base, &node->attrs.element_type->base)) {
		/* FIXME: correct type! */
		index = addr->node->inputs[1]->origin;
	} else {
		char bits[index_type->nbits];
		memset(bits, '0', index_type->nbits);
		index = jive_bitconstant(addr->node->graph, index_type->nbits, bits);
	}
	
	return index;
}

static jive_binop_reduction_path_t
jive_arrayindex_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, const jive_output * operand1, const jive_output * operand2)
{
	const jive_arrayindex_node_attrs * attrs = (const jive_arrayindex_node_attrs *) attrs_;
	const jive_output * base1 = get_array_base(operand1, attrs->element_type);
	const jive_output * base2 = get_array_base(operand2, attrs->element_type);

	if (base1 == base2)
		return jive_binop_reduction_factor;
	
	return jive_binop_reduction_none;
}

static jive_output * 
jive_arrayindex_reduce_operand_pair_(jive_binop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand1, jive_output * operand2)
{
	const jive_arrayindex_node_attrs * attrs = (const jive_arrayindex_node_attrs *) attrs_;

	if (path == jive_binop_reduction_factor) {
		const jive_output * base1 = get_array_base(operand1, attrs->element_type);
		const jive_output * base2 = get_array_base(operand2, attrs->element_type);
	
		jive_output * index1 = get_array_index(operand1, base1, attrs->element_type,
			&attrs->difference_type);
		jive_output * index2 = get_array_index(operand2, base2, attrs->element_type,
			&attrs->difference_type);
	
		index2 = jive_bitnegate(index2);
		return jive_bitsum(2, (jive_output *[]){index1, index2});
	}

	return NULL;
}

jive_node *
jive_arrayindex_node_create(jive_region * region,
	jive_output * addr1, jive_output * addr2,
	const jive_value_type * element_type, const jive_type * difference_type)
{
	jive_context * context = region->graph->context;
	jive_arrayindex_node * node = jive_context_malloc(context, sizeof(*node));
	
	node->base.class_ = &JIVE_ARRAYINDEX_NODE;
	
	JIVE_DECLARE_ADDRESS_TYPE(address_type);
	const jive_type * operand_types[2] = {address_type, address_type};
	jive_output * operands[2] = {addr1, addr2};
	JIVE_DEBUG_ASSERT(difference_type->class_ == &JIVE_BITSTRING_TYPE);
	const jive_type * output_types[1] = {difference_type};
	
	jive_node_init_(&node->base, region,
		2, operand_types, operands,
		1, output_types);
	node->attrs.element_type = (jive_value_type *) jive_type_copy(&element_type->base, context);
	node->attrs.difference_type = *(const jive_bitstring_type *)difference_type;
	
	return &node->base;
}

jive_node *
jive_arrayindex_create(jive_region * region,
	jive_output * addr1, jive_output * addr2,
	const jive_value_type * element_type, const jive_type * difference_type)
{
	return jive_arrayindex_node_create(region, addr1, addr2, element_type, difference_type);
}

jive_output *
jive_arrayindex(jive_output * addr1, jive_output * addr2,
	const jive_value_type * element_type, const jive_type * difference_type)
{
	jive_region * region = jive_region_innermost(2, (jive_output *[]){addr1, addr2});
	
	jive_arrayindex_node_attrs attrs;
	attrs.element_type = (jive_value_type *) element_type;
	attrs.difference_type = *(const jive_bitstring_type *)difference_type;
	
	jive_output * operands[2] = {addr1, addr2};
	const jive_binary_operation_normal_form * nf =
		(const jive_binary_operation_normal_form *)
		jive_graph_get_nodeclass_form(region->graph, &JIVE_ARRAYINDEX_NODE);
	return jive_binary_operation_normalized_create_new(nf, region, &attrs.base, 2, operands);
}

/* label_to_address node */

static void
jive_label_to_address_node_fini_(jive_node * self);

static char *
jive_label_to_address_node_get_label_(const jive_node * self);

static const jive_node_attrs *
jive_label_to_address_node_get_attrs_(const jive_node * self);

static bool
jive_label_to_address_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_label_to_address_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_LABEL_TO_ADDRESS_NODE = {
	.parent = &JIVE_NODE,
	.fini = jive_label_to_address_node_fini_, /* override */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_label_to_address_node_get_label_, /* override */
	.get_attrs = jive_label_to_address_node_get_attrs_, /* override */
	.match_attrs = jive_label_to_address_node_match_attrs_, /* override */
	.create = jive_label_to_address_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_label_to_address_node_init_(
	jive_label_to_address_node * self,
	jive_graph * graph,
	const jive_label * label)
{
	JIVE_DECLARE_ADDRESS_TYPE(addrtype);
	jive_node_init_(&self->base, graph->root_region,
		0, NULL, NULL,
		1, &addrtype);
	
	self->attrs.label = label;
}

static void
jive_label_to_address_node_fini_(jive_node * self_)
{
	jive_label_to_address_node * self = (jive_label_to_address_node *) self_;
	
	jive_node_fini_(&self->base);
}

static char *
jive_label_to_address_node_get_label_(const jive_node * self_)
{
	const jive_label_to_address_node * self = (const jive_label_to_address_node *) self_;
	
	const char * label_name = jive_label_get_asmname(self->attrs.label);
	size_t size = strlen(label_name) + strlen("addrof:") + 1;
	char * tmp = malloc(size);
	snprintf(tmp, size, "addrof:%s", label_name);
	return tmp;
}

static jive_node *
jive_label_to_address_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_label_to_address_node_attrs * attrs = (const jive_label_to_address_node_attrs *)attrs_;
	
	return jive_label_to_address_node_create(region->graph, attrs->label);
}

static const jive_node_attrs *
jive_label_to_address_node_get_attrs_(const jive_node * self_)
{
	const jive_label_to_address_node * self = (const jive_label_to_address_node *) self_;

	return &self->attrs.base;
}

static bool
jive_label_to_address_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_label_to_address_node_attrs * first =
		&((const jive_label_to_address_node *)self)->attrs;
	const jive_label_to_address_node_attrs * second =
		(const jive_label_to_address_node_attrs *) attrs;
	
	return first->label == second->label;
}

jive_node *
jive_label_to_address_node_create(struct jive_graph * graph, const jive_label * label)
{
	jive_label_to_address_node * node = jive_context_malloc(graph->context, sizeof(*node));
	node->base.class_ = &JIVE_LABEL_TO_ADDRESS_NODE;
	jive_label_to_address_node_init_(node, graph, label);

	return &node->base;
}

jive_output *
jive_label_to_address_create(struct jive_graph * graph, const jive_label * label)
{
	return jive_label_to_address_node_create(graph, label)->outputs[0];
}

/* label_to_bitstring_node */

static void
jive_label_to_bitstring_node_fini_(jive_node * self);

static char *
jive_label_to_bitstring_node_get_label_(const jive_node * self);

static const jive_node_attrs *
jive_label_to_bitstring_node_get_attrs_(const jive_node * self);

static bool
jive_label_to_bitstring_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_label_to_bitstring_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_LABEL_TO_BITSTRING_NODE = {
	.parent = &JIVE_NODE,
	.fini = jive_label_to_bitstring_node_fini_, /* override */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_label_to_bitstring_node_get_label_, /* override */
	.get_attrs = jive_label_to_bitstring_node_get_attrs_, /* override */
	.match_attrs = jive_label_to_bitstring_node_match_attrs_, /* override */
	.create = jive_label_to_bitstring_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_label_to_bitstring_node_init_(
	jive_label_to_bitstring_node * self,
	jive_graph * graph,
	const jive_label * label, size_t nbits)
{
	JIVE_DECLARE_BITSTRING_TYPE(btype, nbits);
	jive_node_init_(&self->base, graph->root_region,
		0, NULL, NULL,
		1, &btype);

	self->attrs.nbits = nbits;
	self->attrs.label = label;
}

static void
jive_label_to_bitstring_node_fini_(jive_node * self_)
{
	jive_label_to_bitstring_node * self = (jive_label_to_bitstring_node *) self_;

	jive_node_fini_(&self->base);
}

static char *
jive_label_to_bitstring_node_get_label_(const jive_node * self_)
{
	const jive_label_to_bitstring_node * self = (const jive_label_to_bitstring_node *) self_;
	
	const char * label_name = jive_label_get_asmname(self->attrs.label);
	size_t size = strlen(label_name) + strlen("addrof:") + 1;
	char * tmp = malloc(size);
	snprintf(tmp, size, "addrof:%s", label_name);
	return tmp;
}

static jive_node *
jive_label_to_bitstring_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_label_to_bitstring_node_attrs * attrs =
		(const jive_label_to_bitstring_node_attrs *) attrs_;

	return jive_label_to_bitstring_node_create(region->graph, attrs->label, attrs->nbits);
}

static const jive_node_attrs *
jive_label_to_bitstring_node_get_attrs_(const jive_node * self_)
{
	const jive_label_to_bitstring_node * self = (const jive_label_to_bitstring_node *) self_;

	return &self->attrs.base;
}

static bool
jive_label_to_bitstring_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_label_to_bitstring_node_attrs * first =
		&((const jive_label_to_bitstring_node *)self)->attrs;
	const jive_label_to_bitstring_node_attrs * second =
		(const jive_label_to_bitstring_node_attrs *) attrs;

	return (first->label == second->label) && (first->nbits == second->nbits);
}

jive_node *
jive_label_to_bitstring_node_create(struct jive_graph * graph, const jive_label * label, size_t nbits)
{
	jive_label_to_bitstring_node * node = jive_context_malloc(graph->context, sizeof(*node));
	node->base.class_ = &JIVE_LABEL_TO_BITSTRING_NODE;
	jive_label_to_bitstring_node_init_(node, graph, label, nbits);

	return &node->base;
}

jive_output *
jive_label_to_bitstring_create(struct jive_graph * graph, const jive_label * label, size_t nbits)
{
	return jive_label_to_bitstring_node_create(graph, label, nbits)->outputs[0];
}

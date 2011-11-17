#include <jive/arch/loadstore.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/bitstring/type.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/arch/addresstype.h>

static void
jive_load_node_fini_(jive_node * self_);

static const jive_node_attrs *
jive_load_node_get_attrs_(const jive_node * self_);

static bool
jive_load_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_load_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_LOAD_NODE = {
	.parent = &JIVE_NODE,
	.name = "LOAD",
	.fini = jive_load_node_fini_, /* override */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_load_node_get_attrs_, /* override */
	.match_attrs = jive_load_node_match_attrs_, /* override */
	.create = jive_load_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_load_node_fini_(jive_node * self_)
{
	jive_context * context = self_->graph->context;
	jive_load_node * self = (jive_load_node *) self_;
	
	jive_type_fini(self->attrs.type);
	jive_context_free(context, self->attrs.type);
	
	jive_node_fini_(&self->base);
}

static const jive_node_attrs *
jive_load_node_get_attrs_(const jive_node * self_)
{
	const jive_load_node * self = (const jive_load_node *) self_;
	return &self->attrs.base;
}

static bool
jive_load_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_load_node * self = (const jive_load_node *) self_;
	const jive_load_node_attrs * attrs = (const jive_load_node_attrs *) attrs_;
	return jive_type_equals(self->attrs.type, attrs->type);
}

static jive_node *
jive_load_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	const jive_load_node_attrs * attrs = (const jive_load_node_attrs *) attrs_;
	return jive_load_node_create(region, operands[0], attrs->type, 0, NULL);
}

jive_node *
jive_load_node_create(jive_region * region,
	jive_output * address,
	const struct jive_type * datatype,
	size_t nstates, jive_output * const states[])
{
	jive_context * context = region->graph->context;
	jive_load_node * node = jive_context_malloc(context, sizeof(*node));
	
	node->base.class_ = &JIVE_LOAD_NODE;
	const jive_type * address_type = jive_output_get_type(address);
	if (address_type->class_ != &JIVE_ADDRESS_TYPE && address_type->class_ != &JIVE_BITSTRING_TYPE) {
		char * operand_type_name = jive_type_get_label(address_type);
		
		char * error_msg = "Type mismatch (and additionally memory exhaustion";
		if (operand_type_name) {
			error_msg = jive_context_strjoin(context,
				"Type mismatch: required 'address' or 'bitstring', got '",
				operand_type_name, "'", NULL);
			free(operand_type_name);
		}
		jive_context_fatal_error(context, error_msg);
	}
	if (jive_type_isinstance(datatype, &JIVE_VALUE_TYPE)) {
		char * operand_type_name = jive_type_get_label(datatype);
		
		char * error_msg = "Type mismatch (and additionally memory exhaustion";
		if (operand_type_name) {
			error_msg = jive_context_strjoin(context,
				"Type mismatch: required 'valuetype', got '",
				operand_type_name, "'", NULL);
			free(operand_type_name);
		}
		jive_context_fatal_error(context, error_msg);
	}

	jive_node_init_(&node->base, region,
		1, &address_type, &address,
		1, &datatype);
	node->attrs.type = jive_type_copy(datatype, context);
	
	size_t n;
	for (n = 0; n < nstates; n++) {
		const jive_type * type = jive_output_get_type(states[n]);
		jive_node_add_input(&node->base, type, states[n]);
	}
	
	return &node->base;
}

struct jive_output *
jive_load_create(struct jive_output * address,
	const struct jive_type * datatype,
	size_t nstates, struct jive_output * const states[])
{
	size_t i;
	jive_output * outputs[nstates+1];
	for(i = 0; i < nstates; i++){
		outputs[i] = states[i];
	}
	outputs[nstates] = address;

	jive_region * region = jive_region_innermost(nstates+1, outputs);
	jive_node * node = jive_load_node_create(region, address, datatype, nstates, states);

	return node->outputs[0];	
}

static void
jive_store_node_fini_(jive_node * self_);

static const jive_node_attrs *
jive_store_node_get_attrs_(const jive_node * self_);

static bool
jive_store_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_store_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_STORE_NODE = {
	.parent = &JIVE_NODE,
	.name = "STORE",
	.fini = jive_store_node_fini_, /* override */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_store_node_get_attrs_, /* override */
	.match_attrs = jive_store_node_match_attrs_, /* override */
	.create = jive_store_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_store_node_fini_(jive_node * self_)
{
	jive_context * context = self_->graph->context;
	jive_store_node * self = (jive_store_node *) self_;
	
	jive_type_fini(self->attrs.type);
	jive_context_free(context, self->attrs.type);
	
	jive_node_fini_(&self->base);
}

static const jive_node_attrs *
jive_store_node_get_attrs_(const jive_node * self_)
{
	const jive_store_node * self = (const jive_store_node *) self_;
	return &self->attrs.base;
}

static bool
jive_store_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_store_node * self = (const jive_store_node *) self_;
	const jive_store_node_attrs * attrs = (const jive_store_node_attrs *) attrs_;
	return jive_type_equals(self->attrs.type, attrs->type);
}

static jive_node *
jive_store_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);
	const jive_store_node_attrs * attrs = (const jive_store_node_attrs *) attrs_;
	return jive_store_node_create(region, operands[0], attrs->type, operands[1], 0, NULL);
}

jive_node *
jive_store_node_create(jive_region * region,
	jive_output * address,
	const jive_type * datatype, jive_output * value,
	size_t nstates, jive_output * const states[])
{
	jive_context * context = region->graph->context;
	jive_store_node * node = jive_context_malloc(context, sizeof(*node));
	
	node->base.class_ = &JIVE_STORE_NODE;
	const jive_type * address_type = jive_output_get_type(address);
	if (address_type->class_ != &JIVE_ADDRESS_TYPE && address_type->class_ != &JIVE_BITSTRING_TYPE) {
		char * operand_type_name = jive_type_get_label(address_type);
		
		char * error_msg = "Type mismatch (and additionally memory exhaustion";
		if (operand_type_name) {
			error_msg = jive_context_strjoin(context,
				"Type mismatch: required 'address' or 'bitstring', got '",
				operand_type_name, "'", NULL);
			free(operand_type_name);
		}
		jive_context_fatal_error(context, error_msg);
	}
	if (jive_type_isinstance(datatype, &JIVE_VALUE_TYPE)) {
		char * operand_type_name = jive_type_get_label(datatype);
		
		char * error_msg = "Type mismatch (and additionally memory exhaustion";
		if (operand_type_name) {
			error_msg = jive_context_strjoin(context,
				"Type mismatch: required 'valuetype', got '",
				operand_type_name, "'", NULL);
			free(operand_type_name);
		}
		jive_context_fatal_error(context, error_msg);
	}
	
	const jive_type * operand_types[2] = {address_type, datatype};
	jive_output * operands[2] = {address, value};
	
	jive_node_init_(&node->base, region,
		2, operand_types, operands,
		0, NULL);
	node->attrs.type = jive_type_copy(datatype, context);
	
	size_t n;
	for (n = 0; n < nstates; n++) {
		const jive_type * type = jive_output_get_type(states[n]);
		jive_node_add_input(&node->base, type, states[n]);
		jive_node_add_output(&node->base, type);
	}
	
	return &node->base;
}

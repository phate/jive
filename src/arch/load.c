/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/load.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/bitstring/type.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/store.h>

/* load_node_normal_form */

static inline bool
store_reduce(jive_output * address, size_t nstates, jive_output * const states[])
{
	if (nstates == 0)
		return false;

	jive_node * store = states[0]->node;
	if (!jive_node_isinstance(store, &JIVE_STORE_NODE))
		return false;

	size_t n;
	for (n = 0; n < nstates; n++) {
		if (states[n]->node != store)
			return false;
	}

	if (store->inputs[0]->origin != address)
		return false;

	return true;	
}

static bool
jive_load_node_normalize_node_(const jive_node_normal_form * self_, jive_node * node)
{
	const jive_load_node_normal_form * self = (const jive_load_node_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;

	const jive_node_attrs * attrs = jive_node_get_attrs(node);

	if (self->enable_reducible) {
		size_t nstates = node->ninputs-1;
		jive_output * states[nstates];

		size_t n;
		for (n = 0; n < nstates; n++)
			states[n] = node->inputs[n+1]->origin;

		if (store_reduce(node->inputs[0]->origin, nstates, states)) {
			jive_node * store = node->inputs[1]->origin->node;
			jive_output_replace(node->outputs[0], store->inputs[1]->origin);
			/* FIXME: not sure whether "destroy" is really appropriate? */
			jive_node_destroy(node);
			return false;
		}
	}

	if (self->base.enable_cse) {
		size_t n;
		jive_output * operands[node->ninputs];
		for (n = 0; n < node->ninputs; n++)
			operands[n] = node->inputs[n]->origin;

		jive_node * new_node = jive_node_cse(node->region, self->base.node_class, attrs,
			node->ninputs, operands);
		JIVE_DEBUG_ASSERT(new_node);
		if (new_node != node) {
			for (n = 0;  n < node->noutputs; n++)
				jive_output_replace(node->outputs[n], new_node->outputs[n]);
			/* FIXME: not sure whether "destroy" is really appropriate? */
			jive_node_destroy(node);
			return false;
		}
	}

	return true;
}

static bool
jive_load_node_operands_are_normalized_(const jive_node_normal_form * self_, size_t noperands,
	jive_output * const operands[], const jive_node_attrs * attrs)
{
	const jive_load_node_normal_form * self = (const jive_load_node_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;

	jive_region * region = operands[0]->node->region;
	const jive_node_class * cls = self->base.node_class;

	if (self->enable_reducible) {
		if (store_reduce(operands[0], noperands-1, &operands[1]))
			return false;
	}

	if (self->base.enable_cse && jive_node_cse(region, cls, attrs, noperands, operands))
		return false;

	return true;
}

static jive_output *
jive_load_node_normalized_create_(const jive_load_node_normal_form * self,
	struct jive_region * region, const jive_node_attrs * attrs, jive_output * address,
	size_t nstates, jive_output * const states[])
{
	size_t n;
	size_t noperands = nstates+1;
	jive_output * operands[noperands];
	operands[0] = address;
	for(n = 0; n < nstates; n++)
		operands[n+1] = states[n];

	const jive_node_class * cls = self->base.node_class;

	if (self->base.enable_mutable && self->enable_reducible) {
		if (store_reduce(address, nstates, states))
			return states[0]->node->inputs[1]->origin;
	}

	if (self->base.enable_mutable && self->base.enable_cse) {
		jive_node * node = jive_node_cse(region, cls, attrs, noperands, operands);
		if (node)
			return node->outputs[0];
	}

	return cls->create(region, attrs, nstates+1, operands)->outputs[0];
}

static void
jive_load_node_set_reducible_(jive_load_node_normal_form * self, bool enable)
{
	if (self->enable_reducible == enable)
		return;

	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->base.subclasses, child, normal_form_subclass_list) {
		jive_load_node_set_reducible((jive_load_node_normal_form *)child, enable);
	}
	
	self->enable_reducible = enable;
	if (self->base.enable_mutable && self->enable_reducible)
		jive_graph_mark_denormalized(self->base.graph);
}

const jive_load_node_normal_form_class JIVE_LOAD_NODE_NORMAL_FORM_ = {
	base : { /* jive_node_normal_form_class */
		parent : &JIVE_NODE_NORMAL_FORM,
		fini : jive_node_normal_form_fini_, /* inherit */
		normalize_node : jive_load_node_normalize_node_, /* override */
		operands_are_normalized : jive_load_node_operands_are_normalized_, /* override */
		normalized_create : NULL, /* inherit */
		set_mutable : jive_node_normal_form_set_mutable_, /* inherit */
		set_cse : jive_node_normal_form_set_cse_ /* inherit */
	},
	set_reducible : jive_load_node_set_reducible_, 
	normalized_create : jive_load_node_normalized_create_ 
};

/* load node */

static void
jive_load_node_fini_(jive_node * self_);

static jive_node_normal_form *
jive_load_node_get_default_normal_form_(const jive_node_class * cls,
	jive_node_normal_form * parent_, struct jive_graph * graph);

static const jive_node_attrs *
jive_load_node_get_attrs_(const jive_node * self_);

static bool
jive_load_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static void
jive_load_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

static jive_node *
jive_load_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_LOAD_NODE = {
	parent : &JIVE_NODE,
	name : "LOAD",
	fini : jive_load_node_fini_, /* override */
	get_default_normal_form : jive_load_node_get_default_normal_form_, /* override */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : jive_load_node_get_attrs_, /* override */
	match_attrs : jive_load_node_match_attrs_, /* override */
	check_operands : jive_load_node_check_operands_, /* override */
	create : jive_load_node_create_, /* override */
	get_aux_rescls : jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_load_node_fini_(jive_node * self_)
{
	jive_context * context = self_->graph->context;
	jive_load_node * self = (jive_load_node *) self_;
	
	jive_type_fini(&self->attrs.datatype->base);
	jive_context_free(context, self->attrs.datatype);
	
	jive_node_fini_(&self->base);
}

static jive_node_normal_form *
jive_load_node_get_default_normal_form_(const jive_node_class * cls,
	jive_node_normal_form * parent_, struct jive_graph * graph)
{
	jive_context * context = graph->context;
	jive_load_node_normal_form * nf = jive_context_malloc(context, sizeof(*nf));

	jive_node_normal_form_init_(&nf->base, cls, parent_, graph);
	nf->base.class_ = &JIVE_LOAD_NODE_NORMAL_FORM;
	nf->enable_reducible = true;

	return &nf->base;
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
	bool dtype = jive_type_equals(&self->attrs.datatype->base, &attrs->datatype->base);
	return (dtype && (self->attrs.nbits == attrs->nbits));
}

static void
jive_load_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	JIVE_DEBUG_ASSERT(noperands > 0);

	const jive_address_output * addro = jive_address_output_const_cast(operands[0]);
	const jive_bitstring_output * bitso = jive_bitstring_output_const_cast(operands[0]);

	if (!addro && !bitso)
		jive_context_fatal_error(context, "Type mismatch: required address or bitstring type.");

	/* FIXME: check states */
}

static jive_node *
jive_load_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_load_node_attrs * attrs = (const jive_load_node_attrs *) attrs_;

	if(jive_output_isinstance(operands[0], &JIVE_BITSTRING_OUTPUT)){
		return jive_load_by_bitstring_node_create(region, operands[0], attrs->nbits, attrs->datatype,
			noperands-1, &operands[1]);
	} else {
		return jive_load_by_address_node_create(region, operands[0], attrs->datatype,
			noperands-1, &operands[1]);
	}
}

void
jive_load_node_init_(jive_load_node * self, jive_region * region,
	jive_output * address, const jive_type * address_type,
	const jive_value_type * datatype,
	size_t nstates, jive_output * const states[])
{
	jive_context * context = region->graph->context;
const jive_type *  tmparray0[] = {&datatype->base};

	jive_node_init_(&self->base, region,
		1, &address_type, &address,
		1, tmparray0);

	self->attrs.datatype = (jive_value_type *) jive_type_copy(&datatype->base, context);
	self->attrs.nbits = 0;
	if(jive_type_isinstance(address_type, &JIVE_BITSTRING_TYPE)) {
		const jive_bitstring_type * btype = (const jive_bitstring_type *) address_type;
		self->attrs.nbits = btype->nbits;	
	}
	
	/* FIXME: check the type of the states */
	size_t n;
	for (n = 0; n < nstates; n++) {
		const jive_type * type = jive_output_get_type(states[n]);
		jive_node_add_input(&self->base, type, states[n]);
	}
}

static inline jive_region *
load_node_region_innermost(jive_output * address, size_t nstates, jive_output * const states[])
{
	size_t i;
	jive_output * outputs[nstates+1];
	for(i = 0; i < nstates; i++){
		outputs[i] = states[i];
	}
	outputs[nstates] = address;
	
	return jive_region_innermost(nstates+1, outputs);
}

jive_node *
jive_load_by_address_node_create(jive_region * region,
	jive_output * address,
	const jive_value_type * datatype,
	size_t nstates, jive_output * const states[])
{
	jive_load_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	
	node->base.class_ = &JIVE_LOAD_NODE;
	JIVE_DECLARE_ADDRESS_TYPE(address_type);
	jive_load_node_init_(node, region, address, address_type, datatype, nstates, states);
	
	return &node->base;
}

jive_output *
jive_load_by_address_create(jive_output * address,
	const jive_value_type * datatype,
	size_t nstates, jive_output * const states[])
{
	const jive_load_node_normal_form * nf = (const jive_load_node_normal_form *)
		jive_graph_get_nodeclass_form(address->node->region->graph, &JIVE_LOAD_NODE);
	
	jive_region * region = load_node_region_innermost(address, nstates, states);

	jive_load_node_attrs attrs;
	attrs.nbits = 0;
	attrs.datatype = (jive_value_type *)datatype;
	
	return jive_load_node_normalized_create(nf, region, &attrs.base, address, nstates, states);
}

jive_node *
jive_load_by_bitstring_node_create(jive_region * region,
	jive_output * address, size_t nbits,
	const jive_value_type * datatype,
	size_t nstates, jive_output * const states[])
{
	jive_load_node * node = jive_context_malloc(region->graph->context, sizeof(*node));

	node->base.class_ = &JIVE_LOAD_NODE;
	JIVE_DECLARE_BITSTRING_TYPE(address_type, nbits);
	jive_load_node_init_(node, region, address, address_type, datatype, nstates, states);

	return &node->base;
}

jive_output *
jive_load_by_bitstring_create(jive_output * address, size_t nbits,
	const jive_value_type * datatype,
	size_t nstates, jive_output * const states[])
{
	const jive_load_node_normal_form * nf = (const jive_load_node_normal_form *)
		jive_graph_get_nodeclass_form(address->node->region->graph, &JIVE_LOAD_NODE);

	jive_region * region = load_node_region_innermost(address, nstates, states);

	jive_load_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.datatype = (jive_value_type *)datatype;

	return jive_load_node_normalized_create(nf, region, &attrs.base, address, nstates, states);
}


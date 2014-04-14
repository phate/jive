/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/store.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/bitstring/type.h>
#include <jive/arch/addresstype.h>
#include <jive/types/record/rcdgroup.h>
#include <jive/arch/address.h>
#include <jive/arch/address-transform.h>
#include <jive/arch/memorytype.h>
#include <jive/types/union/unnunify.h>
#include <jive/types/union/unntype.h>

/* store node normal form */

static inline bool
store_reduce(jive_output * address, jive_output * value,
	size_t nstates, jive_output * const istates[], jive_output * ostates[])
{
	if (nstates == 0)
		return false;

	jive_node * store = istates[0]->node;
	if (!jive_node_isinstance(store, &JIVE_STORE_NODE))
		return false;

	size_t n;
	for(n = 0; n < nstates; n++) {
		if (istates[n]->node != store)
			return false;
	}

	if(store->inputs[0]->origin != address)
		return false;

	if (store->inputs[1]->origin != value)
		return false;

	ostates = &store->outputs[2];

	return true;
}

static inline void
unify_reduce(const jive_store_node_normal_form * self, struct jive_region * region,
	const jive_node_attrs * attrs_, jive_output * address, jive_output * value,
	size_t nstates, jive_output * const istates[], jive_output * ostates[])
{
	jive_unify_node * unify_node = jive_unify_node_cast(value->node);
	const jive_union_declaration * decl = unify_node->attrs.decl;

	if (decl->nelements == 0) {
		size_t n;
		for (n = 0; n < nstates; n++)
			ostates[n] = istates[n];
		return;
	}
	
	const jive_store_node_attrs * attrs = (const jive_store_node_attrs *) attrs_;
	
	jive_store_node_attrs elem_attrs;
	elem_attrs.nbits = attrs->nbits;
	elem_attrs.datatype = (jive_value_type *) decl->elements[unify_node->attrs.option];
	
	jive_store_node_normalized_create(self, region, &elem_attrs, address,
		unify_node->inputs[0]->origin, nstates, istates, ostates);	
}

static inline void
group_reduce(const jive_store_node_normal_form * self, struct jive_region * region,
	const jive_node_attrs * attrs_, jive_output * address, jive_output * value,
	size_t nstates, jive_output * const istates[], jive_output * ostates[])
{
	if (nstates == 0) {
		jive_output * operands[2] = {address, value};
		self->base.node_class->create(region, attrs_, 2, operands);
		return;
	}

	jive_group_node * group_node = jive_group_node_cast(value->node);
	const jive_record_declaration * decl = group_node->attrs.decl;

	if (decl->nelements == 0) {
		size_t n;
		for (n = 0; n < nstates; n++)
			ostates[n] = istates[n];
		return;
	}

	const jive_store_node_attrs * attrs = (const jive_store_node_attrs *) attrs_;
	if (jive_output_isinstance(address, &JIVE_BITSTRING_OUTPUT))
		address = jive_bitstring_to_address_create(address, attrs->nbits, jive_output_get_type(address));

	size_t n;
	jive_node * split_nodes[nstates];
	JIVE_DECLARE_MEMORY_TYPE(memtype);
	for (n = 0; n < nstates; n++)
		split_nodes[n] = jive_state_split(memtype, istates[n], decl->nelements);

	size_t e;
	jive_output * elems_ostates[decl->nelements][nstates];
	for (e = 0; e < decl->nelements; e++) {
		jive_output * elem_address = jive_memberof(address, decl, e);

		jive_store_node_attrs elem_attrs;
		elem_attrs.nbits = attrs->nbits;
		elem_attrs.datatype = (jive_value_type *) decl->elements[e];

		jive_output * elem_istates[nstates];
		for (n = 0; n < nstates; n++)
			elem_istates[n] = split_nodes[n]->outputs[e];

		jive_store_node_normalized_create(self, region, &elem_attrs, elem_address,
			group_node->inputs[e]->origin, nstates, elem_istates, elems_ostates[e]);	
	}

	for(n = 0; n < nstates; n++) {
		jive_output * tmp[decl->nelements];
		for (e = 0; e < decl->nelements; e++)
			tmp[e] = elems_ostates[e][n];
		ostates[n] = jive_state_merge(memtype, decl->nelements, tmp);
	}
}

static bool
jive_store_node_normalize_node_(const jive_node_normal_form * self_, jive_node * node)
{
	const jive_store_node_normal_form * self = (const jive_store_node_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;

	const jive_node_attrs * attrs = jive_node_get_attrs(node);

	if (self->enable_reducible) {
		size_t nstates = node->ninputs-2;
		jive_output * istates[nstates];
		jive_output * ostates[nstates];
		
		size_t n;
		for (n = 0; n < nstates; n++)
			istates[n] = node->inputs[n+2]->origin;

		bool reduction = false;
		if (jive_node_isinstance(node->inputs[1]->origin->node, &JIVE_GROUP_NODE)) {
			group_reduce(self, node->region, attrs, node->inputs[0]->origin, node->inputs[1]->origin,
				nstates, istates, ostates);
			reduction = true;
		} else if (jive_node_isinstance(node->inputs[1]->origin->node, &JIVE_UNIFY_NODE)) {
			unify_reduce(self, node->region, attrs, node->inputs[0]->origin, node->inputs[1]->origin,
				nstates, istates, ostates);
			reduction = true;
		} else if (jive_node_isinstance(node->inputs[1]->origin->node, &JIVE_EMPTY_UNIFY_NODE)) {
			for (n = 0; n < nstates; n++)
				ostates[n] = istates[n];	
		} else if (store_reduce(node->inputs[0]->origin, node->inputs[1]->origin,
			nstates, istates, ostates)) {
			reduction = true;
		}

		if (reduction) {
			for(n = 0; n < nstates; n++)
				jive_output_replace(node->outputs[n], ostates[n]);
			/* FIXME: not sure whether "destroy" is really appropriate? */
			jive_node_destroy(node);
			return false;
		}
	}

	if (self->base.enable_cse) {
		size_t n;
		jive_output * operands[node->ninputs];
		for(n = 0; n < node->ninputs; n++)
			operands[n] = node->inputs[n]->origin;

		jive_node * new_node = jive_node_cse(node->region, self->base.node_class, attrs,
			node->ninputs, operands);
		JIVE_DEBUG_ASSERT(new_node);
		if (new_node != node) {
			for(n = 0; n < node->noutputs; n++)
				jive_output_replace(node->outputs[n], new_node->outputs[n]);
			/* FIXME: not srue whether "destroy" is really appropriate? */
			jive_node_destroy(node);
			return false;	
		}
	}

	return true;	
}

static bool
jive_store_node_operands_are_normalized_(const jive_node_normal_form * self_, size_t noperands,
	jive_output * const operands[], const jive_node_attrs * attrs)
{
	const jive_store_node_normal_form * self = (const jive_store_node_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;

	jive_region * region = operands[0]->node->region;
	const jive_node_class * cls = self->base.node_class;

	if (self->enable_reducible) {
		if (jive_node_isinstance(operands[1]->node, &JIVE_GROUP_NODE))
			return false;
	
		if (jive_node_isinstance(operands[1]->node, &JIVE_UNIFY_NODE))
			return false;

		jive_output * ostates[noperands-2];
		if (store_reduce(operands[0], operands[1], noperands-2, &operands[2], ostates))
			return false;
	}

	if (self->base.enable_cse && jive_node_cse(region, cls, attrs, noperands, operands))	
		return false;

	return true;
}

static void
jive_store_node_normalized_create_(const jive_store_node_normal_form * self,
	struct jive_region * region, const jive_node_attrs * attrs, jive_output * address,
	jive_output * value, size_t nstates, jive_output * const istates[], jive_output * ostates[])
{
	size_t n;
	size_t noperands = nstates + 2;
	jive_output * operands[noperands];
	operands[0] = address;
	operands[1] = value;
	for(n = 0; n < nstates; n++)
		operands[n+2] = istates[n];

	const jive_node_class * cls = self->base.node_class;

	if (self->base.enable_mutable && self->enable_reducible) {
		if (jive_node_isinstance(value->node, &JIVE_GROUP_NODE)) {
			group_reduce(self, region, attrs, address, value, nstates, istates, ostates);
			return;
		}
	
		if (jive_node_isinstance(value->node, &JIVE_UNIFY_NODE)) {
			unify_reduce(self, region, attrs, address, value, nstates, istates, ostates);
			return;
		}

		if (store_reduce(address, value, nstates, istates, ostates))
			return;
	}

	if (self->base.enable_mutable && self->base.enable_cse) {
		jive_node * node = jive_node_cse(region, cls, attrs, noperands, operands);
		if (node) {
			for(n = 0; n < node->noutputs; n++)
				ostates[n] = node->outputs[n];
			return;
		}
	}	

	jive_node * node = cls->create(region, attrs, nstates+2, operands);
	for(n = 0; n < node->noutputs; n++)
		ostates[n] = node->outputs[n];
}

static void
jive_store_node_set_reducible_(jive_store_node_normal_form * self, bool enable)
{
	if (self->enable_reducible == enable)
		return;

	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->base.subclasses, child, normal_form_subclass_list) {
		jive_store_node_set_reducible((jive_store_node_normal_form *)child, enable);
	}
	
	self->enable_reducible = enable;
	if (self->base.enable_mutable && self->enable_reducible)
		jive_graph_mark_denormalized(self->base.graph);
}

const jive_store_node_normal_form_class JIVE_STORE_NODE_NORMAL_FORM_ = {
	base : { /* jive_node_normal_form_class */
		parent : &JIVE_NODE_NORMAL_FORM,
		fini : jive_node_normal_form_fini_, /* inherit */
		normalize_node : jive_store_node_normalize_node_, /* override */
		operands_are_normalized : jive_store_node_operands_are_normalized_, /* override */
		normalized_create : NULL, /* inherit */
		set_mutable : jive_node_normal_form_set_mutable_, /* inherit */
		set_cse : jive_node_normal_form_set_cse_, /* inherit */
	},
	set_reducible : jive_store_node_set_reducible_,
	normalized_create : jive_store_node_normalized_create_
};

/* store_node */

static void
jive_store_node_fini_(jive_node * self_);

static jive_node_normal_form *
jive_store_node_get_default_normal_form_(const jive_node_class * cls,
	jive_node_normal_form * parent_, struct jive_graph * graph);

static const jive_node_attrs *
jive_store_node_get_attrs_(const jive_node * self_);

static bool
jive_store_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static void
jive_store_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

static jive_node *
jive_store_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_STORE_NODE = {
	parent : &JIVE_NODE,
	name : "STORE",
	fini : jive_store_node_fini_, /* override */
	get_default_normal_form : jive_store_node_get_default_normal_form_, /* override */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : jive_store_node_get_attrs_, /* override */
	match_attrs : jive_store_node_match_attrs_, /* override */
	check_operands : jive_store_node_check_operands_, /* override */
	create : jive_store_node_create_, /* override */
};

static void
jive_store_node_fini_(jive_node * self_)
{
	jive_context * context = self_->graph->context;
	jive_store_node * self = (jive_store_node *) self_;

	jive_type_destroy(self->attrs.datatype);
	
	jive_node_fini_(self);
}

static jive_node_normal_form *
jive_store_node_get_default_normal_form_(const jive_node_class * cls,
	jive_node_normal_form * parent_, struct jive_graph * graph)
{
	jive_context * context = graph->context;
	jive_store_node_normal_form * nf = jive_context_malloc(context, sizeof(*nf));

	jive_node_normal_form_init_(&nf->base, cls, parent_, graph);
	nf->base.class_ = &JIVE_STORE_NODE_NORMAL_FORM;
	nf->enable_reducible = true;

	return &nf->base;
}

static const jive_node_attrs *
jive_store_node_get_attrs_(const jive_node * self_)
{
	const jive_store_node * self = (const jive_store_node *) self_;
	return &self->attrs;
}

static bool
jive_store_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_store_node * self = (const jive_store_node *) self_;
	const jive_store_node_attrs * attrs = (const jive_store_node_attrs *) attrs_;
	bool dtype = jive_type_equals(self->attrs.datatype, attrs->datatype);
	return (dtype && (self->attrs.nbits == attrs->nbits));
}

static void
jive_store_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	JIVE_DEBUG_ASSERT(noperands > 1);

	const jive_address_output * addro = jive_address_output_const_cast(operands[0]);
	const jive_bitstring_output * bitso = jive_bitstring_output_const_cast(operands[0]);

	if (!addro && !bitso)
		jive_context_fatal_error(context, "Type mismatch: required address or bitstring type.");

	JIVE_DECLARE_VALUE_TYPE(vtype);
	if (!jive_output_isinstance(operands[1], &JIVE_VALUE_OUTPUT))
		jive_raise_type_error(vtype, jive_output_get_type(operands[1]), context);

	/* FIXME: check the type of the states */
}

static jive_node *
jive_store_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_store_node_attrs * attrs = (const jive_store_node_attrs *) attrs_;

	if(jive_output_isinstance(operands[0], &JIVE_BITSTRING_OUTPUT)){
		return jive_store_by_bitstring_node_create(region, operands[0], attrs->nbits, attrs->datatype,
			operands[1], noperands-2, &operands[2]);
	} else {
		return jive_store_by_address_node_create(region, operands[0], attrs->datatype, operands[1],
			noperands-2, &operands[2]);
	}
}

void
jive_store_node_init_(jive_store_node * self, jive_region * region,
	jive_output * address, const jive_type * address_type,
	const jive_value_type * datatype, jive_output * value,
	size_t nstates, jive_output * const states[])
{
	const jive_type * operand_types[2] = {address_type, datatype};
	jive_output * operands[2] = {address, value};
	
	jive_node_init_(self, region,
		2, operand_types, operands,
		0, NULL);
	self->attrs.datatype = (jive_value_type *) jive_type_copy(datatype);
	self->attrs.nbits = 0;
	if (jive_type_isinstance(address_type, &JIVE_BITSTRING_TYPE)) {
		const jive_bitstring_type * btype = (const jive_bitstring_type *) address_type;
		self->attrs.nbits = btype->nbits;
	}

	/* FIXME: check the type of the states */	
	size_t n;
	for (n = 0; n < nstates; n++) {
		const jive_type * type = jive_output_get_type(states[n]);
		jive_node_add_input(self, type, states[n]);
		jive_node_add_output(self, type);
	}
}

static inline jive_region *
store_node_region_innermost(jive_output * address, jive_output * value,
	size_t nstates, jive_output * const states[])
{
	size_t i;
	jive_output * outputs[nstates+2];
	for(i = 0; i < nstates; i++){
		outputs[i] = states[i];
	}
	outputs[nstates] = address;
	outputs[nstates+1] = value;
	
	return jive_region_innermost(nstates+2, outputs);
}

jive_node *
jive_store_by_address_node_create(jive_region * region, jive_output * address,
	const jive_value_type * datatype, jive_output * value,
	size_t nstates, jive_output * const states[])
{
	jive_store_node * node = new jive_store_node;
	
	node->class_ = &JIVE_STORE_NODE;
	JIVE_DECLARE_ADDRESS_TYPE(address_type);
	jive_store_node_init_(node, region, address, address_type, datatype, value, nstates, states);
	
	return node;
}

void
jive_store_by_address_create(jive_output * address,
	const jive_value_type * datatype, jive_output * value,
	size_t nstates, jive_output * const istates[], jive_output * ostates[])
{
	const jive_store_node_normal_form * nf = (const jive_store_node_normal_form *)
		jive_graph_get_nodeclass_form(address->node->region->graph, &JIVE_STORE_NODE);

	jive_region * region = store_node_region_innermost(address, value, nstates, istates);

	jive_store_node_attrs attrs;
	attrs.nbits = 0;
	attrs.datatype = (jive_value_type *) datatype;

	jive_store_node_normalized_create(nf, region, &attrs, address, value,
		nstates, istates, ostates); 
}

jive_node *
jive_store_by_bitstring_node_create(jive_region * region,
	jive_output * address, size_t nbits,
	const jive_value_type * datatype, jive_output * value,
	size_t nstates, jive_output * const states[])
{
	jive_store_node * node = new jive_store_node;

	node->class_ = &JIVE_STORE_NODE;
	JIVE_DECLARE_BITSTRING_TYPE(address_type, nbits);
	jive_store_node_init_(node, region, address, address_type, datatype, value, nstates, states);

	return node;
}

void
jive_store_by_bitstring_create(jive_output * address, size_t nbits,
	const jive_value_type * datatype, jive_output * value,
	size_t nstates, jive_output * const istates[], jive_output * ostates[])
{
	const jive_store_node_normal_form * nf = (const jive_store_node_normal_form *)
		jive_graph_get_nodeclass_form(address->node->region->graph, &JIVE_STORE_NODE);

	jive_region * region = store_node_region_innermost(address, value, nstates, istates);

	jive_store_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.datatype = (jive_value_type *) datatype;

	jive_store_node_normalized_create(nf, region, &attrs, address, value,
		nstates, istates, ostates);
}

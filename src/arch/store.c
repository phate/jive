/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/store.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/memorytype.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/record/rcdgroup.h>
#include <jive/types/union/unntype.h>
#include <jive/types/union/unnunify.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/valuetype.h>

namespace jive {

store_op::~store_op() noexcept
{
}

bool
store_op::operator==(const operation & other) const noexcept
{
	const store_op * op =
		dynamic_cast<const store_op *>(&other);
	return (
		op &&
		op->address_type() == address_type() &&
		op->data_type() == data_type() &&
		detail::ptr_container_equals(op->state_types(), state_types())
	);
}

size_t
store_op::narguments() const noexcept
{
	return 2 + state_types().size();
}

const jive::base::type &
store_op::argument_type(size_t index) const noexcept
{
	if (index == 0) {
		return address_type();
	} else if (index == 1) {
		return data_type();
	} else {
		return *state_types()[index - 2];
	}
}

size_t
store_op::nresults() const noexcept
{
	return state_types().size();
}

const jive::base::type &
store_op::result_type(size_t index) const noexcept
{
	return *state_types()[index];
}

jive_node *
store_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(
		*this,
		&JIVE_STORE_NODE,
		region,
		arguments, arguments + narguments);
}

std::string
store_op::debug_string() const
{
	return "STORE";
}

std::unique_ptr<jive::operation>
store_op::copy() const
{
	return std::unique_ptr<jive::operation>(new store_op(*this));
}

}

/* store node normal form */

static bool
jive_store_node_normalize_node_(const jive_node_normal_form * self_, jive_node * node)
{
	const jive_store_node_normal_form * self = (const jive_store_node_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;

	const jive_node_attrs * attrs = jive_node_get_attrs(node);

	if (self->base.enable_cse) {
		size_t n;
		jive::output * operands[node->ninputs];
		for(n = 0; n < node->ninputs; n++)
			operands[n] = node->inputs[n]->origin();

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
	jive::output * const operands[], const jive_node_attrs * attrs)
{
	const jive_store_node_normal_form * self = (const jive_store_node_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;

	jive_region * region = operands[0]->node()->region;
	const jive_node_class * cls = self->base.node_class;

	if (self->base.enable_cse && jive_node_cse(region, cls, attrs, noperands, operands))
		return false;

	return true;
}

static std::vector<jive::output *>
jive_store_node_normalized_create_(
	const jive_store_node_normal_form * self,
	jive_region * region,
	const jive::store_op & op,
	jive::output * address,
	jive::output * value,
	size_t nstates, jive::output * const istates[])
{
	size_t narguments = nstates + 2;
	jive::output * arguments[narguments];
	arguments[0] = address;
	arguments[1] = value;
	for (size_t n = 0; n < nstates; n++) {
		arguments[n+2] = istates[n];
	}

	const jive_node_class * cls = self->base.node_class;

	jive_node * node = nullptr;

	if (self->base.enable_mutable && self->base.enable_cse) {
		node = jive_node_cse(region, cls, &op, narguments, arguments);
	}

	if (!node) {
		node = op.create_node(region, narguments, arguments);
	}

	return std::vector<jive::output *>(node->outputs);
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

const jive_node_class JIVE_STORE_NODE = {
	parent : &JIVE_NODE,
	name : "STORE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_store_node_get_default_normal_form_, /* override */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

static jive_node_normal_form *
jive_store_node_get_default_normal_form_(const jive_node_class * cls,
	jive_node_normal_form * parent_, struct jive_graph * graph)
{
	jive_context * context = graph->context;
	jive_store_node_normal_form * nf = new jive_store_node_normal_form;

	jive_node_normal_form_init_(&nf->base, cls, parent_, graph);
	nf->base.class_ = &JIVE_STORE_NODE_NORMAL_FORM;
	nf->enable_reducible = true;

	return &nf->base;
}

static inline jive_region *
store_node_region_innermost(jive::output * address, jive::output * value,
	size_t nstates, jive::output * const states[])
{
	size_t i;
	jive::output * outputs[nstates+2];
	for(i = 0; i < nstates; i++){
		outputs[i] = states[i];
	}
	outputs[nstates] = address;
	outputs[nstates+1] = value;

	return jive_region_innermost(nstates+2, outputs);
}

std::vector<jive::output *>
jive_store_by_address_create(jive::output * address,
	const jive::value::type * datatype, jive::output * value,
	size_t nstates, jive::output * const istates[])
{
	const jive_store_node_normal_form * nf = (const jive_store_node_normal_form *)
		jive_graph_get_nodeclass_form(address->node()->region->graph, &JIVE_STORE_NODE);
	
	jive_region * region = store_node_region_innermost(address, value, nstates, istates);

	std::vector<std::unique_ptr<jive::state::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::state::type &>(istates[n]->type()).copy());
	}

	jive::store_op op(jive::addr::type(), state_types, *datatype);

	return jive_store_node_normalized_create_(nf, region, op, address, value, nstates, istates);
}

std::vector<jive::output *>
jive_store_by_bitstring_create(jive::output * address, size_t nbits,
	const jive::value::type * datatype, jive::output * value,
	size_t nstates, jive::output * const istates[])
{
	const jive_store_node_normal_form * nf = (const jive_store_node_normal_form *)
		jive_graph_get_nodeclass_form(address->node()->region->graph, &JIVE_STORE_NODE);
	
	jive_region * region = store_node_region_innermost(address, value, nstates, istates);

	std::vector<std::unique_ptr<jive::state::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::state::type &>(istates[n]->type()).copy());
	}

	jive::store_op op(jive::bits::type(nbits), state_types, *datatype);

	return jive_store_node_normalized_create_(nf, region, op, address, value, nstates, istates);
}

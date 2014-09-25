/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/load.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/store.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/valuetype.h>

namespace jive {

load_op::~load_op() noexcept
{
}

bool
load_op::operator==(const operation & other) const noexcept
{
	const load_op * op =
		dynamic_cast<const load_op *>(&other);
	return (
		op &&
		op->address_type() == address_type() &&
		op->data_type() == data_type() &&
		detail::ptr_container_equals(op->state_types(), state_types())
	);
}

size_t
load_op::narguments() const noexcept
{
	return 1 + state_types().size();
}

const jive::base::type &
load_op::argument_type(size_t index) const noexcept
{
	if (index == 0) {
		return address_type();
	} else {
		return *state_types()[index - 1];
	}
}

size_t
load_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
load_op::result_type(size_t index) const noexcept
{
	return data_type();
}

jive_node *
load_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(
		*this,
		&JIVE_LOAD_NODE,
		region,
		arguments, arguments + narguments);
}

std::string
load_op::debug_string() const
{
	return "LOAD";
}

}

/* load_node_normal_form */

static inline bool
store_reduce(jive::output * address, size_t nstates, jive::output * const states[])
{
	if (nstates == 0)
		return false;

	jive_node * store = states[0]->node();
	if (!jive_node_isinstance(store, &JIVE_STORE_NODE))
		return false;

	size_t n;
	for (n = 0; n < nstates; n++) {
		if (states[n]->node() != store)
			return false;
	}

	if (store->inputs[0]->origin() != address)
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
		jive::output * states[nstates];

		size_t n;
		for (n = 0; n < nstates; n++)
			states[n] = node->inputs[n+1]->origin();

		if (store_reduce(node->inputs[0]->origin(), nstates, states)) {
			jive_node * store = node->producer(1);
			jive_output_replace(node->outputs[0], store->inputs[1]->origin());
			/* FIXME: not sure whether "destroy" is really appropriate? */
			jive_node_destroy(node);
			return false;
		}
	}

	if (self->base.enable_cse) {
		size_t n;
		jive::output * operands[node->ninputs];
		for (n = 0; n < node->ninputs; n++)
			operands[n] = node->inputs[n]->origin();

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
	jive::output * const operands[], const jive_node_attrs * attrs)
{
	const jive_load_node_normal_form * self = (const jive_load_node_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;

	jive_region * region = operands[0]->node()->region;
	const jive_node_class * cls = self->base.node_class;

	if (self->enable_reducible) {
		if (store_reduce(operands[0], noperands-1, &operands[1]))
			return false;
	}

	if (self->base.enable_cse && jive_node_cse(region, cls, attrs, noperands, operands))
		return false;

	return true;
}

static jive::output *
jive_load_node_normalized_create_(const jive_load_node_normal_form * self,
	struct jive_region * region, const jive::operation * op, jive::output * address,
	size_t nstates, jive::output * const states[])
{
	size_t n;
	size_t noperands = nstates+1;
	jive::output * operands[noperands];
	operands[0] = address;
	for(n = 0; n < nstates; n++)
		operands[n+1] = states[n];

	const jive_node_class * cls = self->base.node_class;

	if (self->base.enable_mutable && self->enable_reducible) {
		if (store_reduce(address, nstates, states))
			return states[0]->node()->inputs[1]->origin();
	}

	if (self->base.enable_mutable && self->base.enable_cse) {
		jive_node * node = jive_node_cse(region, cls, op, noperands, operands);
		if (node)
			return node->outputs[0];
	}

	return op->create_node(region, nstates+1, operands)->outputs[0];
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

static jive_node_normal_form *
jive_load_node_get_default_normal_form_(const jive_node_class * cls,
	jive_node_normal_form * parent_, struct jive_graph * graph);

const jive_node_class JIVE_LOAD_NODE = {
	parent : &JIVE_NODE,
	name : "LOAD",
	fini : jive_node_fini_,
	get_default_normal_form : jive_load_node_get_default_normal_form_, /* override */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

static jive_node_normal_form *
jive_load_node_get_default_normal_form_(const jive_node_class * cls,
	jive_node_normal_form * parent_, struct jive_graph * graph)
{
	jive_context * context = graph->context;
	jive_load_node_normal_form * nf = new jive_load_node_normal_form;

	jive_node_normal_form_init_(&nf->base, cls, parent_, graph);
	nf->base.class_ = &JIVE_LOAD_NODE_NORMAL_FORM;
	nf->enable_reducible = true;

	return &nf->base;
}

namespace {

template<typename T>
class ptr_array {
public:
	typedef const T value_type;
	typedef const T * iterator;

	inline ptr_array(const T * begin, const T * end) noexcept
		: begin_(begin), end_(end)
	{
	}

	inline iterator begin() const { return begin_; }
	inline iterator end() const { return end_; }

private:
	const T * begin_;
	const T * end_;
};

template<typename T>
inline ptr_array<T>
make_ptr_array(const T * begin, const T * end)
{
	return ptr_array<T>(begin, end);
}

}

static inline jive_region *
load_node_region_innermost(jive::output * address, size_t nstates, jive::output * const states[])
{
	size_t i;
	jive::output * outputs[nstates+1];
	for(i = 0; i < nstates; i++){
		outputs[i] = states[i];
	}
	outputs[nstates] = address;
	
	return jive_region_innermost(nstates+1, outputs);
}

jive::output *
jive_load_by_address_create(jive::output * address,
	const jive::value::type * datatype,
	size_t nstates, jive::output * const states[])
{
	const jive_load_node_normal_form * nf = (const jive_load_node_normal_form *)
		jive_graph_get_nodeclass_form(address->node()->region->graph, &JIVE_LOAD_NODE);
	
	jive_region * region = load_node_region_innermost(address, nstates, states);

	std::vector<std::unique_ptr<jive::state::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::state::type &>(states[n]->type()).copy());
	}

	jive::load_op op(jive::addr::type(), state_types, *datatype);

	return jive_load_node_normalized_create(nf, region, &op, address, nstates, states);
}

jive::output *
jive_load_by_bitstring_create(jive::output * address, size_t nbits,
	const jive::value::type * datatype,
	size_t nstates, jive::output * const states[])
{
	const jive_load_node_normal_form * nf = (const jive_load_node_normal_form *)
		jive_graph_get_nodeclass_form(address->node()->region->graph, &JIVE_LOAD_NODE);
	
	jive_region * region = load_node_region_innermost(address, nstates, states);

	std::vector<std::unique_ptr<jive::state::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::state::type &>(states[n]->type()).copy());
	}

	jive::load_op op(jive::bits::type(nbits), state_types, *datatype);

	return jive_load_node_normalized_create(nf, region, &op, address, nstates, states);
}


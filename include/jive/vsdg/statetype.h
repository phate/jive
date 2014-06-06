/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_STATETYPE_H
#define JIVE_VSDG_STATETYPE_H

#include <memory>

#include <jive/vsdg/basetype.h>
#include <jive/vsdg/node.h>

namespace jive {
namespace state {

class type : public jive_type {
public:
	virtual ~type() noexcept;

	virtual jive::state::type * copy() const override = 0;

protected:
	inline constexpr type() noexcept : jive_type() {};
};

class input : public jive_input {
public:
	virtual ~input() noexcept;

protected:
	input(struct jive_node * node, size_t index, jive_output * origin);
};

class output : public jive_output {
public:
	virtual ~output() noexcept;

protected:
	output(struct jive_node * node, size_t index);
};

class gate : public jive_gate {
public:
	virtual ~gate() noexcept;

protected:
	gate(jive_graph * graph, const char name[]);
};

}

/* state multiplexing support */

class statemux_operation final : public operation {
public:
	inline statemux_operation(
		size_t noutputs,
		const jive_type & type)
		: noutputs_(noutputs)
		, type_(type.copy())
	{
	}

	inline statemux_operation(
		const statemux_operation & other)
		: noutputs_(other.noutputs())
		, type_(other.type().copy())
	{
	}

	inline statemux_operation(
		statemux_operation && other) noexcept = default;

	inline size_t noutputs() const noexcept { return noutputs_; }
	inline const jive_type & type() const noexcept { return *type_; }
private:
	size_t noutputs_;
	std::unique_ptr<jive_type> type_;
};

}

typedef jive::operation_node<jive::statemux_operation> jive_statemux_node;

jive_node *
jive_statemux_node_create(struct jive_region * region,
	const jive_type * statetype,
	size_t noperands, jive_output * const operands[],
	size_t noutputs);

jive_output *
jive_state_merge(const jive_type * statetype, size_t nstates, jive_output * const states[]);

jive_node *
jive_state_split(const jive_type * statetype, jive_output * state, size_t nstates);

#endif

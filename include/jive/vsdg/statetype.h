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

class jive_state_type : public jive_type {
public:
	virtual ~jive_state_type() noexcept;

protected:
	inline constexpr jive_state_type() noexcept : jive_type() {};
};

class jive_state_input : public jive_input {
public:
	virtual ~jive_state_input() noexcept;

protected:
	jive_state_input(struct jive_node * node, size_t index, jive_output * origin);
};

class jive_state_output : public jive_output {
public:
	virtual ~jive_state_output() noexcept;

protected:
	jive_state_output(struct jive_node * node, size_t index);
};

class jive_state_gate : public jive_gate {
public:
	virtual ~jive_state_gate() noexcept;

protected:
	jive_state_gate(jive_graph * graph, const char name[]);
};

/* state multiplexing support */

namespace jive {

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

/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_RVSDG_GATE_H
#define JIVE_RVSDG_GATE_H

#include <jive/rvsdg/node.h>
#include <jive/rvsdg/resource.h>
#include <jive/util/intrusive-hash.h>
#include <jive/util/intrusive-list.h>

/* gate interference */

namespace jive {
	class gate;
	class graph;
}

struct jive_gate_interference;

typedef struct jive_gate_interference_part jive_gate_interference_part;
struct jive_gate_interference_part {
	jive::gate * gate;
	jive_gate_interference * whole;
private:
	jive::detail::intrusive_hash_anchor<jive_gate_interference_part> hash_chain;
public:
	typedef jive::detail::intrusive_hash_accessor<
		jive::gate*,
		jive_gate_interference_part,
		&jive_gate_interference_part::gate,
		&jive_gate_interference_part::hash_chain
	> hash_chain_accessor;
};

typedef struct jive_gate_interference jive_gate_interference;
struct jive_gate_interference {
	jive_gate_interference_part first;
	jive_gate_interference_part second;
	size_t count;
};

typedef jive::detail::intrusive_hash<
	const jive::gate *,
	jive_gate_interference_part,
	jive_gate_interference_part::hash_chain_accessor
> jive_gate_interference_hash;

namespace jive {

class input;
class output;
class resource_class;
class type;

/* gate */

typedef jive::detail::intrusive_list<
	jive::input,
	jive::input::gate_input_accessor
> gate_input_list;

typedef jive::detail::intrusive_list<
	jive::output,
	jive::output::gate_output_accessor
> gate_output_list;

class gate final {
	jive::detail::intrusive_list_anchor<
		jive::gate
	> graph_gate_anchor_;

public:
	~gate() noexcept;

	typedef jive::detail::intrusive_list_accessor<
		jive::gate,
		&jive::gate::graph_gate_anchor_
	> graph_gate_accessor;

private:
	gate(
		jive::graph * graph,
		const std::string & name,
		const jive::type & type);

	gate(
		jive::graph * graph,
		const std::string & name,
		const jive::resource_class * rescls);

	gate(const gate&) = delete;

	gate(gate&&) = delete;

	gate &
	operator=(const gate&) = delete;

	gate &
	operator=(gate&&) = delete;

public:
	static inline gate *
	create(
		jive::graph * graph,
		const std::string & name,
		const jive::type & type)
	{
		return new jive::gate(graph, name, type);
	}

	static inline gate *
	create(
		jive::graph * graph,
		const std::string & name,
		const jive::resource_class * rescls)
	{
		return new jive::gate(graph, name, rescls);
	}

	static inline gate *
	create(jive::graph * graph, const jive::gate * gate)
	{
		if (gate->rescls() != &jive_root_resource_class)
			return create(graph, gate->name(), gate->rescls());

		return create(graph, gate->name(), gate->type());
	}

	const jive::type &
	type() const noexcept
	{
		return *type_;
	}

	inline std::string
	debug_string() const
	{
		return name();
	}

	inline jive::graph *
	graph() const noexcept
	{
		return graph_;
	}

	inline const std::string &
	name() const noexcept
	{
		return name_;
	}

	inline const jive::resource_class *
	rescls() const noexcept
	{
		return rescls_;
	}

	void
	add_interference(jive::gate * other);

	void
	clear_interferences();

	gate_input_list inputs;

	gate_output_list outputs;

private:
	std::string name_;
	jive::graph * graph_;
	std::unique_ptr<jive::type> type_;
	const jive::resource_class * rescls_;
	jive_gate_interference_hash interference_;

};

}

#endif

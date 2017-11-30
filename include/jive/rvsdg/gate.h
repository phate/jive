/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_RVSDG_GATE_H
#define JIVE_RVSDG_GATE_H

#include <jive/util/intrusive-hash.h>

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

class gate final {
public:
	~gate() noexcept;

	gate(
		jive::graph * graph,
		const std::string & name,
		const jive::type & type);

	gate(
		jive::graph * graph,
		const std::string & name,
		const struct jive::resource_class * rescls);

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

	inline const struct jive::resource_class *
	rescls() const noexcept
	{
		return rescls_;
	}

	void
	add_interference(jive::gate * other);

	void
	clear_interferences();

	struct {
		jive::gate * prev;
		jive::gate * next;
	} graph_gate_list;

	struct {
		jive::input * first;
		jive::input * last;
	} inputs;

	struct {
		jive::output * first;
		jive::output * last;
	}	outputs;

private:
	std::string name_;
	jive::graph * graph_;
	jive_gate_interference_hash interference_;
	const struct jive::resource_class * rescls_;

	/*
		FIXME: This attribute is necessary as long as the number of inputs do not coincide with the
		number given by the operation. Once this is fixed, the attribute can be removed and the type
		can be taken from the operation.
	*/
	std::unique_ptr<jive::type> type_;
};

}

#endif

/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_GATE_H
#define JIVE_VSDG_GATE_H

#include <jive/vsdg/gate-interference.h>

namespace jive {
namespace base {
	class type;
}

class graph;
class input;
class output;
class resource_class;

class gate final {
public:
	~gate() noexcept;

	gate(
		jive::graph * graph,
		const std::string & name,
		const jive::base::type & type);

	gate(
		jive::graph * graph,
		const std::string & name,
		const struct jive::resource_class * rescls);

	const jive::base::type &
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

	bool may_spill;
	jive_gate_interference_hash interference;

private:
	std::string name_;
	jive::graph * graph_;
	const struct jive::resource_class * rescls_;

	/*
		FIXME: This attribute is necessary as long as the number of inputs do not coincide with the
		number given by the operation. Once this is fixed, the attribute can be removed and the type
		can be taken from the operation.
	*/
	std::unique_ptr<jive::base::type> type_;
};

}

#endif

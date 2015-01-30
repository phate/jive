/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_STATETYPE_H
#define JIVE_VSDG_STATETYPE_H

#include <memory>
#include <vector>

#include <jive/vsdg/basetype.h>
#include <jive/vsdg/node.h>

namespace jive {
namespace state {

class type : public jive::base::type {
public:
	virtual ~type() noexcept;

	virtual jive::state::type * copy() const override = 0;

protected:
	inline constexpr type() noexcept : jive::base::type() {};
};

class input : public jive::input {
public:
	virtual ~input() noexcept;

protected:
	inline
	input(
		struct jive_node * node,
		size_t index,
		jive::output * origin,
		const jive::base::type & type)
	: jive::input(node, index, origin, type)
	{}

private:
	input(const input & rhs) = delete;
	input& operator=(const input & rhs) = delete;
};

class output : public jive::output {
public:
	virtual ~output() noexcept;

protected:
	output(struct jive_node * node, size_t index);

private:
	output(const output & rhs) = delete;
	output& operator=(const output & rhs) = delete;
};

class gate : public jive::gate {
public:
	virtual ~gate() noexcept;

protected:
	gate(jive_graph * graph, const char name[]);

private:
	gate(const output & rhs) = delete;
	gate& operator=(const output & rhs) = delete;
};

class mux_op final : public operation {
public:
	virtual
	~mux_op() noexcept;

	inline
	mux_op(const type & state_type, size_t narguments, size_t nresults)
		: state_type_(state_type.copy())
		, narguments_(narguments)
		, nresults_(nresults)
	{
	}

	inline
	mux_op(const mux_op & other)
		: state_type_(other.state_type_->copy())
		, narguments_(other.narguments_)
		, nresults_(other.nresults_)
	{
	}

	inline
	mux_op(mux_op && other) = default;

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;
	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::unique_ptr<type> state_type_;
	size_t narguments_;
	size_t nresults_;
};

}
}

jive::output *
jive_state_merge(
	const jive::state::type * statetype,
	size_t nstates,
	jive::output * const states[]);

std::vector<jive::output *>
jive_state_split(
	const jive::state::type * statetype,
	jive::output * state,
	size_t nstates);


// FIXME: temporary overloads below, until it has been made syntactically sure
// that only state types are passed as arguments
static inline jive::output *
jive_state_merge(
	const jive::base::type * statetype,
	size_t nstates,
	jive::output * const states[])
{
	return jive_state_merge(&dynamic_cast<const jive::state::type &>(*statetype), nstates, states);
}

static inline std::vector<jive::output *>
jive_state_split(
	const jive::base::type * statetype,
	jive::output * state,
	size_t nstates)
{
	return jive_state_split(&dynamic_cast<const jive::state::type &>(*statetype), state, nstates);
}


#endif

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
#include <jive/vsdg/operators/simple.h>

namespace jive {
namespace state {

class mux_op final : public simple_op {
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
	std::unique_ptr<base::type> state_type_;
	size_t narguments_;
	size_t nresults_;
};

}
}

jive::oport*
jive_state_merge(
	const jive::state::type * statetype,
	size_t nstates,
	jive::oport * const states[]);

std::vector<jive::oport*>
jive_state_split(
	const jive::state::type * statetype,
	jive::oport * state,
	size_t nstates);


// FIXME: temporary overloads below, until it has been made syntactically sure
// that only state types are passed as arguments
static inline jive::oport*
jive_state_merge(
	const jive::base::type * statetype,
	size_t nstates,
	jive::oport * const states[])
{
	return jive_state_merge(&dynamic_cast<const jive::state::type &>(*statetype), nstates, states);
}

static inline std::vector<jive::oport*>
jive_state_split(
	const jive::base::type * statetype,
	jive::oport * state,
	size_t nstates)
{
	return jive_state_split(&dynamic_cast<const jive::state::type &>(*statetype), state, nstates);
}


#endif

/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDGROUP_H
#define JIVE_TYPES_RECORD_RCDGROUP_H

#include <jive/types/record/rcdtype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/simple.h>

namespace jive {
namespace rcd {

class group_op final : public jive::simple_op {
public:
	virtual
	~group_op() noexcept;

	inline
	group_op(std::shared_ptr<const rcd::declaration> & declaration) noexcept
		: result_type_(declaration)
	{}

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

	inline const std::shared_ptr<const rcd::declaration> &
	declaration() const noexcept
	{
		return result_type_.declaration();
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	const jive::rcd::type result_type_;
};

}
}

jive::output *
jive_group_create(std::shared_ptr<const jive::rcd::declaration> & decl,
	size_t narguments, jive::output * const * arguments);

jive::output *
jive_empty_group_create(jive::graph * graph, std::shared_ptr<const jive::rcd::declaration> & decl);

#endif

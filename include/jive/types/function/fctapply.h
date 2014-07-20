/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTAPPLY_H
#define JIVE_TYPES_FUNCTION_FCTAPPLY_H

#include <jive/types/function/fcttype.h>
#include <jive/vsdg/node.h>

namespace jive {
namespace fct {

class apply_op final : public jive::operation {
public:
	virtual
	~apply_op() noexcept;

	explicit apply_op(const type & function_type);

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

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	inline const type &
	function_type() const noexcept { return function_type_; }

private:
	type function_type_;
	
};

}
}

typedef jive::operation_node<jive::fct::apply_op> jive_apply_node;

extern const jive_node_class JIVE_APPLY_NODE;

std::vector<jive::output *>
jive_apply_create(jive::output * function, size_t narguments, jive::output * const arguments[]);

#endif

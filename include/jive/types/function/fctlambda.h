/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTLAMBDA_H
#define JIVE_TYPES_FUNCTION_FCTLAMBDA_H

#include <memory>
#include <vector>

#include <jive/types/function/fcttype.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/structural.h>
#include <jive/vsdg/structural_node.h>

namespace jive {
namespace fct {

class lambda_op final : public structural_op {
public:
	virtual
	~lambda_op() noexcept;

	inline
	lambda_op(const lambda_op & other) = default;

	inline
	lambda_op(lambda_op && other) = default;

	inline
	lambda_op(jive::fct::type function_type) noexcept
		: function_type_(std::move(function_type))
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;
	virtual std::string
	debug_string() const override;

	inline const jive::fct::type &
	function_type() const noexcept
	{
		return function_type_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	jive::fct::type function_type_;
};

}
}

bool
jive_lambda_is_self_recursive(const jive::node * self);

void
jive_inline_lambda_apply(jive::node * apply_node);

namespace jive {

class argument;

class lambda_builder final {
public:
	inline
	lambda_builder() noexcept
	: node_(nullptr)
	{}

	inline jive::region *
	begin(jive::region * parent, jive::fct::type type)
	{
		if (node_)
			node_->subregion(0);

		std::vector<jive::argument*> arguments;
		node_ = parent->add_structural_node(jive::fct::lambda_op(std::move(type)), 1);
		for (size_t n = 0; n < type.narguments(); n++)
			arguments.push_back(node_->subregion(0)->add_argument(nullptr, type.argument_type(n)));

		return node_->subregion(0);
	}

	inline jive::region *
	region() const noexcept
	{
		return node_ ? node_->subregion(0) : nullptr;
	}

	inline jive::output *
	add_dependency(jive::output * value)
	{
		if (!node_)
			return nullptr;

		auto input = node_->add_input(&value->type(), value);
		return node_->subregion(0)->add_argument(input, value->type());
	}

	inline jive::structural_node *
	end(const std::vector<jive::output*> & results)
	{
		if (!node_)
			return nullptr;

		const auto & op = node_->operation();
		const auto & ftype = static_cast<const jive::fct::type*>(&op.result_type(0));
		if (results.size() != ftype->nresults())
			throw jive::compiler_error("Incorrect number of results.");

		for (size_t n = 0; n < results.size(); n++)
			node_->subregion(0)->add_result(results[n], nullptr, ftype->result_type(n));
		node_->add_output(ftype);

		auto node = node_;
		node_ = nullptr;
		return node;
	}

private:
	jive::structural_node * node_;
};

}

#endif

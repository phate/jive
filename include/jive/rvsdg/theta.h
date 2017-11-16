/*
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_RVSDG_THETA_H
#define JIVE_RVSDG_THETA_H

#include <jive/rvsdg/control.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/structural-node.h>

namespace jive {

class theta_op final : public structural_op {
public:
	virtual
	~theta_op() noexcept;
	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

static inline bool
is_theta_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::theta_op*>(&op) != nullptr;
}

static inline bool
is_theta_node(const jive::node * node) noexcept
{
	return is_opnode<theta_op>(node);
}

class theta_node;

class loopvar final {
	friend theta_node;

public:
	inline constexpr
	loopvar()
	: input_(nullptr)
	, output_(nullptr)
	{}

private:
	inline constexpr
	loopvar(
		jive::structural_input * input,
		jive::structural_output * output)
	: input_(input)
	, output_(output)
	{}

public:
	inline jive::structural_input *
	input() const noexcept
	{
		return input_;
	}

	inline jive::structural_output *
	output() const noexcept
	{
		return output_;
	}

	inline jive::argument *
	argument() const noexcept
	{
		if (input_ == nullptr)
			return nullptr;

		JIVE_DEBUG_ASSERT(input_->arguments.first != nullptr
		&& input_->arguments.first == input_->arguments.last);
		return input_->arguments.first;
	}

	inline jive::result *
	result() const noexcept
	{
		if (output_ == nullptr)
			return nullptr;

		JIVE_DEBUG_ASSERT(output_->results.first != nullptr
		&& output_->results.first == output_->results.last);
		return output_->results.first;
	}

	inline bool
	operator==(const loopvar & other) const noexcept
	{
		return input_ == other.input_ && output_ == other.output_;
	}

	inline bool
	operator!=(const loopvar & other) const noexcept
	{
		return !(*this == other);
	}

private:
	jive::structural_input * input_;
	jive::structural_output * output_;
};

class theta_node final : public structural_node {
public:
	class loopvar_iterator {
	public:
		inline constexpr
		loopvar_iterator(const jive::loopvar & lv) noexcept
		: lv_(lv)
		{}

		inline const loopvar_iterator &
		operator++() noexcept
		{
			auto input = lv_.input();
			if (input == nullptr)
				return *this;

			auto node = input->node();
			auto index = input->index();
			if (index == node->ninputs()-1) {
				lv_ = loopvar(nullptr, nullptr);
				return *this;
			}

			index++;
			lv_ = loopvar(node->input(index), node->output(index));
			return *this;
		}

		inline const loopvar_iterator
		operator++(int) noexcept
		{
			loopvar_iterator it(*this);
			++(*this);
			return it;
		}

		inline bool
		operator==(const loopvar_iterator & other) const noexcept
		{
			return lv_ == other.lv_;
		}

		inline bool
		operator!=(const loopvar_iterator & other) const noexcept
		{
			return !(*this == other);
		}

		inline loopvar &
		operator*() noexcept
		{
			return lv_;
		}

		inline loopvar *
		operator->() noexcept
		{
			return &lv_;
		}

	private:
		loopvar lv_;
	};

	virtual
	~theta_node();

private:
	inline
	theta_node(jive::region * parent)
	: structural_node(jive::theta_op(), parent, 1)
	{
		auto predicate = jive_control_false(subregion());
		subregion()->add_result(predicate, nullptr, jive::ctl::type(2));
	}

public:
	static jive::theta_node *
	create(jive::region * parent)
	{
		return new jive::theta_node(parent);
	}

	inline jive::region *
	subregion() const noexcept
	{
		return structural_node::subregion(0);
	}

	inline jive::result *
	predicate() const noexcept
	{
		auto result = subregion()->result(0);
		JIVE_DEBUG_ASSERT(dynamic_cast<const jive::ctl::type*>(&result->type()));
		return result;
	}

	inline void
	set_predicate(jive::output * p)
	{
		auto node = predicate()->origin()->node();
		predicate()->divert_origin(p);
		if (node && !node->has_users())
			remove(node);
	}

	inline size_t
	nloopvars() const noexcept
	{
		JIVE_DEBUG_ASSERT(ninputs() == noutputs());
		return ninputs();
	}

	inline theta_node::loopvar_iterator
	begin() const
	{
		if (ninputs() == 0)
			return loopvar_iterator({});

		return loopvar_iterator({input(0), output(0)});
	}

	inline theta_node::loopvar_iterator
	end() const
	{
		return loopvar_iterator({});
	}

	inline std::shared_ptr<jive::loopvar>
	add_loopvar(jive::output * origin)
	{
		auto input = add_input(origin->type(), origin);
		auto output = add_output(origin->type());
		auto argument = subregion()->add_argument(input, origin->type());
		subregion()->add_result(argument, output, origin->type());
		return std::make_shared<loopvar>(loopvar(input, output));
	}
};

}

#endif
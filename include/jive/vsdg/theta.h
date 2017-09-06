/*
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_THETA_H
#define JIVE_VSDG_THETA_H

#include <jive/vsdg/controltype.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/structural_node.h>

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

class theta;
class theta_builder;

class loopvar final {
	friend theta;
	friend theta_builder;

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

class theta_builder final {
public:
	inline
	theta_builder() noexcept
	: node_(nullptr)
	{}

	inline jive::region *
	region() const noexcept
	{
		return node_ ? node_->subregion(0) : nullptr;
	}

	inline jive::region *
	begin(jive::region * parent)
	{
		if (node_)
			return node_->subregion(0);

		node_ = parent->add_structural_node(jive::theta_op(), 1);
		return node_->subregion(0);
	}

	inline std::shared_ptr<jive::loopvar>
	add_loopvar(jive::output * origin)
	{
		if (!node_)
			return nullptr;

		auto input = node_->add_input(origin->type(), origin);
		node_->subregion(0)->add_argument(input, origin->type());
		loopvars_.push_back(std::make_shared<loopvar>(loopvar(input, nullptr)));
		return loopvars_.back();
	}

	inline jive::structural_node *
	end(
		jive::output * predicate,
		const std::unordered_map<std::shared_ptr<loopvar>, jive::output*> & lvmap)
	{
		if (!node_)
			return nullptr;

		node_->subregion(0)->add_result(predicate, nullptr, jive::ctl::type(2));
		for (const auto & lv : loopvars_) {
			auto it = lvmap.find(lv);
			auto value = it != lvmap.end() ? it->second : lv->argument();
			auto output = node_->add_output(lv->input()->type());
			node_->subregion(0)->add_result(value, output, lv->input()->type());
			lv->output_ = output;
		}

		auto node = node_;
		node_ = nullptr;
		loopvars_.clear();

		return node;
	}

private:
	jive::structural_node * node_;
	std::vector<std::shared_ptr<loopvar>> loopvars_;
};

class theta final {
public:
	inline
	theta(jive::structural_node * node)
	: node_(node)
	{
		if (!dynamic_cast<const jive::theta_op*>(&node->operation()))
			throw jive::compiler_error("Expected theta node.");
	}

private:
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
			auto new_input = static_cast<jive::structural_input*>(node->input(index));
			auto new_output = static_cast<jive::structural_output*>(node->output(index));
			lv_ = loopvar(new_input, new_output);
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

public:
	inline jive::structural_node *
	node() const noexcept
	{
		return node_;
	}

	inline jive::result *
	predicate() const noexcept
	{
		auto result = node_->subregion(0)->result(0);
		JIVE_DEBUG_ASSERT(dynamic_cast<const jive::ctl::type*>(&result->type()));
		return result;
	}

	inline size_t
	nloopvars() const noexcept
	{
		JIVE_DEBUG_ASSERT(node_->ninputs() == node_->noutputs());
		return node_->ninputs();
	}

	inline theta::loopvar_iterator
	begin() const
	{
		return loopvar_iterator({node_->input(0), node_->output(0)});
	}

	inline theta::loopvar_iterator
	end() const
	{
		return loopvar_iterator({nullptr, nullptr});
	}

private:
	jive::structural_node * node_;
};

}

#endif

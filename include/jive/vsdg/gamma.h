/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_GAMMA_H
#define JIVE_VSDG_GAMMA_H

#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/structural_node.h>

namespace jive {

class output;
class type;

class gamma_op final : public structural_op {
public:
	virtual
	~gamma_op() noexcept;

	inline
	gamma_op(size_t nalternatives) noexcept
		: predicate_type_(nalternatives)
	{
	}

	inline size_t
	nalternatives() const noexcept
	{
		return predicate_type_.nalternatives();
	}

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	virtual bool
	operator==(const operation & other) const noexcept override;

private:
	jive::ctl::type predicate_type_;
};

static inline bool
is_gamma_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::gamma_op*>(&op) != nullptr;
}

class gamma;
class gamma_builder;

class entryvar final {
	friend gamma;
	friend gamma_builder;

private:
	inline constexpr
	entryvar(jive::structural_input * input)
	: input_(input)
	{}

public:
	inline bool
	operator==(const entryvar & other) const noexcept
	{
		return input_ == other.input_;
	}

	inline bool
	operator!=(const entryvar & other) const noexcept
	{
		return !(*this == other);
	}

	inline jive::structural_input *
	input() const noexcept
	{
		return input_;
	}

	inline size_t
	narguments() const noexcept
	{
		return static_cast<const jive::structural_node*>(input()->node())->nsubregions();
	}

	inline jive::argument *
	argument(size_t n) const noexcept
	{
		JIVE_DEBUG_ASSERT(n < narguments());
		auto subregion = static_cast<const jive::structural_node*>(input()->node())->subregion(n);
		auto argument = subregion->argument(input()->index()-1);
		JIVE_DEBUG_ASSERT(argument->input() == input());
		return argument;
	}

private:
	jive::structural_input * input_;
};

class exitvar final {
	friend gamma;

private:
	inline constexpr
	exitvar(jive::structural_output * output)
	: output_(output)
	{}

public:
	inline bool
	operator==(const exitvar & other) const noexcept
	{
		return output_ == other.output_;
	}

	inline bool
	operator!=(const exitvar & other) const noexcept
	{
		return !(*this == other);
	}

	inline jive::structural_output *
	output() const noexcept
	{
		return output_;
	}

	inline size_t
	nresults() const noexcept
	{
		return static_cast<const jive::structural_node*>(output()->node())->nsubregions();
	}

	inline jive::result *
	result(size_t n) const noexcept
	{
		JIVE_DEBUG_ASSERT(n < nresults());
		auto subregion = static_cast<const jive::structural_node*>(output()->node())->subregion(n);
		auto result = subregion->result(output()->index());
		JIVE_DEBUG_ASSERT(result->output() == output());
		return result;
	}

private:
	jive::structural_output * output_;
};

class gamma final {
public:
	inline
	gamma(jive::structural_node * node)
	: node_(node)
	{
		if (!dynamic_cast<const jive::gamma_op*>(&node->operation()))
			throw jive::compiler_error("Expected gamma node.");
	}

private:
	class entryvar_iterator {
	public:
		inline constexpr
		entryvar_iterator(jive::structural_input * input) noexcept
		: var_(input)
		{}

		inline const entryvar_iterator &
		operator++() noexcept
		{
			auto input = var_.input();
			if (input == nullptr)
				return *this;

			auto node = input->node();
			auto index = input->index();
			if (index == node->ninputs()-1) {
				var_ = entryvar(nullptr);
				return *this;
			}

			var_ = entryvar(static_cast<structural_input*>(node->input(++index)));
			return *this;
		}

		inline const entryvar_iterator
		operator++(int) noexcept
		{
			entryvar_iterator it(*this);
			++(*this);
			return it;
		}

		inline bool
		operator==(const entryvar_iterator & other) const noexcept
		{
			return var_ == other.var_;
		}

		inline bool
		operator!=(const entryvar_iterator & other) const noexcept
		{
			return !(*this == other);
		}

		inline entryvar &
		operator*() noexcept
		{
			return var_;
		}

		inline entryvar *
		operator->() noexcept
		{
			return &var_;
		}

	private:
		entryvar var_;
	};

	class exitvar_iterator {
	public:
		inline constexpr
		exitvar_iterator(jive::structural_output * output) noexcept
		: var_(output)
		{}

		inline const exitvar_iterator &
		operator++() noexcept
		{
			auto output = var_.output();
			if (output == nullptr)
				return *this;

			auto node = output->node();
			auto index = output->index();
			if (index == node->noutputs()-1) {
				var_ = exitvar(nullptr);
				return *this;
			}

			var_ = exitvar(static_cast<structural_output*>(node->output(++index)));
			return *this;
		}

		inline const exitvar_iterator
		operator++(int) noexcept
		{
			exitvar_iterator it(*this);
			++(*this);
			return it;
		}

		inline bool
		operator==(const exitvar_iterator & other) const noexcept
		{
			return var_ == other.var_;
		}

		inline bool
		operator!=(const exitvar_iterator & other) const noexcept
		{
			return !(*this == other);
		}

		inline exitvar &
		operator*() noexcept
		{
			return var_;
		}

		inline exitvar *
		operator->() noexcept
		{
			return &var_;
		}

	private:
		exitvar var_;
	};

public:
	inline jive::structural_node *
	node() const noexcept
	{
		return node_;
	}

	inline jive::region *
	region() const noexcept
	{
		return node_->region();
	}

	inline jive::structural_input *
	predicate() const noexcept
	{
		JIVE_DEBUG_ASSERT(dynamic_cast<const jive::ctl::type*>(&node()->input(0)->type()));
		return node()->input(0);
	}

	inline size_t
	nsubregions() const noexcept
	{
		return node_->nsubregions();
	}

	inline jive::region *
	subregion(size_t n) const noexcept
	{
		return node_->subregion(n);
	}

	inline gamma::entryvar_iterator
	begin_entryvar() const
	{
		return entryvar_iterator(node()->input(1));
	}

	inline gamma::entryvar_iterator
	end_entryvar() const
	{
		return entryvar_iterator(nullptr);
	}

	inline gamma::exitvar_iterator
	begin_exitvar() const
	{
		return exitvar_iterator(node()->output(0));
	}

	inline gamma::exitvar_iterator
	end_exitvar() const
	{
		return exitvar_iterator(nullptr);
	}

	inline std::shared_ptr<jive::entryvar>
	add_entryvar(jive::output * origin)
	{
		auto input = node_->add_input(origin->type(), origin);
		for (size_t n = 0; n < nsubregions(); n++)
			node_->subregion(n)->add_argument(input, origin->type());

		return std::shared_ptr<entryvar>(new entryvar(input));
	}

	inline std::shared_ptr<jive::exitvar>
	add_exitvar(const std::vector<jive::output*> & values)
	{
		if (values.size() != nsubregions())
			throw jive::compiler_error("Incorrect number of values.");
		JIVE_DEBUG_ASSERT(values.size() != 0);

		const auto & type = values[0]->type();
		auto output = node_->add_output(type);
		for (size_t n = 0; n < nsubregions(); n++)
			node_->subregion(n)->add_result(values[n], output, type);

		return std::shared_ptr<exitvar>(new exitvar(output));
	}

private:
	jive::structural_node * node_;
};

class gamma_builder final {
public:
	inline size_t
	nsubregions() const noexcept
	{
		return gamma_ ? gamma_->node()->nsubregions() : 0;
	}

	inline jive::region *
	region(size_t n) const noexcept
	{
		return gamma_ ? gamma_->node()->subregion(n) : nullptr;
	}

	inline jive::input *
	predicate() const noexcept
	{
		return gamma_ ? gamma_->node()->input(0) : nullptr;
	}

	inline void
	begin(jive::output * predicate)
	{
		if (gamma_)
			return;

		auto ctl = dynamic_cast<const jive::ctl::type*>(&predicate->type());
		if (!ctl)
			throw jive::type_error("ctl", predicate->type().debug_string());

		auto region = predicate->region();
		size_t nalternatives = ctl->nalternatives();
		auto node = region->add_structural_node(jive::gamma_op(nalternatives), nalternatives);
		node->add_input(*ctl, predicate);
		gamma_ = std::make_unique<gamma>(node);
	}

	inline std::shared_ptr<jive::entryvar>
	add_entryvar(jive::output * origin)
	{
		return gamma_ ? gamma_->add_entryvar(origin) : nullptr;
	}

	inline std::shared_ptr<jive::exitvar>
	add_exitvar(const std::vector<jive::output*> & values)
	{
		return gamma_ ? gamma_->add_exitvar(values) : nullptr;
	}

	inline std::unique_ptr<jive::gamma>
	end()
	{
		return std::move(gamma_);
	}

private:
	std::unique_ptr<jive::gamma> gamma_;
};

}

#endif

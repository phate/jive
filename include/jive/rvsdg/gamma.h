/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_RVSDG_GAMMA_H
#define JIVE_RVSDG_GAMMA_H

#include <jive/rvsdg/controltype.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/structural-node.h>
#include <jive/rvsdg/structural-normal-form.h>

namespace jive {

/* gamma normal form */

class gamma_normal_form final : public structural_normal_form {
public:
	virtual
	~gamma_normal_form() noexcept;

	gamma_normal_form(
		const std::type_info & operator_class,
		jive::node_normal_form * parent,
		jive::graph * graph) noexcept;

	virtual bool
	normalize_node(jive::node * node) const override;

	virtual void
	set_predicate_reduction(bool enable);

	inline bool
	get_predicate_reduction() const noexcept
	{
		return enable_predicate_reduction_;
	}

	virtual void
	set_invariant_reduction(bool enable);

	inline bool
	get_invariant_reduction() const noexcept
	{
		return enable_invariant_reduction_;
	}

	virtual void
	set_control_constant_reduction(bool enable);

	inline bool
	get_control_constant_reduction() const noexcept
	{
		return enable_control_constant_reduction_;
	}

private:
	bool enable_predicate_reduction_;
	bool enable_invariant_reduction_;
	bool enable_control_constant_reduction_;
};

/* gamma operation */

class output;
class type;

class gamma_op final : public structural_op {
public:
	virtual
	~gamma_op() noexcept;

	inline constexpr
	gamma_op(size_t nalternatives) noexcept
	: structural_op()
	, nalternatives_(nalternatives)
	{}

	inline size_t
	nalternatives() const noexcept
	{
		return nalternatives_;
	}

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	virtual bool
	operator==(const operation & other) const noexcept override;

	static jive::gamma_normal_form *
	normal_form(jive::graph * graph) noexcept
	{
		return static_cast<jive::gamma_normal_form*>(graph->node_normal_form(typeid(gamma_op)));
	}

private:
	size_t nalternatives_;
};

static inline bool
is_gamma_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::gamma_op*>(&op) != nullptr;
}

static inline bool
is_gamma_node(const jive::node * node) noexcept
{
	return is_opnode<gamma_op>(node);
}

class gamma_node;

class entryvar final {
	friend gamma_node;

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
	friend gamma_node;

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

class gamma_node : public jive::structural_node {
public:
	virtual
	~gamma_node();

private:
	inline
	gamma_node(jive::output * predicate, size_t nalternatives)
	: structural_node(jive::gamma_op(nalternatives), predicate->region(), nalternatives)
	{
		add_input(jive::ctl::type(nalternatives), predicate);
	}

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
	static jive::gamma_node *
	create(jive::output * predicate, size_t nalternatives)
	{
		return new jive::gamma_node(predicate, nalternatives);
	}

	inline jive::structural_input *
	predicate() const noexcept
	{
		JIVE_DEBUG_ASSERT(dynamic_cast<const jive::ctl::type*>(&input(0)->type()));
		return input(0);
	}

	inline gamma_node::entryvar_iterator
	begin_entryvar() const
	{
		return entryvar_iterator(input(1));
	}

	inline gamma_node::entryvar_iterator
	end_entryvar() const
	{
		return entryvar_iterator(nullptr);
	}

	inline gamma_node::exitvar_iterator
	begin_exitvar() const
	{
		return exitvar_iterator(output(0));
	}

	inline gamma_node::exitvar_iterator
	end_exitvar() const
	{
		return exitvar_iterator(nullptr);
	}

	inline std::shared_ptr<jive::entryvar>
	add_entryvar(jive::output * origin)
	{
		auto input = add_input(origin->type(), origin);
		for (size_t n = 0; n < nsubregions(); n++)
			subregion(n)->add_argument(input, origin->type());

		return std::shared_ptr<entryvar>(new entryvar(input));
	}

	inline std::shared_ptr<jive::exitvar>
	add_exitvar(const std::vector<jive::output*> & values)
	{
		if (values.size() != nsubregions())
			throw jive::compiler_error("Incorrect number of values.");
		JIVE_DEBUG_ASSERT(values.size() != 0);

		const auto & type = values[0]->type();
		auto output = add_output(type);
		for (size_t n = 0; n < nsubregions(); n++)
			subregion(n)->add_result(values[n], output, type);

		return std::shared_ptr<exitvar>(new exitvar(output));
	}

	virtual jive::gamma_node *
	copy(jive::region * region, jive::substitution_map & smap) const override;
};

}

#endif

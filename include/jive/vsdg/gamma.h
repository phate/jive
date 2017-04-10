/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_GAMMA_H
#define JIVE_VSDG_GAMMA_H

#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators/structural.h>
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

	virtual size_t
	narguments() const noexcept override;

	virtual const base::type &
	argument_type(size_t index) const noexcept override;

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

class gamma_builder;

class entryvar final {
	friend gamma_builder;

private:
	inline
	entryvar(const std::vector<jive::argument*> & arguments)
	: arguments_(arguments)
	{}

public:
	inline jive::structural_input *
	input() const noexcept
	{
		JIVE_DEBUG_ASSERT(arguments_.size() != 0);
		return arguments_[0]->input();
	}

	inline size_t
	narguments() const noexcept
	{
		return arguments_.size();
	}

	inline jive::argument *
	argument(size_t n) const noexcept
	{
		JIVE_DEBUG_ASSERT(n < narguments());
		return arguments_[n];
	}

private:
	std::vector<jive::argument*> arguments_;
};

class exitvar final {
	friend gamma_builder;

private:
	inline
	exitvar(const std::vector<jive::result*> & results)
	: results_(results)
	{}

public:
	inline jive::structural_output *
	output() const noexcept
	{
		JIVE_DEBUG_ASSERT(results_.size() != 0);
		return results_[0]->output();
	}

	inline size_t
	nresults() const noexcept
	{
		return results_.size();
	}

	inline jive::result *
	result(size_t n) const noexcept
	{
		JIVE_DEBUG_ASSERT(n < nresults());
		return results_[n];
	}

private:
	std::vector<jive::result*> results_;
};

class gamma_builder final {
public:
	inline
	gamma_builder() noexcept
	: node_(nullptr)
	{}

	inline size_t
	nsubregions() const noexcept
	{
		return node_ ? node_->nsubregions() : 0;
	}

	inline jive::region *
	region(size_t n) const noexcept
	{
		return node_ ? node_->subregion(n) : nullptr;
	}

	inline jive::iport *
	predicate() const noexcept
	{
		return node_ ? node_->input(0) : nullptr;
	}

	inline void
	begin(jive::oport * predicate)
	{
		if (node_)
			return;

		auto ctl = dynamic_cast<const jive::ctl::type*>(&predicate->type());
		if (!ctl)
			throw jive::type_error("ctl", ctl->debug_string());

		auto region = predicate->region();
		size_t nalternatives = ctl->nalternatives();
		node_ = region->add_structural_node(jive::gamma_op(nalternatives), nalternatives);
		node_->add_input(ctl, predicate);
	}

	inline std::shared_ptr<jive::entryvar>
	add_entryvar(jive::oport * origin)
	{
		if (!node_)
			return nullptr;

		std::vector<jive::argument*> arguments;
		auto input = node_->add_input(&origin->type(), origin);
		for (size_t n = 0; n < nsubregions(); n++)
			arguments.push_back(node_->subregion(n)->add_argument(input, origin->type()));

		return std::shared_ptr<entryvar>(new entryvar(arguments));
	}

	inline std::shared_ptr<jive::exitvar>
	add_exitvar(const std::vector<jive::oport*> & values)
	{
		if (!node_)
			return nullptr;

		if (values.size() != nsubregions())
			throw jive::compiler_error("Incorrect number of values.");
		JIVE_DEBUG_ASSERT(values.size() != 0);

		auto type = &values[0]->type();
		std::vector<jive::result*> results;
		auto output = node_->add_output(type);
		for (size_t n = 0; n < nsubregions(); n++)
			results.push_back(node_->subregion(n)->add_result(values[n], output, *type));

		return std::shared_ptr<exitvar>(new exitvar(results));
	}

	inline jive::structural_node *
	end()
	{
		auto node = node_;
		node_ = nullptr;
		return node;
	}

private:
	jive::structural_node * node_;
};

}

#endif

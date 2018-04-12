/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_H
#define JIVE_TYPES_RECORD_H

#include <jive/rvsdg/simple-node.h>
#include <jive/rvsdg/type.h>
#include <jive/rvsdg/unary.h>

namespace jive {

/* declaration */

class rcddeclaration final {
public:
	inline
	~rcddeclaration()
	{}

private:
	inline
	rcddeclaration()
	{}

	rcddeclaration(const rcddeclaration &) = delete;

	rcddeclaration &
	operator=(const rcddeclaration &) = delete;

public:
	inline size_t
	nelements() const noexcept
	{
		return types_.size();
	}

	const valuetype &
	element(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < nelements());
		return *static_cast<const valuetype*>(types_[index].get());
	}

	void
	append(const jive::valuetype & type)
	{
		types_.push_back(type.copy());
	}

	static inline std::unique_ptr<rcddeclaration>
	create()
	{
		return std::unique_ptr<rcddeclaration>(new rcddeclaration());
	}

	static inline std::unique_ptr<rcddeclaration>
	create(const std::vector<const valuetype*> & types)
	{
		auto dcl = create();
		for (const auto & type : types)
			dcl->append(*type);

		return dcl;
	}

private:
	std::vector<std::unique_ptr<jive::type>> types_;
};

void
unregister_rcddeclarations(const jive::graph * graph);

/* record type */

class rcdtype final : public jive::valuetype {
public:
	virtual
	~rcdtype() noexcept;

	inline
	rcdtype(const rcddeclaration * dcl) noexcept
	: dcl_(dcl)
	{}

	inline const rcddeclaration *
	declaration() const noexcept
	{
		return dcl_;
	}

	virtual
	std::string debug_string() const override;

	virtual bool
	operator==(const jive::type & type) const noexcept override;

	virtual std::unique_ptr<jive::type>
	copy() const override;

private:
	const rcddeclaration * dcl_;
};

/* group operator */

class group_op final : public jive::simple_op {
public:
	virtual
	~group_op() noexcept;

	inline
	group_op(const rcddeclaration * dcl) noexcept
	: simple_op(create_operands(dcl), {rcdtype(dcl)})
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const rcddeclaration *
	declaration() const noexcept
	{
		return static_cast<const rcdtype*>(&result(0).type())->declaration();
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::output *
	create(
		jive::graph * graph,
		const rcddeclaration * dcl)
	{
		group_op op(dcl);
		return simple_node::create_normalized(graph->root(), op, {})[0];
	}

	static inline jive::output *
	create(
		const rcddeclaration * dcl,
		const std::vector<jive::output*> & operands)
	{
		if (operands.empty())
			throw compiler_error("Expected more than one operand.");

		group_op op(dcl);
		return simple_node::create_normalized(operands[0]->region(), op, operands)[0];
	}

private:
	static std::vector<jive::port>
	create_operands(const rcddeclaration * dcl);
};

/* select operator */

class select_op final : public jive::unary_op {
public:
	virtual
	~select_op() noexcept;

private:
	inline
	select_op(const jive::rcdtype & type, size_t index) noexcept
	: unary_op(type, type.declaration()->element(index))
	, index_(index)
	{}

public:
	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline size_t
	index() const noexcept
	{
		return index_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::output *
	create(jive::output * operand, size_t index)
	{
		auto rt = dynamic_cast<const rcdtype*>(&operand->type());
		if (!rt) throw type_error("rcd", operand->type().debug_string());

		select_op op(*rt, index);
		return simple_node::create_normalized(operand->region(), op, {operand})[0];
	}

private:
	size_t index_;
};

static inline bool
is_select_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const select_op*>(&op) != nullptr;
}

static inline bool
is_select_node(const jive::node * node) noexcept
{
	return is_opnode<select_op>(node);
}

}

#endif

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

#include <algorithm>

namespace jive {

/* declaration */

class rcddeclaration final {
public:
	inline
	rcddeclaration()
	{}

	inline
	rcddeclaration(const std::vector<const valuetype*> & types)
	: types_(types.size())
	{
		std::transform(types.begin(), types.end(), types_.begin(),
			[](const auto & t){ return t->copy(); });
	}

	rcddeclaration(const rcddeclaration &) = delete;

	rcddeclaration &
	operator=(const rcddeclaration &) = delete;

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

private:
	std::vector<std::unique_ptr<jive::type>> types_;
};

/* record type */

class rcdtype final : public jive::valuetype {
public:
	virtual
	~rcdtype() noexcept;

	inline
	rcdtype(std::shared_ptr<const rcddeclaration> decl) noexcept
		: decl_(std::move(decl))
	{}

	inline const std::shared_ptr<const rcddeclaration> &
	declaration() const noexcept
	{
		return decl_;
	}

	virtual std::string debug_string() const override;

	virtual bool
	operator==(const jive::type & type) const noexcept override;

	virtual std::unique_ptr<jive::type>
	copy() const override;

private:
	std::shared_ptr<const rcddeclaration> decl_;
};

/* group operator */

class group_op final : public jive::simple_op {
public:
	virtual
	~group_op() noexcept;

	inline
	group_op(
		std::shared_ptr<const rcddeclaration> & declaration) noexcept
	: result_(rcdtype(declaration))
	{
		for (size_t n = 0; n < declaration->nelements(); n++)
			arguments_.push_back({declaration->element(n)});
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const std::shared_ptr<const rcddeclaration> &
	declaration() const noexcept
	{
		return static_cast<const rcdtype*>(&result_.type())->declaration();
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	jive::port result_;
	std::vector<jive::port> arguments_;
};

}

jive::output *
jive_group_create(std::shared_ptr<const jive::rcddeclaration> & decl,
	size_t narguments, jive::output * const * arguments);

jive::output *
jive_empty_group_create(jive::graph * graph, std::shared_ptr<const jive::rcddeclaration> & decl);

/* select operator */

namespace jive {

class select_op final : public jive::unary_op {
public:
	virtual
	~select_op() noexcept;

private:
	inline
	select_op(const jive::rcdtype & type, size_t index) noexcept
	: index_(index)
	, result_(type.declaration()->element(index))
	, argument_(type)
	{}

public:
	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

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
		return jive::create_normalized(operand->region(), op, {operand})[0];
	}

private:
	size_t index_;
	jive::port result_;
	jive::port argument_;
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

/*
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_UNNTYPE_H
#define JIVE_TYPES_UNION_UNNTYPE_H

#include <jive/vsdg/valuetype.h>

namespace jive {
namespace unn {

/* declaration */

struct declaration {
	size_t nelements;
	const jive::value::type ** elements;
};

/* type */

class type final : public jive::value::type {
public:
	virtual ~type() noexcept;

	inline constexpr
	type(const jive::unn::declaration * decl) noexcept
		: decl_(decl)
	{
	}

	inline const jive::unn::declaration * declaration() const noexcept { return decl_; }

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive::unn::type * copy() const override;

	virtual jive::input * create_input(jive_node * node, size_t index,
		jive::output * origin) const override;

	virtual jive::output * create_output(jive_node * node, size_t index) const override;

	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const override;

private:
	const jive::unn::declaration * decl_;
};

/* input */

class input final : public jive::value::input {
public:
	virtual ~input() noexcept;

	inline
	input(
		const jive::unn::declaration * decl,
		struct jive_node * node,
		size_t index,
		jive::output * origin)
	: jive::value::input(node, index, origin, jive::unn::type(decl))
	, type_(decl)
	{}

	inline const jive::unn::declaration *
	declaration() const noexcept
	{
		return static_cast<const jive::unn::type*>(&type())->declaration();
	}

private:
	input(const input & rhs) = delete;
	input& operator=(const input & rhs) = delete;

	jive::unn::type type_;
};

/* output */

class output final : public jive::value::output {
public:
	virtual ~output() noexcept;

	output(const jive::unn::declaration * decl, jive_node * node, size_t index);

	virtual const jive::unn::type & type() const noexcept { return type_; }

	inline const jive::unn::declaration * declaration() const noexcept { return type_.declaration(); }

private:
	output(const output & rhs) = delete;
	output& operator=(const output & rhs) = delete;

	jive::unn::type type_;
};

/* gate */

class gate final : public jive::value::gate {
public:
	virtual ~gate() noexcept;

	gate(const jive::unn::declaration * decl, jive_graph * graph, const char name[]);

	virtual const jive::unn::type & type() const noexcept { return type_; }

	inline const jive::unn::declaration * declaration() const noexcept { return type_.declaration(); }

private:
	gate(const gate & rhs) = delete;
	gate& operator=(const gate & rhs) = delete;

	jive::unn::type type_;
};

}
}

#endif

/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
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

	type(const jive::unn::declaration * decl) noexcept;

	inline const jive::unn::declaration * declaration() const noexcept { return decl_; }

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive::unn::type * copy() const override;

	virtual jive_input * create_input(jive_node * node, size_t index,
		jive_output * origin) const override;

	virtual jive_output * create_output(jive_node * node, size_t index) const override;

	virtual jive_gate * create_gate(jive_graph * graph, const char * name) const override;

private:
	const jive::unn::declaration * decl_;
};

/* input */

class input final : public jive::value::input {
public:
	virtual ~input() noexcept;

	input(const jive::unn::declaration * decl, struct jive_node * node, size_t index,
		jive_output * origin);

	virtual const jive::unn::type & type() const noexcept { return type_; }

	inline const jive::unn::declaration * declaration() const noexcept { return type_.declaration(); }

private:
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
	jive::unn::type type_;
};

}
}

#endif

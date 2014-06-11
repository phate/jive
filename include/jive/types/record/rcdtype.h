/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDTYPE_H
#define JIVE_TYPES_RECORD_RCDTYPE_H

#include <jive/vsdg/valuetype.h>

namespace jive {
namespace rcd {

/* declaration */

struct declaration {
	size_t nelements;
	const jive::value::type ** elements;
};

/* type */

class type final : public jive::value::type {
public:
	virtual ~type() noexcept;

	type(const jive::rcd::declaration * decl) noexcept;

	inline const jive::rcd::declaration * declaration() const noexcept { return decl_; }

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive::base::type & type) const noexcept override;

	virtual jive::rcd::type * copy() const override;

	virtual jive::input * create_input(jive_node * node, size_t index,
		jive::output * origin) const override;

	virtual jive::output * create_output(jive_node * node, size_t index) const override;

	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const override;

private:
	const jive::rcd::declaration * decl_;
};

/* input */

class input final : public jive::value::input {
public:
	virtual ~input() noexcept;

	input(const jive::rcd::declaration * decl, struct jive_node * node, size_t index,
		jive::output * origin);

	virtual const jive::rcd::type & type() const noexcept { return type_; }

	inline const jive::rcd::declaration * declaration() const noexcept { return type_.declaration(); }

private:
	input(const input & rhs) = delete;
	input& operator=(const input & rhs) = delete;

	jive::rcd::type type_;
};

/* output */

class output final : public jive::value::output {
public:
	virtual ~output() noexcept;

	output(const jive::rcd::declaration * decl, struct jive_node * nodex, size_t index);

	virtual const jive::rcd::type & type() const noexcept { return type_; }

	inline const jive::rcd::declaration * declaration() const noexcept { return type_.declaration(); }

private:
	output(const output & rhs) = delete;
	output& operator=(const output & rhs) = delete;

	jive::rcd::type type_;
};

/* gate */

class gate final : public jive::value::gate {
public:
	virtual ~gate() noexcept;

	gate(const jive::rcd::declaration * decl, jive_graph * graph, const char name[]);

	virtual const jive::rcd::type & type() const noexcept { return type_; }

	inline const jive::rcd::declaration * declaration() const noexcept { return type_.declaration(); }

private:
	gate(const gate & rhs) = delete;
	gate& operator=(const gate & rhs) = delete;

	jive::rcd::type type_;
};

}
}

#endif

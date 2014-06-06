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
	const jive_value_type ** elements;
};

/* type */

class type final : public jive_value_type {
public:
	virtual ~type() noexcept;

	type(const jive::rcd::declaration * decl) noexcept;

	inline const jive::rcd::declaration * declaration() const noexcept { return decl_; }

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive_type & type) const noexcept override;

	virtual jive::rcd::type * copy() const override;

	virtual jive_input * create_input(jive_node * node, size_t index,
		jive_output * origin) const override;

	virtual jive_output * create_output(jive_node * node, size_t index) const override;

	virtual jive_gate * create_gate(jive_graph * graph, const char * name) const override;

private:
	const jive::rcd::declaration * decl_;
};

/* input */

class input final : public jive_value_input {
public:
	virtual ~input() noexcept;

	input(const jive::rcd::declaration * decl, struct jive_node * node, size_t index,
		jive_output * origin);

	virtual const jive::rcd::type & type() const noexcept { return type_; }

	inline const jive::rcd::declaration * declaration() const noexcept { return type_.declaration(); }

private:
	jive::rcd::type type_;
};

/* output */

class output final : public jive_value_output {
public:
	virtual ~output() noexcept;

	output(const jive::rcd::declaration * decl, struct jive_node * nodex, size_t index);

	virtual const jive::rcd::type & type() const noexcept { return type_; }

	inline const jive::rcd::declaration * declaration() const noexcept { return type_.declaration(); }

private:
	jive::rcd::type type_;
};

/* gate */

class gate final : public jive_value_gate {
public:
	virtual ~gate() noexcept;

	gate(const jive::rcd::declaration * decl, jive_graph * graph, const char name[]);

	virtual const jive::rcd::type & type() const noexcept { return type_; }

	inline const jive::rcd::declaration * declaration() const noexcept { return type_.declaration(); }

private:
	jive::rcd::type type_;
};

}
}

#endif

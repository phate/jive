/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_UNNTYPE_H
#define JIVE_TYPES_UNION_UNNTYPE_H

#include <jive/vsdg/valuetype.h>

/* union declaration */

typedef struct jive_union_declaration jive_union_declaration;

struct jive_union_declaration {
	size_t nelements;
	const jive_value_type ** elements;
};

/* union type */

typedef struct jive_union_type jive_union_type;

extern const jive_type_class JIVE_UNION_TYPE;

class jive_union_type final : public jive_value_type {
public:
	virtual ~jive_union_type() noexcept;

	jive_union_type(const jive_union_declaration * decl) noexcept;

	inline const jive_union_declaration * declaration() const noexcept { return decl_; }

	virtual void label(jive_buffer & buffer) const override;

private:
	const jive_union_declaration * decl_;
};

JIVE_EXPORTED_INLINE const jive_union_type *
jive_union_type_const_cast(const jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_UNION_TYPE))
		return (const jive_union_type *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_union_type *
jive_union_type_cast(jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_UNION_TYPE))
		return (jive_union_type *)self;
	else
		return NULL;
}

/* union input */

class jive_union_input final : public jive_value_input {
public:
	virtual ~jive_union_input() noexcept;

	jive_union_input(const jive_union_declaration * decl, struct jive_node * node, size_t index,
		jive_output * origin);

	virtual const jive_union_type & type() const noexcept { return type_; }

	inline const jive_union_declaration * declaration() const noexcept { return type_.declaration(); }

private:
	jive_union_type type_;
};

/* union output */

class jive_union_output final : public jive_value_output {
public:
	virtual ~jive_union_output() noexcept;

	jive_union_output(const jive_union_declaration * decl, jive_node * node, size_t index);

	virtual const jive_union_type & type() const noexcept { return type_; }

	inline const jive_union_declaration * declaration() const noexcept { return type_.declaration(); }

private:
	jive_union_type type_;
};

/* union gate */

class jive_union_gate final : public jive_value_gate {
public:
	virtual ~jive_union_gate() noexcept;

	jive_union_gate(const jive_union_declaration * decl, jive_graph * graph,
		const char name[]);

	virtual const jive_union_type & type() const noexcept { return type_; }

	inline const jive_union_declaration * declaration() const noexcept { return type_.declaration(); }

private:
	jive_union_type type_;
};

#endif

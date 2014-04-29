/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDTYPE_H
#define JIVE_TYPES_RECORD_RCDTYPE_H

#include <jive/vsdg/valuetype.h>

/* record declaration */

typedef struct jive_record_declaration jive_record_declaration;

struct jive_record_declaration {
	size_t nelements;
	const jive_value_type ** elements;
};

/* record type */

typedef struct jive_record_type jive_record_type;

extern const jive_type_class JIVE_RECORD_TYPE;
class jive_record_type final : public jive_value_type {
public:
	virtual ~jive_record_type() noexcept;

	jive_record_type(const jive_record_declaration * decl) noexcept;

	inline const jive_record_declaration * declaration() const noexcept { return decl_; }

private:
	const jive_record_declaration * decl_;
};

JIVE_EXPORTED_INLINE const jive_record_type *
jive_record_type_const_cast(const jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_RECORD_TYPE))
		return (const jive_record_type *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_record_type *
jive_record_type_cast(jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_RECORD_TYPE))
		return (jive_record_type *)self;
	else
		return NULL;
}

/* record input */

typedef struct jive_record_input jive_record_input;

extern const jive_input_class JIVE_RECORD_INPUT;
class jive_record_input final : public jive_value_input {
public:
	virtual ~jive_record_input() noexcept;

	jive_record_input(const jive_record_declaration * decl, struct jive_node * node, size_t index,
		jive_output * origin);

	virtual const jive_record_type & type() const noexcept { return type_; }

	inline const jive_record_declaration * declaration() const noexcept { return type_.declaration(); }

private:
	jive_record_type type_;
};

/* record output */

typedef struct jive_record_output jive_record_output;

extern const jive_output_class JIVE_RECORD_OUTPUT;
class jive_record_output final : public jive_value_output {
public:
	virtual ~jive_record_output() noexcept;

	jive_record_output(const jive_record_declaration * decl, struct jive_node * nodex,
		size_t index);

	virtual const jive_record_type & type() const noexcept { return type_; }

	inline const jive_record_declaration * declaration() const noexcept { return type_.declaration(); }

private:
	jive_record_type type_;
};

/* record gate */

typedef struct jive_record_gate jive_record_gate;

extern const jive_gate_class JIVE_RECORD_GATE;
class jive_record_gate final : public jive_value_gate {
public:
	virtual ~jive_record_gate() noexcept;

	jive_record_gate(const jive_record_declaration * decl, jive_graph * graph,
		const char name[]);

	virtual const jive_record_type & type() const noexcept { return type_; }

	inline const jive_record_declaration * declaration() const noexcept { return type_.declaration(); }

private:
	jive_record_type type_;
};

JIVE_EXPORTED_INLINE const jive_record_gate *
jive_record_gate_const_cast(const jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_RECORD_GATE))
		return (const jive_record_gate *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_record_gate *
jive_record_gate_cast(jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_RECORD_GATE))
		return (jive_record_gate *)self;
	else
		return NULL;
}

#endif

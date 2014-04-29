/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTTYPE_H
#define JIVE_TYPES_FLOAT_FLTTYPE_H

#include <jive/vsdg/valuetype.h>

/* float type */

typedef struct jive_float_type jive_float_type;

extern const jive_type_class JIVE_FLOAT_TYPE;
#define JIVE_DECLARE_FLOAT_TYPE(name) \
	jive_float_type name##_struct; \
	const jive_type * name = &name##_struct

class jive_float_type final : public jive_value_type {
public:
	virtual ~jive_float_type() noexcept;

	jive_float_type() noexcept;
};

JIVE_EXPORTED_INLINE const jive_float_type *
jive_float_type_const_cast(const jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_FLOAT_TYPE))
		return (const jive_float_type *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_float_type *
jive_float_type_cast(jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_FLOAT_TYPE))
		return (jive_float_type *)self;
	else
		return NULL;
}

/* float input */

typedef struct jive_float_input jive_float_input;

extern const jive_input_class JIVE_FLOAT_INPUT;
class jive_float_input final : public jive_value_input {
public:
	virtual ~jive_float_input() noexcept;

	jive_float_input(struct jive_node * node, size_t index, jive_output * origin);

	virtual const jive_float_type & type() const noexcept { return type_; }

private:
	jive_float_type type_;
};

/* float output */

typedef struct jive_float_output jive_float_output;

extern const jive_output_class JIVE_FLOAT_OUTPUT;
class jive_float_output final : public jive_value_output {
public:
	virtual ~jive_float_output() noexcept;

	jive_float_output(struct jive_node * node, size_t index);

	virtual const jive_float_type & type() const noexcept { return type_; }

private:
	jive_float_type type_;
};

/* float gate */

typedef struct jive_float_gate jive_float_gate;

extern const jive_gate_class JIVE_FLOAT_GATE;
class jive_float_gate final : public jive_value_gate {
public:
	virtual ~jive_float_gate() noexcept;

	jive_float_gate(jive_graph * graph, const char name[]);

	virtual const jive_float_type & type() const noexcept { return type_; }

private:
	jive_float_type type_;
};

JIVE_EXPORTED_INLINE const jive_float_gate *
jive_float_gate_const_cast(const jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_FLOAT_GATE))
		return (const jive_float_gate *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_float_gate *
jive_float_gate_cast(jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_FLOAT_GATE))
		return (jive_float_gate *)self;
	else
		return NULL;
}

#endif

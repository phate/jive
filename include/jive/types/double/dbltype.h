/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_DOUBLE_DBLTYPE_H
#define JIVE_TYPES_DOUBLE_DBLTYPE_H

#include <jive/vsdg/valuetype.h>

/* double type */

typedef struct jive_double_type jive_double_type;

extern const jive_type_class JIVE_DOUBLE_TYPE;
#define JIVE_DECLARE_DOUBLE_TYPE(name) \
	jive_double_type name##_struct; \
	const jive_type * name = &name##_struct

class jive_double_type final : public jive_value_type {
public:
	virtual ~jive_double_type() noexcept;

	jive_double_type() noexcept;
};

JIVE_EXPORTED_INLINE const jive_double_type *
jive_double_type_const_cast(const jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_DOUBLE_TYPE))
		return (const jive_double_type *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_double_type *
jive_double_type_cast(jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_DOUBLE_TYPE))
		return (jive_double_type *)self;
	else
		return NULL;
}

/* double input */

class jive_double_input final : public jive_value_input {
public:
	virtual ~jive_double_input() noexcept;
	jive_double_input(struct jive_node * node, size_t index, jive_output * origin);

	virtual const jive_double_type & type() const noexcept { return type_; }

private:
	jive_double_type type_;
};

/* double output */

typedef struct jive_double_output jive_double_output;

extern const jive_output_class JIVE_DOUBLE_OUTPUT;
class jive_double_output final : public jive_value_output {
public:
	virtual ~jive_double_output() noexcept;

	jive_double_output(struct jive_node * node, size_t index);

	virtual const jive_double_type & type() const noexcept { return type_; }

private:
	jive_double_type type_;
};

/* double gate */

typedef struct jive_double_gate jive_double_gate;

extern const jive_gate_class JIVE_DOUBLE_GATE;
class jive_double_gate final : public jive_value_gate {
public:
	virtual ~jive_double_gate() noexcept;

	jive_double_gate(jive_graph * graph, const char name[]);

	virtual const jive_double_type & type() const noexcept { return type_; }

private:
	jive_double_type type_;
};

#endif

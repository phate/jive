/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_REAL_RLTYPE_H
#define JIVE_TYPES_REAL_RLTYPE_H

#include <jive/vsdg/valuetype.h>

/* real type */

typedef struct jive_real_type jive_real_type;

extern const jive_type_class JIVE_REAL_TYPE;
#define JIVE_DECLARE_REAL_TYPE(name) \
	jive_real_type name##_struct; \
	const jive_type * name = &name##_struct

class jive_real_type final : public jive_value_type {
public:
	virtual ~jive_real_type() noexcept;

	jive_real_type() noexcept;
};

JIVE_EXPORTED_INLINE struct jive_real_type *
jive_real_type_cast(struct jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_REAL_TYPE))
		return (struct jive_real_type *) type;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_real_type *
jive_real_type_const_cast(const struct jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_REAL_TYPE))
		return (const struct jive_real_type *) type;
	else
		return NULL;
}

/* real input */

class jive_real_input final : public jive_value_input {
public:
	virtual ~jive_real_input() noexcept;

	jive_real_input(struct jive_node * node, size_t index, jive_output * initial_operand);

	virtual const jive_real_type & type() const noexcept { return type_; }

private:
	jive_real_type type_;
};

/* real output */

typedef struct jive_real_output jive_real_output;

extern const jive_output_class JIVE_REAL_OUTPUT;
class jive_real_output final : public jive_value_output {
public:
	virtual ~jive_real_output() noexcept;

	jive_real_output(struct jive_node * node, size_t index);

	virtual const jive_real_type & type() const noexcept { return type_; }

private:
	jive_real_type type_;
};

/* real gate */

typedef struct jive_real_gate jive_real_gate;

extern const jive_gate_class JIVE_REAL_GATE;
class jive_real_gate final : public jive_value_gate {
public:
	virtual ~jive_real_gate() noexcept;

	jive_real_gate(jive_graph * graph, const char name[]);

	virtual const jive_real_type & type() const noexcept { return type_; }

private:
	jive_real_type type_;
};

#endif

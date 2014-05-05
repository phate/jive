/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ITGTYPE_H
#define JIVE_TYPES_INTEGRAL_ITGTYPE_H

#include <jive/vsdg/valuetype.h>

/* integral type */

typedef struct jive_integral_type jive_integral_type;

extern const jive_type_class JIVE_INTEGRAL_TYPE;
class jive_integral_type final : public jive_value_type {
public:
	virtual ~jive_integral_type() noexcept;

	jive_integral_type() noexcept;
};

JIVE_EXPORTED_INLINE struct jive_integral_type *
jive_integral_type_cast(struct jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_INTEGRAL_TYPE))
		return (struct jive_integral_type *) type;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_integral_type *
jive_integral_type_const_cast(const struct jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_INTEGRAL_TYPE))
		return (const struct jive_integral_type *) type;
	else
		return NULL;
}

/* integral input */

class jive_integral_input final : public jive_value_input {
public:
	virtual ~jive_integral_input() noexcept;

	jive_integral_input(struct jive_node * node, size_t index, jive_output * origin);

	virtual const jive_integral_type & type() const noexcept { return type_; }

private:
	jive_integral_type type_;
};

/* integral output */

class jive_integral_output final : public jive_value_output {
public:
	virtual ~jive_integral_output() noexcept;

	jive_integral_output(jive_node * node, size_t index);

	virtual const jive_integral_type & type() const noexcept { return type_; }

private:
	jive_integral_type type_;
};

/* integral gate */

class jive_integral_gate final : public jive_value_gate {
public:
	virtual ~jive_integral_gate() noexcept;

	jive_integral_gate(jive_graph * graph, const char name[]);

	virtual const jive_integral_type & type() const noexcept { return type_; }

private:
	jive_integral_type type_;
};

#endif

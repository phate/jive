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
class jive_float_type final : public jive_value_type {
public:
	virtual ~jive_float_type() noexcept;

	jive_float_type() noexcept;

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive_type & other) const noexcept override;
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

class jive_float_input final : public jive_value_input {
public:
	virtual ~jive_float_input() noexcept;

	jive_float_input(struct jive_node * node, size_t index, jive_output * origin);

	virtual const jive_float_type & type() const noexcept { return type_; }

private:
	jive_float_type type_;
};

/* float output */

class jive_float_output final : public jive_value_output {
public:
	virtual ~jive_float_output() noexcept;

	jive_float_output(struct jive_node * node, size_t index);

	virtual const jive_float_type & type() const noexcept { return type_; }

private:
	jive_float_type type_;
};

/* float gate */

class jive_float_gate final : public jive_value_gate {
public:
	virtual ~jive_float_gate() noexcept;

	jive_float_gate(jive_graph * graph, const char name[]);

	virtual const jive_float_type & type() const noexcept { return type_; }

private:
	jive_float_type type_;
};

#endif

/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_ADDRESSTYPE_H
#define JIVE_ARCH_ADDRESSTYPE_H

#include <jive/vsdg/valuetype.h>

/* address type */

typedef struct jive_address_type jive_address_type;

extern const jive_type_class JIVE_ADDRESS_TYPE;
class jive_address_type final : public jive_value_type {
public:
	virtual ~jive_address_type() noexcept;

	jive_address_type() noexcept;

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive_type & other) const noexcept override;

	virtual std::unique_ptr<jive_type> copy() const override;

	virtual jive_input * create_input(jive_node * node, size_t index,
		jive_output * origin) const override;

	virtual jive_output * create_output(jive_node * node, size_t index) const override;
};

JIVE_EXPORTED_INLINE const jive_address_type *
jive_address_type_const_cast(const jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_ADDRESS_TYPE))
		return (const jive_address_type *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_address_type *
jive_address_type_cast(jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_ADDRESS_TYPE))
		return (jive_address_type *)self;
	else
		return NULL;
}

/* address input */

class jive_address_input final : public jive_value_input {
public:
	virtual ~jive_address_input() noexcept;

	jive_address_input(struct jive_node * node, size_t index, jive_output * origin);

	virtual const jive_address_type & type() const noexcept { return type_; }

private:
	jive_address_type type_;
};

/* address output */

class jive_address_output final : public jive_value_output {
public:
	virtual ~jive_address_output() noexcept;

	jive_address_output(jive_node * node, size_t index);

	virtual const jive_address_type & type() const noexcept { return type_; }

private:
	jive_address_type type_;
};

/* address gate */

class jive_address_gate final : public jive_value_gate {
public:
	virtual ~jive_address_gate() noexcept;

	jive_address_gate(jive_graph * graph, const char name[]);

	virtual const jive_address_type & type() const noexcept { return type_; }

private:
	jive_address_type type_;
};

#endif

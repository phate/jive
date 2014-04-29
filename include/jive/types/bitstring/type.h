/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_TYPE_H
#define JIVE_TYPES_BITSTRING_TYPE_H

#include <jive/vsdg/valuetype.h>

/* bitstring type */

typedef struct jive_bitstring_type jive_bitstring_type;

extern const jive_type_class JIVE_BITSTRING_TYPE;
#define JIVE_DECLARE_BITSTRING_TYPE(name, nbits) \
	jive_bitstring_type name##_struct(nbits); \
	const jive_type * name = &name##_struct

class jive_bitstring_type final : public jive_value_type {
public:
	virtual ~jive_bitstring_type() noexcept;

	jive_bitstring_type(size_t nbits) noexcept;

	inline size_t nbits() const noexcept { return nbits_; }

private:
	size_t nbits_;
};

JIVE_EXPORTED_INLINE jive_bitstring_type *
jive_bitstring_type_cast(jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_BITSTRING_TYPE))
		return (jive_bitstring_type *) type;
	else
		return 0;
}

JIVE_EXPORTED_INLINE const jive_bitstring_type *
jive_bitstring_type_const_cast(const jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_BITSTRING_TYPE))
		return (const jive_bitstring_type *) type;
	else
		return 0;
}


/* bitstring input */

typedef struct jive_bitstring_input jive_bitstring_input;

extern const jive_input_class JIVE_BITSTRING_INPUT;
class jive_bitstring_input final : public jive_value_input {
public:
	virtual ~jive_bitstring_input() noexcept;

	jive_bitstring_input(size_t nbits, struct jive_node * node, size_t index,
		jive_output * origin) noexcept;

	virtual const jive_bitstring_type & type() const noexcept { return type_; }

	inline size_t nbits() const noexcept { return type_.nbits(); }
	
private:
	jive_bitstring_type type_;
};

JIVE_EXPORTED_INLINE size_t
jive_bitstring_input_nbits(const jive_bitstring_input * self)
{
	return self->nbits();
}

/* bitstring output */

typedef struct jive_bitstring_output jive_bitstring_output;

extern const jive_output_class JIVE_BITSTRING_OUTPUT;
class jive_bitstring_output final : public jive_value_output {
public:
	virtual ~jive_bitstring_output() noexcept;

	jive_bitstring_output(size_t nbits, struct jive_node * node, size_t index);

	virtual const jive_bitstring_type & type() const noexcept { return type_; }

	inline size_t nbits() const noexcept { return type_.nbits(); }

private:
	jive_bitstring_type type_;
};

JIVE_EXPORTED_INLINE size_t
jive_bitstring_output_nbits(const jive_bitstring_output * self)
{
	return self->nbits();
}

/* bitstring gate */

typedef struct jive_bitstring_gate jive_bitstring_gate;

extern const jive_gate_class JIVE_BITSTRING_GATE;
class jive_bitstring_gate final : public jive_value_gate {
public:
	virtual ~jive_bitstring_gate() noexcept;

	jive_bitstring_gate(size_t nbits, jive_graph * graph, const char name[]);

	virtual const jive_bitstring_type & type() const noexcept { return type_; }

	inline size_t nbits() const noexcept { return type_.nbits(); }	

private:
	jive_bitstring_type type_;
};

JIVE_EXPORTED_INLINE jive_bitstring_gate *
jive_bitstring_gate_cast(jive_gate * gate)
{
	if (jive_gate_isinstance(gate, &JIVE_BITSTRING_GATE))
		return (jive_bitstring_gate *) gate;
	else
		return 0;
}

JIVE_EXPORTED_INLINE const jive_bitstring_gate *
jive_bitstring_gate_const_cast(const jive_gate * gate)
{
	if (jive_gate_isinstance(gate, &JIVE_BITSTRING_GATE))
		return (const jive_bitstring_gate *) gate;
	else
		return 0;
}

JIVE_EXPORTED_INLINE size_t
jive_bitstring_gate_nbits(const jive_bitstring_gate * self)
{
	return self->nbits();
}

#endif

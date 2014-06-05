/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_TYPE_H
#define JIVE_TYPES_BITSTRING_TYPE_H

#include <jive/vsdg/valuetype.h>

/* bitstring type */

class jive_bitstring_type final : public jive_value_type {
public:
	virtual ~jive_bitstring_type() noexcept;

	jive_bitstring_type(size_t nbits) noexcept;

	inline size_t nbits() const noexcept { return nbits_; }

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive_type & other) const noexcept override;

	virtual jive_bitstring_type * copy() const override;

	virtual jive_input * create_input(jive_node * node, size_t index,
		jive_output * origin) const override;

	virtual jive_output * create_output(jive_node * node, size_t index) const override;

	virtual jive_gate * create_gate(jive_graph * graph, const char * name) const override;

private:
	size_t nbits_;
};

/* bitstring input */

class jive_bitstring_input final : public jive_value_input {
public:
	virtual ~jive_bitstring_input() noexcept;

	jive_bitstring_input(size_t nbits, struct jive_node * node, size_t index,
		jive_output * origin);

	virtual const jive_bitstring_type & type() const noexcept { return type_; }

	inline size_t nbits() const noexcept { return type_.nbits(); }
	
private:
	jive_bitstring_type type_;
};

/* bitstring output */

class jive_bitstring_output final : public jive_value_output {
public:
	virtual ~jive_bitstring_output() noexcept;

	jive_bitstring_output(size_t nbits, struct jive_node * node, size_t index);

	virtual const jive_bitstring_type & type() const noexcept { return type_; }

	inline size_t nbits() const noexcept { return type_.nbits(); }

private:
	jive_bitstring_type type_;
};

/* bitstring gate */

class jive_bitstring_gate final : public jive_value_gate {
public:
	virtual ~jive_bitstring_gate() noexcept;

	jive_bitstring_gate(size_t nbits, jive_graph * graph, const char name[]);

	virtual const jive_bitstring_type & type() const noexcept { return type_; }

	inline size_t nbits() const noexcept { return type_.nbits(); }	

private:
	jive_bitstring_type type_;
};

#endif

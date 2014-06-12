/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_TYPE_H
#define JIVE_TYPES_BITSTRING_TYPE_H

#include <jive/vsdg/valuetype.h>

namespace jive {
namespace bits {

/* bitstring type */

class type final : public jive::value::type {
public:
	virtual ~type() noexcept;

	inline constexpr type(size_t nbits) noexcept
		: nbits_(nbits)
	{
	}

	inline size_t nbits() const noexcept { return nbits_; }

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive::bits::type * copy() const override;

	virtual jive::input * create_input(jive_node * node, size_t index,
		jive::output * origin) const override;

	virtual jive::output * create_output(jive_node * node, size_t index) const override;

	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const override;

private:
	size_t nbits_;
};

/* bitstring input */

class input final : public jive::value::input {
public:
	virtual ~input() noexcept;

	input(size_t nbits, struct jive_node * node, size_t index, jive::output * origin);

	virtual const jive::bits::type & type() const noexcept { return type_; }

	inline size_t nbits() const noexcept { return type_.nbits(); }
	
private:
	input(const input & rhs) = delete;
	input& operator=(const input & rhs) = delete;

	jive::bits::type type_;
};

/* bitstring output */

class output final : public jive::value::output {
public:
	virtual ~output() noexcept;

	output(size_t nbits, struct jive_node * node, size_t index);

	virtual const jive::bits::type & type() const noexcept { return type_; }

	inline size_t nbits() const noexcept { return type_.nbits(); }

private:
	output(const output & rhs) = delete;
	output& operator=(const output & rhs) = delete;

	jive::bits::type type_;
};

/* bitstring gate */

class gate final : public jive::value::gate {
public:
	virtual ~gate() noexcept;

	gate(size_t nbits, jive_graph * graph, const char name[]);

	virtual const jive::bits::type & type() const noexcept { return type_; }

	inline size_t nbits() const noexcept { return type_.nbits(); }	

private:
	gate(const gate & rhs) = delete;
	gate& operator=(const gate & rhs) = delete;

	jive::bits::type type_;
};

}
}

#endif

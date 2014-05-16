/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMORYTYPE_H
#define JIVE_ARCH_MEMORYTYPE_H

#include <jive/vsdg/statetype.h>

class jive_memory_type final : public jive_state_type {
public:
	virtual ~jive_memory_type() noexcept;

	inline constexpr jive_memory_type() noexcept : jive_state_type() {};

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive_type & other) const noexcept override;

	virtual jive_memory_type * copy() const override;

	virtual jive_input * create_input(jive_node * node, size_t index,
		jive_output * origin) const override;

	virtual jive_output * create_output(jive_node * node, size_t index) const override;

	virtual jive_gate * create_gate(jive_graph * graph, const char * name) const override;
};

class jive_memory_input final : public jive_state_input {
public:
	virtual ~jive_memory_input() noexcept;

	jive_memory_input(struct jive_node * node, size_t index, jive_output * origin);

	virtual const jive_memory_type & type() const noexcept { return type_; }

private:
	jive_memory_type type_;
};

class jive_memory_output final : public jive_state_output {
public:
	virtual ~jive_memory_output() noexcept;

	jive_memory_output(jive_node * node, size_t index);

	virtual const jive_memory_type & type() const noexcept { return type_; }

private:
	jive_memory_type type_;
};

class jive_memory_gate final : public jive_state_gate {
public:
	virtual ~jive_memory_gate() noexcept;

	jive_memory_gate(jive_graph * graph, const char name[]);

	virtual const jive_memory_type & type() const noexcept { return type_; }

private:
	jive_memory_type type_;
};

#endif

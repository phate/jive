/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMORYTYPE_H
#define JIVE_ARCH_MEMORYTYPE_H

#include <jive/vsdg/statetype.h>

typedef struct jive_memory_type jive_memory_type;

extern const jive_type_class JIVE_MEMORY_TYPE;
class jive_memory_type final : public jive_state_type {
public:
	virtual ~jive_memory_type() noexcept;

	jive_memory_type() noexcept;

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive_type & other) const noexcept override;

	virtual std::unique_ptr<jive_type> copy() const override;
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

/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMORYTYPE_H
#define JIVE_ARCH_MEMORYTYPE_H

#include <jive/vsdg/statetype.h>

typedef struct jive_memory_type jive_memory_type;
typedef struct jive_memory_output jive_memory_output;
typedef struct jive_memory_output jive_memory;
typedef struct jive_memory_gate jive_memory_gate;
typedef struct jive_memory_resource jive_memory_resource;

extern const jive_type_class JIVE_MEMORY_TYPE;
#define JIVE_DECLARE_MEMORY_TYPE(name) \
	jive_memory_type name##_struct; \
	const jive_type * name = &name##_struct

class jive_memory_type final : public jive_state_type {
public:
	virtual ~jive_memory_type() noexcept;

	jive_memory_type() noexcept;
};

class jive_memory_input final : public jive_state_input {
public:
	virtual ~jive_memory_input() noexcept;

	jive_memory_input(struct jive_node * node, size_t index, jive_output * origin);

	virtual const jive_memory_type & type() const noexcept { return type_; }

private:
	jive_memory_type type_;
};

extern const jive_output_class JIVE_MEMORY_OUTPUT;
class jive_memory_output final : public jive_state_output {
public:
	virtual ~jive_memory_output() noexcept;

	jive_memory_output(jive_node * node, size_t index);

	virtual const jive_memory_type & type() const noexcept { return type_; }

private:
	jive_memory_type type_;
};

extern const jive_gate_class JIVE_MEMORY_GATE;
class jive_memory_gate final : public jive_state_gate {
public:
	virtual ~jive_memory_gate() noexcept;

	jive_memory_gate(jive_graph * graph, const char name[]);

	virtual const jive_memory_type & type() const noexcept { return type_; }

private:
	jive_memory_type type_;
};

#endif

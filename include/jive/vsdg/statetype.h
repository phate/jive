/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_STATETYPE_H
#define JIVE_VSDG_STATETYPE_H

#include <jive/vsdg/basetype.h>
#include <jive/vsdg/node.h>

typedef struct jive_state_type jive_state_type;

extern const jive_type_class JIVE_STATE_TYPE;
class jive_state_type : public jive_type {
public:
	virtual ~jive_state_type() noexcept;

protected:
	jive_state_type(const jive_type_class * class_) noexcept;
};

class jive_state_input : public jive_input {
public:
	virtual ~jive_state_input() noexcept;

protected:
	jive_state_input(struct jive_node * node, size_t index, jive_output * origin);
};

class jive_state_output : public jive_output {
public:
	virtual ~jive_state_output() noexcept;

protected:
	jive_state_output(struct jive_node * node, size_t index);
};

class jive_state_gate : public jive_gate {
public:
	virtual ~jive_state_gate() noexcept;

protected:
	jive_state_gate(jive_graph * graph, const char name[]);
};

/* state multiplexing support */

typedef struct jive_statemux_node_attrs jive_statemux_node_attrs;
typedef struct jive_statemux_node jive_statemux_node;

struct jive_statemux_node_attrs : public jive_node_attrs {
	size_t noutputs;
	struct jive_type * type; /* note: dynamically allocated */
};

struct jive_statemux_node : public jive_node {
	jive_statemux_node_attrs attrs;
};

jive_node *
jive_statemux_node_create(struct jive_region * region,
	const jive_type * statetype,
	size_t noperands, jive_output * const operands[],
	size_t noutputs);

jive_output *
jive_state_merge(const jive_type * statetype, size_t nstates, jive_output * const states[]);

jive_node *
jive_state_split(const jive_type * statetype, jive_output * state, size_t nstates);

#endif

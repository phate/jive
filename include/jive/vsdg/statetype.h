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
typedef struct jive_state_input jive_state_input;
typedef struct jive_state_output jive_state_output;
typedef struct jive_state_output jive_state;
typedef struct jive_state_gate jive_state_gate;
typedef struct jive_state_resource jive_state_resource;

extern const jive_type_class JIVE_STATE_TYPE;
#define JIVE_DECLARE_STATE_TYPE(name) \
	jive_state_type name##_struct; name##_struct.class_ = &JIVE_STATE_TYPE; \
	const jive_type * name = &name##_struct

struct jive_state_type : public jive_type {
};

extern const jive_input_class JIVE_STATE_INPUT;
struct jive_state_input : public jive_input {
};

extern const jive_output_class JIVE_STATE_OUTPUT;
struct jive_state_output : public jive_output {
};

extern const jive_gate_class JIVE_STATE_GATE;
struct jive_state_gate {
	jive_gate base;
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

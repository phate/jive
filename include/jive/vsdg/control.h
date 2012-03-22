#ifndef JIVE_VSDG_CONTROL_H
#define JIVE_VSDG_CONTROL_H

#include <jive/vsdg/controltype.h>
#include <jive/vsdg/gamma.h>
#include <jive/vsdg/node.h>

struct jive_output;
struct jive_type;
struct jive_region;

extern const jive_node_class JIVE_THETA_HEAD_NODE;
extern const jive_node_class JIVE_THETA_TAIL_NODE;
extern const jive_node_class JIVE_THETA_NODE;

struct jive_node *
jive_theta_create(
	struct jive_region * region,
	size_t nvalues, const struct jive_type * types[const], struct jive_output * values[const]);

extern const jive_node_class JIVE_CONTROL_FALSE_NODE;
extern const jive_node_class JIVE_CONTROL_TRUE_NODE;

struct jive_node *
jive_control_false_create(struct jive_region * region);

struct jive_output *
jive_control_false(struct jive_graph * graph);

struct jive_node *
jive_control_true_create(struct jive_region * region);

struct jive_output *
jive_control_true(struct jive_graph * graph);

#endif

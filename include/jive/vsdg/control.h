#ifndef JIVE_VSDG_CONTROL_H
#define JIVE_VSDG_CONTROL_H

#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node.h>

struct jive_output;
struct jive_type;
struct jive_region;

extern const jive_node_class JIVE_GAMMA_TAIL_NODE;
extern const jive_node_class JIVE_GAMMA_NODE;

extern const jive_node_class JIVE_THETA_HEAD_NODE;
extern const jive_node_class JIVE_THETA_TAIL_NODE;
extern const jive_node_class JIVE_THETA_NODE;

struct jive_output * const *
jive_choose(struct jive_output * predicate,
	size_t nvalues, const struct jive_type * types[const],
	struct jive_output * false_values[const],
	struct jive_output * true_values[const]);

struct jive_node *
jive_gamma_create(
	struct jive_region * region,
	struct jive_output * predicate,
	size_t nvalues, const struct jive_type * types[const],
	struct jive_output * false_values[const],
	struct jive_output * true_values[const]);

struct jive_node *
jive_theta_create(
	struct jive_region * region,
	size_t nvalues, const struct jive_type * types[const], struct jive_output * values[const]);

#endif

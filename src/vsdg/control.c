#include <jive/vsdg/control.h>

#include <string.h>
#include <stdio.h>

#include <jive/context.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

const jive_node_class JIVE_GAMMA_TAIL_NODE = {
	.parent = &JIVE_NODE,
	.name = "GAMMA_TAIL",
	.fini = _jive_node_fini,  /* inherit */
	.get_default_normal_form = _jive_node_get_default_normal_form,  /* inherit */
	.get_label = _jive_node_get_label,  /* inherit */
	.get_attrs = _jive_node_get_attrs,  /* inherit */
	.match_attrs = _jive_node_match_attrs,  /* inherit */
	.create = _jive_node_create,  /* inherit */
	.get_aux_rescls = _jive_node_get_aux_rescls  /* inherit */
};

const jive_node_class JIVE_GAMMA_NODE = {
	.parent = &JIVE_NODE,
	.name = "GAMMA",
	.fini = _jive_node_fini,  /* inherit */
	.get_default_normal_form = _jive_node_get_default_normal_form,  /* inherit */
	.get_label = _jive_node_get_label,  /* inherit */
	.get_attrs = _jive_node_get_attrs,  /* inherit */
	.match_attrs = _jive_node_match_attrs,  /* inherit */
	.create = _jive_node_create,  /* inherit */
	.get_aux_rescls = _jive_node_get_aux_rescls  /* inherit */
};

static jive_node *
jive_gamma_tail_node_create(jive_region * region)
{
	jive_node * self = jive_context_malloc(region->graph->context, sizeof(*self));
	JIVE_DECLARE_ANCHOR_TYPE(anchor);
	self->class_ = &JIVE_GAMMA_TAIL_NODE;
	_jive_node_init(self, region,
		0, NULL, NULL,
		1, &anchor);
	
	region->bottom = self;
	return self;
}

static jive_node *
jive_gamma_node_create(jive_region * region,
	jive_output * predicate,
	jive_output * false_alternative,
	jive_output * true_alternative)
{
	jive_node * self = jive_context_malloc(region->graph->context, sizeof(*self));;
	JIVE_DECLARE_ANCHOR_TYPE(anchor);
	JIVE_DECLARE_CONTROL_TYPE(ctl);
	self->class_ = &JIVE_GAMMA_NODE;
	_jive_node_init(self, region,
		3, (const jive_type *[]){anchor, anchor, ctl}, (jive_output *[]){false_alternative, true_alternative, predicate},
		0, NULL);
	
	return self;
}

jive_node *
jive_gamma_create(
	jive_region * region,
	jive_output * predicate,
	size_t nvalues, const jive_type * types[const],
	jive_output * false_values[const],
	jive_output * true_values[const])
{
	jive_region * false_region = jive_region_create_subregion(region);
	jive_region * true_region = jive_region_create_subregion(region);
	jive_node * false_alt = jive_gamma_tail_node_create(false_region);
	jive_node * true_alt = jive_gamma_tail_node_create(true_region);
	jive_node * gamma = jive_gamma_node_create(region, predicate, false_alt->outputs[0], true_alt->outputs[0]);
	
	size_t n;
	for (n = 0; n < nvalues; n++) {
		char name[80];
		snprintf(name, sizeof(name), "gamma_%08x_%d", (intptr_t)gamma, n);
		jive_gate * gate = jive_type_create_gate(types[n], region->graph, name);
		jive_node_gate_input(false_alt, gate, false_values[n]);
		jive_node_gate_input(true_alt, gate, true_values[n]);
		jive_node_gate_output(gamma, gate);
	}
	return gamma;
}

jive_output * const *
jive_choose(jive_output * predicate,
	size_t nvalues, const jive_type * types[const],
	jive_output * false_values[const],
	jive_output * true_values[const])
{
	jive_output * tmp[nvalues * 2 + 1];
	size_t n;
	tmp[0] = predicate;
	for(n = 0; n < nvalues; n++) tmp[n + 1] = false_values[n];
	for(n = 0; n < nvalues; n++) tmp[n + nvalues + 1] = true_values[n];
	jive_region * region = jive_region_innermost(nvalues * 2 + 1, tmp);
	
	return jive_gamma_create(region, predicate, nvalues, types, false_values, true_values)->outputs;
}

const jive_node_class JIVE_THETA_HEAD_NODE;
const jive_node_class JIVE_THETA_TAIL_NODE;
const jive_node_class JIVE_THETA_NODE;

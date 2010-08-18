#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/valuetype-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/crossings-private.h>
#include <jive/vsdg/resource-interference-private.h>
#include <jive/vsdg/regcls-count-private.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>

#include <jive/util/list.h>

static const jive_value_type jive_value_type_singleton = {
	.base = {.class_ = &JIVE_VALUE_TYPE}
};

const jive_type_class JIVE_VALUE_TYPE = {
	.parent = &JIVE_TYPE,
	.get_label = _jive_type_get_label, /* inherit */
	.create_input = _jive_value_type_create_input, /* override */
	.create_output = _jive_value_type_create_output, /* override */
	.create_resource = _jive_value_type_create_resource, /* override */
	.create_gate = _jive_value_type_create_gate, /* override */
	.equals = _jive_type_equals, /* inherit */
	.accepts = _jive_type_accepts /* inherit */
};

const jive_input_class JIVE_VALUE_INPUT = {
	.parent = &JIVE_INPUT,
	.fini = _jive_input_fini, /* inherit */
	.get_label = _jive_input_get_label, /* inherit */
	.get_type = _jive_value_input_get_type, /* override */
	.get_constraint = _jive_value_input_get_constraint /* override */
};

const jive_output_class JIVE_VALUE_OUTPUT = {
	.parent = &JIVE_OUTPUT,
	.fini = _jive_output_fini, /* inherit */
	.get_label = _jive_output_get_label, /* inherit */
	.get_type = _jive_value_output_get_type, /* override */
	.get_constraint = _jive_value_output_get_constraint /* override */
};

const jive_gate_class JIVE_VALUE_GATE = {
	.parent = &JIVE_GATE,
	.fini = _jive_gate_fini, /* inherit */
	.get_label = _jive_gate_get_label, /* inherit */
	.get_type = _jive_value_gate_get_type, /* override */
	.get_constraint = _jive_value_gate_get_constraint, /* override */
	.create_input = _jive_value_gate_create_input, /* override */
	.create_output = _jive_value_gate_create_output /* override */
};

const jive_resource_class JIVE_VALUE_RESOURCE = {
	.parent = &JIVE_RESOURCE,
	.fini = _jive_value_resource_fini, /* override */
	.get_label = _jive_resource_get_label, /* inherit */
	.get_type = _jive_value_resource_get_type, /* override */
	.can_merge = _jive_value_resource_can_merge, /* override */
	.merge = _jive_value_resource_merge, /* override */
	.get_cpureg = _jive_value_resource_get_cpureg, /* override */
	.get_regcls = _jive_value_resource_get_regcls, /* override */
	.get_real_regcls = _jive_value_resource_get_real_regcls, /* override */
	.add_squeeze = _jive_value_resource_add_squeeze, /* override */
	.sub_squeeze = _jive_value_resource_sub_squeeze, /* override */
	.deny_register = _jive_value_resource_deny_register, /* override */
	.recompute_allowed_registers = _jive_value_resource_recompute_allowed_registers /* override */
};

jive_input *
_jive_value_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_value_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.class_ = &JIVE_VALUE_INPUT;
	_jive_value_input_init(input, node, index, initial_operand);
	return &input->base; 
}

jive_output *
_jive_value_type_create_output(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_value_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.class_ = &JIVE_VALUE_OUTPUT;
	_jive_value_output_init(output, node, index);
	return &output->base; 
}

jive_resource *
_jive_value_type_create_resource(const jive_type * self, struct jive_graph * graph)
{
	jive_value_resource * resource = jive_context_malloc(graph->context, sizeof(*resource));
	resource->base.class_ = &JIVE_VALUE_RESOURCE;
	_jive_value_resource_init(resource, graph);
	return &resource->base; 
}

jive_gate *
_jive_value_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_value_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.class_ = &JIVE_VALUE_GATE;
	_jive_value_gate_init(gate, graph, name);
	return &gate->base; 
}

/* value inputs */

void
_jive_value_input_init(jive_value_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	_jive_input_init(&self->base, node, index, origin);
	self->required_regcls = 0;
}

const jive_type *
_jive_value_input_get_type(const jive_input * self)
{
	return &jive_value_type_singleton.base;
}

jive_resource *
_jive_value_input_get_constraint(const jive_input * self_)
{
	jive_value_input * self = (jive_value_input *) self_;
	jive_value_resource * resource = (jive_value_resource *) _jive_input_get_constraint(self_);
	/* don't overwrite regcls specified by gate (should be done differently though) */
	if (!self->base.gate) jive_value_resource_set_regcls(resource, self->required_regcls);
	return &resource->base;
}

/* value outputs */

void
_jive_value_output_init(jive_value_output * self, struct jive_node * node, size_t index)
{
	_jive_output_init(&self->base, node, index);
	self->required_regcls = 0;
}

const jive_type *
_jive_value_output_get_type(const jive_output * self)
{
	return &jive_value_type_singleton.base;
}

jive_resource *
_jive_value_output_get_constraint(const jive_output * self_)
{
	jive_value_output * self = (jive_value_output *) self_;
	jive_value_resource * resource = (jive_value_resource *) _jive_output_get_constraint(self_);
	/* don't overwrite regcls specified by gate (should be done differently though) */
	if (!self->base.gate) jive_value_resource_set_regcls(resource, self->required_regcls);
	return &resource->base;
}

/* value resource, i.e. registers */

static void
jive_value_resource_clear_allowed_registers(jive_value_resource * self)
{
	struct jive_allowed_registers_hash_iterator i;
	i = jive_allowed_registers_hash_begin(&self->allowed_registers);
	while(i.entry) {
		jive_value_allowed_register * allowed = i.entry;
		jive_allowed_registers_hash_iterator_next(&i);
		jive_allowed_registers_hash_remove(&self->allowed_registers, allowed);
		jive_context_free(self->base.graph->context, allowed);
	}
}

void
_jive_value_resource_init(jive_value_resource * self, struct jive_graph * graph)
{
	_jive_resource_init(&self->base, graph);
	self->cpureg = 0;
	self->regcls = 0;
	self->squeeze = 0;
	jive_allowed_registers_hash_init(&self->allowed_registers, graph->context);
}

void
_jive_value_resource_fini(jive_resource * self_)
{
	jive_value_resource * self = (jive_value_resource *) self_;
	if (self->regcls) jive_graph_valueres_tracker_remove(&self->base.graph->valueres, self);
	jive_value_resource_clear_allowed_registers(self);
	jive_allowed_registers_hash_fini(&self->allowed_registers);
	_jive_resource_fini(&self->base);
}

const jive_type *
_jive_value_resource_get_type(const jive_resource * self)
{
	return &jive_value_type_singleton.base;
}

const jive_regcls *
jive_value_resource_check_change_regcls(const jive_value_resource * self, const jive_regcls * new_regcls)
{
	const jive_regcls * old_regcls = self->regcls, * overflow;
	
	const jive_node_resource_interaction * xpoint;
	JIVE_LIST_ITERATE(self->base.node_interaction, xpoint, same_resource_list) {
		if (xpoint->before_count) {
			overflow = jive_regcls_count_check_change(&xpoint->node->use_count_before, old_regcls, new_regcls);
			if (overflow) return overflow;
		}
		if (xpoint->after_count) {
			overflow = jive_regcls_count_check_change(&xpoint->node->use_count_after, old_regcls, new_regcls);
			if (overflow) return overflow;
		}
	}
	
	return 0;
}

bool
_jive_value_resource_can_merge(const jive_resource * self_, const jive_resource * other_)
{
	const jive_value_resource * self = (jive_value_resource *) self_;
	if (!_jive_resource_can_merge(&self->base, other_)) return false;
	
	const jive_value_resource * other = (jive_value_resource *) other_;
	
	if (self->cpureg && other->cpureg && (self->cpureg != other->cpureg)) return false;
	
	const jive_regcls * old_regcls = self->regcls, * new_regcls;
	if (old_regcls) {
		new_regcls = jive_regcls_intersection(old_regcls, jive_resource_get_real_regcls(&other->base));
		if (!new_regcls) return false;
	} else new_regcls = jive_resource_get_real_regcls(&other->base);
	
	const jive_regcls * overflow = jive_value_resource_check_change_regcls(self, new_regcls);
	if (overflow) return false;
	overflow = jive_value_resource_check_change_regcls(other, new_regcls);
	if (overflow) return false;
	
	/* FIXME: maybe check for cpureg assignment? */
	return true;
}

void
_jive_value_resource_merge(jive_resource * self_, jive_resource * other_)
{
	if (!other_) return;
	_jive_resource_merge(self_, other_);
	
	jive_value_resource * self = (jive_value_resource *) self_;
	jive_value_resource * other = (jive_value_resource *) other_;
	
	/* FIXME: maybe check for cpureg assignment? */
	const jive_regcls * new_regcls = jive_regcls_intersection(self->regcls, other->regcls);
	DEBUG_ASSERT((!self->regcls && !other->regcls) || new_regcls);
	jive_value_resource_set_regcls(self, new_regcls);
}

const struct jive_cpureg *
_jive_value_resource_get_cpureg(const jive_resource * self_)
{
	jive_value_resource * self = (jive_value_resource *) self_;
	return self->cpureg;
}

const struct jive_regcls *
_jive_value_resource_get_regcls(const jive_resource * self_)
{
	jive_value_resource * self = (jive_value_resource *) self_;
	return self->regcls;
}

const struct jive_regcls *
_jive_value_resource_get_real_regcls(const jive_resource * self_)
{
	jive_value_resource * self = (jive_value_resource *) self_;
	if (self->cpureg) return self->cpureg->regcls;
	else return self->regcls;
}

void
_jive_value_resource_add_squeeze(jive_resource * self_, const jive_regcls * regcls)
{
	jive_value_resource * self = (jive_value_resource *) self_;
	if (self->cpureg || !self->regcls || !regcls) return;
	
	if (self->regcls) jive_graph_valueres_tracker_remove(&self->base.graph->valueres, self);
	if (jive_regcls_intersection(self->regcls, regcls)) self->squeeze++;
	if (self->regcls) jive_graph_valueres_tracker_add(&self->base.graph->valueres, self);
}

void
_jive_value_resource_sub_squeeze(jive_resource * self_, const jive_regcls * regcls)
{
	jive_value_resource * self = (jive_value_resource *) self_;
	if (self->cpureg || !self->regcls || !regcls) return;
	
	if (self->regcls) jive_graph_valueres_tracker_remove(&self->base.graph->valueres, self);
	if (jive_regcls_intersection(self->regcls, regcls)) self->squeeze--;
	if (self->regcls) jive_graph_valueres_tracker_add(&self->base.graph->valueres, self);
}

void
_jive_value_resource_deny_register(jive_resource * self_, const jive_cpureg * reg)
{
	jive_value_resource * self = (jive_value_resource *) self_;
	jive_value_allowed_register * allowed = jive_allowed_registers_hash_lookup(&self->allowed_registers, reg);
	if (!allowed) return;
	
	if (self->regcls) jive_graph_valueres_tracker_remove(&self->base.graph->valueres, self);
	
	jive_allowed_registers_hash_remove(&self->allowed_registers, allowed);
	jive_context_free(self->base.graph->context, allowed);
	
	if (self->regcls) jive_graph_valueres_tracker_add(&self->base.graph->valueres, self);
}

static void
_recompute_allowed_registers(jive_value_resource * self)
{
	jive_value_resource_clear_allowed_registers(self);
	self->squeeze = 0;
	
	if (self->cpureg) {
		jive_value_allowed_register * allowed = jive_context_malloc(self->base.graph->context, sizeof(*allowed));
		allowed->reg = self->cpureg;
		jive_allowed_registers_hash_insert(&self->allowed_registers, allowed);
	} else  if (self->regcls) {
		size_t n;
		for(n=0; n<self->regcls->nregs; n++) {
			jive_value_allowed_register * allowed = jive_context_malloc(self->base.graph->context, sizeof(*allowed));
			allowed->reg = &self->regcls->regs[n];
			jive_allowed_registers_hash_insert(&self->allowed_registers, allowed);
		}
		
		struct jive_resource_interference_hash_iterator i;
		JIVE_HASH_ITERATE(jive_resource_interference_hash, self->base.interference, i) {
			jive_resource * other = i.entry->resource;
			const jive_cpureg * reg = jive_resource_get_cpureg(other);
			if (reg) {
				jive_value_allowed_register * allowed = jive_allowed_registers_hash_lookup(&self->allowed_registers, reg);
				if (!allowed) continue;
				jive_allowed_registers_hash_remove(&self->allowed_registers, allowed);
				jive_context_free(self->base.graph->context, allowed);
			} else {
				const jive_regcls * regcls = jive_resource_get_regcls(other);
				if (regcls && jive_regcls_intersection(self->regcls, regcls)) self->squeeze ++;
			}
		}
	}
}

void
_jive_value_resource_recompute_allowed_registers(jive_resource * self_)
{
	/* recompute both "squeeze" as well as the set of allowed registers
	from scratch; this must be done whenever an assignment is removed
	from a neighbour, as an incremental recomputation is just not
	feasible in this case */
	jive_value_resource * self = (jive_value_resource *) self_;
	if (self->regcls) jive_graph_valueres_tracker_remove(&self->base.graph->valueres, self);
	_recompute_allowed_registers(self);
	if (self->regcls) jive_graph_valueres_tracker_add(&self->base.graph->valueres, self);
}

static void
jive_value_resource_nodes_change_regcls(jive_resource * self, const jive_regcls * old_regcls, const jive_regcls * new_regcls)
{
	jive_node_resource_interaction * xpoint;
	JIVE_LIST_ITERATE(self->node_interaction, xpoint, same_resource_list) {
		if (xpoint->before_count)
			jive_regcls_count_change(&xpoint->node->use_count_before, xpoint->node->graph->context, old_regcls, new_regcls);
		if (xpoint->after_count)
			jive_regcls_count_change(&xpoint->node->use_count_after, xpoint->node->graph->context, old_regcls, new_regcls);
	}
}

void
jive_value_resource_set_regcls(jive_value_resource * self, const jive_regcls * regcls)
{
	/* avoid doing expensive stuff if nothing changed */
	if (self->regcls == regcls) return;
	
	if (self->regcls) jive_graph_valueres_tracker_remove(&self->base.graph->valueres, self);
	
	if (self->regcls && !self->cpureg) {
		const jive_regcls * old_regcls = self->regcls;
		struct jive_resource_interference_hash_iterator i;
		JIVE_HASH_ITERATE(jive_resource_interference_hash, self->base.interference, i) {
			jive_resource * other = i.entry->resource;
			other->class_->sub_squeeze(other, old_regcls);
		}
	}
	
	const jive_regcls * old_regcls = jive_resource_get_real_regcls(&self->base);
	self->regcls = regcls;
	const jive_regcls * new_regcls = jive_resource_get_real_regcls(&self->base);
	jive_value_resource_nodes_change_regcls(&self->base, old_regcls, new_regcls);
	
	if (regcls && !self->cpureg) {
		struct jive_resource_interference_hash_iterator i;
		JIVE_HASH_ITERATE(jive_resource_interference_hash, self->base.interference, i) {
			jive_resource * other = i.entry->resource;
			other->class_->add_squeeze(other, regcls);
		}
	}
	
	_recompute_allowed_registers(self);
	
	if (self->regcls) jive_graph_valueres_tracker_add(&self->base.graph->valueres, self);
}

void
jive_value_resource_recompute_regcls(jive_value_resource * self)
{
	const jive_regcls * regcls = 0;
	jive_input * input;
	JIVE_LIST_ITERATE(self->base.inputs, input, resource_input_list)
		regcls = jive_regcls_intersection(regcls, ((jive_value_input *)input)->required_regcls);
	
	jive_output * output;
	JIVE_LIST_ITERATE(self->base.outputs, output, resource_output_list)
		regcls = jive_regcls_intersection(regcls, ((jive_value_output *)output)->required_regcls);
	
	jive_gate * gate;
	JIVE_LIST_ITERATE(self->base.gates, gate, resource_gate_list)
		regcls = jive_regcls_intersection(regcls, ((jive_value_gate *)gate)->required_regcls);
	
	jive_value_resource_set_regcls(self, regcls);
}

void
jive_value_resource_set_cpureg(jive_value_resource * self, const jive_cpureg * cpureg)
{
	if (self->regcls) jive_graph_valueres_tracker_remove(&self->base.graph->valueres, self);
	
	const jive_regcls * old_regcls = jive_resource_get_real_regcls(&self->base);
	if (self->cpureg) {
		self->cpureg = 0;
		/* this register may have become available for assignment to
		others, need to recalculate allowed registers for all
		neighbours */
		struct jive_resource_interference_hash_iterator i;
		JIVE_HASH_ITERATE(jive_resource_interference_hash, self->base.interference, i) {
			jive_resource * other = i.entry->resource;
			other->class_->recompute_allowed_registers(other);
		}
	}
	self->cpureg = cpureg;
	const jive_regcls * new_regcls = jive_resource_get_real_regcls(&self->base);
	jive_value_resource_nodes_change_regcls(&self->base, old_regcls, new_regcls);
	
	if (cpureg) {
		struct jive_resource_interference_hash_iterator i;
		JIVE_HASH_ITERATE(jive_resource_interference_hash, self->base.interference, i) {
			jive_resource * other = i.entry->resource;
			other->class_->deny_register(other, cpureg);
			other->class_->sub_squeeze(other, self->regcls);
		}
	}
	
	_recompute_allowed_registers(self);
	
	if (self->regcls) jive_graph_valueres_tracker_add(&self->base.graph->valueres, self);
}

/* value gates */

void
_jive_value_gate_init(jive_value_gate * self, struct jive_graph * graph, const char * name)
{
	_jive_gate_init(&self->base, graph, name);
	self->required_regcls = 0;
}

const jive_type *
_jive_value_gate_get_type(const jive_gate * self)
{
	return &jive_value_type_singleton.base;
}

jive_resource *
_jive_value_gate_get_constraint(jive_gate * self_)
{
	if (self_->resource) return self_->resource;
	jive_value_gate * self = (jive_value_gate *) self_;
	jive_value_resource * resource = (jive_value_resource *)_jive_gate_get_constraint(self_);
	jive_value_resource_set_regcls(resource, self->required_regcls);
	return &resource->base;
}

jive_input *
_jive_value_gate_create_input(const jive_gate * self_, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	const jive_value_gate * self = (const jive_value_gate *) self_;
	jive_value_input * input = (jive_value_input *) _jive_gate_create_input(&self->base, node, index, initial_operand);
	input->required_regcls = self->required_regcls;
	return &input->base;
}

jive_output *
_jive_value_gate_create_output(const jive_gate * self_, struct jive_node * node, size_t index)
{
	const jive_value_gate * self = (const jive_value_gate *) self_;
	jive_value_output * output = (jive_value_output *) _jive_gate_create_output(&self->base, node, index);
	output->required_regcls = self->required_regcls;
	return &output->base;
}


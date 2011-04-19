#include <jive/vsdg/negotiator.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

JIVE_DEFINE_HASH_TYPE(jive_negotiator_node_hash, jive_negotiator_constraint, jive_node *, hash_key.node, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_negotiator_gate_hash, jive_negotiator_constraint, jive_gate *, hash_key.gate, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_negotiator_input_hash, jive_negotiator_port, jive_input *, hash_key.input, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_negotiator_output_hash, jive_negotiator_port, jive_output *, hash_key.output, hash_chain);

static inline jive_negotiator_option *
jive_negotiator_option_create(const jive_negotiator * self)
{
	return self->class_->option_create(self);
}

static inline void
jive_negotiator_option_destroy(const jive_negotiator * self, jive_negotiator_option * option)
{
	self->class_->option_fini(self, option);
	jive_context_free(self->context, option);
}

static inline bool
jive_negotiator_option_equals(const jive_negotiator * self, const jive_negotiator_option * o1, const jive_negotiator_option * o2)
{
	return self->class_->option_equals(self, o1, o2);
}

static inline bool
jive_negotiator_option_specialize(const jive_negotiator * self, jive_negotiator_option * option)
{
	return self->class_->option_specialize(self, option);
}

static inline bool
jive_negotiator_option_intersect(const jive_negotiator * self, jive_negotiator_option * dst, const jive_negotiator_option * src)
{
	return self->class_->option_intersect(self, dst, src);
}

static inline bool
jive_negotiator_option_assign(const jive_negotiator * self, jive_negotiator_option * dst, const jive_negotiator_option * src)
{
	return self->class_->option_assign(self, dst, src);
}

static inline void
jive_negotiator_constraint_revalidate(jive_negotiator_constraint * self, jive_negotiator_port * port)
{
	self->class_->revalidate(self, port);
}

jive_negotiator_port *
jive_negotiator_port_create(jive_negotiator_constraint * constraint, jive_negotiator_connection * connection, const jive_negotiator_option * option)
{
	jive_negotiator * negotiator = constraint->negotiator;
	jive_negotiator_port * self = jive_context_malloc(negotiator->context, sizeof(*self));
	
	self->constraint = constraint;
	JIVE_LIST_PUSH_BACK(constraint->ports, self, constraint_port_list);
	
	self->connection = connection;
	JIVE_LIST_PUSH_BACK(connection->ports, self, connection_port_list);
	
	self->option = jive_negotiator_option_create(negotiator);
	jive_negotiator_option_assign(negotiator, self->option, option);
	
	jive_negotiator_connection_invalidate(connection);
	
	return self;
}

void
jive_negotiator_port_destroy(jive_negotiator_port * self)
{
	JIVE_LIST_REMOVE(self->constraint->ports, self, constraint_port_list);
	JIVE_LIST_REMOVE(self->connection->ports, self, connection_port_list);
	
	jive_negotiator_option_destroy(self->constraint->negotiator, self->option);
	
	jive_context_free(self->constraint->negotiator->context, self);
}

void
jive_negotiator_port_specialize(jive_negotiator_port * self)
{
	jive_negotiator * negotiator = self->constraint->negotiator;
	if (!jive_negotiator_option_specialize(negotiator, self->option))
		return;
	jive_negotiator_connection_invalidate(self->connection);
	jive_negotiator_constraint_revalidate(self->constraint, self);
}

jive_negotiator_connection *
jive_negotiator_connection_create(jive_negotiator * negotiator)
{
	jive_negotiator_connection * self = jive_context_malloc(negotiator->context, sizeof(*self));
	self->negotiator = negotiator;
	self->ports.first = self->ports.last = 0;
	self->validated = true;
	JIVE_LIST_PUSH_BACK(negotiator->validated_connections, self, negotiator_connection_list);
	return self;
}

void
jive_negotiator_connection_destroy(jive_negotiator_connection * self)
{
	JIVE_DEBUG_ASSERT(self->ports.first == 0 && self->ports.last == 0);
	if (self->validated) {
		JIVE_LIST_REMOVE(self->negotiator->validated_connections, self, negotiator_connection_list);
	} else {
		JIVE_LIST_REMOVE(self->negotiator->invalidated_connections, self, negotiator_connection_list);
	}
	jive_context_free(self->negotiator->context, self);
}

void
jive_negotiator_connection_revalidate(jive_negotiator_connection * self)
{
	jive_negotiator * negotiator = self->negotiator;
	
	/* compute common intersection of options */
	jive_negotiator_port * port;
	jive_negotiator_option * option = 0;
	JIVE_LIST_ITERATE(self->ports, port, connection_port_list) {
		if (option) {
			jive_negotiator_option_intersect(negotiator, option, port->option);
		} else {
			option = negotiator->tmp_option;
			jive_negotiator_option_assign(negotiator, option, port->option);
		}
	}
	
	struct {
		jive_negotiator_port * first;
		jive_negotiator_port * last;
	} unsatisfied = { 0, 0 };
	
	/* apply new constraint to all ports, determine those that are
	incompatible with changed option */
	jive_negotiator_port * next;
	JIVE_LIST_ITERATE_SAFE(self->ports, port, next, connection_port_list) {
		if (jive_negotiator_option_equals(negotiator, port->option, option))
			continue;
		if (jive_negotiator_option_intersect(negotiator, port->option, option))
			jive_negotiator_constraint_revalidate(port->constraint, port);
		else {
			JIVE_LIST_REMOVE(self->ports, port, connection_port_list);
			JIVE_LIST_PUSH_BACK(unsatisfied, port, connection_port_list);
		}
	}
	
	/* if any ports with non-matchable options remain, split them off
	and group them in a new connection */
	if (!unsatisfied.first)
		return;
	
	jive_negotiator_connection * new_connection = jive_negotiator_connection_create(negotiator);
	JIVE_LIST_ITERATE_SAFE(unsatisfied, port, next, connection_port_list) {
		JIVE_LIST_REMOVE(unsatisfied, port, connection_port_list);
		JIVE_LIST_PUSH_BACK(new_connection->ports, port, connection_port_list);
		port->connection = new_connection;
		jive_negotiator_connection_invalidate(new_connection);
	}
}

void
jive_negotiator_connection_invalidate(jive_negotiator_connection * self)
{
	if (!self->validated)
		return;
	self->validated = false;
	JIVE_LIST_REMOVE(self->negotiator->validated_connections, self, negotiator_connection_list);
	JIVE_LIST_PUSH_BACK(self->negotiator->invalidated_connections, self, negotiator_connection_list);
}

static void
jive_negotiator_connection_merge(jive_negotiator_connection * self, jive_negotiator_connection * other)
{
	if (self == other)
		return;
	jive_negotiator_port * port;
	jive_negotiator_port * next;
	JIVE_LIST_ITERATE_SAFE(other->ports, next, port, connection_port_list) {
		JIVE_LIST_REMOVE(other->ports, port, connection_port_list);
		JIVE_LIST_PUSH_BACK(self->ports, port, connection_port_list);
		port->connection = self;
	}
	
	jive_negotiator_connection_destroy(other);
	jive_negotiator_connection_invalidate(self);
}

/* constraint methods */

void
jive_negotiator_constraint_init_(jive_negotiator_constraint * self, jive_negotiator * negotiator, const jive_negotiator_constraint_class * class_)
{
	self->class_ = class_;
	self->negotiator = negotiator;
	self->ports.first = self->ports.last = 0;
	
	JIVE_LIST_PUSH_BACK(negotiator->constraints, self, negotiator_constraint_list);
	
	self->hash_key.node = 0;
}

void
jive_negotiator_constraint_fini_(jive_negotiator_constraint * self)
{
	JIVE_LIST_REMOVE(self->negotiator->constraints, self, negotiator_constraint_list);
}

void
jive_negotiator_constraint_destroy(jive_negotiator_constraint * self)
{
	JIVE_DEBUG_ASSERT(self->ports.first == 0 && self->ports.last == 0);
	self->class_->fini(self);
	jive_context_free(self->negotiator->context, self);
}

/* identity constraint */

static inline void
jive_negotiator_identity_constraint_revalidate(jive_negotiator_constraint * self, jive_negotiator_port * port)
{
	jive_negotiator * negotiator = self->negotiator;
	jive_negotiator_port * tmp;
	JIVE_LIST_ITERATE(self->ports, tmp, constraint_port_list) {
		if (tmp == port)
			continue;
		if (jive_negotiator_option_assign(negotiator, tmp->option, port->option))
			jive_negotiator_connection_invalidate(tmp->connection);
	}
}

static const jive_negotiator_constraint_class JIVE_NEGOTIATOR_IDENTITY_CONSTRAINT_CLASS = {
	.fini = jive_negotiator_constraint_fini_,
	.revalidate = jive_negotiator_identity_constraint_revalidate
};

jive_negotiator_constraint *
jive_negotiator_identity_constraint_create(jive_negotiator * self)
{
	jive_negotiator_constraint * constraint = jive_context_malloc(self->context, sizeof(*constraint));
	jive_negotiator_constraint_init_(constraint, self, &JIVE_NEGOTIATOR_IDENTITY_CONSTRAINT_CLASS);
	return constraint;
}

/* negotiator high-level interface */

void
jive_negotiator_init_(jive_negotiator * self, const jive_negotiator_class * class_, jive_graph * graph)
{
	self->class_ = class_;
	self->graph = graph;
	self->context = graph->context;
	jive_negotiator_input_hash_init(&self->input_map, graph->context);
	jive_negotiator_output_hash_init(&self->output_map, graph->context);
	jive_negotiator_node_hash_init(&self->node_map, graph->context);
	jive_negotiator_gate_hash_init(&self->gate_map, graph->context);
	
	self->validated_connections.first = self->validated_connections.last = 0;
	self->invalidated_connections.first = self->invalidated_connections.last = 0;
	self->constraints.first = self->constraints.last = 0;
	
	self->tmp_option = jive_negotiator_option_create(self);
}

void
jive_negotiator_fini_(jive_negotiator * self)
{
	jive_negotiator_option_destroy(self, self->tmp_option);
	
	while(self->constraints.first) {
		jive_negotiator_constraint * constraint = self->constraints.first;
		JIVE_LIST_REMOVE(self->constraints, constraint, negotiator_constraint_list);
		
		jive_negotiator_port * port, * next;
		JIVE_LIST_ITERATE_SAFE(constraint->ports, port, next, constraint_port_list) {
			jive_negotiator_port_destroy(port);
		}
		
		jive_negotiator_constraint_destroy(constraint);
	}
	
	while(self->validated_connections.first) {
		jive_negotiator_connection_destroy(self->validated_connections.first);
	}
	
	while(self->invalidated_connections.first) {
		jive_negotiator_connection_destroy(self->invalidated_connections.first);
	}
	
	jive_negotiator_input_hash_fini(&self->input_map);
	jive_negotiator_output_hash_fini(&self->output_map);
	jive_negotiator_node_hash_fini(&self->node_map);
	jive_negotiator_gate_hash_fini(&self->gate_map);
}

void
jive_negotiator_negotiate(jive_negotiator * self)
{
	while(self->invalidated_connections.first) {
		jive_negotiator_connection * connection = self->invalidated_connections.first;
		connection->validated = true;
		JIVE_LIST_REMOVE(self->invalidated_connections, connection, negotiator_connection_list);
		JIVE_LIST_PUSH_BACK(self->validated_connections, connection, negotiator_connection_list);
		jive_negotiator_connection_revalidate(connection);
	}
}

void
jive_negotiator_fully_specialize(jive_negotiator * self)
{
	struct jive_negotiator_input_hash_iterator i;
	JIVE_HASH_ITERATE(jive_negotiator_input_hash, self->input_map, i) {
		jive_negotiator_port * port = i.entry;
		jive_negotiator_port_specialize(port);
		jive_negotiator_negotiate(self);
	}
	struct jive_negotiator_output_hash_iterator j;
	JIVE_HASH_ITERATE(jive_negotiator_output_hash, self->output_map, j) {
		jive_negotiator_port * port = j.entry;
		jive_negotiator_port_specialize(port);
		jive_negotiator_negotiate(self);
	}
}

jive_negotiator_port *
jive_negotiator_map_input(const jive_negotiator * self, jive_input * input)
{
	return jive_negotiator_input_hash_lookup(&self->input_map, input);
}
	
jive_negotiator_port *
jive_negotiator_map_output(const jive_negotiator * self, jive_output * output)
{
	return jive_negotiator_output_hash_lookup(&self->output_map, output);
}
	
jive_negotiator_constraint *
jive_negotiator_map_gate(const jive_negotiator * self, jive_gate * gate)
{
	return jive_negotiator_gate_hash_lookup(&self->gate_map, gate);
}
	
jive_negotiator_connection *
jive_negotiator_create_input_connection(jive_negotiator * self, jive_input * input)
{
	jive_negotiator_port * output_port = jive_negotiator_map_output(self, input->origin);
	jive_negotiator_connection * connection;
	if (!output_port)
		connection = jive_negotiator_connection_create(self);
	else
		connection = output_port->connection;
	return connection;
}

jive_negotiator_connection *
jive_negotiator_create_output_connection(jive_negotiator * self, jive_output * output)
{
	jive_negotiator_connection * connection = 0;
	jive_input * user;
	JIVE_LIST_ITERATE(output->users, user, output_users_list) {
		jive_negotiator_port * port = jive_negotiator_map_input(self, user);
		if (connection && port)
			jive_negotiator_connection_merge(connection, port->connection);
		else if (port)
			connection = port->connection;
	}
	if (!connection)
		connection = jive_negotiator_connection_create(self);
	return connection;
}

jive_negotiator_constraint *
jive_negotiator_annotate_gate(jive_negotiator * self, jive_gate * gate)
{
	jive_negotiator_constraint * constraint = jive_negotiator_map_gate(self, gate);
	if (!constraint) {
		constraint = jive_negotiator_identity_constraint_create(self);
		constraint->hash_key.gate = gate;
		jive_negotiator_gate_hash_insert(&self->gate_map, constraint);
	}
	return constraint;
}

jive_negotiator_constraint *
jive_negotiator_annotate_identity(jive_negotiator * self,
	jive_input * const inputs[], size_t ninputs,
	jive_output * const outputs[], size_t noutputs,
	const jive_negotiator_option * option)
{
	jive_negotiator_constraint * constraint = jive_negotiator_identity_constraint_create(self);
	size_t n;
	for(n = 0; n < ninputs; n++) {
		jive_input * input = inputs[n];
		jive_negotiator_connection * connection = jive_negotiator_create_input_connection(self, input);
		jive_negotiator_port * port = jive_negotiator_port_create(constraint, connection, option);
		port->hash_key.input = input;
		jive_negotiator_input_hash_insert(&self->input_map, port);
	}
	
	for(n = 0; n < noutputs; n++) {
		jive_output * output = outputs[n];
		jive_negotiator_connection * connection = jive_negotiator_create_output_connection(self, output);
		jive_negotiator_port * port = jive_negotiator_port_create(constraint, connection, option);
		port->hash_key.output = output;
		jive_negotiator_output_hash_insert(&self->output_map, port);
	}
	return constraint;
}

jive_negotiator_constraint *
jive_negotiator_annotate_identity_node(jive_negotiator * self, jive_node * node, const jive_negotiator_option * option)
{
	size_t ninputs = 0;
	size_t noutputs = 0;
	/* FIXME: this assumes that all "gates" are at the end of the list
	-- while plausible, this is not strictly correct */
	while(ninputs < node->ninputs && !node->inputs[ninputs]->gate)
		ninputs ++;
	while(noutputs < node->noutputs && !node->outputs[noutputs]->gate)
		noutputs ++;
	jive_negotiator_constraint * constraint;
	constraint = jive_negotiator_annotate_identity(self, node->inputs, ninputs, node->outputs, noutputs, option);
	constraint->hash_key.node = node;
	jive_negotiator_node_hash_insert(&self->node_map, constraint);
	
	return constraint;
}

jive_negotiator_port *
jive_negotiator_annotate_simple_input(jive_negotiator * self, jive_input * input, const jive_negotiator_option * option)
{
	jive_negotiator_constraint * constraint = jive_negotiator_identity_constraint_create(self);
	jive_negotiator_connection * connection = jive_negotiator_create_input_connection(self, input);
	jive_negotiator_port * port = jive_negotiator_port_create(constraint, connection, option);
	port->hash_key.input = input;
	jive_negotiator_input_hash_insert(&self->input_map, port);
	
	return port;
}

jive_negotiator_port *
jive_negotiator_annotate_simple_output(jive_negotiator * self, jive_output * output, const jive_negotiator_option * option)
{
	jive_negotiator_constraint * constraint = jive_negotiator_identity_constraint_create(self);
	jive_negotiator_connection * connection = jive_negotiator_create_output_connection(self, output);
	jive_negotiator_port * port = jive_negotiator_port_create(constraint, connection, option);
	port->hash_key.output = output;
	jive_negotiator_output_hash_insert(&self->output_map, port);
	
	return port;
}

void
jive_negotiator_annotate_node_(jive_negotiator * self, jive_node * node)
{
	size_t n;
	for(n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		if (!input->gate) continue;
		if (!self->class_->option_gate_default(self, self->tmp_option, input->gate))
			continue;
		jive_negotiator_constraint * constraint = jive_negotiator_annotate_gate(self, input->gate);
		jive_negotiator_connection * connection = jive_negotiator_create_input_connection(self, input);
		jive_negotiator_port * port = jive_negotiator_port_create(constraint, connection, self->tmp_option);
		port->hash_key.input = input;
		jive_negotiator_input_hash_insert(&self->input_map, port);
	}
	for(n = 0; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		if (!output->gate) continue;
		if (!self->class_->option_gate_default(self, self->tmp_option, output->gate))
			continue;
		jive_negotiator_constraint * constraint = jive_negotiator_annotate_gate(self, output->gate);
		jive_negotiator_connection * connection = jive_negotiator_create_output_connection(self, output);
		jive_negotiator_port * port = jive_negotiator_port_create(constraint, connection, self->tmp_option);
		port->hash_key.output = output;
		jive_negotiator_output_hash_insert(&self->output_map, port);
	}
	self->class_->annotate_node_proper(self, node);
}

void
jive_negotiator_annotate_node_proper_(jive_negotiator * self, jive_node * node)
{
}

bool
jive_negotiator_option_gate_default_(const jive_negotiator * self, jive_negotiator_option * dst, const jive_gate * gate)
{
	return false;
}

void
jive_negotiator_process_region_(jive_negotiator * self, jive_region * region)
{
	jive_node * node;
	JIVE_LIST_ITERATE(region->nodes, node, region_nodes_list)
		self->class_->annotate_node(self, node);
	jive_negotiator_negotiate(self);
}

void
jive_negotiator_process(jive_negotiator * self)
{
	jive_region * region = self->graph->root_region;
	while(region->subregions.first)
		region = region->subregions.first;
	
	while(region) {
		self->class_->process_region(self, region);
		if (region->region_subregions_list.next)
			region = region->region_subregions_list.next;
		else
			region = region->parent;
	}
	jive_negotiator_fully_specialize(self);
}

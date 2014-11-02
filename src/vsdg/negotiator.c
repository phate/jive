/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/negotiator.h>

#include <jive/common.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/notifiers.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/traverser.h>

jive_negotiator_option::~jive_negotiator_option() noexcept
{
}

/* required forward decls */

void
jive_negotiator_connection_destroy(jive_negotiator_connection * self);

/* options */

static inline jive_negotiator_option *
jive_negotiator_option_create(const jive_negotiator * self)
{
	return self->class_->option_create(self);
}

/* operation */

namespace jive {

bool
negotiator_split_operation::operator==(const operation& gen_other) const noexcept
{
	const negotiator_split_operation * other =
		dynamic_cast<const negotiator_split_operation *>(&gen_other);
	
	return
		other &&
		input_type() == other->input_type() &&
		input_option() == other->input_option() &&
		output_type() == other->output_type() &&
		output_option() == other->output_option() &&
		negotiator() == other->negotiator();
}

const base::type &
negotiator_split_operation::argument_type(size_t index) const noexcept
{
	return *input_type_;
}

const base::type &
negotiator_split_operation::result_type(size_t index) const noexcept
{
	return *output_type_;
}

jive_node *
negotiator_split_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(*this, region, arguments, arguments + narguments);
}


std::string
negotiator_split_operation::debug_string() const
{
	return "NEGOTIATOR_SPLIT";
}

jive_unop_reduction_path_t
negotiator_split_operation::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	return jive_unop_reduction_none;
}

jive::output *
negotiator_split_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	return nullptr;
}

std::unique_ptr<jive::operation>
negotiator_split_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new negotiator_split_operation(*this));
}

}

static jive::output *
jive_negotiator_split(jive_negotiator * negotiator, const jive::base::type * operand_type,
	jive::output * operand, const jive_negotiator_option * input_option,
	const jive::base::type * output_type, const jive_negotiator_option * output_option)
{
	jive_graph * graph = operand->node()->region->graph;
	
	jive::negotiator_split_operation op(
		negotiator,
		*operand_type, *input_option,
		*output_type, *output_option);

	// Directly create node without going through normalization -- at this
	// point, normalization *must* not interfere in any way.
	jive_node * node = op.create_node(operand->node()->region, 1, &operand);
	jive_graph_mark_denormalized(node->graph);

	jive_negotiator_annotate_simple_input(negotiator, node->inputs[0], input_option);
	jive_negotiator_annotate_simple_output(negotiator, node->outputs[0], output_option);

	return node->outputs[0];
}

/* constraints */

static inline void
jive_negotiator_constraint_revalidate(
	jive_negotiator_constraint * self,
	jive_negotiator_port * port)
{
	self->class_->revalidate(self, port);
}

jive_negotiator_port *
jive_negotiator_port_create(
	jive_negotiator_constraint * constraint,
	jive_negotiator_connection * connection,
	const jive_negotiator_option * option)
{
	jive_negotiator * negotiator = constraint->negotiator;
	jive_negotiator_port * self = new jive_negotiator_port;
	
	self->constraint = constraint;
	JIVE_LIST_PUSH_BACK(constraint->ports, self, constraint_port_list);
	
	self->connection = connection;
	JIVE_LIST_PUSH_BACK(connection->ports, self, connection_port_list);
	
	self->specialized = false;
	JIVE_LIST_PUSH_BACK(negotiator->unspecialized_ports, self, specialized_list);
	
	self->option = option->copy();
	
	self->attach = jive_negotiator_port_attach_none;
	
	jive_negotiator_connection_invalidate(connection);
	
	return self;
}

void
jive_negotiator_port_divert(
	jive_negotiator_port * self,
	jive_negotiator_connection * new_connection)
{
	jive_negotiator_connection * old_connection = self->connection;
	JIVE_LIST_REMOVE(old_connection->ports, self, connection_port_list);
	JIVE_LIST_PUSH_BACK(new_connection->ports, self, connection_port_list);
	self->connection = new_connection;
	
	/* don't need to keep around connections with no port attached any longer */
	if (old_connection->ports.first == 0)
		jive_negotiator_connection_destroy(old_connection);
	
	jive_negotiator_connection_invalidate(new_connection);
}

void
jive_negotiator_port_split(jive_negotiator_port * self)
{
	jive_negotiator_connection * old_connection = self->connection;
	jive_negotiator * negotiator = old_connection->negotiator;
	jive_negotiator_connection * new_connection = jive_negotiator_connection_create(negotiator);
	
	JIVE_LIST_REMOVE(old_connection->ports, self, connection_port_list);
	JIVE_LIST_PUSH_BACK(new_connection->ports, self, connection_port_list);
	self->connection = new_connection;
	
	if (old_connection->ports.first == 0)
		jive_negotiator_connection_destroy(old_connection);
}

void
jive_negotiator_port_destroy(jive_negotiator_port * self)
{
	JIVE_LIST_REMOVE(self->constraint->ports, self, constraint_port_list);
	JIVE_LIST_REMOVE(self->connection->ports, self, connection_port_list);
	
	jive_negotiator * negotiator = self->constraint->negotiator;
	
	switch (self->attach) {
		case jive_negotiator_port_attach_none:
		case jive_negotiator_port_attach_input:
			negotiator->input_map.erase(self);
			break;
		case jive_negotiator_port_attach_output:
			negotiator->output_map.erase(self);
			break;
	}
	
	delete self->option;
	delete self;
}

void
jive_negotiator_port_specialize(jive_negotiator_port * self)
{
	jive_negotiator * negotiator = self->constraint->negotiator;
	
	JIVE_DEBUG_ASSERT(!self->specialized);
	JIVE_LIST_REMOVE(negotiator->unspecialized_ports, self, specialized_list);
	JIVE_LIST_PUSH_BACK(negotiator->specialized_ports, self, specialized_list);
	self->specialized = true;
	
	if (!self->option->specialize())
		return;
	jive_negotiator_connection_invalidate(self->connection);
	jive_negotiator_constraint_revalidate(self->constraint, self);
}

jive_negotiator_connection *
jive_negotiator_connection_create(jive_negotiator * negotiator)
{
	jive_negotiator_connection * self = new jive_negotiator_connection;
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
	delete self;
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
			option->intersect(*port->option);
		} else {
			option = negotiator->tmp_option;
			option->assign(*port->option);
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
		if (*port->option == *option)
			continue;
		if (port->option->intersect(*option))
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
jive_negotiator_connection_merge(
	jive_negotiator_connection * self,
	jive_negotiator_connection * other)
{
	if (self == other)
		return;
	jive_negotiator_port * port;
	jive_negotiator_port * next;
	JIVE_LIST_ITERATE_SAFE(other->ports, port, next, connection_port_list) {
		JIVE_LIST_REMOVE(other->ports, port, connection_port_list);
		JIVE_LIST_PUSH_BACK(self->ports, port, connection_port_list);
		port->connection = self;
	}
	
	jive_negotiator_connection_destroy(other);
	jive_negotiator_connection_invalidate(self);
}

/* constraint methods */

void
jive_negotiator_constraint_init_(
	jive_negotiator_constraint * self,
	jive_negotiator * negotiator,
	const jive_negotiator_constraint_class * class_)
{
	self->class_ = class_;
	self->negotiator = negotiator;
	self->ports.first = self->ports.last = 0;
	
	JIVE_LIST_PUSH_BACK(negotiator->constraints, self, negotiator_constraint_list);
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
	delete self;
}

/* identity constraint */

static inline void
jive_negotiator_identity_constraint_revalidate(
	jive_negotiator_constraint * self,
	jive_negotiator_port * port)
{
	jive_negotiator * negotiator = self->negotiator;
	jive_negotiator_port * tmp;
	JIVE_LIST_ITERATE(self->ports, tmp, constraint_port_list) {
		if (tmp == port)
			continue;
		if (tmp->option->assign(*port->option))
			jive_negotiator_connection_invalidate(tmp->connection);
	}
}

static const jive_negotiator_constraint_class JIVE_NEGOTIATOR_IDENTITY_CONSTRAINT_CLASS = {
	fini : jive_negotiator_constraint_fini_,
	revalidate : jive_negotiator_identity_constraint_revalidate
};

jive_negotiator_constraint *
jive_negotiator_identity_constraint_create(jive_negotiator * self)
{
	jive_negotiator_constraint * constraint = new jive_negotiator_constraint;
	jive_negotiator_constraint_init_(constraint, self, &JIVE_NEGOTIATOR_IDENTITY_CONSTRAINT_CLASS);
	return constraint;
}

/* glue code */

static void
jive_negotiator_on_node_create_(
	void * closure,
	jive_node * node)
{
	jive_negotiator * self = (jive_negotiator *) closure;
	if (dynamic_cast<const jive::negotiator_split_operation *>(&node->operation())) {
		self->split_nodes.insert(node);
	}
}

static void
jive_negotiator_on_node_destroy_(
	void * closure,
	jive_node * node)
{
	jive_negotiator * self = (jive_negotiator *) closure;
	self->split_nodes.erase(node);
}

/* negotiator high-level interface */

void
jive_negotiator_init_(
	jive_negotiator * self,
	const jive_negotiator_class * class_,
	jive_graph * graph)
{
	self->class_ = class_;
	self->graph = graph;

	self->validated_connections.first = self->validated_connections.last = 0;
	self->invalidated_connections.first = self->invalidated_connections.last = 0;
	self->constraints.first = self->constraints.last = 0;
	self->unspecialized_ports.first = self->unspecialized_ports.last = 0;
	self->specialized_ports.first = self->specialized_ports.last = 0;
	
	self->tmp_option = jive_negotiator_option_create(self);
	
	self->node_create_callback = jive_node_notifier_slot_connect(
		&graph->on_node_create, jive_negotiator_on_node_create_, self);
	self->node_destroy_callback = jive_node_notifier_slot_connect(
		&graph->on_node_destroy, jive_negotiator_on_node_destroy_, self);
}

void
jive_negotiator_fini_(jive_negotiator * self)
{
	jive_notifier_disconnect(self->node_create_callback);
	jive_notifier_disconnect(self->node_destroy_callback);
	
	delete self->tmp_option;
	
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
	while (self->unspecialized_ports.first) {
		jive_negotiator_port * port = self->unspecialized_ports.first;
		jive_negotiator_port_specialize(port);
		jive_negotiator_negotiate(self);
	}
}

jive_negotiator_port *
jive_negotiator_map_input(const jive_negotiator * self, jive::input * input)
{
	auto i = self->input_map.find(input);
	if (i != self->input_map.end())
		return i.ptr();
	else
		return nullptr;
}
	
jive_negotiator_port *
jive_negotiator_map_output(const jive_negotiator * self, jive::output * output)
{
	auto i = self->output_map.find(output);
	if (i != self->output_map.end())
		return i.ptr();
	else
		return nullptr;
}
	
jive_negotiator_constraint *
jive_negotiator_map_gate(const jive_negotiator * self, jive::gate * gate)
{
	auto i = self->gate_map.find(gate);
	if (i != self->gate_map.end())
		return i.ptr();
	else
		return nullptr;
}
	
jive_negotiator_connection *
jive_negotiator_create_input_connection(jive_negotiator * self, jive::input * input)
{
	jive_negotiator_port * output_port = jive_negotiator_map_output(self, input->origin());
	jive_negotiator_connection * connection;
	if (!output_port)
		connection = jive_negotiator_connection_create(self);
	else
		connection = output_port->connection;
	return connection;
}

jive_negotiator_connection *
jive_negotiator_create_output_connection(jive_negotiator * self, jive::output * output)
{
	jive_negotiator_connection * connection = 0;
	jive::input * user;
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
jive_negotiator_annotate_gate(jive_negotiator * self, jive::gate * gate)
{
	jive_negotiator_constraint * constraint = jive_negotiator_map_gate(self, gate);
	if (!constraint) {
		constraint = jive_negotiator_identity_constraint_create(self);
		constraint->hash_key_.gate = gate;
		self->gate_map.insert(constraint);
	}
	return constraint;
}

jive_negotiator_constraint *
jive_negotiator_annotate_identity(jive_negotiator * self,
	size_t ninputs, jive::input * const inputs[],
	size_t noutputs, jive::output * const outputs[],
	const jive_negotiator_option * option)
{
	jive_negotiator_constraint * constraint = jive_negotiator_identity_constraint_create(self);
	size_t n;
	for(n = 0; n < ninputs; n++) {
		jive::input * input = inputs[n];
		jive_negotiator_connection * connection = jive_negotiator_create_input_connection(self, input);
		jive_negotiator_port * port = jive_negotiator_port_create(constraint, connection, option);
		port->hash_key_.input = input;
		self->input_map.insert(port);
		port->attach = jive_negotiator_port_attach_input;
	}
	
	for(n = 0; n < noutputs; n++) {
		jive::output * output = outputs[n];
		jive_negotiator_connection * connection = jive_negotiator_create_output_connection(self, output);
		jive_negotiator_port * port = jive_negotiator_port_create(constraint, connection, option);
		port->hash_key_.output = output;
		self->output_map.insert(port);
		port->attach = jive_negotiator_port_attach_output;
	}
	return constraint;
}

jive_negotiator_constraint *
jive_negotiator_annotate_identity_node(
	jive_negotiator * self,
	jive_node * node,
	const jive_negotiator_option * option)
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
	constraint = jive_negotiator_annotate_identity(
		self, ninputs, &node->inputs[0], noutputs, &node->outputs[0], option);
	constraint->hash_key_.node = node;
	self->node_map.insert(constraint);
	
	return constraint;
}

jive_negotiator_port *
jive_negotiator_annotate_simple_input(jive_negotiator * self, jive::input * input,
	const jive_negotiator_option * option)
{
	jive_negotiator_constraint * constraint = jive_negotiator_identity_constraint_create(self);
	jive_negotiator_connection * connection = jive_negotiator_create_input_connection(self, input);
	jive_negotiator_port * port = jive_negotiator_port_create(constraint, connection, option);
	port->hash_key_.input = input;
	self->input_map.insert(port);
	port->attach = jive_negotiator_port_attach_input;
	
	return port;
}

jive_negotiator_port *
jive_negotiator_annotate_simple_output(jive_negotiator * self, jive::output * output,
	const jive_negotiator_option * option)
{
	jive_negotiator_constraint * constraint = jive_negotiator_identity_constraint_create(self);
	jive_negotiator_connection * connection = jive_negotiator_create_output_connection(self, output);
	jive_negotiator_port * port = jive_negotiator_port_create(constraint, connection, option);
	port->hash_key_.output = output;
	self->output_map.insert(port);
	port->attach = jive_negotiator_port_attach_output;
	
	return port;
}

void
jive_negotiator_annotate_node_(jive_negotiator * self, jive_node * node)
{
	size_t n;
	for(n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		if (!input->gate) continue;
		if (!self->class_->option_gate_default(self, self->tmp_option, input->gate))
			continue;
		jive_negotiator_constraint * constraint =
			jive_negotiator_annotate_gate(self, input->gate);
		jive_negotiator_connection * connection =
			jive_negotiator_create_input_connection(self, input);
		jive_negotiator_port * port =
			jive_negotiator_port_create(constraint, connection, self->tmp_option);
		port->hash_key_.input = input;
		self->input_map.insert(port);
	}
	for(n = 0; n < node->noutputs; n++) {
		jive::output * output = node->outputs[n];
		if (!output->gate) continue;
		if (!self->class_->option_gate_default(self, self->tmp_option, output->gate))
			continue;
		jive_negotiator_constraint * constraint =
			jive_negotiator_annotate_gate(self, output->gate);
		jive_negotiator_connection * connection =
			jive_negotiator_create_output_connection(self, output);
		jive_negotiator_port * port =
			jive_negotiator_port_create(constraint, connection, self->tmp_option);
		port->hash_key_.output = output;
		self->output_map.insert(port);
	}
	self->class_->annotate_node_proper(self, node);
}

void
jive_negotiator_annotate_node_proper_(jive_negotiator * self, jive_node * node)
{
}

bool
jive_negotiator_option_gate_default_(const jive_negotiator * self, jive_negotiator_option * dst,
	const jive::gate * gate)
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

static void
jive_negotiator_maybe_split_edge(jive_negotiator * self, jive::output * origin, jive::input * input)
{
	jive_negotiator_port * origin_port = jive_negotiator_map_output(self, origin);
	if (!origin_port)
		return;
	
	jive_negotiator_port * input_port = jive_negotiator_map_input(self, input);
	if (!input_port)
		return;
	
	if (*origin_port->option == *input_port->option)
		return;
	
	const jive::base::type * type = &input->type();
	jive::output * split_output = jive_negotiator_split(self,
		type, input->origin(), origin_port->option,
		type, input_port->option);
	
	jive_negotiator_port * split_output_port = jive_negotiator_map_output(self, split_output);
	
	input->divert_origin(split_output);
	jive_negotiator_port_divert(input_port, split_output_port->connection);
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
	
	jive_negotiator_insert_split_nodes(self);
}

void
jive_negotiator_insert_split_nodes(jive_negotiator * self)
{
	jive_traverser * trav = jive_topdown_traverser_create(self->graph);
	
	jive_node * node  = jive_traverser_next(trav);
	for (; node; node = jive_traverser_next(trav)) {
		size_t n;
		for (n = 0; n < node->ninputs; n++) {
			jive::input * input = node->inputs[n];
			jive_negotiator_maybe_split_edge(self, input->origin(), input);
		}
	}
	
	jive_traverser_destroy(trav);
}

void
jive_negotiator_remove_split_nodes(jive_negotiator * self)
{
	auto i = self->split_nodes.begin();
	while (i != self->split_nodes.end()) {
		jive_node * node = *i;
		++i;
		jive_negotiator_port * input_port = jive_negotiator_map_input(self, node->inputs[0]);
		jive_negotiator_port * output_port = jive_negotiator_map_output(self, node->outputs[0]);
		jive_negotiator_port_destroy(input_port);
		jive_negotiator_port_destroy(output_port);
		jive_output_replace(node->outputs[0], node->inputs[0]->origin());
		jive_node_destroy(node);
	}
}

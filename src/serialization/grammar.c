/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/serialization/grammar.h>

#include <jive/arch/instruction.h>
#include <jive/serialization/nodecls-registry.h>
#include <jive/serialization/rescls-registry.h>
#include <jive/serialization/typecls-registry.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node.h>

void
jive_serialize_char_token(jive_serialization_driver * self,
	char c, jive_token_ostream * os)
{
	jive_token token;
	token.type = (jive_token_type) c;
	jive_token_ostream_put(os, &token);
}

bool
jive_deserialize_char_token(jive_serialization_driver * self,
	jive_token_istream * is, char c)
{
	const jive_token * token = jive_token_istream_current(is);
	if (token->type != (jive_token_type) c) {
		char msg[80] = "Expected '?'";
		msg[10] = c;
		self->error(self, msg);
		return false;
	}
	jive_token_istream_advance(is);
	return true;
}

void
jive_serialize_string(jive_serialization_driver * self,
	const char * str, size_t len, jive_token_ostream * os)
{
	jive_token token;
	token.type = jive_token_string;
	token.v.string.str = str;
	token.v.string.len = len;
	jive_token_ostream_put(os, &token);
}

bool
jive_deserialize_string(jive_serialization_driver * self,
	jive_token_istream * is, char ** str, size_t * len)
{
	const jive_token * token = jive_token_istream_current(is);
	if (token->type != jive_token_string) {
		self->error(self, "Expected string");
		return false;
	}
	*str = jive_context_malloc(self->context, token->v.string.len + 1);
	*len = token->v.string.len;
	memcpy(*str, token->v.string.str, token->v.string.len);
	(*str)[token->v.string.len] = 0;
	jive_token_istream_advance(is);
	return true;
}

void
jive_serialize_uint(jive_serialization_driver * self,
	uint64_t value, jive_token_ostream * os)
{
	jive_token_ostream_integral(os, value);
}

bool
jive_deserialize_uint(jive_serialization_driver * self,
	jive_token_istream * is, uint64_t * value)
{
	const jive_token * token = jive_token_istream_current(is);
	if (token->type != jive_token_integral) {
		self->error(self, "Expected unsigned integral value");
		return false;
	}
	*value = token->v.integral;
	jive_token_istream_advance(is);
	return true;
}

void
jive_serialize_int(jive_serialization_driver * self,
	int64_t value, jive_token_ostream * os)
{
	uint64_t abs_value;
	if (value < 0) {
		abs_value = -value;
		jive_token_ostream_char(os, '-');
	} else {
		abs_value = value;
	}
	jive_token_ostream_integral(os, abs_value);
}

bool
jive_deserialize_int(jive_serialization_driver * self,
	jive_token_istream * is, int64_t * value)
{
	const jive_token * current = jive_token_istream_current(is);
	const jive_token * next = jive_token_istream_next(is);
	if (current->type == jive_token_integral) {
		if (current->v.integral <= (uint64_t) 0x7fffffffffffffffLL) {
			*value = current->v.integral;
			jive_token_istream_advance(is);
			return true;
		} else {
			self->error(self, "Integral value out of range");
			return false;
		}
	} else if (current->type == jive_token_minus && next->type == jive_token_integral) {
		if (current->v.integral <= 1 + ~ (uint64_t) (-0x7fffffffffffffffLL-1)) {
			*value = - next->v.integral;
			jive_token_istream_advance(is);
			jive_token_istream_advance(is);
			return true;
		} else {
			self->error(self, "Integral value out of range");
			return false;
		}
	}
	self->error(self, "Expected integral value");
	return false;
}

void
jive_serialize_type(jive_serialization_driver * self,
	const jive::base::type * type, jive_token_ostream * os)
{
	const jive_serialization_typecls * sercls;
	sercls = jive_serialization_typecls_lookup_by_cls(
		self->typecls_registry, typeid(*type));
	
	jive_token_ostream_identifier(os, sercls->tag);
	jive_serialize_char_token(self, '<', os);
	sercls->serialize(sercls, self, type, os);
	jive_serialize_char_token(self, '>', os);
}


bool
jive_deserialize_type(jive_serialization_driver * self,
	jive_token_istream * is, jive::base::type ** type)
{
	const jive_serialization_typecls * sercls = 0;
	const jive_token * token;
	
	token = jive_token_istream_current(is);
	if (token->type == jive_token_identifier) {
		sercls = jive_serialization_typecls_lookup_by_tag(
			self->typecls_registry, token->v.identifier);
	}
	if (!sercls) {
		self->error(self, "Expected type identifier");
		return false;
	}
	jive_token_istream_advance(is);
	
	if (!jive_deserialize_char_token(self, is, '<'))
		return false;
	
	if (!sercls->deserialize(sercls, self, is, type))
		return false;
	
	if (!jive_deserialize_char_token(self, is, '>')) {
		if (*type) {
			delete *type;
		}
		return false;
	}
	
	return true;
}

void
jive_serialize_rescls(jive_serialization_driver * self,
	const jive_resource_class * rescls, jive_token_ostream * os)
{
	const jive_serialization_rescls * sercls;
	sercls = jive_serialization_rescls_lookup_by_cls(
		self->rescls_registry, rescls);
	
	jive_token_ostream_identifier(os, sercls->tag);
	jive_serialize_char_token(self, '<', os);
	if (sercls->is_meta_class)
		sercls->serialize(sercls, self, rescls, os);
	jive_serialize_char_token(self, '>', os);
}

bool
jive_deserialize_rescls(jive_serialization_driver * self,
	jive_token_istream * is, const jive_resource_class ** rescls)
{
	const jive_serialization_rescls * sercls = 0;
	const jive_token * token;
	
	token = jive_token_istream_current(is);
	if (token->type == jive_token_identifier) {
		sercls = jive_serialization_rescls_lookup_by_tag(
			self->rescls_registry, token->v.identifier);
	}
	if (!sercls) {
		self->error(self, "Expected resource class identifier");
		return false;
	}
	jive_token_istream_advance(is);
	
	if (!jive_deserialize_char_token(self, is, '<'))
		return false;
	
	if (sercls->is_meta_class) {
		if (!sercls->deserialize(sercls, self, is, rescls))
			return false;
	} else
		*rescls = (const jive_resource_class *) sercls->cls;
	
	if (!jive_deserialize_char_token(self, is, '>'))
		return false;
	
	return true;
}

void
jive_serialize_gateexpr(jive_serialization_driver * self,
	jive_gate * gate, jive_token_ostream * os)
{
	jive_serialize_string(self, gate->name, strlen(gate->name), os);
	jive_serialize_rescls(self, gate->required_rescls, os);
	jive_serialize_type(self, &gate->type(), os);
}

bool
jive_deserialize_gateexpr(jive_serialization_driver * self,
	jive_token_istream * is, jive_graph * graph, jive_gate ** gate)
{
	char * name;
	size_t name_len;
	const jive_resource_class * rescls;
	jive::base::type * type;
	
	if (!jive_deserialize_string(self, is, &name, &name_len))
		return false;
	if (!jive_deserialize_rescls(self, is, &rescls)) {
		jive_context_free(self->context, name);
		return false;
	}
	if (!jive_deserialize_type(self, is, &type)) {
		jive_context_free(self->context, name);
		return false;
	}
	*gate = type->create_gate(graph, name);
	(*gate)->required_rescls = rescls;

	delete type;
	jive_context_free(self->context, name);
	
	return true;
}

void
jive_serialize_defined_gate(jive_serialization_driver * self,
	jive_gate * gate, jive_token_ostream * os)
{
	const jive_serialization_gatesym * sym =
		jive_serialization_symtab_gate_to_name(&self->symtab, gate);
	const char * gate_ident = sym ? sym->name : "%unnamed_gate%";
	jive_token_ostream_identifier(os, gate_ident);
}

bool
jive_deserialize_defined_gate(jive_serialization_driver * self,
	jive_token_istream * is, jive_gate ** gate)
{
	const jive_token * token = jive_token_istream_current(is);
	if (token->type != jive_token_identifier) {
		self->error(self, "Expected gate identifier");
		return false;
	}
	const jive_serialization_gatesym * sym =
		jive_serialization_symtab_name_to_gate(&self->symtab, token->v.identifier);
	if (!sym || !sym->gate) {
		self->error(self, "Expected gate identifier");
		return false;
	}
	*gate = sym->gate;
	jive_token_istream_advance(is);
	
	return true;
}

void
jive_serialize_defined_label(jive_serialization_driver * self,
	jive_label * label, jive_token_ostream * os)
{
	const jive_serialization_labelsym * sym =
		jive_serialization_symtab_label_to_name(&self->symtab, label);
	const char * label_ident = sym ? sym->name : "%unnamed_label%";
	jive_token_ostream_identifier(os, label_ident);
}

bool
jive_deserialize_defined_label(jive_serialization_driver * self,
	jive_token_istream * is, jive_label ** label)
{
	const jive_token * token = jive_token_istream_current(is);
	if (token->type != jive_token_identifier) {
		self->error(self, "Expected label identifier");
		return false;
	}
	const jive_serialization_labelsym * sym =
		jive_serialization_symtab_name_to_label(&self->symtab, token->v.identifier);
	if (!sym || !sym->label) {
		self->error(self, "Expected label identifier");
		return false;
	}
	*label = sym->label;
	jive_token_istream_advance(is);
	
	return true;
}

void
jive_serialize_defined_node(jive_serialization_driver * self,
	jive_node * node, jive_token_ostream * os)
{
	const jive_serialization_nodesym * sym =
		jive_serialization_symtab_node_to_name(&self->symtab, node);
	const char * node_ident = sym ? sym->name : "%unnamed_node%";
	jive_token_ostream_identifier(os, node_ident);
}

bool
jive_deserialize_defined_node(jive_serialization_driver * self,
	jive_token_istream * is, jive_node ** node)
{
	const jive_token * token = jive_token_istream_current(is);
	if (token->type != jive_token_identifier) {
		self->error(self, "Expected node identifier");
		return false;
	}
	const jive_serialization_nodesym * sym =
		jive_serialization_symtab_name_to_node(&self->symtab, token->v.identifier);
	if (!sym || !sym->node) {
		self->error(self, "Expected node identifier");
		return false;
	}
	*node = sym->node;
	jive_token_istream_advance(is);
	
	return true;
}

void
jive_serialize_portinfo(jive_serialization_driver * self,
	jive_portinfo * input, jive_token_ostream * os)
{
	jive_output * origin = input->origin;
	const jive_serialization_outputsym * sym =
		jive_serialization_symtab_output_to_name(&self->symtab, origin);
	const char * output_ident = sym ? sym->name : "%unnamed_output%";
	jive_token_ostream_identifier(os, output_ident);
	
	jive_serialize_char_token(self, ':', os);
	jive_serialize_rescls(self, input->required_rescls, os);
	
	if (input->gate) {
		jive_serialize_char_token(self, ':', os);
		jive_serialize_defined_gate(self, input->gate, os);
	}
}

bool
jive_deserialize_portinfo(jive_serialization_driver * self,
	jive_token_istream * is, jive_portinfo * port)
{
	port->origin = 0;
	port->required_rescls = 0;
	port->gate = 0;
	
	const jive_token * token;
	token = jive_token_istream_current(is);
	if (token->type != jive_token_identifier) {
		self->error(self, "Expected output identifier");
		return false;
	}
	const jive_serialization_outputsym * sym =
		jive_serialization_symtab_name_to_output(&self->symtab, token->v.identifier);
	if (!sym || !sym->output) {
		self->error(self, "Expected output identifier");
		return false;
	}
	port->origin = sym->output;
	jive_token_istream_advance(is);
	
	if (!jive_deserialize_char_token(self, is, ':'))
		return false;
	
	if (!jive_deserialize_rescls(self, is, &port->required_rescls))
		return false;
	
	if (jive_token_istream_current(is)->type == jive_token_colon) {
		if (!jive_deserialize_char_token(self, is, ':'))
			return false;
		
		if (!jive_deserialize_defined_gate(self, is, &port->gate))
			return false;
	}
	
	return true;
}

void
jive_serialize_portsinfo(jive_serialization_driver * self,
	jive_portsinfo * ports, jive_token_ostream * os)
{
	size_t n;
	
	jive_serialize_char_token(self, '(', os);
	
	for (n = 0; n < ports->nnormal; ++n)
		jive_serialize_portinfo(self, &ports->ports[n], os);
	
	jive_serialize_char_token(self, ';', os);
	
	for (n = ports->nnormal; n < ports->ntotal; ++n)
		jive_serialize_portinfo(self, &ports->ports[n], os);
	
	jive_serialize_char_token(self, ')', os);
}

bool
jive_deserialize_portsinfo(jive_serialization_driver * self,
	jive_token_istream * is, jive_portsinfo * ports)
{
	jive_portsinfo_init(ports, self->context);
	
	if (!jive_deserialize_char_token(self, is, '('))
		return false;
	
	while (jive_token_istream_current(is)->type == jive_token_identifier) {
		jive_portinfo * port = jive_portsinfo_append(ports);
		if (!jive_deserialize_portinfo(self, is, port)) {
			jive_portsinfo_fini(ports);
			return false;
		}
		ports->nnormal ++;
	}
	
	if (!jive_deserialize_char_token(self, is, ';'))
		return false;
	
	while (jive_token_istream_current(is)->type == jive_token_identifier) {
		jive_portinfo * port = jive_portsinfo_append(ports);
		if (!jive_deserialize_portinfo(self, is, port)) {
			jive_portsinfo_fini(ports);
			return false;
		}
	}
	
	if (!jive_deserialize_char_token(self, is, ')'))
		return false;
	
	return true;
}

void
jive_serialize_nodeexpr(jive_serialization_driver * self,
	jive_node * node, jive_token_ostream * os)
{
	size_t n;
	jive_portsinfo ports;
	
	/* inputs */
	jive_portsinfo_init(&ports, self->context);
	for (n = 0; n < node->ninputs; ++n) {
		jive_portinfo * port = jive_portsinfo_append(&ports);
		port->origin = node->inputs[n]->origin();
		port->required_rescls = node->inputs[n]->required_rescls;
		port->gate = node->inputs[n]->gate;
	}
	ports.nnormal = node->noperands;
	jive_serialize_portsinfo(self, &ports, os);
	jive_portsinfo_fini(&ports);
	
	/* attributes */
	const jive_serialization_nodecls * sercls;
	sercls = jive_serialization_nodecls_lookup_by_cls(
		self->nodecls_registry, node->class_);
	jive_token_ostream_identifier(os, sercls->tag);
	jive_serialize_char_token(self, '<', os);
	sercls->serialize(sercls, self, jive_node_get_attrs(node), os);
	jive_serialize_char_token(self, '>', os);
	
	/* outputs */
	jive_portsinfo_init(&ports, self->context);
	size_t nnormal = node->noutputs;
	for (n = 0; n < node->noutputs; ++n) {
		jive_portinfo * port = jive_portsinfo_append(&ports);
		port->origin = node->outputs[n];
		port->required_rescls = node->outputs[n]->required_rescls;
		port->gate = node->outputs[n]->gate;
		if (port->gate && n < nnormal)
			nnormal = n;
	}
	ports.nnormal = nnormal;
	jive_serialize_portsinfo(self, &ports, os);
	jive_portsinfo_fini(&ports);
}

bool
jive_deserialize_nodeexpr(jive_serialization_driver * self,
	jive_token_istream * is, jive_region * region, jive_node ** node)
{
	size_t n;
	jive_portsinfo ports;
	
	/* inputs */
	if (!jive_deserialize_portsinfo(self, is, &ports))
		return false;
	
	/* node class */
	const jive_serialization_nodecls * sercls = 0;
	const jive_token * token = jive_token_istream_current(is);
	if (token->type == jive_token_identifier) {
		sercls = jive_serialization_nodecls_lookup_by_tag(
			self->nodecls_registry, token->v.identifier);
	}
	if (!sercls) {
		self->error(self, "Expected node class identifier");
		jive_portsinfo_fini(&ports);
		return false;
	}
	jive_token_istream_advance(is);
	
	/* parse attributes & instantiate node */
	if (!jive_deserialize_char_token(self, is, '<')) {
		jive_portsinfo_fini(&ports);
		return false;
	}
	
	jive_output ** origins = jive_context_malloc(self->context,
		sizeof(jive_output *) * ports.nnormal);
	for (n = 0; n < ports.nnormal; ++n)
		origins[n] = ports.ports[n].origin;
	
	if (!sercls->deserialize(sercls, self, region,
		ports.nnormal, origins, is, node)) {
		jive_context_free(self->context, origins);
		jive_portsinfo_fini(&ports);
		return false;
	}
	jive_graph_mark_denormalized(region->graph);
	jive_context_free(self->context, origins);
	
	if (!jive_deserialize_char_token(self, is, '>'))
		return false;
	
	/* add ports & resource class requirements */
	for (n = ports.nnormal; n < ports.ntotal; ++n) {
		if (ports.ports[n].gate) {
			jive_node_gate_input(*node, ports.ports[n].gate,
				ports.ports[n].origin);
		} else {
			const jive_resource_class * rescls = ports.ports[n].required_rescls;
			const jive::base::type * type = jive_resource_class_get_type(rescls);
			jive_input * input = jive_node_add_input(*node, type, ports.ports[n].origin);
			input->required_rescls = rescls;
		}
	}
	for (n = 0; n < ports.ntotal; ++n) {
		(*node)->inputs[n]->required_rescls = ports.ports[n].required_rescls;
	}
	
	jive_portsinfo_fini(&ports);
	
	/* outputs */
	if (!jive_deserialize_char_token(self, is, '('))
		return false;
	
	size_t index = 0;
	/* result outputs */
	while (jive_token_istream_current(is)->type == jive_token_identifier) {
		if (index >= (*node)->noutputs) {
			self->error(self, "Too many names for outputs");
			return false;
		}
		char * name = jive_serialization_symtab_strdup(&self->symtab,
			jive_token_istream_current(is)->v.identifier);
		jive_token_istream_advance(is);
		
		if (!jive_deserialize_char_token(self, is, ':')) {
			jive_serialization_symtab_strfree(&self->symtab, name);
			return false;
		}
		
		const jive_resource_class * required_rescls;
		if (!jive_deserialize_rescls(self, is, &required_rescls)) {
			jive_serialization_symtab_strfree(&self->symtab, name);
			return false;
		}
		
		jive_serialization_symtab_insert_outputsym(&self->symtab,
			(*node)->outputs[index], name);
		
		index ++;
	}
	
	if (!jive_deserialize_char_token(self, is, ';'))
		return false;
	
	if (index != (*node)->noutputs) {
		self->error(self, "Too few names for outputs");
		return false;
	}
	
	/* additional outputs */
	while (jive_token_istream_current(is)->type == jive_token_identifier) {
		char * name = jive_serialization_symtab_strdup(&self->symtab,
			jive_token_istream_current(is)->v.identifier);
		jive_token_istream_advance(is);
		
		if (!jive_deserialize_char_token(self, is, ':')) {
			jive_serialization_symtab_strfree(&self->symtab, name);
			return false;
		}
		
		const jive_resource_class * required_rescls;
		if (!jive_deserialize_rescls(self, is, &required_rescls)) {
			jive_serialization_symtab_strfree(&self->symtab, name);
			return false;
		}
		
		jive_gate * gate = 0;
		if (jive_token_istream_current(is)->type == jive_token_colon) {
			jive_token_istream_advance(is);
			if (!jive_deserialize_defined_gate(self, is, &gate)) {
				jive_serialization_symtab_strfree(&self->symtab, name);
				return false;
			}
		}
		
		jive_output * output;
		if (gate)
			output = jive_node_gate_output(*node, gate);
		else
			output = jive_node_add_output(*node, jive_resource_class_get_type(required_rescls));
		
		output->required_rescls = required_rescls;
		
		jive_serialization_symtab_insert_outputsym(&self->symtab,
			output, name);
	}
	
	if (!jive_deserialize_char_token(self, is, ')'))
		return false;
	
	return true;
}

void
jive_serialize_label(jive_serialization_driver * self,
	const jive_label * label, jive_token_ostream * os)
{
	if (jive_label_isinstance(label, &JIVE_LABEL_CURRENT))
		jive_serialize_char_token(self, '.', os);
	else if (jive_label_isinstance(label, &JIVE_LABEL_FPOFFSET))
		jive_token_ostream_identifier(os, "frameptr"); /* FIXME: keyword */
	else if (jive_label_isinstance(label, &JIVE_LABEL_SPOFFSET))
		jive_token_ostream_identifier(os, "stackptr"); /* FIXME: keyword */
	else
		jive_serialize_defined_label(self, (jive_label *) label, os); /* FIXME: const-ness */
}

bool
jive_deserialize_label(jive_serialization_driver * self,
	jive_token_istream * is, const jive_label ** label)
{
	const jive_token * token = jive_token_istream_current(is);
	switch (token->type) {
		case jive_token_dot: {
			jive_token_istream_advance(is);
			*label = &jive_label_current;
			return true;
		}
		case jive_token_frameptr: {
			jive_token_istream_advance(is);
			*label = &jive_label_fpoffset;
			return true;
		}
		case jive_token_stackptr: {
			jive_token_istream_advance(is);
			*label = &jive_label_fpoffset;
			return true;
		}
		case jive_token_identifier: {
			jive_label * tmp;
			if (!jive_deserialize_defined_label(self, is, &tmp))
				return false;
			*label = tmp;
			return true;
		}
		default: {
			self->error(self, "Expected '.', 'frameptr', 'stackptr' or label identifier");
			return false;
		}
	}
}

void
jive_serialize_immediate(jive_serialization_driver * self,
	const jive_immediate * imm, jive_token_ostream * os)
{
	jive_serialize_uint(self, imm->offset, os);
	if (imm->add_label) {
		jive_serialize_char_token(self, '+', os);
		jive_serialize_label(self, imm->add_label, os);
	}
	if (imm->sub_label) {
		jive_serialize_char_token(self, '-', os);
		jive_serialize_label(self, imm->add_label, os);
	}
}

bool
jive_deserialize_immediate(jive_serialization_driver * self,
	jive_token_istream * is, jive_immediate * imm)
{
	uint64_t offset;
	if (!jive_deserialize_uint(self, is, &offset))
		return false;
	
	const jive_label * add_label = NULL;
	const jive_label * sub_label = NULL;
	
	if (jive_token_istream_current(is)->type == jive_token_plus) {
		jive_token_istream_advance(is);
		if (!jive_deserialize_label(self, is, &add_label))
			return false;
	}
	
	if (jive_token_istream_current(is)->type == jive_token_minus) {
		jive_token_istream_advance(is);
		if (!jive_deserialize_label(self, is, &sub_label))
			return false;
	}
	
	jive_immediate_init(imm, offset, add_label, sub_label, NULL);
	return true;
}

void
jive_serialize_nodedef(jive_serialization_driver * self,
	jive_serialization_namegen * namegen,
	jive_node * node, jive_token_ostream * os)
{
	namegen->name_node(namegen, &self->symtab, node);
	size_t n;
	for (n = 0; n < node->noutputs; ++n)
		namegen->name_output(namegen, &self->symtab, node->outputs[n]);
	jive_serialize_defined_node(self, node, os);
	jive_serialize_char_token(self, '=', os);
	jive_token_ostream_identifier(os, "node"); /* FIXME: keyword */
	jive_serialize_nodeexpr(self, node, os);
}

void
jive_serialize_gatedef(jive_serialization_driver * self,
	jive_serialization_namegen * namegen,
	struct jive_gate * gate, jive_token_ostream * os)
{
	namegen->name_gate(namegen, &self->symtab, gate);
	jive_serialize_defined_gate(self, gate, os);
	jive_serialize_char_token(self, '=', os);
	jive_token_ostream_identifier(os, "gate"); /* FIXME: keyword */
	jive_serialize_gateexpr(self, gate, os);
}

void
jive_serialize_regiondef(jive_serialization_driver * self,
	jive_serialization_namegen * namegen,
	struct jive_region * region, jive_token_ostream * os)
{
	jive_token_ostream_identifier(os, "region"); /* FIXME: keyword */
	jive_serialize_char_token(self, '{', os);
	jive_serialize_regionbody(self, namegen, region, os);
	jive_serialize_char_token(self, '}', os);
}

/* FIXME: merge with implementation used for copying! */

typedef struct jive_level_nodes jive_level_nodes;
struct jive_level_nodes {
	jive_node ** items;
	size_t nitems, space;
};

typedef struct jive_sorted_nodes jive_sorted_nodes;
struct jive_sorted_nodes {
	jive_level_nodes * depths;
	size_t min_depth;
	size_t max_depth_plus_one;
	size_t space;
};

static void
jive_sorted_nodes_init(jive_sorted_nodes * self)
{
	self->depths = 0;
	self->min_depth = (size_t)-1;
	self->max_depth_plus_one = 0;
	self->space = 0;
}

static void
jive_sorted_nodes_fini(jive_sorted_nodes * self, jive_context * context)
{
	size_t n;
	for (n = 0; n < self->max_depth_plus_one; n++)
		jive_context_free(context, self->depths[n].items);
	jive_context_free(context, self->depths);
}

static void
jive_level_nodes_append(jive_level_nodes * level, jive_context * context, jive_node * node)
{
	if (level->nitems == level->space) {
		level->space =  level->space * 2 + 1;
		level->items = jive_context_realloc(context, level->items, level->space * sizeof(jive_node *));
	}
	level->items[level->nitems ++] = node;
}

static void
jive_sorted_nodes_append(jive_sorted_nodes * self, jive_context * context, jive_node * node)
{
	if (node->depth_from_root >= self->space) {
		size_t new_space = self->space * 2;
		if (new_space <= node->depth_from_root)
			new_space = node->depth_from_root + 1;
		self->depths = jive_context_realloc(context, self->depths, new_space * sizeof(self->depths[0]));
		size_t n;
		for (n = self->space; n < new_space; n++) {
			self->depths[n].items = 0;
			self->depths[n].space = 0;
			self->depths[n].nitems = 0;
		}
		self->space = new_space;
	}
	jive_level_nodes_append(&self->depths[node->depth_from_root], context, node);
	if (node->depth_from_root + 1 > self->max_depth_plus_one)
		self->max_depth_plus_one = node->depth_from_root + 1;
	if (node->depth_from_root < self->min_depth)
		self->min_depth = node->depth_from_root;
}

void
jive_serialize_regionbody(jive_serialization_driver * self,
	jive_serialization_namegen * namegen,
	struct jive_region * region, jive_token_ostream * os)
{
	/*FIXME: serialization does not take care of graph tail node*/
	jive_context * context = self->context;
	jive_sorted_nodes sorted;
	jive_sorted_nodes_init(&sorted);
	
	jive_node * node;
	JIVE_LIST_ITERATE(region->nodes, node, region_nodes_list)
		jive_sorted_nodes_append(&sorted, context, node);
	
	size_t n;
	for (n = sorted.min_depth; n < sorted.max_depth_plus_one; ++n) {
		size_t k;
		const jive_level_nodes * level = &sorted.depths[n];
		for (k = 0; k < level->nitems; ++k) {
			node = level->items[k];

			size_t j;
			for (j = 0; j < node->ninputs; ++j) {
				jive_input * input = node->inputs[j];
				if (dynamic_cast<jive::achr::input*>(input))
					jive_serialize_regiondef(self, namegen, input->origin()->node()->region, os);
			}
			if (jive_node_isinstance(node, &JIVE_GRAPH_TAIL_NODE))
				continue;
			jive_serialize_nodedef(self, namegen, node, os);
			jive_serialize_char_token(self, ';', os);
		}
	}
	jive_sorted_nodes_fini(&sorted, context);
}

bool
jive_deserialize_regionbody(jive_serialization_driver * self,
	jive_token_istream * is,
	jive_region * region)
{
	for (;;) {
		const jive_token * token = jive_token_istream_current(is);
		if (token->type == jive_token_closebrace)
			break;
		if (token->type == jive_token_end)
			break;
		if (!jive_deserialize_def(self, is, region))
			return false;
	}
	
	return true;
}

bool
jive_deserialize_def(jive_serialization_driver * self,
	jive_token_istream * is,
	struct jive_region * region)
{
	const jive_token * token = jive_token_istream_current(is);
	
	switch (token->type) {
		case jive_token_region: {
			jive_token_istream_advance(is);
			if (!jive_deserialize_char_token(self, is, '{'))
				return false;
			jive_region * subregion = jive_region_create_subregion(region);
			if (!jive_deserialize_regionbody(self, is, subregion))
				return false;
			if (!jive_deserialize_char_token(self, is, '}'))
				return false;
			return true;
		}
		case jive_token_identifier: {
			char * name = jive_serialization_symtab_strdup(&self->symtab, token->v.identifier);
			jive_token_istream_advance(is);
			if (!jive_deserialize_char_token(self, is, '=')) {
				jive_serialization_symtab_strfree(&self->symtab, name);
				return false;
			}
			
			/* parse defined item */
			token = jive_token_istream_current(is);
			switch (token->type) {
				case jive_token_gate: {
					jive_token_istream_advance(is);
					jive_gate * gate;
					if (!jive_deserialize_gateexpr(self, is, region->graph, &gate)) {
						jive_serialization_symtab_strfree(&self->symtab, name);
						return false;
					}
					jive_serialization_symtab_insert_gatesym(
						&self->symtab, gate, name);
					break;
				}
				case jive_token_node: {
					jive_token_istream_advance(is);
					jive_node * node;
					if (!jive_deserialize_nodeexpr(self, is, region, &node)) {
						jive_serialization_symtab_strfree(&self->symtab, name);
						return false;
					}
					jive_serialization_symtab_insert_nodesym(
						&self->symtab, node, name);
					break;
				}
				default: {
					self->error(self, "Expected 'gate' or 'node'");
					return false;
				}
			}
			if (!jive_deserialize_char_token(self, is, ';'))
				return false;
			return true;
		}
		default: {
			self->error(self, "Expected 'region' or undeclared node or gate identifier");
			return false;
		}
	}
}

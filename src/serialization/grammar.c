/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
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

namespace jive {
namespace serialization {

parse_error::~parse_error() noexcept {}

void
parser_driver::parse_char_token(char expected)
{
	const jive_token * token = jive_token_istream_current(&is_);
	if (token->type != static_cast<jive_token_type>(expected)) {
		char msg[80] = "Expected '?'";
		msg[10] = expected;
		throw parse_error(msg);
	}
	jive_token_istream_advance(&is_);
}

uint64_t
parser_driver::parse_uint()
{
	const jive_token * token = jive_token_istream_current(&is_);
	if (token->type != jive_token_integral) {
		throw parse_error("Expected unsigned integral value");
	}
	uint64_t value = token->v.integral;
	jive_token_istream_advance(&is_);
	return value;
}

int64_t
parser_driver::parse_int()
{
	const jive_token * current = jive_token_istream_current(&is_);
	const jive_token * next = jive_token_istream_next(&is_);
	if (current->type == jive_token_integral) {
		if (current->v.integral <= (uint64_t) 0x7fffffffffffffffLL) {
			int64_t value = current->v.integral;
			jive_token_istream_advance(&is_);
			return value;
		} else {
			throw parse_error("Integral value out of range");
		}
	} else if (current->type == jive_token_minus && next->type == jive_token_integral) {
		if (current->v.integral <= (uint64_t) 0x7fffffffffffffffLL) {
			int64_t value = - next->v.integral;
			jive_token_istream_advance(&is_);
			jive_token_istream_advance(&is_);
			return value;
		} else {
			throw parse_error("Integral value out of range");
		}
	}
	throw parse_error("Expected integral value");
}

std::string
parser_driver::parse_string()
{
	const jive_token * token = jive_token_istream_current(&is_);
	if (token->type != jive_token_string) {
		throw parse_error("Expected string");
	}
	std::string result(token->v.string.str, token->v.string.len);
	jive_token_istream_advance(&is_);
	return result;
}

std::string
parser_driver::parse_identifier()
{
	const jive_token * token = jive_token_istream_current(&is_);
	if (token->type != jive_token_identifier) {
		throw parse_error("Expected identifier");
	}
	std::string result(token->v.identifier);
	jive_token_istream_advance(&is_);
	return result;
}

const jive_label *
parser_driver::parse_defined_label()
{
	const jive_token * token = jive_token_istream_current(&is_);
	if (token->type != jive_token_identifier) {
		throw parse_error("Expected label identifier");
	}
	const jive_serialization_labelsym * sym =
		jive_serialization_symtab_name_to_label(&driver().symtab, token->v.identifier);
	if (!sym || !sym->label) {
		throw parse_error("Expected label identifier");
	}
	const jive_label * label = sym->label;
	jive_token_istream_advance(&is_);
	
	return label;
}

const jive_label *
parser_driver::parse_label()
{
	switch (peek_token_type()) {
		case jive_token_dot: {
			jive_token_istream_advance(&is_);
			return &jive_label_current;
		}
		case jive_token_frameptr: {
			jive_token_istream_advance(&is_);
			return &jive_label_fpoffset;
		}
		case jive_token_stackptr: {
			jive_token_istream_advance(&is_);
			return &jive_label_fpoffset;
		}
		case jive_token_identifier: {
			return parse_defined_label();
		}
		default: {
			throw parse_error("Expected '.', 'frameptr', 'stackptr' or label identifier");
		}
	}
}

const jive_resource_class *
parser_driver::parse_resource_class()
{
	std::string identifier = parse_identifier();
	const jive_serialization_rescls * sercls =
		jive_serialization_rescls_lookup_by_tag(
			driver().rescls_registry, identifier.c_str());

	if (!sercls) {
		throw parse_error("Expected resource class identifier");
	}

	parse_char_token('<');

	const jive_resource_class * rescls = nullptr;
	if (sercls->is_meta_class) {
		if (!sercls->deserialize(sercls, &driver(), &istream(), &rescls)) {
			throw parse_error("Unable to parse resource class");
		}
	} else {
		rescls = (const jive_resource_class *) sercls->cls;
	}

	parse_char_token('>');

	return rescls;
}

const jive_resource_class *
parser_driver::parse_resource_class_or_null()
{
	std::string identifier = parse_identifier();
	if (identifier == "none") {
		return nullptr;
	}
	const jive_serialization_rescls * sercls =
		jive_serialization_rescls_lookup_by_tag(
			driver().rescls_registry, identifier.c_str());

	if (!sercls) {
		throw parse_error("Expected resource class identifier");
	}

	parse_char_token('<');

	const jive_resource_class * rescls = nullptr;
	if (sercls->is_meta_class) {
		if (!sercls->deserialize(sercls, &driver(), &istream(), &rescls)) {
			throw parse_error("Unable to parse resource class");
		}
	} else {
		rescls = (const jive_resource_class *) sercls->cls;
	}

	parse_char_token('>');

	return rescls;
}

jive_token_type
parser_driver::peek_token_type() const noexcept
{
	const jive_token * token = jive_token_istream_current(&is_);
	return token->type;
}

void
output_driver::put_char_token(char c)
{
	jive_token token;
	token.type = static_cast<jive_token_type>(c);
	jive_token_ostream_put(&os_, &token);
}

void
output_driver::put_uint(uint64_t value)
{
	jive_token_ostream_integral(&os_, value);
}

void
output_driver::put_int(int64_t value)
{
	uint64_t abs_value;
	if (value < 0) {
		abs_value = -value;
		put_char_token('-');
	} else {
		abs_value = value;
	}
	put_uint(abs_value);
}

void
output_driver::put_string(const std::string & s)
{
	jive_token token;
	token.type = jive_token_string;
	token.v.string.str = s.c_str();
	token.v.string.len = s.size();
	jive_token_ostream_put(&os_, &token);
}

void
output_driver::put_identifier(const std::string & identifier)
{
	jive_token_ostream_identifier(&ostream(), identifier.c_str());
}

void
output_driver::put_defined_label(const jive_label * label)
{
	const jive_serialization_labelsym * sym =
		jive_serialization_symtab_label_to_name(&driver_.symtab, label);
	const char * label_ident = sym ? sym->name.c_str() : "%unnamed_label%";
	jive_token_ostream_identifier(&os_, label_ident);
}

void
output_driver::put_label(const jive_label * label)
{
	if (jive_label_isinstance(label, &JIVE_LABEL_CURRENT)) {
		put_char_token('.');
	} else if (jive_label_isinstance(label, &JIVE_LABEL_FPOFFSET)) {
		jive_token_ostream_identifier(&os_, "frameptr"); /* FIXME: keyword */
	} else if (jive_label_isinstance(label, &JIVE_LABEL_SPOFFSET)) {
		jive_token_ostream_identifier(&os_, "stackptr"); /* FIXME: keyword */
	} else {
		put_defined_label(label); /* FIXME: const-ness */
	}
}

void
output_driver::put_resource_class(const jive_resource_class * rescls)
{
	const jive_serialization_rescls * sercls =
		jive_serialization_rescls_lookup_by_cls(
			driver().rescls_registry, rescls);

	put_identifier(sercls->tag);
	put_char_token('<');
	if (sercls->is_meta_class) {
		sercls->serialize(sercls, &driver(), rescls, &ostream());
	}
	put_char_token('>');
}

void
output_driver::put_resource_class_or_null(const jive_resource_class * rescls)
{
	if (!rescls) {
		put_identifier("none");
	} else {
		put_resource_class(rescls);
	}
}

}
}

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
	jive_token_istream * is, std::string & str)
{
	const jive_token * token = jive_token_istream_current(is);
	if (token->type != jive_token_string) {
		self->error(self, "Expected string");
		return false;
	}
	str = std::string(token->v.string.str, token->v.string.len);
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
		if (current->v.integral <= (uint64_t) 0x7fffffffffffffffLL) {
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
	jive::gate * gate, jive_token_ostream * os)
{
	jive_serialize_string(self, gate->name.c_str(), gate->name.size(), os);
	jive_serialize_rescls(self, gate->required_rescls, os);
	jive_serialize_type(self, &gate->type(), os);
}

bool
jive_deserialize_gateexpr(jive_serialization_driver * self,
	jive_token_istream * is, jive_graph * graph, jive::gate ** gate)
{
	std::string name;
	const jive_resource_class * rescls;
	jive::base::type * type;
	
	if (!jive_deserialize_string(self, is, name))
		return false;

	if (!jive_deserialize_rescls(self, is, &rescls))
		return false;

	if (!jive_deserialize_type(self, is, &type))
		return false;

	*gate = jive_graph_create_gate(graph, name, *type);
	(*gate)->required_rescls = rescls;

	delete type;
	
	return true;
}

void
jive_serialize_defined_gate(jive_serialization_driver * self,
	jive::gate * gate, jive_token_ostream * os)
{
	const jive_serialization_gatesym * sym =
		jive_serialization_symtab_gate_to_name(&self->symtab, gate);
	const char * gate_ident = sym ? sym->name.c_str() : "%unnamed_gate%";
	jive_token_ostream_identifier(os, gate_ident);
}

bool
jive_deserialize_defined_gate(jive_serialization_driver * self,
	jive_token_istream * is, jive::gate ** gate)
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
	const char * label_ident = sym ? sym->name.c_str() : "%unnamed_label%";
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
	const char * node_ident = sym ? sym->name.c_str() : "%unnamed_node%";
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
	jive::output * origin = input->origin;
	const jive_serialization_outputsym * sym =
		jive_serialization_symtab_output_to_name(&self->symtab, origin);
	const char * output_ident = sym ? sym->name.c_str() : "%unnamed_output%";
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
	JIVE_DEBUG_ASSERT(ports->nnormal <= ports->ports.size());
	
	jive_serialize_char_token(self, '(', os);
	
	for (size_t n = 0; n < ports->nnormal; ++n)
		jive_serialize_portinfo(self, &ports->ports[n], os);
	
	jive_serialize_char_token(self, ';', os);
	
	for (size_t n = ports->nnormal; n < ports->ports.size(); ++n)
		jive_serialize_portinfo(self, &ports->ports[n], os);
	
	jive_serialize_char_token(self, ')', os);
}

bool
jive_deserialize_portsinfo(jive_serialization_driver * self,
	jive_token_istream * is, jive_portsinfo * ports)
{
	if (!jive_deserialize_char_token(self, is, '('))
		return false;
	
	while (jive_token_istream_current(is)->type == jive_token_identifier) {
		jive_portinfo port;
		if (!jive_deserialize_portinfo(self, is, &port)) {
			return false;
		}
		ports->ports.push_back(port);
		ports->nnormal ++;
	}
	
	if (!jive_deserialize_char_token(self, is, ';'))
		return false;
	
	while (jive_token_istream_current(is)->type == jive_token_identifier) {
		jive_portinfo port;
		if (!jive_deserialize_portinfo(self, is, &port)) {
			return false;
		}
		ports->ports.push_back(port);
	}
	
	if (!jive_deserialize_char_token(self, is, ')'))
		return false;
	
	return true;
}

#include <iostream>

void
jive_serialize_nodeexpr(jive_serialization_driver * self,
	jive_node * node, jive_token_ostream * os)
{
	size_t n;
	jive_portsinfo inports;
	
	/* inputs */
	for (n = 0; n < node->ninputs; ++n) {
		jive_portinfo port;
		port.origin = node->inputs[n]->origin();
		port.required_rescls = node->inputs[n]->required_rescls;
		port.gate = node->inputs[n]->gate;
		inports.ports.push_back(port);
	}
	inports.nnormal = node->noperands;
	jive_serialize_portsinfo(self, &inports, os);
	
	/* attributes */
	const jive::serialization::opcls_handler * sercls =
		jive::serialization::opcls_registry::instance().lookup(typeid(node->operation()));
	if (!sercls) {
		std::cout << typeid(node->operation()).name() << "\n";
		throw jive::serialization::parse_error(typeid(node->operation()).name());
	}
	jive_token_ostream_identifier(os, sercls->tag().c_str());
	jive_serialize_char_token(self, '<', os);
	jive::serialization::output_driver output_driver(*self, *os);
	sercls->serialize(node->operation(), output_driver);
	jive_serialize_char_token(self, '>', os);
	
	/* outputs */
	jive_portsinfo outports;
	size_t nnormal = node->noutputs;
	for (n = 0; n < node->noutputs; ++n) {
		jive_portinfo port;
		port.origin = node->outputs[n];
		port.required_rescls = node->outputs[n]->required_rescls;
		port.gate = node->outputs[n]->gate;
		if (port.gate && n < nnormal)
			nnormal = n;
		outports.ports.push_back(port);
	}
	outports.nnormal = nnormal;
	jive_serialize_portsinfo(self, &outports, os);
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
	
	/* operator class */
	const jive::serialization::opcls_handler * sercls = nullptr;
	const jive_token * token = jive_token_istream_current(is);
	if (token->type == jive_token_identifier) {
		sercls = jive::serialization::opcls_registry::instance().lookup(token->v.identifier);
	}
	if (!sercls) {
		self->error(self, "Expected node class identifier");
		return false;
	}
	jive_token_istream_advance(is);
	
	/* parse attributes & instantiate node */
	if (!jive_deserialize_char_token(self, is, '<')) {
		return false;
	}

	std::vector<jive::output*> origins(ports.nnormal);
	for (n = 0; n < ports.nnormal; ++n)
		origins[n] = ports.ports[n].origin;

	jive::serialization::parser_driver parser_driver(*self, *is);
	std::unique_ptr<jive::operation> op = sercls->deserialize(parser_driver);
	*node = op->create_node(region, origins.size(), &origins[0]);

	jive_graph_mark_denormalized(region->graph);
	
	if (!jive_deserialize_char_token(self, is, '>'))
		return false;
	
	/* add ports & resource class requirements */
	for (n = ports.nnormal; n < ports.ports.size(); ++n) {
		if (ports.ports[n].gate) {
			jive_node_gate_input(*node, ports.ports[n].gate,
				ports.ports[n].origin);
		} else {
			const jive_resource_class * rescls = ports.ports[n].required_rescls;
			const jive::base::type * type = jive_resource_class_get_type(rescls);
			jive::input * input = jive_node_add_input(*node, type, ports.ports[n].origin);
			input->required_rescls = rescls;
		}
	}
	for (n = 0; n < ports.ports.size(); ++n) {
		(*node)->inputs[n]->required_rescls = ports.ports[n].required_rescls;
	}
	
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
		std::string name = jive_token_istream_current(is)->v.identifier;
		jive_token_istream_advance(is);
		
		if (!jive_deserialize_char_token(self, is, ':'))
			return false;
		
		const jive_resource_class * required_rescls;
		if (!jive_deserialize_rescls(self, is, &required_rescls))
			return false;
		
		jive_serialization_symtab_insert_outputsym(&self->symtab,
			(*node)->outputs[index], name.c_str());
		
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
		std::string name = jive_token_istream_current(is)->v.identifier;
		jive_token_istream_advance(is);
		
		if (!jive_deserialize_char_token(self, is, ':'))
			return false;
		
		const jive_resource_class * required_rescls;
		if (!jive_deserialize_rescls(self, is, &required_rescls))
			return false;
		
		jive::gate * gate = 0;
		if (jive_token_istream_current(is)->type == jive_token_colon) {
			jive_token_istream_advance(is);
			if (!jive_deserialize_defined_gate(self, is, &gate))
				return false;
		}
		
		jive::output * output;
		if (gate)
			output = jive_node_gate_output(*node, gate);
		else
			output = jive_node_add_output(*node, jive_resource_class_get_type(required_rescls));
		
		output->required_rescls = required_rescls;
		
		jive_serialization_symtab_insert_outputsym(&self->symtab, output, name.c_str());
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
	jive::gate * gate, jive_token_ostream * os)
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

/* FIXME: merge with implementation used for copying!
	We are sorting nodes of a region according to their depth. This could be done more sanely
	and offered somewhere else in a more general matter.
*/

typedef struct jive_level_nodes jive_level_nodes;
struct jive_level_nodes {
	std::vector<jive_node*> items;
};

typedef struct jive_sorted_nodes jive_sorted_nodes;
struct jive_sorted_nodes {
	std::vector<jive_level_nodes> depths;
	size_t min_depth;
	size_t max_depth_plus_one;
};

static void
jive_sorted_nodes_init(jive_sorted_nodes * self)
{
	self->min_depth = (size_t)-1;
	self->max_depth_plus_one = 0;
}

static void
jive_level_nodes_append(jive_level_nodes * level, jive_node * node)
{
	level->items.push_back(node);
}

static void
jive_sorted_nodes_append(jive_sorted_nodes * self, jive_node * node)
{
	if (node->depth_from_root >= self->depths.size()) {
		size_t new_space = self->depths.size() * 2;
		if (new_space <= node->depth_from_root)
			new_space = node->depth_from_root + 1;
		self->depths.resize(new_space);
	}
	jive_level_nodes_append(&self->depths[node->depth_from_root], node);
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
	jive_sorted_nodes sorted;
	jive_sorted_nodes_init(&sorted);
	
	jive_node * node;
	JIVE_LIST_ITERATE(region->nodes, node, region_nodes_list)
		jive_sorted_nodes_append(&sorted, node);
	
	size_t n;
	for (n = sorted.min_depth; n < sorted.max_depth_plus_one; ++n) {
		size_t k;
		const jive_level_nodes * level = &sorted.depths[n];
		for (k = 0; k < level->items.size(); ++k) {
			node = level->items[k];

			size_t j;
			for (j = 0; j < node->ninputs; ++j) {
				jive::input * input = node->inputs[j];
				if (dynamic_cast<const jive::achr::type*>(&input->type()))
					jive_serialize_regiondef(self, namegen, input->origin()->node()->region, os);
			}
			if (jive::graph_tail_operation() == node->operation()) {
				continue;
			}
			jive_serialize_nodedef(self, namegen, node, os);
			jive_serialize_char_token(self, ';', os);
		}
	}
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
			std::string name = token->v.identifier;
			jive_token_istream_advance(is);
			if (!jive_deserialize_char_token(self, is, '='))
				return false;
			
			/* parse defined item */
			token = jive_token_istream_current(is);
			switch (token->type) {
				case jive_token_gate: {
					jive_token_istream_advance(is);
					jive::gate * gate;
					if (!jive_deserialize_gateexpr(self, is, region->graph, &gate))
						return false;

					jive_serialization_symtab_insert_gatesym(&self->symtab, gate, name.c_str());
					break;
				}
				case jive_token_node: {
					jive_token_istream_advance(is);
					jive_node * node;
					if (!jive_deserialize_nodeexpr(self, is, region, &node))
						return false;

					jive_serialization_symtab_insert_nodesym(&self->symtab, node, name.c_str());
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

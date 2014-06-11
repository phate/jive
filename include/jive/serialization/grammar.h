/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_SERIALIZATION_GRAMMAR_H
#define JIVE_SERIALIZATION_GRAMMAR_H

#include <jive/context.h>
#include <jive/serialization/driver.h>
#include <jive/serialization/token-stream.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/resource.h>

namespace jive {
	class output;
}

struct jive_graph;
struct jive_immediate;
struct jive_node;
struct jive_region;

typedef struct jive_portinfo jive_portinfo;
struct jive_portinfo {
	jive::output * origin;
	const jive_resource_class * required_rescls;
	jive_gate * gate;
};

typedef struct jive_portsinfo jive_portsinfo;
struct jive_portsinfo {
	size_t nnormal;
	size_t ntotal;
	jive_portinfo * ports;
	jive_context * context;
};

typedef struct jive_serialization_namegen jive_serialization_namegen;
struct jive_serialization_namegen {
	void (*name_gate)(
		jive_serialization_namegen * self,
		jive_serialization_symtab * symtab,
		struct jive_gate * gate);
	void (*name_label)(
		jive_serialization_namegen * self,
		jive_serialization_symtab * symtab,
		struct jive_label * label);
	void (*name_node)(
		jive_serialization_namegen * self,
		jive_serialization_symtab * symtab,
		struct jive_node * node);
	void (*name_output)(
		jive_serialization_namegen * self,
		jive_serialization_symtab * symtab,
		jive::output * output);
};

JIVE_EXPORTED_INLINE void
jive_portsinfo_init(jive_portsinfo * self, jive_context * ctx)
{
	self->nnormal = self->ntotal = 0;
	self->ports = 0;
	self->context = ctx;
}
JIVE_EXPORTED_INLINE void
jive_portsinfo_fini(jive_portsinfo * self)
{
	jive_context_free(self->context, self->ports);
}
JIVE_EXPORTED_INLINE jive_portinfo *
jive_portsinfo_append(jive_portsinfo * self)
{
	self->ports = jive_context_realloc(self->context, self->ports,
		sizeof(self->ports[0]) * (self->ntotal + 1));
	self->ntotal ++;
	return &self->ports[self->ntotal - 1];
}

void
jive_serialize_char_token(jive_serialization_driver * self,
	char c, jive_token_ostream * os);

bool
jive_deserialize_char_token(jive_serialization_driver * self,
	jive_token_istream * is, char c);

void
jive_serialize_string(jive_serialization_driver * self,
	const char * str, size_t len, jive_token_ostream * os);

bool
jive_deserialize_string(jive_serialization_driver * self,
	jive_token_istream * is, char ** str, size_t * len);

void
jive_serialize_uint(jive_serialization_driver * self,
	uint64_t value, jive_token_ostream * os);

bool
jive_deserialize_uint(jive_serialization_driver * self,
	jive_token_istream * is, uint64_t * value);

/* int := ['-'] uint */

void
jive_serialize_int(jive_serialization_driver * self,
	int64_t value, jive_token_ostream * os);

bool
jive_deserialize_int(jive_serialization_driver * self,
	jive_token_istream * is, int64_t * value);

/* type := typeid '<' [class-specific-attrs] '>' */

void
jive_serialize_type(jive_serialization_driver * self,
	const jive::base::type * type, jive_token_ostream * os);

bool
jive_deserialize_type(jive_serialization_driver * self,
	jive_token_istream * is, jive::base::type ** type);

/* rescls := resclsid '<' [class-specific-attrs] '>' */

void
jive_serialize_rescls(jive_serialization_driver * self,
	const jive_resource_class * rescls, jive_token_ostream * os);

bool
jive_deserialize_rescls(jive_serialization_driver * self,
	jive_token_istream * is, const jive_resource_class ** rescls);

/* gateexpr := string rescls type */

void
jive_serialize_gateexpr(jive_serialization_driver * self,
	jive_gate * gate, jive_token_ostream * os);

bool
jive_deserialize_gateexpr(jive_serialization_driver * self,
	jive_token_istream * is, struct jive_graph * graph, jive_gate ** gate);

/* defined_gate := gate_ident */

void
jive_serialize_defined_gate(jive_serialization_driver * self,
	jive_gate * gate, jive_token_ostream * os);

bool
jive_deserialize_defined_gate(jive_serialization_driver * self,
	jive_token_istream * is, jive_gate ** gate);

/* defined_label := label_ident */

void
jive_serialize_defined_label(jive_serialization_driver * self,
	struct jive_label * label, jive_token_ostream * os);

bool
jive_deserialize_defined_label(jive_serialization_driver * self,
	jive_token_istream * is, struct jive_label ** label);

/* defined_node := node_ident */

void
jive_serialize_defined_node(jive_serialization_driver * self,
	struct jive_node * node, jive_token_ostream * os);

bool
jive_deserialize_defined_node(jive_serialization_driver * self,
	jive_token_istream * is, struct jive_node ** node);

/* portinfo := output_ident ':' rescls [ ':' gate_ident ] */

void
jive_serialize_portinfo(jive_serialization_driver * self,
	jive_portinfo * port, jive_token_ostream * os);

bool
jive_deserialize_portinfo(jive_serialization_driver * self,
	jive_token_istream * is, jive_portinfo * port);

/* portsinfo := '(' [portinfo [portinfo...]] ';' [portinfo [portinfo...]]  ')' */

void
jive_serialize_portsinfo(jive_serialization_driver * self,
	jive_portsinfo * ports, jive_token_ostream * os);

bool
jive_deserialize_portsinfo(jive_serialization_driver * self,
	jive_token_istream * is, jive_portsinfo * ports);

/* nodeexpr := portsinfo nodeclsid '<' [class-specific-attrs] '>' portsinfo */

void
jive_serialize_nodeexpr(jive_serialization_driver * self,
	struct jive_node * node, jive_token_ostream * os);

bool
jive_deserialize_nodeexpr(jive_serialization_driver * self,
	jive_token_istream * is, struct jive_region * region, struct jive_node ** node);

/* label := ('.' | 'frameptr' | 'stackptr' | defined_label) */

void
jive_serialize_label(jive_serialization_driver * self,
	const struct jive_label * label, jive_token_ostream * os);

bool
jive_deserialize_label(jive_serialization_driver * self,
	jive_token_istream * is, const struct jive_label ** label);

/* immediate := uint [ '+' label ] [ '-' label ] */

void
jive_serialize_immediate(jive_serialization_driver * self,
	const struct jive_immediate * imm,
	jive_token_ostream * os);

bool
jive_deserialize_immediate(jive_serialization_driver * self,
	jive_token_istream * is, struct jive_immediate * imm);

/* def := nodedef | gatedef | regiondef */
/* nodedef := new_node_id '=' 'node' node_expr ';' */
/* gatedef := new_gate_id '=' 'gate' gate_expr ';' */
/* regiondef := 'region' '{' regionbody '}' */
/* regionbody := [def [def...]] */

void
jive_serialize_nodedef(jive_serialization_driver * self,
	jive_serialization_namegen * namegen,
	struct jive_node * node, jive_token_ostream * os);

void
jive_serialize_gatedef(jive_serialization_driver * self,
	jive_serialization_namegen * namegen,
	struct jive_gate * gate, jive_token_ostream * os);

void
jive_serialize_regiondef(jive_serialization_driver * self,
	jive_serialization_namegen * namegen,
	struct jive_region * region, jive_token_ostream * os);

bool
jive_deserialize_def(jive_serialization_driver * self,
	jive_token_istream * is, struct jive_region * region);

void
jive_serialize_regionbody(jive_serialization_driver * self,
	jive_serialization_namegen * namegen,
	struct jive_region * region, jive_token_ostream * os);

bool
jive_deserialize_regionbody(jive_serialization_driver * self,
	jive_token_istream * is, struct jive_region * region);

#endif

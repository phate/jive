/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_SERIALIZATION_GRAMMAR_H
#define JIVE_SERIALIZATION_GRAMMAR_H

#include <stdexcept>
#include <vector>

#include <jive/serialization/driver.h>
#include <jive/serialization/token-stream.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/resource.h>

namespace jive {
class output;
class region;

namespace serialization {

class parse_error : std::runtime_error {
public:
	virtual
	~parse_error() noexcept;

	inline
	parse_error(std::string what) : std::runtime_error(std::move(what)) {}
};

class parser_driver {
public:
	parser_driver(
		jive_serialization_driver & driver,
		jive_token_istream & is)
		: driver_(driver)
		, is_(is)
	{
	}

	/* note: all parse functions below may throw parse_error */

	void
	parse_char_token(char expected);

	uint64_t
	parse_uint();

	int64_t
	parse_int();

	std::string
	parse_string();

	std::string
	parse_identifier();

	const jive_label *
	parse_defined_label();

	const jive_label *
	parse_label();

	const jive_resource_class *
	parse_resource_class();

	const jive_resource_class *
	parse_resource_class_or_null();

	jive_token_type
	peek_token_type() const noexcept;

	inline jive_token_istream &
	istream() noexcept
	{
		return is_;
	}

	jive_serialization_driver &
	driver() noexcept
	{
		return driver_;
	}

private:
	jive_serialization_driver & driver_;
	jive_token_istream & is_;
};

class output_driver {
public:
	output_driver(
		jive_serialization_driver & driver,
		jive_token_ostream & os)
		: driver_(driver)
		, os_(os)
	{
	}

	void
	put_char_token(char token);

	void
	put_uint(uint64_t value);

	void
	put_int(int64_t value);

	void
	put_string(const std::string& s);

	void
	put_identifier(const std::string& identifier);

	void
	put_defined_label(const jive_label * label);

	void
	put_label(const jive_label * label);

	void
	put_resource_class(const jive_resource_class * rescls);

	void
	put_resource_class_or_null(const jive_resource_class * rescls);

	inline jive_token_ostream &
	ostream() noexcept
	{
		return os_;
	}

	jive_serialization_driver &
	driver() noexcept
	{
		return driver_;
	}

private:
	jive_serialization_driver & driver_;
	jive_token_ostream & os_;
};

}

}

struct jive_graph;
struct jive_immediate;
struct jive_node;

typedef struct jive_portinfo jive_portinfo;
struct jive_portinfo {
	jive::output * origin;
	const jive_resource_class * required_rescls;
	jive::gate * gate;
};

struct jive_portsinfo {
	inline jive_portsinfo() : nnormal(0) {}

	std::vector<jive_portinfo> ports;
	size_t nnormal;
};

typedef struct jive_serialization_namegen jive_serialization_namegen;
struct jive_serialization_namegen {
	void (*name_gate)(
		jive_serialization_namegen * self,
		jive_serialization_symtab * symtab,
		jive::gate * gate);
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
	jive_token_istream * is, std::string & str);

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
	jive::gate * gate, jive_token_ostream * os);

bool
jive_deserialize_gateexpr(jive_serialization_driver * self,
	jive_token_istream * is, struct jive_graph * graph, jive::gate ** gate);

/* defined_gate := gate_ident */

void
jive_serialize_defined_gate(jive_serialization_driver * self,
	jive::gate * gate, jive_token_ostream * os);

bool
jive_deserialize_defined_gate(jive_serialization_driver * self,
	jive_token_istream * is, jive::gate ** gate);

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
	jive_token_istream * is, struct jive::region * region, struct jive_node ** node);

/* label := ('.' | 'frameptr' | 'stackptr' | defined_label) */

void
jive_serialize_label(jive_serialization_driver * self,
	const struct jive_label * label, jive_token_ostream * os);

bool
jive_deserialize_label(jive_serialization_driver * self,
	jive_token_istream * is, const struct jive_label ** label);

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
	jive::gate * gate, jive_token_ostream * os);

void
jive_serialize_regiondef(jive_serialization_driver * self,
	jive_serialization_namegen * namegen,
	struct jive::region * region, jive_token_ostream * os);

bool
jive_deserialize_def(jive_serialization_driver * self,
	jive_token_istream * is, struct jive::region * region);

void
jive_serialize_regionbody(jive_serialization_driver * self,
	jive_serialization_namegen * namegen,
	struct jive::region * region, jive_token_ostream * os);

bool
jive_deserialize_regionbody(jive_serialization_driver * self,
	jive_token_istream * is, struct jive::region * region);

#endif

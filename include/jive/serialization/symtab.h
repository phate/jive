/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_SERIALIZATION_SYMTAB_H
#define JIVE_SERIALIZATION_SYMTAB_H

#include <stddef.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/util/intrusive-hash.h>

#include <string>

namespace jive {
	class gate;
	class output;
}

struct jive_context;
struct jive_label;
struct jive_node;

typedef struct jive_serialization_gatesym jive_serialization_gatesym;
typedef struct jive_serialization_labelsym jive_serialization_labelsym;
typedef struct jive_serialization_nodesym jive_serialization_nodesym;
typedef struct jive_serialization_outputsym jive_serialization_outputsym;
typedef struct jive_serialization_symtab jive_serialization_symtab;

struct jive_serialization_gatesym {
	jive::gate * gate;
	std::string name;
private:
	jive::detail::intrusive_hash_anchor<jive_serialization_gatesym> name_hash_chain;
	jive::detail::intrusive_hash_anchor<jive_serialization_gatesym> gate_hash_chain;
public:
	typedef jive::detail::intrusive_hash_accessor<
		std::string,
		jive_serialization_gatesym,
		&jive_serialization_gatesym::name,
		&jive_serialization_gatesym::name_hash_chain
	> name_hash_chain_accessor;
	typedef jive::detail::intrusive_hash_accessor<
		jive::gate *,
		jive_serialization_gatesym,
		&jive_serialization_gatesym::gate,
		&jive_serialization_gatesym::gate_hash_chain
	> gate_hash_chain_accessor;
};
typedef jive::detail::intrusive_hash<
	const std::string,
	jive_serialization_gatesym,
	jive_serialization_gatesym::name_hash_chain_accessor,
	std::hash<std::string>,
	jive::detail::safe_equal<std::string>
> jive_serialization_gatesym_dict;

typedef jive::detail::intrusive_hash<
	const jive::gate *,
	jive_serialization_gatesym,
	jive_serialization_gatesym::gate_hash_chain_accessor
> jive_serialization_gatesym_hash;


struct jive_serialization_labelsym {
	struct jive_label * label;
	std::string name;
private:
	jive::detail::intrusive_hash_anchor<jive_serialization_labelsym> name_hash_chain;
	jive::detail::intrusive_hash_anchor<jive_serialization_labelsym> label_hash_chain;
public:
	typedef jive::detail::intrusive_hash_accessor<
		std::string,
		jive_serialization_labelsym,
		&jive_serialization_labelsym::name,
		&jive_serialization_labelsym::name_hash_chain
	> name_hash_chain_accessor;
	typedef jive::detail::intrusive_hash_accessor<
		struct jive_label *,
		jive_serialization_labelsym,
		&jive_serialization_labelsym::label,
		&jive_serialization_labelsym::label_hash_chain
	> label_hash_chain_accessor;
};
typedef jive::detail::intrusive_hash<
	const std::string,
	jive_serialization_labelsym,
	jive_serialization_labelsym::name_hash_chain_accessor,
	std::hash<std::string>,
	jive::detail::safe_equal<std::string>
> jive_serialization_labelsym_dict;

typedef jive::detail::intrusive_hash<
	const struct jive_label *,
	jive_serialization_labelsym,
	jive_serialization_labelsym::label_hash_chain_accessor
> jive_serialization_labelsym_hash;


struct jive_serialization_nodesym {
	struct jive_node * node;
	std::string name;
private:
	jive::detail::intrusive_hash_anchor<jive_serialization_nodesym> name_hash_chain;
	jive::detail::intrusive_hash_anchor<jive_serialization_nodesym> node_hash_chain;
public:
	typedef jive::detail::intrusive_hash_accessor<
		std::string,
		jive_serialization_nodesym,
		&jive_serialization_nodesym::name,
		&jive_serialization_nodesym::name_hash_chain
	> name_hash_chain_accessor;
	typedef jive::detail::intrusive_hash_accessor<
		struct jive_node *,
		jive_serialization_nodesym,
		&jive_serialization_nodesym::node,
		&jive_serialization_nodesym::node_hash_chain
	> node_hash_chain_accessor;
};
typedef jive::detail::intrusive_hash<
	const std::string,
	jive_serialization_nodesym,
	jive_serialization_nodesym::name_hash_chain_accessor,
	std::hash<std::string>,
	jive::detail::safe_equal<std::string>
> jive_serialization_nodesym_dict;

typedef jive::detail::intrusive_hash<
	const struct jive_node *,
	jive_serialization_nodesym,
	jive_serialization_nodesym::node_hash_chain_accessor
> jive_serialization_nodesym_hash;


struct jive_serialization_outputsym {
	jive::output * output;
	std::string name;
private:
	jive::detail::intrusive_hash_anchor<jive_serialization_outputsym> name_hash_chain;
	jive::detail::intrusive_hash_anchor<jive_serialization_outputsym> output_hash_chain;
public:
	typedef jive::detail::intrusive_hash_accessor<
		std::string,
		jive_serialization_outputsym,
		&jive_serialization_outputsym::name,
		&jive_serialization_outputsym::name_hash_chain
	> name_hash_chain_accessor;
	typedef jive::detail::intrusive_hash_accessor<
		jive::output *,
		jive_serialization_outputsym,
		&jive_serialization_outputsym::output,
		&jive_serialization_outputsym::output_hash_chain
	> output_hash_chain_accessor;
};
typedef jive::detail::intrusive_hash<
	const std::string,
	jive_serialization_outputsym,
	jive_serialization_outputsym::name_hash_chain_accessor,
	std::hash<std::string>,
	jive::detail::safe_equal<std::string>
> jive_serialization_outputsym_dict;

typedef jive::detail::intrusive_hash<
	const jive::output *,
	jive_serialization_outputsym,
	jive_serialization_outputsym::output_hash_chain_accessor
> jive_serialization_outputsym_hash;


struct jive_serialization_symtab {
	jive_context * context;
	jive_serialization_gatesym_hash gate_to_name;
	jive_serialization_gatesym_dict name_to_gate;
	jive_serialization_labelsym_hash label_to_name;
	jive_serialization_labelsym_dict name_to_label;
	jive_serialization_nodesym_hash node_to_name;
	jive_serialization_nodesym_dict name_to_node;
	jive_serialization_outputsym_hash output_to_name;
	jive_serialization_outputsym_dict name_to_output;
};

void
jive_serialization_symtab_init(jive_serialization_symtab * self, jive_context * ctx);

void
jive_serialization_symtab_fini(jive_serialization_symtab * self);

void
jive_serialization_symtab_insert_gatesym(
	jive_serialization_symtab * self,
	jive::gate * gate,
	const std::string & name);

void
jive_serialization_symtab_remove_gatesym(
	jive_serialization_symtab * self,
	jive_serialization_gatesym * sym);

const jive_serialization_gatesym *
jive_serialization_symtab_gate_to_name(
	jive_serialization_symtab * self,
	const jive::gate * gate);

const jive_serialization_gatesym *
jive_serialization_symtab_name_to_gate(
	jive_serialization_symtab * self,
	const char * name);

void
jive_serialization_symtab_insert_labelsym(
	jive_serialization_symtab * self,
	struct jive_label * label,
	const std::string & name);

void
jive_serialization_symtab_remove_labelsym(
	jive_serialization_symtab * self,
	jive_serialization_labelsym * sym);

const jive_serialization_labelsym *
jive_serialization_symtab_label_to_name(
	jive_serialization_symtab * self,
	const struct jive_label * label);

const jive_serialization_labelsym *
jive_serialization_symtab_name_to_label(
	jive_serialization_symtab * self,
	const char * name);

void
jive_serialization_symtab_insert_nodesym(
	jive_serialization_symtab * self,
	struct jive_node * node,
	const std::string & name);

void
jive_serialization_symtab_remove_nodesym(
	jive_serialization_symtab * self,
	jive_serialization_nodesym * sym);

const jive_serialization_nodesym *
jive_serialization_symtab_node_to_name(
	jive_serialization_symtab * self,
	const struct jive_node * node);

const jive_serialization_nodesym *
jive_serialization_symtab_name_to_node(
	jive_serialization_symtab * self,
	const char * name);

void
jive_serialization_symtab_insert_outputsym(
	jive_serialization_symtab * self,
	jive::output * output,
	const std::string & name);

void
jive_serialization_symtab_remove_outputsym(
	jive_serialization_symtab * self,
	jive_serialization_outputsym * sym);

const jive_serialization_outputsym *
jive_serialization_symtab_output_to_name(
	jive_serialization_symtab * self,
	const jive::output * output);

const jive_serialization_outputsym *
jive_serialization_symtab_name_to_output(
	jive_serialization_symtab * self,
	const char * name);

#endif

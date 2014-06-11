/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_SERIALIZATION_SYMTAB_H
#define JIVE_SERIALIZATION_SYMTAB_H

#include <stddef.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/util/dict.h>
#include <jive/util/hash.h>

namespace jive {
	class output;
}

struct jive_context;
struct jive_gate;
struct jive_label;
struct jive_node;

typedef struct jive_serialization_gatesym jive_serialization_gatesym;
typedef struct jive_serialization_gatesym_hash jive_serialization_gatesym_hash;
typedef struct jive_serialization_gatesym_dict jive_serialization_gatesym_dict;
typedef struct jive_serialization_labelsym jive_serialization_labelsym;
typedef struct jive_serialization_labelsym_hash jive_serialization_labelsym_hash;
typedef struct jive_serialization_labelsym_dict jive_serialization_labelsym_dict;
typedef struct jive_serialization_nodesym jive_serialization_nodesym;
typedef struct jive_serialization_nodesym_hash jive_serialization_nodesym_hash;
typedef struct jive_serialization_nodesym_dict jive_serialization_nodesym_dict;
typedef struct jive_serialization_outputsym jive_serialization_outputsym;
typedef struct jive_serialization_outputsym_hash jive_serialization_outputsym_hash;
typedef struct jive_serialization_outputsym_dict jive_serialization_outputsym_dict;
typedef struct jive_serialization_symtab jive_serialization_symtab;

struct jive_serialization_gatesym {
	struct jive_gate * gate;
	char * name;
	struct {
		jive_serialization_gatesym * prev;
		jive_serialization_gatesym * next;
	} gate_hash_chain;
	struct {
		jive_serialization_gatesym * prev;
		jive_serialization_gatesym * next;
	} name_hash_chain;
};
JIVE_DECLARE_HASH_TYPE(jive_serialization_gatesym_hash, jive_serialization_gatesym, struct jive_gate *, gate, gate_hash_chain);
JIVE_DECLARE_DICT_TYPE(jive_serialization_gatesym_dict, jive_serialization_gatesym, name, name_hash_chain);

struct jive_serialization_labelsym {
	struct jive_label * label;
	char * name;
	struct {
		jive_serialization_labelsym * prev;
		jive_serialization_labelsym * next;
	} label_hash_chain;
	struct {
		jive_serialization_labelsym * prev;
		jive_serialization_labelsym * next;
	} name_hash_chain;
};
JIVE_DECLARE_HASH_TYPE(jive_serialization_labelsym_hash, jive_serialization_labelsym, struct jive_label *, label, label_hash_chain);
JIVE_DECLARE_DICT_TYPE(jive_serialization_labelsym_dict, jive_serialization_labelsym, name, name_hash_chain);

struct jive_serialization_nodesym {
	struct jive_node * node;
	char * name;
	struct {
		jive_serialization_nodesym * prev;
		jive_serialization_nodesym * next;
	} node_hash_chain;
	struct {
		jive_serialization_nodesym * prev;
		jive_serialization_nodesym * next;
	} name_hash_chain;
};
JIVE_DECLARE_HASH_TYPE(jive_serialization_nodesym_hash, jive_serialization_nodesym, struct jive_node *, node, node_hash_chain);
JIVE_DECLARE_DICT_TYPE(jive_serialization_nodesym_dict, jive_serialization_nodesym, name, name_hash_chain);

struct jive_serialization_outputsym {
	jive::output * output;
	char * name;
	struct {
		jive_serialization_outputsym * prev;
		jive_serialization_outputsym * next;
	} output_hash_chain;
	struct {
		jive_serialization_outputsym * prev;
		jive_serialization_outputsym * next;
	} name_hash_chain;
};
JIVE_DECLARE_HASH_TYPE(jive_serialization_outputsym_hash, jive_serialization_outputsym,
	jive::output *, output, output_hash_chain);
JIVE_DECLARE_DICT_TYPE(jive_serialization_outputsym_dict, jive_serialization_outputsym, name, name_hash_chain);

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

JIVE_EXPORTED_INLINE char *
jive_serialization_symtab_strdup(
	jive_serialization_symtab * self,
	const char * str)
{
	return jive_context_strdup(self->context, str);
}

JIVE_EXPORTED_INLINE void
jive_serialization_symtab_strfree(
	jive_serialization_symtab * self,
	char * str)
{
	return jive_context_free(self->context, str);
}

void
jive_serialization_symtab_insert_gatesym(
	jive_serialization_symtab * self,
	struct jive_gate * gate,
	char * name);

void
jive_serialization_symtab_remove_gatesym(
	jive_serialization_symtab * self,
	jive_serialization_gatesym * sym);

const jive_serialization_gatesym *
jive_serialization_symtab_gate_to_name(
	jive_serialization_symtab * self,
	const struct jive_gate * gate);

const jive_serialization_gatesym *
jive_serialization_symtab_name_to_gate(
	jive_serialization_symtab * self,
	const char * name);

void
jive_serialization_symtab_insert_labelsym(
	jive_serialization_symtab * self,
	struct jive_label * label,
	char * name);

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
	char * name);

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
	char * name);

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

/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/serialization/symtab.h>

void
jive_serialization_symtab_init(jive_serialization_symtab * self, jive_context * ctx)
{
	self->context = ctx;
}

void
jive_serialization_symtab_fini(jive_serialization_symtab * self)
{
	for (auto i = self->gate_to_name.begin(); i != self->gate_to_name.end();) {
		auto j = i; i++;
		jive_serialization_symtab_remove_gatesym(self, j.ptr());
	}

	for (auto i = self->label_to_name.begin(); i != self->label_to_name.end();) {
		auto j = i; i++;
		jive_serialization_symtab_remove_labelsym(self, j.ptr());
	}

	for (auto i = self->node_to_name.begin(); i != self->node_to_name.end();) {
		auto j = i; i++;
		jive_serialization_symtab_remove_nodesym(self, j.ptr());
	}

	for (auto i = self->output_to_name.begin(); i != self->output_to_name.end();) {
		auto j = i; i++;
		jive_serialization_symtab_remove_outputsym(self, j.ptr());
	}
}

void
jive_serialization_symtab_insert_gatesym(
	jive_serialization_symtab * self,
	jive::gate * gate,
	const std::string & name)
{
	jive_serialization_gatesym * sym = new jive_serialization_gatesym;
	sym->gate = gate;
	sym->name = name;
	self->gate_to_name.insert(sym);
	self->name_to_gate.insert(sym);
}

void
jive_serialization_symtab_remove_gatesym(
	jive_serialization_symtab * self,
	jive_serialization_gatesym * sym)
{
	self->gate_to_name.erase(sym);
	self->name_to_gate.erase(sym);
	delete sym;
}

const jive_serialization_gatesym *
jive_serialization_symtab_gate_to_name(
	jive_serialization_symtab * self,
	const jive::gate * gate)
{
	auto i = self->gate_to_name.find(gate);
	if (i != self->gate_to_name.end())
		return i.ptr();
	else
		return nullptr;
}

const jive_serialization_gatesym *
jive_serialization_symtab_name_to_gate(
	jive_serialization_symtab * self,
	const char * name)
{
	auto i = self->name_to_gate.find(name);
	if (i != self->name_to_gate.end())
		return i.ptr();
	else
		return nullptr;
}

void
jive_serialization_symtab_insert_labelsym(
	jive_serialization_symtab * self,
	struct jive_label * label,
	const std::string & name)
{
	jive_serialization_labelsym * sym = new jive_serialization_labelsym;
	sym->label = label;
	sym->name = name;
	self->label_to_name.insert(sym);
	self->name_to_label.insert(sym);
}

void
jive_serialization_symtab_remove_labelsym(
	jive_serialization_symtab * self,
	jive_serialization_labelsym * sym)
{
	self->label_to_name.erase(sym);
	self->name_to_label.erase(sym);
	delete sym;
}

const jive_serialization_labelsym *
jive_serialization_symtab_label_to_name(
	jive_serialization_symtab * self,
	const struct jive_label * label)
{
	auto i = self->label_to_name.find(label);
	if (i != self->label_to_name.end())
		return i.ptr();
	else
		return nullptr;
}

const jive_serialization_labelsym *
jive_serialization_symtab_name_to_label(
	jive_serialization_symtab * self,
	const char * name)
{
	auto i = self->name_to_label.find(name);
	if (i != self->name_to_label.end())
		return i.ptr();
	else
		return nullptr;
}

void
jive_serialization_symtab_insert_nodesym(
	jive_serialization_symtab * self,
	struct jive_node * node,
	const std::string & name)
{
	jive_serialization_nodesym * sym = new jive_serialization_nodesym;
	sym->node = node;
	sym->name = name;
	self->node_to_name.insert(sym);
	self->name_to_node.insert(sym);
}

void
jive_serialization_symtab_remove_nodesym(
	jive_serialization_symtab * self,
	jive_serialization_nodesym * sym)
{
	self->node_to_name.erase(sym);
	self->name_to_node.erase(sym);
	delete sym;
}

const jive_serialization_nodesym *
jive_serialization_symtab_node_to_name(
	jive_serialization_symtab * self,
	const struct jive_node * node)
{
	auto i = self->node_to_name.find(node);
	if (i != self->node_to_name.end())
		return i.ptr();
	else
		return nullptr;
}

const jive_serialization_nodesym *
jive_serialization_symtab_name_to_node(
	jive_serialization_symtab * self,
	const char * name)
{
	auto i = self->name_to_node.find(name);
	if (i != self->name_to_node.end())
		return i.ptr();
	else
		return nullptr;
}

void
jive_serialization_symtab_insert_outputsym(
	jive_serialization_symtab * self,
	jive::output * output,
	const std::string & name)
{
	jive_serialization_outputsym * sym = new jive_serialization_outputsym;
	sym->output = output;
	sym->name = name;
	self->output_to_name.insert(sym);
	self->name_to_output.insert(sym);
}

void
jive_serialization_symtab_remove_outputsym(
	jive_serialization_symtab * self,
	jive_serialization_outputsym * sym)
{
	self->output_to_name.erase(sym);
	self->name_to_output.erase(sym);
	delete sym;
}

const jive_serialization_outputsym *
jive_serialization_symtab_output_to_name(
	jive_serialization_symtab * self,
	const jive::output * output)
{
	auto i = self->output_to_name.find(output);
	if (i != self->output_to_name.end())
		return i.ptr();
	else
		return nullptr;
}

const jive_serialization_outputsym *
jive_serialization_symtab_name_to_output(
	jive_serialization_symtab * self,
	const char * name)
{
	auto i = self->name_to_output.find(name);
	if (i != self->name_to_output.end())
		return i.ptr();
	else
		return nullptr;
}

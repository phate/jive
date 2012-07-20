#include <jive/serialization/symtab.h>

JIVE_DEFINE_HASH_TYPE(jive_serialization_gatesym_hash, jive_serialization_gatesym, struct jive_gate *, gate, gate_hash_chain);
JIVE_DEFINE_DICT_TYPE(jive_serialization_gatesym_dict, jive_serialization_gatesym, name, name_hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_serialization_nodesym_hash, jive_serialization_nodesym, struct jive_node *, node, node_hash_chain);
JIVE_DEFINE_DICT_TYPE(jive_serialization_nodesym_dict, jive_serialization_nodesym, name, name_hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_serialization_outputsym_hash, jive_serialization_outputsym, struct jive_output *, output, output_hash_chain);
JIVE_DEFINE_DICT_TYPE(jive_serialization_outputsym_dict, jive_serialization_outputsym, name, name_hash_chain);

void
jive_serialization_symtab_init(jive_serialization_symtab * self, jive_context * ctx)
{
	self->context = ctx;
	jive_serialization_gatesym_hash_init(&self->gate_to_name, ctx);
	jive_serialization_gatesym_dict_init(&self->name_to_gate, ctx);
	jive_serialization_nodesym_hash_init(&self->node_to_name, ctx);
	jive_serialization_nodesym_dict_init(&self->name_to_node, ctx);
	jive_serialization_outputsym_hash_init(&self->output_to_name, ctx);
	jive_serialization_outputsym_dict_init(&self->name_to_output, ctx);
}

void
jive_serialization_symtab_fini(jive_serialization_symtab * self)
{
	struct jive_serialization_gatesym_hash_iterator gate_iter;
	gate_iter = jive_serialization_gatesym_hash_begin(&self->gate_to_name);
	while (gate_iter.entry) {
		jive_serialization_gatesym * sym = gate_iter.entry;
		jive_serialization_gatesym_hash_iterator_next(&gate_iter);
		jive_serialization_symtab_remove_gatesym(self, sym);
	}
	
	struct jive_serialization_nodesym_hash_iterator node_iter;
	node_iter = jive_serialization_nodesym_hash_begin(&self->node_to_name);
	while (node_iter.entry) {
		jive_serialization_nodesym * sym = node_iter.entry;
		jive_serialization_nodesym_hash_iterator_next(&node_iter);
		jive_serialization_symtab_remove_nodesym(self, sym);
	}
	
	struct jive_serialization_outputsym_hash_iterator output_iter;
	output_iter = jive_serialization_outputsym_hash_begin(&self->output_to_name);
	while (output_iter.entry) {
		jive_serialization_outputsym * sym = output_iter.entry;
		jive_serialization_outputsym_hash_iterator_next(&output_iter);
		jive_serialization_symtab_remove_outputsym(self, sym);
	}
	
	jive_serialization_gatesym_hash_fini(&self->gate_to_name);
	jive_serialization_gatesym_dict_fini(&self->name_to_gate);
	jive_serialization_nodesym_hash_fini(&self->node_to_name);
	jive_serialization_nodesym_dict_fini(&self->name_to_node);
	jive_serialization_outputsym_hash_fini(&self->output_to_name);
	jive_serialization_outputsym_dict_fini(&self->name_to_output);
}

void
jive_serialization_symtab_insert_gatesym(
	jive_serialization_symtab * self,
	struct jive_gate * gate,
	char * name)
{
	jive_serialization_gatesym * sym;
	sym = jive_context_malloc(self->name_to_gate.context, sizeof(*sym));
	sym->gate = gate;
	sym->name = name;
	jive_serialization_gatesym_hash_insert(&self->gate_to_name, sym);
	jive_serialization_gatesym_dict_insert(&self->name_to_gate, sym);
}

void
jive_serialization_symtab_remove_gatesym(
	jive_serialization_symtab * self,
	jive_serialization_gatesym * sym)
{
	jive_serialization_gatesym_hash_remove(&self->gate_to_name, sym);
	jive_serialization_gatesym_dict_remove(&self->name_to_gate, sym);
	jive_context_free(self->name_to_gate.context, sym->name);
	jive_context_free(self->name_to_gate.context, sym);
}

const jive_serialization_gatesym *
jive_serialization_symtab_gate_to_name(
	jive_serialization_symtab * self,
	const struct jive_gate * gate)
{
	return jive_serialization_gatesym_hash_lookup(&self->gate_to_name, gate);
}

const jive_serialization_gatesym *
jive_serialization_symtab_name_to_gate(
	jive_serialization_symtab * self,
	const char * name)
{
	return jive_serialization_gatesym_dict_lookup(&self->name_to_gate, name);
}

void
jive_serialization_symtab_insert_nodesym(
	jive_serialization_symtab * self,
	struct jive_node * node,
	char * name)
{
	jive_serialization_nodesym * sym;
	sym = jive_context_malloc(self->name_to_node.context, sizeof(*sym));
	sym->node = node;
	sym->name = name;
	jive_serialization_nodesym_hash_insert(&self->node_to_name, sym);
	jive_serialization_nodesym_dict_insert(&self->name_to_node, sym);
}

void
jive_serialization_symtab_remove_nodesym(
	jive_serialization_symtab * self,
	jive_serialization_nodesym * sym)
{
	jive_serialization_nodesym_hash_remove(&self->node_to_name, sym);
	jive_serialization_nodesym_dict_remove(&self->name_to_node, sym);
	jive_context_free(self->name_to_node.context, sym->name);
	jive_context_free(self->name_to_node.context, sym);
}

const jive_serialization_nodesym *
jive_serialization_symtab_node_to_name(
	jive_serialization_symtab * self,
	const struct jive_node * node)
{
	return jive_serialization_nodesym_hash_lookup(&self->node_to_name, node);
}

const jive_serialization_nodesym *
jive_serialization_symtab_name_to_node(
	jive_serialization_symtab * self,
	const char * name)
{
	return jive_serialization_nodesym_dict_lookup(&self->name_to_node, name);
}

void
jive_serialization_symtab_insert_outputsym(
	jive_serialization_symtab * self,
	struct jive_output * output,
	char * name)
{
	jive_serialization_outputsym * sym;
	sym = jive_context_malloc(self->name_to_output.context, sizeof(*sym));
	sym->output = output;
	sym->name = name;
	jive_serialization_outputsym_hash_insert(&self->output_to_name, sym);
	jive_serialization_outputsym_dict_insert(&self->name_to_output, sym);
}

void
jive_serialization_symtab_remove_outputsym(
	jive_serialization_symtab * self,
	jive_serialization_outputsym * sym)
{
	jive_serialization_outputsym_hash_remove(&self->output_to_name, sym);
	jive_serialization_outputsym_dict_remove(&self->name_to_output, sym);
	jive_context_free(self->name_to_output.context, sym->name);
	jive_context_free(self->name_to_output.context, sym);
}

const jive_serialization_outputsym *
jive_serialization_symtab_output_to_name(
	jive_serialization_symtab * self,
	const struct jive_output * output)
{
	return jive_serialization_outputsym_hash_lookup(&self->output_to_name, output);
}

const jive_serialization_outputsym *
jive_serialization_symtab_name_to_output(
	jive_serialization_symtab * self,
	const char * name)
{
	return jive_serialization_outputsym_dict_lookup(&self->name_to_output, name);
}

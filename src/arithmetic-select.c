#include <jive/arithmetic-select.h>

#include <jive/internal/metacontainers.h>
#include <jive/machine.h>
#include <jive/graph.h>
#include <jive/nodeclass.h>
#include <jive/bitstring.h>
#include <jive/internal/bitstringstr.h>
#include <jive/fixed.h>

#include "debug.h"

static inline void *
jive_nodeset_alloc(void * old, size_t old_size, size_t new_size, jive_node * node)
{
	return jive_malloc(node->graph, new_size);
}

typedef struct jive_nodeset jive_nodeset;
DEFINE_SET_TYPE(jive_nodeset, jive_node *, jive_nodeset_alloc)

typedef struct regop_group regop_group;
struct regop_group {
	jive_cpureg_classmask_t mask;
	struct jive_nodeset nodes;
};

typedef struct regop_hashmap regop_hashmap;
static inline long
regop_hashmap_hash(jive_node * node) {return (long)node;}
static inline void *
regop_hashmap_realloc(size_t size, jive_node * key, regop_group * value)
{
	return jive_malloc(key->graph, size);
}

DEFINE_HASHMAP_TYPE(regop_hashmap, jive_node *, regop_group *, regop_hashmap_hash, regop_hashmap_realloc);

static regop_group *
regop_group_create(regop_hashmap * map, jive_node * node)
{
	regop_group * group = jive_malloc(node->graph, sizeof(*group));
	
	jive_nodeset_init(&group->nodes);
	jive_nodeset_add(&group->nodes, node);
	group->mask = (jive_cpureg_classmask_t) -1;
	
	regop_hashmap_set(map, node, group);
	
	return group;
}

static regop_group *
regop_group_lookup(regop_hashmap * map, jive_node * node)
{
	const struct regop_hashmap_entry * entry;
	entry = regop_hashmap_lookup(map, node);
	if (entry) return entry->value;
	return 0;
}

static regop_group *
regop_group_for_value(regop_hashmap * map, jive_value * value)
{
	jive_node * node = value->node;
	if (node->type == &JIVE_BITSLICE) {
		node = jive_node_iterate_operands(node)->value->node;
	}
	return regop_group_lookup(map, node);
}

static regop_group *
regop_group_union(regop_hashmap * map, regop_group * first, regop_group * second)
{
	size_t n;
	for(n=0; n<second->nodes.nitems; n++) {
		jive_node * node = second->nodes.items[n];
		jive_nodeset_add(&first->nodes, node);
		DEBUG_ASSERT(regop_hashmap_lookup(map, node)->value == second);
		regop_hashmap_set(map, node, first);
	}
	
	return first;
}

static bool
jive_node_is_regop(const jive_node * node)
{
	const jive_node_class * type = node->type;
	
	return
		(type == &JIVE_BITCONCAT) ||
		(type == &JIVE_INTNEG) ||
		(type == &JIVE_INTSUM) ||
		(type == &JIVE_BITAND) ||
		(type == &JIVE_BITOR) ||
		(type == &JIVE_BITXOR) ||
		(type == &JIVE_INTPRODUCT);
}

void
jive_arithmetic_select(jive_graph * graph,
	const jive_machine * machine)
{
	/* traverse graph:
	- find "undecided" arithmetic operations
	- annotate each with the set of "permissible" ops
	- find "connected sets", try to negotiate common
	  register class
	- split incommensurable parts through "transfer" nodes
	  (somehow...) */
	
	regop_hashmap map;
	regop_hashmap_init(&map);
	
	jive_graph_traverser * trav;
	
	trav = jive_graph_traverse_topdown(graph);
	
	jive_node * current;
	while( (current = jive_graph_traverse_next(trav)) != 0) {
		if (!jive_node_is_regop(current)) continue;
		
		regop_group * regop = regop_group_create(&map, current);
		machine->classify_operation(machine, current, &regop->mask);
		
		jive_cpureg_classmask_t mask = regop->mask;
		jive_operand * operand;
		
		operand = jive_node_iterate_operands(current);
		while(operand) {
			regop_group * group = regop_group_for_value(&map, operand->value);
			if (group)
				mask &= group->mask;
			operand = operand->next;
		}
		
		if (!mask) continue;
		
		regop->mask = mask;
		
		operand = jive_node_iterate_operands(current);
		while(operand) {
			regop_group * group = regop_group_for_value(&map, operand->value);
			if (group)
				regop = regop_group_union(&map, group, regop);
			operand = operand->next;
		}
	}
	
	/* now do something with the operations... */
	jive_graph_traverse_finish(trav);
	
	trav = jive_graph_traverse_bottomup(graph);
	while( (current = jive_graph_traverse_next(trav)) != 0) {
		regop_group * regop = regop_group_lookup(&map, current);
		if (!regop) continue;
		size_t n;
		for(n=0; n<machine->nregcls; n++)
			if (regop->mask & (1<<n)) break;
		jive_cpureg_class_t regcls = &machine->regcls[n];
		machine->transform_operation(machine, current, regcls);
	}
	
	jive_graph_traverse_finish(trav);
}

static jive_value *
wrap_fixedand_create(jive_value * a, jive_value * b, unsigned int unused)
{
	return jive_fixedand_create(a, b);
}

static jive_value *
wrap_fixedor_create(jive_value * a, jive_value * b, unsigned int unused)
{
	return jive_fixedor_create(a, b);
}

static jive_value *
wrap_fixedxor_create(jive_value * a, jive_value * b, unsigned int unused)
{
	return jive_fixedxor_create(a, b);
}

typedef jive_value * (*reduction_function_t)(jive_value *, jive_value *, unsigned int);

static jive_node *
transform_binary(jive_bitstring_node * node, unsigned int width, reduction_function_t reduce)
{
	jive_value * values[node->ninputs];
	size_t n;
	/* FIXME: the bit-extension mode is generally too strict */
	for(n=0; n<node->ninputs; n++) {
		values[n] = (jive_value *) node->inputs[n].value;
		values[n] = jive_extend_slice(values[n], 0, width, true);
	}
	for(n=1; n<node->ninputs; n++) {
		values[0] = reduce(values[0], values[n], width);
	}
	
	jive_value_replace((jive_value *)&node->output, values[0]);
	return values[0]->node;
}

jive_node *
jive_arithmetic_transform_single(jive_node * _node, unsigned int width)
{
	if (!jive_node_is_instance(_node, &JIVE_BITSTRING_NODE))
		return _node;
	const jive_node_class * type = _node->type;
	
	/* FIXME: handle "chaining" for multi-word arithmetic */
	
	jive_bitstring_node * node;
	node = (jive_bitstring_node *) _node;
	
	if (type == &JIVE_INTNEG) {
		jive_value * value = (jive_value *) node->inputs[0].value;
		value = jive_extend_slice(value, 0, width, true);
		value = jive_fixedneg_create(value, width);
		
		jive_value_replace((jive_value *)&node->output, value);
		return value->node;
	}
	
	if (type == &JIVE_INTSUM)
		return transform_binary(node, width, &jive_fixedadd_create);
	
	if (type == &JIVE_BITAND)
		return transform_binary(node, width, &wrap_fixedand_create);
	
	if (type == &JIVE_BITOR)
		return transform_binary(node, width, &wrap_fixedor_create);
	
	if (type == &JIVE_BITXOR)
		return transform_binary(node, width, &wrap_fixedxor_create);
	
	return _node;
#if 0
		(type == &JIVE_INTPRODUCT);
#endif
}

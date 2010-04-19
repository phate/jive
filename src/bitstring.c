#include <string.h>
#include <stdio.h>

#include <jive/graph.h>
#include <jive/nodeclass.h>
#include <jive/bitstring.h>
#include <jive/internal/bitstringstr.h>
#include <jive/internal/bitops.h>

#include "debug.h"

#define BITS_PER_LONG (sizeof(long)*8)

/* private helpers */

static inline jive_value *
jive_bitstring_output(jive_node * node)
{
	return (jive_value *)&((jive_bitstring_node *)node)->output;
}

/* bitstring base class */

static char *
jive_value_bits_repr(const void * _self)
{
	const jive_value_bits * self = _self;
	char tmp[32];
	snprintf(tmp, sizeof(tmp), "bits[%d]", self->nbits);
	return strdup(tmp);
}

const jive_value_class JIVE_VALUE_BITS = {
	0, "bitstring", sizeof(jive_value_bits), &jive_value_bits_repr
};

static char *
jive_operand_bits_repr(const void * _self)
{
	const jive_operand_bits * self = _self;
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "%d", self->index);
	return strdup(tmp);
}

const jive_operand_class JIVE_OPERAND_BITS ={ 
	0, "input bits", sizeof(jive_operand_bits), &jive_operand_bits_repr
};

static void
jive_bitstring_invalidate_inputs(void * node)
{
	jive_value_bits * port = (jive_value_bits *) jive_bitstring_output(node);
	jive_output_edge_iterator i;
	/* if already invalidated, terminate */
	if (!port->_value_range.uptodate) return;
	port->_value_range.uptodate = false;
	JIVE_ITERATE_OUTPUTS(i, node)
		if (i->origin.port == (jive_value *)port) jive_node_invalidate(i->target.node);
}

const jive_node_class JIVE_BITSTRING_NODE = {
	0, "BITSTRING", sizeof(jive_bitstring_node), 0,
	
	.repr = 0,
	.equiv = 0,
	.invalidate_inputs = &jive_bitstring_invalidate_inputs

};

/* common public functions */

void
jive_bitstring_value_range_numeric(jive_bitstring_value_range * value_range)
{
	/* if data type is too large to be represented in a machine
	word, give up */
	if (value_range->nbits > BITS_PER_LONG) {
		value_range->numeric = false;
		return;
	}
	/* compute maximum set of bits that can be zero or
	can be one */
	unsigned long min_bits = 0, max_bits = 0, bit = 1;
	size_t n;
	for(n=0; n<value_range->nbits-1; n++) {
		switch(value_range->bits[n]) {
			case '0': break;
			case '1': min_bits |= bit;
			default: max_bits |= bit;
		}
		bit <<= 1;
	}
	
	/* pretend upper-most bit is swapped and correct for sign
	afterwards */
	switch (value_range->bits[n]) {
		case '0': min_bits |= bit;
		default: max_bits |=bit;
		case '1': break;
	}
	
	value_range->low = min_bits - bit;
	value_range->high = max_bits - bit;
	value_range->numeric = true;
}

void
jive_bitstring_value_range_bits(jive_bitstring_value_range * value_range)
{
	if (!value_range->numeric) {
		memset(value_range->bits, 'D', value_range->nbits);
		return;
	}
	
	/* find highest common bit */
	int n=value_range->nbits-1;
	while(n>=0) {
		long bit=1<<n;
		if ((value_range->low & bit) == (value_range->high & bit)) {
			if (value_range->low & bit) value_range->bits[n]='1';
			else value_range->bits[n]='0';
		} else break;
		n--;
	}
	while(n>=0) {
		value_range->bits[n]='D';
		n--;
	}
}

void
jive_operand_bits_init(jive_operand_bits * input, jive_value * value, unsigned int index)
{
	DEBUG_ASSERT( jive_value_is_instance(value, &JIVE_VALUE_BITS) );
	input->type = &JIVE_OPERAND_BITS;
	jive_operand_init((jive_operand *) input, value);
	input->index = index;
}

void
jive_value_bits_init(jive_value_bits * value, jive_node * node, unsigned int nbits)
{
	value->type = &JIVE_VALUE_BITS;
	
	jive_value_init((jive_value *) value, node);
	value->nbits = nbits;
	jive_graph * graph = node->graph;
	jive_bitstring_value_range * value_range = &value->_value_range;
	
	value_range->bits = jive_malloc(graph, nbits);
	memset(value_range->bits, 'D', nbits);
	value_range->uptodate = false;
	value_range->nbits = nbits;
	jive_bitstring_value_range_numeric(value_range);
}

bool
jive_match_bitstring_node_inputs(const jive_node * _node, size_t ninputs, jive_value * const inputs[])
{
	size_t n;
	if (!jive_node_is_instance(_node, &JIVE_BITSTRING_NODE)) return false;
	const jive_bitstring_node * node = (jive_bitstring_node *) _node;
	
	if (node->ninputs != ninputs) return false;
	for(n=0; n<ninputs; n++)
		if (node->inputs[n].value != (jive_value_bits *)inputs[n]) return false;
	return true;
}

jive_operand_list *
jive_input_bits_list_create(jive_graph * graph, size_t noperands, jive_value * const values[], jive_operand_bits ** operands)
{
	*operands = 0;
	if (!noperands) return 0;
	
	*operands = (jive_operand_bits *) jive_malloc(graph, sizeof(jive_operand_bits) * noperands);
	
	size_t n;
	for(n=0; n<noperands; n++) {
		(*operands)[n].type = &JIVE_OPERAND_BITS;
		DEBUG_ASSERT( jive_value_is_instance(values[n], &JIVE_VALUE_BITS) );
		jive_operand_init((jive_operand *)&(*operands)[n], values[n]);
	}
	
	return (jive_operand_list *)(*operands);
}

static inline void *
jive_bitstring_create(jive_graph * graph, const jive_node_class * type, size_t ninputs, jive_value * const inout[])
{
	jive_operand_bits * inputs;
	jive_operand_list * list = jive_input_bits_list_create(graph, ninputs, inout, &inputs);
	jive_node * _node = jive_node_create(graph, type, ninputs, list);
	jive_bitstring_node * node = (jive_bitstring_node *) _node;
	node->ninputs = ninputs;
	node->inputs = inputs;
	
	return node;
}

jive_value *
jive_bitstring_input(const jive_node * node, size_t index)
{
	DEBUG_ASSERT(jive_node_is_instance(node, &JIVE_BITSTRING_NODE));
	return (jive_value *)((const jive_bitstring_node *)node)->inputs[index].value;
}

size_t
jive_bitstring_ninputs(const jive_node * node)
{
	return ((const jive_bitstring_node *)node)->ninputs;
}

/* bitsymbolicconstant */

static char *
jive_bitsymbolicconstant_repr(const void * _self)
{
	const struct _jive_bitsymbolicconstant * self = _self;
	
	return strdup(self->name);
}

const jive_node_class JIVE_BITSYMBOLICCONSTANT = {
	&JIVE_BITSTRING_NODE, "BITSYMBOLICCONSTANT", sizeof(struct _jive_bitsymbolicconstant), 0,
	
	.repr = jive_bitsymbolicconstant_repr,
	.equiv = 0,
	.invalidate_inputs = &jive_bitstring_invalidate_inputs
};

jive_node *
jive_bitsymbolicconstant_rawcreate(jive_graph * graph, const char * name, size_t nbits)
{
	struct _jive_bitsymbolicconstant * node;
	node = jive_bitstring_create(graph, &JIVE_BITSYMBOLICCONSTANT, 0, 0);
	
	size_t namelen = strlen(name);
	char * name_storage = jive_malloc(graph, namelen+1);
	node->name = name_storage;
	memcpy(name_storage, name, namelen+1);
	
	jive_value_bits_init(&node->base.output, (jive_node *)node, nbits);
	
	return (jive_node *)node;
}

jive_value *
jive_bitsymbolicconstant(jive_graph * graph, const char * name, size_t nbits)
{
	jive_output_edge_iterator i;
	JIVE_ITERATE_TOP(i, graph)
		if (jive_match_bitsymbolicconstant_node(i->target.node, name, nbits))
			return (jive_value*) jive_bitstring_output(i->target.node);
	return (jive_value*) jive_bitstring_output(jive_bitsymbolicconstant_rawcreate(graph, name, nbits));
}

const char *
jive_bitsymbolicconstant_name(const jive_node * node)
{
	return ((struct _jive_bitsymbolicconstant *)node)->name;
}

bool
jive_match_bitsymbolicconstant_node(const jive_node * node, const char * name, size_t nbits)
{
	if (node->type != &JIVE_BITSYMBOLICCONSTANT) return false;
	/* FIXME: also compare number of bits */
	return strcmp(jive_bitsymbolicconstant_name(node), name)==0;
}

void
jive_bitsymbolicconstant_set_value_range(jive_node * node, const jive_bitstring_value_range * value_range)
{
	DEBUG_ASSERT(node->type == &JIVE_BITSYMBOLICCONSTANT);
	jive_value_bits * output = (jive_value_bits *) jive_bitstring_output(node);
	jive_output_edge_iterator i;
	JIVE_ITERATE_OUTPUTS(i, node)
		jive_node_invalidate(i->target.node);
	
	output->_value_range.low = value_range->low;
	output->_value_range.high = value_range->high;
	output->_value_range.numeric = value_range->numeric;
	DEBUG_ASSERT(output->_value_range.nbits == value_range->nbits);
	memcpy(output->_value_range.bits, value_range->bits, output->_value_range.nbits);
	output->_value_range.uptodate = true;
}

void
jive_bitsymbolicconstant_set_value_range_numeric(jive_node * node, long low, long high)
{
	DEBUG_ASSERT(node->type == &JIVE_BITSYMBOLICCONSTANT);
	jive_value_bits * output = (jive_value_bits *) jive_bitstring_output(node);
	
	char bits[output->nbits];
	
	jive_bitstring_value_range range;
	range.nbits = output->nbits;
	range.low = low;
	range.high = high;
	range.numeric = true;
	range.bits = bits;
	jive_bitstring_value_range_bits(&range);
	jive_bitsymbolicconstant_set_value_range(node, &range);
}

/* bitconstant */

static char *
jive_bitconstant_repr(const void * _self)
{
	const struct _jive_bitconstant * self = _self;
	char tmp[self->data.nbits+1];
	size_t n;
	for(n=0; n<self->data.nbits; n++)
		tmp[n] = self->data.bits[self->data.nbits-n-1];
	tmp[self->data.nbits]=0;
	
	return strdup(tmp);
}

static void
jive_bitconstant_revalidate(void * _node)
{
	struct _jive_bitconstant * node = _node;
	jive_bitstring_value_range * output = jive_bitstring_output_value_range(_node);
	memcpy(output->bits, node->data.bits, node->data.nbits);
	jive_bitstring_value_range_numeric(output);
	output->uptodate = true;
}

const jive_node_class JIVE_BITCONSTANT = {
	&JIVE_BITSTRING_NODE, "BITCONSTANT", sizeof(struct _jive_bitconstant), 0,
	
	.repr = jive_bitconstant_repr,
	.equiv = 0,
	.invalidate_inputs = &jive_bitstring_invalidate_inputs,
	.revalidate_outputs = &jive_bitconstant_revalidate
};

jive_node *
jive_bitconstant_rawcreate(jive_graph * graph, size_t nbits, const char * bits)
{
	struct _jive_bitconstant * node;
	node = jive_bitstring_create(graph, &JIVE_BITCONSTANT, 0, 0);
	
	char * bits_storage = jive_malloc(graph, nbits);
		
	node->data.bits = bits_storage;
	node->data.nbits = nbits;
	memcpy(bits_storage, bits, nbits);
	
	jive_value_bits_init(&node->base.output, (jive_node *)node, nbits);
	
	return (jive_node *)node;
}

jive_value *
jive_bitconstant(jive_graph * graph, size_t nbits, const char * bits)
{
	jive_output_edge_iterator i;
	JIVE_ITERATE_TOP(i, graph)
		if (jive_match_bitconstant_node(i->target.node, nbits, bits))
			return jive_bitstring_output(i->target.node);
	return jive_bitstring_output(jive_bitconstant_rawcreate(graph, nbits, bits));
}

const jive_bitconstant_nodedata *
jive_bitconstant_info(const jive_node * node)
{
	DEBUG_ASSERT(node->type == &JIVE_BITCONSTANT);
	return &((const struct _jive_bitconstant *)node)->data;
}

bool
jive_match_bitconstant_node(const jive_node * node, size_t nbits, const char * bits)
{
	if (node->type != &JIVE_BITCONSTANT) return false;
	const jive_bitconstant_nodedata * data = jive_bitconstant_info(node);
	return data->nbits == nbits && memcmp(data->bits, bits, nbits)==0;
}

/* bitslice */

static char *
jive_bitslice_repr(const void * _self)
{
	const struct _jive_bitslice * self = _self;
	
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "SLICE[%d:%d]", self->data.low, self->data.high);
	
	return strdup(tmp);
}

static void
jive_bitslice_revalidate(void * _node)
{
	struct _jive_bitslice * node = _node;
	const jive_bitstring_value_range * input = jive_bitstring_input_value_range(node, 0);
	jive_bitstring_value_range * output = jive_bitstring_output_value_range(node);
	
	output->uptodate = true;
	
	memcpy(output->bits, input->bits+node->data.low, output->nbits);
	
	/* when slicing the "top" part of any node, this acts as a "signed shift" */
	if (node->data.high == input->nbits) {
		output->low = input->low >> node->data.low;
		output->high = input->high >> node->data.low;
		output->numeric = input->numeric;
	} else jive_bitstring_value_range_numeric(output);
}

const jive_node_class JIVE_BITSLICE = {
	&JIVE_BITSTRING_NODE, "SLICE", sizeof(struct _jive_bitslice), 0,
	
	.repr = jive_bitslice_repr,
	.equiv = 0,
	.invalidate_inputs = &jive_bitstring_invalidate_inputs,
	.revalidate_outputs = &jive_bitslice_revalidate
};

jive_node *
jive_bitslice_rawcreate(jive_value * input, size_t low, size_t high)
{
	DEBUG_ASSERT(jive_value_is_instance(input, &JIVE_VALUE_BITS));
	unsigned int nbits = ((jive_value_bits *)input)->nbits;
	DEBUG_ASSERT(high<=nbits && high>low);
	
	struct _jive_bitslice * node;
	node = jive_bitstring_create(input->node->graph, &JIVE_BITSLICE, 1, &input);
	
	node->data.low = low;
	node->data.high = high;
	
	jive_value_bits_init(&node->base.output, (jive_node *)node, high-low);
	
	return (jive_node *)node;
}

static jive_value *
slice_concat(const jive_node * _concat, size_t low, size_t high)
{
	const jive_bitstring_node * concat = (jive_bitstring_node *) _concat;
	jive_value * new_inputs[concat->ninputs];
	size_t count = 0, n, pos = 0;
	
	for(n=0; n<concat->ninputs; n++) {
		jive_value * tmp = (jive_value *) concat->inputs[n].value;
		unsigned int nbits = concat->inputs[n].value->nbits;
		
		if (low<pos+nbits && high>pos) {
			size_t this_low = 0, this_high = nbits;
			if (low>pos) this_low = low-pos;
			if (high<pos+nbits) this_high = high-pos;
			tmp = jive_bitslice(tmp, this_low, this_high);
			new_inputs[count++] = tmp;
		}
		
		pos += nbits;
	}
	
	return jive_bitconcat(count, new_inputs);
}

jive_value *
jive_bitslice(jive_value * input, size_t low, size_t high)
{
	unsigned int nbits = ((jive_value_bits *)input)->nbits;
	DEBUG_ASSERT(high<=nbits && high>low);
	
	if (low==0 && high==nbits) return input;
	
	if (input->node->type == &JIVE_BITCONSTANT) {
		const jive_bitconstant_nodedata * data = jive_bitconstant_info(input->node);
		
		return jive_bitconstant(input->node->graph, high-low, data->bits+low);
	}
	if (input->node->type == &JIVE_BITSLICE) {
		const jive_bitslice_nodedata * data = jive_bitslice_info(input->node);
		
		const struct jive_bitstring_node * tmp = (const struct jive_bitstring_node *) input->node;
		
		return jive_bitslice((jive_value *) tmp->inputs[0].value, data->low+low, data->low+high);
	}
	if (input->node->type == &JIVE_BITCONCAT) {
		return slice_concat(input->node, low, high);
	}
	
	jive_output_edge_iterator i;
	JIVE_ITERATE_OUTPUTS(i, input->node)
		if (jive_match_bitslice_node(i->target.node, low, high))
			return jive_bitstring_output(i->target.node);
	return jive_bitstring_output(jive_bitslice_rawcreate(input, low, high));
}

const jive_bitslice_nodedata *
jive_bitslice_info(const jive_node * node)
{
	DEBUG_ASSERT(node->type == &JIVE_BITSLICE);
	return &((const struct _jive_bitslice *)node)->data;
}

bool
jive_bitslice_normalized(const jive_node * _node)
{
	DEBUG_ASSERT(_node->type == &JIVE_BITSLICE);
	
	const struct _jive_bitslice * node = (const struct _jive_bitslice *)_node;
	unsigned int input_nbits = node->base.inputs[0].value->nbits;
	const jive_node * subnode = node->base.inputs[0].value->node;
	
	if (node->data.low==0 && node->data.high==input_nbits) return false;
	if (subnode->type == &JIVE_BITCONSTANT) return false;
	if (subnode->type == &JIVE_BITSLICE) return false;
	if (subnode->type == &JIVE_BITCONCAT) return false;
	
	return true;
}

bool
jive_match_bitslice_node(const jive_node * node, size_t low, size_t high)
{
	if (node->type != &JIVE_BITSLICE) return false;
	const jive_bitslice_nodedata * data = jive_bitslice_info(node);
	return data->low == low && data->high == high;
}

/* bitconcat */

static void
unwrap_slice(const jive_value_bits ** port, size_t * low, size_t * high)
{
	*low = 0;
	*high = (*port)->nbits;
	while((*port)->node->type == & JIVE_BITSLICE) {
		const jive_bitslice_nodedata * data = jive_bitslice_info((*port)->node);
		*port = (jive_value_bits *) jive_bitstring_input((*port)->node, 0);
		*low += data->low;
		*high += data->low;
	}
}

static void
jive_bitconcat_revalidate(void * _node)
{
	size_t n, pos = 0;
	struct jive_bitstring_node * node = _node;
	jive_bitstring_value_range * output = jive_bitstring_output_value_range(node);
	
	output->uptodate = true;
	
	for(n=0; n<node->ninputs; n++) {
		const jive_bitstring_value_range * input = jive_bitstring_input_value_range(node, n);
		memcpy(output->bits+pos, input->bits, input->nbits);
		pos += input->nbits;
	}
		
	if (output->nbits > BITS_PER_LONG) {
		output->numeric = false;
		return;
	}
	
	/* try to find reasonable numeric bounds */
	output->numeric = true;
	bool first = true;
	const jive_value_bits * repeated_input;
	size_t repeated_slice_low, repeated_slice_high;
	
	long low, high;
	
	for(n=node->ninputs; n; n--) {
		const jive_bitstring_value_range * input = jive_bitstring_input_value_range(node, n-1);
		
		const jive_value_bits * input_port = (jive_value_bits *) jive_bitstring_input((jive_node *)node, n-1);
		size_t port_slice_low, port_slice_high;
		unsigned long pattern_low = 0, pattern_high = ~(-1 << input->nbits);
		unwrap_slice(&input_port, &port_slice_low, &port_slice_high);
		
		output->numeric = output->numeric && input->numeric;
		if (first) {
			low = input->low >> input->nbits;
			high = input->high >> input->nbits;
			
			repeated_input = input_port;
			repeated_slice_low = port_slice_low;
			repeated_slice_high = port_slice_high;
			
			first=false;
		}
		
		if (input_port == repeated_input && port_slice_high == repeated_slice_high) {
			pattern_low = input->low & pattern_high;
			pattern_high = input->high & pattern_high;
		}
		
		low = (low << input->nbits) | pattern_low;
		high = (high << input->nbits) | pattern_high;
	}
	
	output->low = low;
	output->high = high;
}

const jive_node_class JIVE_BITCONCAT = {
	&JIVE_BITSTRING_NODE, "CONCAT", sizeof(jive_bitstring_node), jive_node_class_associative,
	
	.repr = 0,
	.equiv = 0,
	.invalidate_inputs = &jive_bitstring_invalidate_inputs,
	.revalidate_outputs = &jive_bitconcat_revalidate
};

jive_node *
jive_bitconcat_rawcreate(size_t ninputs, jive_value * const inputs[])
{
	size_t n;
	unsigned int nbits = 0;
	for(n=0; n<ninputs; n++)
		nbits += ((jive_value_bits *) inputs[n])->nbits;
		
	struct jive_bitstring_node * node;
	node = jive_bitstring_create(inputs[0]->node->graph, &JIVE_BITCONCAT, ninputs, inputs);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node *)node;
}

static bool
can_meld(const jive_node * first, const jive_node * second)
{
	if (first->type == &JIVE_BITCONSTANT && second->type == &JIVE_BITCONSTANT)
		return true;
	if (first->type != &JIVE_BITSLICE || second->type != &JIVE_BITSLICE)
		return false;
	
	const jive_bitslice_nodedata * first_slice, * second_slice;
	
	first_slice = jive_bitslice_info(first);
	second_slice = jive_bitslice_info(second);
	
	if (first_slice->high != second_slice->low) return false;
	
	jive_value_bits * first_in = ((const jive_bitstring_node *)first)->inputs[0].value;
	jive_value_bits * second_in = ((const jive_bitstring_node *)second)->inputs[0].value;
	
	return first_in == second_in;
}

static jive_value *
meld_constants(jive_node * first, jive_node * second)
{
	const jive_bitconstant_nodedata * first_constant, * second_constant;
	first_constant = jive_bitconstant_info(first);
	second_constant = jive_bitconstant_info(second);
	
	unsigned int nbits = first_constant->nbits+second_constant->nbits;
	char bits[nbits];
	memcpy(bits, first_constant->bits, first_constant->nbits);
	memcpy(bits+first_constant->nbits, second_constant->bits, second_constant->nbits);
	return (jive_value *)jive_bitconstant(first->graph, nbits, bits);
}

static jive_value *
meld_slices(jive_node * _first, jive_node * _second)
{
	struct _jive_bitslice * first, * second;
	first = (struct _jive_bitslice *) _first;
	second = (struct _jive_bitslice *) _second;
	
	const jive_bitslice_nodedata * first_slice, * second_slice;
	first_slice = jive_bitslice_info(_first);
	second_slice = jive_bitslice_info(_second);
	
	return jive_bitslice((jive_value *)first->base.inputs[0].value, first_slice->low, second_slice->high);
}

static jive_value *
meld(jive_node * first, jive_node * second)
{
	if (first->type == &JIVE_BITCONSTANT)
		return meld_constants(first, second);
	else
		return meld_slices(first, second);
}

jive_value *
jive_bitconcat(size_t ninputs, jive_value * const inputs[])
{
	JIVE_EXPAND_INPUTS(jive_value, ninputs, inputs, JIVE_BITCONCAT, jive_bitstring_ninputs, jive_bitstring_input);
	
	size_t n=0;
	jive_value ** new_inputs = 0;
	while(n+1 < ninputs) {
		jive_node * n1 = inputs[n]->node, * n2 = inputs[n+1]->node;
		if (can_meld(n1, n2)) {
			if (!new_inputs) {
				new_inputs = alloca(sizeof(jive_value_bits *)*ninputs);
				memcpy(new_inputs, inputs, sizeof(jive_value_bits *)*ninputs);
				inputs = new_inputs;
			}
			
			new_inputs[n] = meld(n1, n2);
			memmove(new_inputs+n+1, new_inputs+n+2, sizeof(jive_value_bits *)*(ninputs-n-2));
			ninputs--;
		} else n++;
	}
	
	if (ninputs==1) return inputs[0];
	
	jive_output_edge_iterator i;
	JIVE_ITERATE_OUTPUTS(i, inputs[0]->node) {
		fflush(stdout);
		if (jive_match_bitconcat_node(i->target.node, ninputs, inputs))
			return jive_bitstring_output(i->target.node);
	}
	
	return jive_bitstring_output(jive_bitconcat_rawcreate(ninputs, inputs));
}

bool
jive_bitconcat_normalized(const jive_node * _node)
{
	DEBUG_ASSERT(_node->type == &JIVE_BITCONCAT);
	size_t n;
	const jive_bitstring_node * node = (const jive_bitstring_node *) _node;
	if (node->inputs[0].value->node->type == &JIVE_BITCONCAT)
		return false;
	for(n=1; n<node->ninputs; n++) {
		if (node->inputs[n].value->node->type == &JIVE_BITCONCAT)
			return false;
		if (can_meld(node->inputs[n-1].value->node, node->inputs[n].value->node))
			return false;
	}
	return true;
}

bool
jive_match_bitconcat_node(const jive_node * node, size_t ninputs, jive_value * const inputs[])
{
	if (node->type != &JIVE_BITCONCAT) return false;
	return jive_match_bitstring_node_inputs(node, ninputs, inputs);
}

/* intneg */

static void
jive_intneg_revalidate(void * _node)
{
	size_t n;
	struct _jive_bitstring * node = _node;
	const jive_bitstring_value_range * input = jive_bitstring_input_value_range(node, 0);
	jive_bitstring_value_range * output = jive_bitstring_output_value_range(node);
	
	char carry='1';
	for(n=0; n<output->nbits; n++) {
		char new_carry = jive_logic_and(carry, jive_logic_not(input->bits[n]));
		output->bits[n] = jive_logic_xor(carry, jive_logic_not(input->bits[n]));
		carry = new_carry;
	}
	
	output->low = - input->high;
	output->high = - input->low;
	output->numeric = input->numeric;
	output->uptodate = true;
}

const jive_node_class JIVE_INTNEG = {
	&JIVE_BITSTRING_NODE, "NEGATE", sizeof(jive_bitstring_node), 0,
	
	.repr = 0,
	.equiv = 0,
	.invalidate_inputs = &jive_bitstring_invalidate_inputs,
	.revalidate_outputs = &jive_intneg_revalidate
};

jive_node *
jive_intneg_rawcreate(jive_value * input)
{
	unsigned int nbits = ((jive_value_bits *)input)->nbits;
	
	struct jive_bitstring_node * node;
	node = jive_bitstring_create(input->node->graph, &JIVE_INTNEG, 1, &input);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node *)node;
}

jive_value *
jive_intneg(jive_value * input)
{
	/* FIXME: simplify sums, product, constants and negations */
	/* FIXME: attempt CSE */
	return jive_bitstring_output(jive_intneg_rawcreate(input));
}

/* intsum */

static void
jive_intsum_revalidate(void * _node)
{
	struct jive_bitstring_node * node = _node;
	size_t n, ninputs = node->ninputs;
	jive_bitstring_value_range * output = jive_bitstring_output_value_range(node);
	
	output->low = output->high = 0;
	output->numeric = (output->nbits<=BITS_PER_LONG);
	memset(output->bits, '0', output->nbits);
	char tmpbits[output->nbits];
	
	for(n=0; n<ninputs; n++) {
		const jive_bitstring_value_range * input = jive_bitstring_input_value_range(node, n);
		memcpy(tmpbits, output->bits, output->nbits);
		jive_multibit_sum(output->bits, tmpbits, input->bits, output->nbits);
		output->low += input->low;
		output->high += input->high;
		output->numeric = output->numeric && input->numeric;
	}
	output->uptodate = true;
}

const jive_node_class JIVE_INTSUM = {
	&JIVE_BITSTRING_NODE, "SUM", sizeof(jive_bitstring_node), 0,
	
	.repr = 0,
	.equiv = 0,
	.invalidate_inputs = &jive_bitstring_invalidate_inputs,
	.revalidate_outputs = &jive_intsum_revalidate
};

jive_node *
jive_intsum_rawcreate(size_t ninputs, jive_value * const inputs[])
{
	unsigned int nbits = ((jive_value_bits *)inputs[0])->nbits;;
	/* FIXME: assert that all inputs have same width */
	
	struct jive_bitstring_node * node;
	node = jive_bitstring_create(inputs[0]->node->graph, &JIVE_INTSUM, ninputs, inputs);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node *)node;
}

jive_value *
jive_intsum(size_t ninputs, jive_value * const inputs[])
{
	/* FIXME: assert that all inputs have same width */
	JIVE_EXPAND_INPUTS(jive_value, ninputs, inputs, JIVE_INTSUM, jive_bitstring_ninputs, jive_bitstring_input);
	
	/* FIXME: simplify constants and negations */
	/* FIXME: strength reduction? */
	/* FIXME: attempt CSE */
	return jive_bitstring_output(jive_intsum_rawcreate(ninputs, inputs));
}

size_t
jive_intsum_ninputs(const jive_node * node)
{
	DEBUG_ASSERT(node->type == &JIVE_INTSUM);
	return ((const jive_bitstring_node *)node)->ninputs;
}

const jive_operand_bits *
jive_intsum_input(const jive_node * node, size_t index)
{
	DEBUG_ASSERT(index < jive_intsum_ninputs(node));
	return &((const jive_bitstring_node *)node)->inputs[index];
}

/* intproduct */

static inline long max(long a, long b) { return a>b ? a : b; }
static inline long min(long a, long b) { return a<b ? a : b; }

static void
jive_intproduct_revalidate(void * _node)
{
	struct jive_bitstring_node * node = _node;
	jive_bitstring_value_range * output = jive_bitstring_output_value_range(node);
	size_t n, ninputs = node->ninputs, nbits=output->nbits;
	
	output->low = output->high = 1;
	output->numeric = (output->nbits<=BITS_PER_LONG);
	memset(output->bits, '0', output->nbits);
	char tmpbits[output->nbits];
	
	for(n=0; n<ninputs; n++) {
		const jive_bitstring_value_range * input = jive_bitstring_input_value_range(node, n);
		memcpy(tmpbits, output->bits, output->nbits);
		jive_multibit_multiply(output->bits, nbits, tmpbits, nbits, input->bits, input->nbits);
		
		long tmp1 = input->low * output->low;
		long tmp2 = input->high * output->low;
		long tmp3 = input->low * output->high;
		long tmp4 = input->high * output->high;
		
		output->low = min(min(tmp1, tmp2), min(tmp3, tmp4));
		output->high = max(max(tmp1, tmp2), max(tmp3, tmp4));
		output->numeric = output->numeric && input->numeric;
	}
	output->uptodate = true;
}

const jive_node_class JIVE_INTPRODUCT = {
	&JIVE_BITSTRING_NODE, "PRODUCT", sizeof(jive_bitstring_node), 0,
	
	.repr = 0,
	.equiv = 0,
	.invalidate_inputs = &jive_bitstring_invalidate_inputs,
	.revalidate_outputs = &jive_intproduct_revalidate
};

jive_node *
jive_intproduct_rawcreate(size_t ninputs, jive_value * const inputs[])
{
	size_t n = 0;
	unsigned int nbits = 0;
	for(n=0; n<ninputs; n++) nbits = ((jive_value_bits *)inputs[n])->nbits;
	
	struct jive_bitstring_node * node;
	node = jive_bitstring_create(inputs[0]->node->graph, &JIVE_INTPRODUCT, ninputs, inputs);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node *)node;
}

jive_value *
jive_intproduct(size_t ninputs, jive_value * const inputs[])
{
	/* FIXME: simplify negations */
	JIVE_EXPAND_INPUTS(jive_value, ninputs, inputs, JIVE_INTPRODUCT, jive_bitstring_ninputs, jive_bitstring_input);
	
	/* FIXME: simplify constants */
	/* FIXME: strength reduction? */
	/* FIXME: attempt CSE */
	return jive_bitstring_output(jive_intproduct_rawcreate(ninputs, inputs));
}

size_t
jive_intproduct_ninputs(const jive_node * node)
{
	DEBUG_ASSERT(node->type == &JIVE_INTPRODUCT);
	return ((const jive_bitstring_node *)node)->ninputs;
}

const jive_operand_bits *
jive_intproduct_input(const jive_node * node, size_t index)
{
	DEBUG_ASSERT(index < jive_intproduct_ninputs(node));
	return &((const jive_bitstring_node *)node)->inputs[index];
}

/* low product */

const jive_node_class JIVE_INTLOWPRODUCT = {
	&JIVE_BITSTRING_NODE, "LOWPRODUCT", sizeof(jive_bitstring_node), 0,
	
	.repr = 0,
	.equiv = 0,
	.invalidate_inputs = &jive_bitstring_invalidate_inputs
};

jive_node *
jive_intlowproduct_rawcreate(size_t ninputs, jive_value * const inputs[])
{
	unsigned int nbits = ((jive_value_bits *)inputs[0])->nbits;;
	/* FIXME: assert that all inputs have same width */
	
	struct jive_bitstring_node * node;
	node = jive_bitstring_create(inputs[0]->node->graph, &JIVE_INTLOWPRODUCT, ninputs, inputs);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node *)node;
}

jive_value *
jive_intlowproduct(size_t ninputs, jive_value * const inputs[])
{
	/* FIXME: assert that all inputs have same width */
	/* FIXME: simplify negations */
	JIVE_EXPAND_INPUTS(jive_value, ninputs, inputs, JIVE_INTLOWPRODUCT, jive_bitstring_ninputs, jive_bitstring_input);
	
	/* FIXME: simplify constants */
	/* FIXME: strength reduction? */
	/* FIXME: attempt CSE */
	return jive_bitstring_output(jive_intlowproduct_rawcreate(ninputs, inputs));
}

size_t
jive_intlowproduct_ninputs(const jive_node * node)
{
	DEBUG_ASSERT(node->type == &JIVE_INTLOWPRODUCT);
	return ((const jive_bitstring_node *)node)->ninputs;
}

const jive_operand_bits *
jive_intlowproduct_input(const jive_node * node, size_t index)
{
	DEBUG_ASSERT(index < jive_intlowproduct_ninputs(node));
	return &((const jive_bitstring_node *)node)->inputs[index];
}

/* signed high product */

const jive_node_class JIVE_INTSIGNEDHIGHPRODUCT = {
	&JIVE_BITSTRING_NODE, "SIGNEDHIPRODUCT", sizeof(jive_bitstring_node), 0,
	
	.repr = 0,
	.equiv = 0,
	.invalidate_inputs = &jive_bitstring_invalidate_inputs
};

jive_node *
jive_intsignedhiproduct_rawcreate(size_t ninputs, jive_value * const inputs[])
{
	unsigned int nbits = ((jive_value_bits *)inputs[0])->nbits;;
	/* FIXME: assert that all inputs have same width */
	
	struct jive_bitstring_node * node;
	node = jive_bitstring_create(inputs[0]->node->graph, &JIVE_INTSIGNEDHIGHPRODUCT, ninputs, inputs);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node *)node;
}

jive_value *
jive_intsignedhiproduct(size_t ninputs, jive_value * const inputs[])
{
	/* FIXME: assert that all inputs have same width */
	/* FIXME: simplify negations */
	
	/* FIXME: simplify constants */
	/* FIXME: strength reduction? */
	/* FIXME: attempt CSE */
	return jive_bitstring_output(jive_intsignedhiproduct_rawcreate(ninputs, inputs));
}

/* unsigned high product */

const jive_node_class JIVE_INTUNSIGNEDHIGHPRODUCT = {
	&JIVE_BITSTRING_NODE, "UNSIGNEDHIPRODUCT", sizeof(jive_bitstring_node), 0,
	
	.repr = 0,
	.equiv = 0,
	.invalidate_inputs = &jive_bitstring_invalidate_inputs
};

jive_node *
jive_intunsignedhiproduct_rawcreate(size_t ninputs, jive_value * const inputs[])
{
	unsigned int nbits = ((jive_value_bits *)inputs[0])->nbits;;
	/* FIXME: assert that all inputs have same width */
	
	struct jive_bitstring_node * node;
	node = jive_bitstring_create(inputs[0]->node->graph, &JIVE_INTUNSIGNEDHIGHPRODUCT, ninputs, inputs);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node *)node;
}

jive_value *
jive_intunsignedhiproduct(size_t ninputs, jive_value * const inputs[])
{
	/* FIXME: assert that all inputs have same width */
	
	/* FIXME: simplify constants and negations */
	/* FIXME: strength reduction? */
	/* FIXME: attempt CSE */
	return jive_bitstring_output(jive_intunsignedhiproduct_rawcreate(ninputs, inputs));
}

const jive_bitstring_value_range *
jive_value_bits_get_value_range(const jive_value * _value)
{
	DEBUG_ASSERT( jive_value_is_instance(_value, &JIVE_VALUE_BITS) );
	jive_value_bits * value = (jive_value_bits *) _value;
	if (!value->_value_range.uptodate) jive_node_revalidate(value->node);
	return &value->_value_range;
}


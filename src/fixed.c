#include <jive/internal/fixedstr.h>
#include <jive/internal/bitstringstr.h>
#include <jive/internal/bitops.h>
#include <jive/internal/compiler.h>
#include <jive/nodeclass.h>
#include <jive/machine.h>

#include "debug.h"

const jive_node_class JIVE_FIXED = {
	0, "FIXED", sizeof(jive_fixed_base), 0,
};

/* get arithmetic width of fixed-bitwidth operation */
unsigned int
jive_fixed_arithmetic_width(const jive_node * node)
{
	DEBUG_ASSERT( jive_node_is_instance(node, &JIVE_FIXED) );
	return ((const jive_fixed_base *) node) -> arithmetic_width;

}

static inline jive_value *
jive_fixed_binaryop_output(jive_node * _node)
{
	jive_fixed_binaryop * node = (jive_fixed_binaryop *) _node;
	return (jive_value *) &node->output;
}

static jive_fixed_binaryop *
jive_fixed_binaryop_create(
	jive_graph * graph,
	const jive_node_class * type,
	jive_value * inputs[2],
	unsigned int arithmetic_width)
{
	jive_operand_bits * operands;
	jive_operand_list * list = jive_input_bits_list_create(graph, 2, inputs, &operands);
	jive_node * _node = jive_node_create(graph, type, 2, list);
	jive_fixed_binaryop * node = (jive_fixed_binaryop *) _node;
	node->inputs = operands;
	node->arithmetic_width = arithmetic_width;
	
	return node;
}

static char *
jive_fixed_binaryop_repr(const void * _self)
{
	const jive_fixed_binaryop * self = _self;
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "%s (%d)", self->base.type->name, self->arithmetic_width);
	
	return strdup(tmp);
}

static bool
jive_fixed_binaryop_equiv(const void * _self, const void * _other)
{
	const jive_fixed_binaryop * self = _self;
	const jive_fixed_binaryop * other = _other;
	
	return self->arithmetic_width == other->arithmetic_width;
}

static void
jive_fixed_binaryop_invalidate_inputs(void * _node)
{
	struct jive_fixed_binaryop * node = _node;
	
	if (!node->output._value_range.uptodate) return;
	
	node->output._value_range.uptodate = false;
	jive_output_edge_iterator i;
	JIVE_ITERATE_OUTPUTS(i, (jive_node *) node)
		if (i->origin.port)
			jive_node_invalidate(i->target.node);
}

static unsigned int
binaryop_input_nbits(jive_value * _a, jive_value * _b)
{
	jive_value_bits * a = jive_value_bits_cast(_a);
	jive_value_bits * b = jive_value_bits_cast(_b);
	
	if (unlikely(a->nbits != b->nbits))
		jive_graph_fatal_error(a->node->graph, "mismatched number of bits");
	
	return a->nbits;
}

static void
validate_arithmetic_width(jive_graph * graph, unsigned int nbits, unsigned int arithmetic_width)
{
	if (unlikely(arithmetic_width > nbits || nbits % arithmetic_width))
		jive_graph_fatal_error(graph, "invalid arithmetic width");
}

static inline jive_value *
jive_fixed_unaryop_output(jive_node * _node)
{
	jive_fixed_unaryop * node = (jive_fixed_unaryop *) _node;
	return (jive_value *) &node->output;
}

static jive_fixed_unaryop *
jive_fixed_unaryop_create(
	jive_graph * graph,
	const jive_node_class * type,
	jive_value * input,
	unsigned int arithmetic_width)
{
	jive_operand_bits * operands;
	jive_operand_list * list = jive_input_bits_list_create(graph, 1, &input, &operands);
	jive_node * _node = jive_node_create(graph, type, 1, list);
	jive_fixed_unaryop * node = (jive_fixed_unaryop *) _node;
	node->inputs = operands;
	node->arithmetic_width = arithmetic_width;
	
	return node;
}

static char *
jive_fixed_unaryop_repr(const void * _self)
{
	const jive_fixed_unaryop * self = _self;
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "%s (%d)", self->base.type->name, self->arithmetic_width);
	
	return strdup(tmp);
}

static bool
jive_fixed_unaryop_equiv(const void * _self, const void * _other)
{
	const jive_fixed_unaryop * self = _self;
	const jive_fixed_unaryop * other = _other;
	
	return self->arithmetic_width == other->arithmetic_width;
}

static void
jive_fixed_unaryop_invalidate_inputs(void * _node)
{
	struct jive_fixed_unaryop * node = _node;
	
	if (!node->output._value_range.uptodate) return;
	
	node->output._value_range.uptodate = false;
	jive_output_edge_iterator i;
	JIVE_ITERATE_OUTPUTS(i, (jive_node *) node)
		if (i->origin.port)
			jive_node_invalidate(i->target.node);
}

/* fixedadd */

static void
jive_fixedadd_revalidate(void * _node)
{
	struct jive_fixed_binaryop * node = _node;
	
	jive_bitstring_value_range * output_range = &node->output._value_range;
	const jive_bitstring_value_range * input_range[2] = {
		jive_value_bits_get_value_range((jive_value *) &node->inputs[0]),
		jive_value_bits_get_value_range((jive_value *) &node->inputs[1])
	};
	size_t nbits = output_range->nbits, width = node->arithmetic_width, n;
	
	for(n=0; n<nbits; n+=width) {
		jive_multibit_sum(output_range->bits+n,
			input_range[0]->bits+n,
			input_range[1]->bits+n, width);
	}
	
	jive_bitstring_value_range_numeric(output_range);
	output_range->uptodate = true;
}

const jive_node_class JIVE_FIXEDADD = {
	&JIVE_FIXED, "FIXEDADD", sizeof(jive_fixed_binaryop), 0,
	
	.repr = &jive_fixed_binaryop_repr,
	.equiv = &jive_fixed_binaryop_equiv,
	.invalidate_inputs = &jive_fixed_binaryop_invalidate_inputs,
	.revalidate_outputs = &jive_fixedadd_revalidate
};

jive_node *
jive_fixedadd_rawcreate(jive_value * a, jive_value * b, unsigned int arithmetic_width)
{
	jive_value * inputs[2] = {a, b};
	unsigned int nbits = binaryop_input_nbits(a, b);
	validate_arithmetic_width(a->node->graph, nbits, arithmetic_width);
	
	jive_fixed_binaryop * node;
	node = jive_fixed_binaryop_create(inputs[0]->node->graph, &JIVE_FIXEDADD, inputs, arithmetic_width);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node*)node;
}

jive_value *
jive_fixedadd_create(jive_value * a, jive_value * b, unsigned int arithmetic_width)
{
	jive_node * node = jive_fixedadd_rawcreate(a, b, arithmetic_width);
	return jive_fixed_binaryop_output(node);
}

/* fixedmul */

static void
jive_fixedmul_revalidate(void * _node)
{
	struct jive_fixed_binaryop * node = _node;
	
	jive_bitstring_value_range * output_range = &node->output._value_range;
	const jive_bitstring_value_range * input_range[2] = {
		jive_value_bits_get_value_range((jive_value *) &node->inputs[0]),
		jive_value_bits_get_value_range((jive_value *) &node->inputs[1])
	};
	size_t nbits = output_range->nbits, width = node->arithmetic_width, n;
	
	for(n=0; n<nbits; n+=width) {
		jive_multibit_multiply_shiftright(
			output_range->bits+n,
			input_range[0]->bits+n,
			input_range[1]->bits+n,
			width, 0);
	}
	
	jive_bitstring_value_range_numeric(output_range);
	output_range->uptodate = true;
}

const jive_node_class JIVE_FIXEDMUL = {
	&JIVE_FIXED, "FIXEDMUL", sizeof(jive_fixed_binaryop), 0,
	
	.repr = &jive_fixed_binaryop_repr,
	.equiv = &jive_fixed_binaryop_equiv,
	.invalidate_inputs = &jive_fixed_binaryop_invalidate_inputs,
	.revalidate_outputs = &jive_fixedmul_revalidate
};

jive_node *
jive_fixedmul_rawcreate(jive_value * a, jive_value * b, unsigned int arithmetic_width)
{
	jive_value * inputs[2] = {a, b};
	unsigned int nbits = binaryop_input_nbits(a, b);
	validate_arithmetic_width(a->node->graph, nbits, arithmetic_width);
	
	jive_fixed_binaryop * node;
	node = jive_fixed_binaryop_create(inputs[0]->node->graph, &JIVE_FIXEDMUL, inputs, arithmetic_width);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node*)node;
}

jive_value *
jive_fixedmul_create(jive_value * a, jive_value * b, unsigned int arithmetic_width)
{
	jive_node * node = jive_fixedmul_rawcreate(a, b, arithmetic_width);
	return jive_fixed_binaryop_output(node);
}

/* fixedmulhi */

static void
jive_fixedmulhi_revalidate(void * _node)
{
	struct jive_fixed_binaryop * node = _node;
	
	jive_bitstring_value_range * output_range = &node->output._value_range;
	const jive_bitstring_value_range * input_range[2] = {
		jive_value_bits_get_value_range((jive_value *) &node->inputs[0]),
		jive_value_bits_get_value_range((jive_value *) &node->inputs[1])
	};
	size_t nbits = output_range->nbits, width = node->arithmetic_width, n;
	
	for(n=0; n<nbits; n+=width) {
		jive_multibit_multiply_shiftright(
			output_range->bits+n,
			input_range[0]->bits+n,
			input_range[1]->bits+n,
			width, width);
	}
	
	jive_bitstring_value_range_numeric(output_range);
	output_range->uptodate = true;
}

const jive_node_class JIVE_FIXEDMULHI = {
	&JIVE_FIXED, "FIXEDMULHI", sizeof(jive_fixed_binaryop), 0,
	
	.repr = &jive_fixed_binaryop_repr,
	.equiv = &jive_fixed_binaryop_equiv,
	.invalidate_inputs = &jive_fixed_binaryop_invalidate_inputs,
	.revalidate_outputs = &jive_fixedmulhi_revalidate
};

jive_node *
jive_fixedmulhi_rawcreate(jive_value * a, jive_value * b, unsigned int arithmetic_width)
{
	jive_value * inputs[2] = {a, b};
	unsigned int nbits = binaryop_input_nbits(a, b);
	validate_arithmetic_width(a->node->graph, nbits, arithmetic_width);
	
	jive_fixed_binaryop * node;
	node = jive_fixed_binaryop_create(inputs[0]->node->graph, &JIVE_FIXEDMULHI, inputs, arithmetic_width);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node*)node;
}

jive_value *
jive_fixedmulhi_create(jive_value * a, jive_value * b, unsigned int arithmetic_width)
{
	jive_node * node = jive_fixedmulhi_rawcreate(a, b, arithmetic_width);
	return jive_fixed_binaryop_output(node);
}

/* fixedand */

static void
jive_fixedand_revalidate(void * _node)
{
	struct jive_fixed_binaryop * node = _node;
	
	jive_bitstring_value_range * output_range = &node->output._value_range;
	const jive_bitstring_value_range * input_range[2] = {
		jive_value_bits_get_value_range((jive_value *) &node->inputs[0]),
		jive_value_bits_get_value_range((jive_value *) &node->inputs[1])
	};
	size_t nbits = output_range->nbits, n;
	
	for(n=0; n<nbits; n++) {
		output_range->bits[n] = jive_logic_and(input_range[0]->bits[n], input_range[1]->bits[n]);
	}
	
	jive_bitstring_value_range_numeric(output_range);
	output_range->uptodate = true;
}

const jive_node_class JIVE_FIXEDAND = {
	&JIVE_FIXED, "FIXEDAND", sizeof(jive_fixed_binaryop), 0,
	
	.repr = &jive_fixed_binaryop_repr,
	.equiv = &jive_fixed_binaryop_equiv,
	.invalidate_inputs = &jive_fixed_binaryop_invalidate_inputs,
	.revalidate_outputs = &jive_fixedand_revalidate
};

jive_node *
jive_fixedand_rawcreate(jive_value * a, jive_value * b)
{
	jive_value * inputs[2] = {a, b};
	unsigned int nbits = binaryop_input_nbits(a, b);
	unsigned int arithmetic_width = nbits;
	
	jive_fixed_binaryop * node;
	node = jive_fixed_binaryop_create(inputs[0]->node->graph, &JIVE_FIXEDAND, inputs, arithmetic_width);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node*)node;
}

jive_value *
jive_fixedand_create(jive_value * a, jive_value * b)
{
	jive_node * node = jive_fixedand_rawcreate(a, b);
	return jive_fixed_binaryop_output(node);
}

/* fixedor */

static void
jive_fixedor_revalidate(void * _node)
{
	struct jive_fixed_binaryop * node = _node;
	
	jive_bitstring_value_range * output_range = &node->output._value_range;
	const jive_bitstring_value_range * input_range[2] = {
		jive_value_bits_get_value_range((jive_value *) &node->inputs[0]),
		jive_value_bits_get_value_range((jive_value *) &node->inputs[1])
	};
	size_t nbits = output_range->nbits, n;
	
	for(n=0; n<nbits; n++) {
		output_range->bits[n] = jive_logic_and(input_range[0]->bits[n], input_range[1]->bits[n]);
	}
	
	jive_bitstring_value_range_numeric(output_range);
	output_range->uptodate = true;
}

const jive_node_class JIVE_FIXEDOR = {
	&JIVE_FIXED, "FIXEDOR", sizeof(jive_fixed_binaryop), 0,
	
	.repr = &jive_fixed_binaryop_repr,
	.equiv = &jive_fixed_binaryop_equiv,
	.invalidate_inputs = &jive_fixed_binaryop_invalidate_inputs,
	.revalidate_outputs = &jive_fixedor_revalidate
};

jive_node *
jive_fixedor_rawcreate(jive_value * a, jive_value * b)
{
	jive_value * inputs[2] = {a, b};
	unsigned int nbits = binaryop_input_nbits(a, b);
	unsigned int arithmetic_width = nbits;
	
	jive_fixed_binaryop * node;
	node = jive_fixed_binaryop_create(inputs[0]->node->graph, &JIVE_FIXEDOR, inputs, arithmetic_width);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node*)node;
}

jive_value *
jive_fixedor_create(jive_value * a, jive_value * b)
{
	jive_node * node = jive_fixedor_rawcreate(a, b);
	return jive_fixed_binaryop_output(node);
}

/* fixedxor */

static void
jive_fixedxor_revalidate(void * _node)
{
	struct jive_fixed_binaryop * node = _node;
	
	jive_bitstring_value_range * output_range = &node->output._value_range;
	const jive_bitstring_value_range * input_range[2] = {
		jive_value_bits_get_value_range((jive_value *) &node->inputs[0]),
		jive_value_bits_get_value_range((jive_value *) &node->inputs[1])
	};
	size_t nbits = output_range->nbits, n;
	
	for(n=0; n<nbits; n++) {
		output_range->bits[n] = jive_logic_and(input_range[0]->bits[n], input_range[1]->bits[n]);
	}
	
	jive_bitstring_value_range_numeric(output_range);
	output_range->uptodate = true;
}

const jive_node_class JIVE_FIXEDXOR = {
	&JIVE_FIXED, "FIXEDXOR", sizeof(jive_fixed_binaryop), 0,
	
	.repr = &jive_fixed_binaryop_repr,
	.equiv = &jive_fixed_binaryop_equiv,
	.invalidate_inputs = &jive_fixed_binaryop_invalidate_inputs,
	.revalidate_outputs = &jive_fixedxor_revalidate
};

jive_node *
jive_fixedxor_rawcreate(jive_value * a, jive_value * b)
{
	jive_value * inputs[2] = {a, b};
	unsigned int nbits = binaryop_input_nbits(a, b);
	unsigned int arithmetic_width = nbits;
	
	jive_fixed_binaryop * node;
	node = jive_fixed_binaryop_create(inputs[0]->node->graph, &JIVE_FIXEDXOR, inputs, arithmetic_width);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node*)node;
}

jive_value *
jive_fixedxor_create(jive_value * a, jive_value * b)
{
	jive_node * node = jive_fixedxor_rawcreate(a, b);
	return jive_fixed_binaryop_output(node);
}

/* fixedneg */

static void
jive_fixedneg_revalidate(void * _node)
{
	struct jive_fixed_unaryop * node = _node;
	
	jive_bitstring_value_range * output_range = &node->output._value_range;
	const jive_bitstring_value_range * input_range = jive_value_bits_get_value_range((jive_value *) &node->inputs[0]);
	
	size_t nbits = output_range->nbits, width = node->arithmetic_width, n;
	
	char carry;
	for(n=0; n<nbits; n+=width) {
		if ((n % width) == 0) carry = '1';
		
		char bit = jive_logic_xor(input_range->bits[n], '1');
		
		char new_carry = jive_logic_carry(
			bit,
			'0',
			carry);
		bit = jive_logic_add(
			bit,
			'0',
			carry);
		
		output_range->bits[n] = bit;
		carry = new_carry;
	}
	
	jive_bitstring_value_range_numeric(output_range);
	output_range->uptodate = true;
}

const jive_node_class JIVE_FIXEDNEG = {
	&JIVE_FIXED, "FIXEDNEG", sizeof(jive_fixed_unaryop), 0,
	
	.repr = &jive_fixed_unaryop_repr,
	.equiv = &jive_fixed_unaryop_equiv,
	.invalidate_inputs = &jive_fixed_unaryop_invalidate_inputs,
	.revalidate_outputs = &jive_fixedneg_revalidate
};

jive_node *
jive_fixedneg_rawcreate(jive_value * a, unsigned int arithmetic_width)
{
	unsigned int nbits = jive_value_bits_cast(a)->nbits;
	
	DEBUG_ASSERT((nbits % arithmetic_width) == 0);
	
	jive_fixed_unaryop * node;
	node = jive_fixed_unaryop_create(a->node->graph, &JIVE_FIXEDNEG, a, arithmetic_width);
	
	jive_value_bits_init(&node->output, (jive_node *)node, nbits);
	
	return (jive_node*)node;
}

jive_value *
jive_fixedneg_create(jive_value * a, unsigned int arithmetic_width)
{
	jive_node * node = jive_fixedneg_rawcreate(a, arithmetic_width);
	return jive_fixed_unaryop_output(node);
}

/* constant bitstrings placed in a specific register class */

static char *
jive_regconstant_repr(const void * _self)
{
	const struct jive_regconstant * self = _self;
	
	return strdup(self->bits);
}

static bool
jive_regconstant_equiv(const void * _self, const void * _other)
{
	const struct jive_regconstant * self = _self;
	const struct jive_regconstant * other = _other;
	
	jive_cpureg_class_t self_class, other_class;
	
	self_class = jive_value_get_cpureg_class((jive_value *) &self->output);
	other_class = jive_value_get_cpureg_class((jive_value *) &other->output);
	
	if (self_class != other_class) return false;
	
	return memcmp(self->bits, other->bits, self_class->nbits) == 0;
}

const jive_node_class JIVE_REGCONSTANT = {
	0, "REGCONSTANT", sizeof(struct jive_regconstant), 0,
	
	.repr = &jive_regconstant_repr,
	.equiv = &jive_regconstant_equiv,
	.invalidate_inputs = 0,
	.revalidate_outputs = 0
};

jive_node *
jive_regconstant_rawcreate(jive_graph * graph, jive_cpureg_class_t regcls, const char * bits)
{
	struct jive_regconstant * node;
	node = (struct jive_regconstant *) jive_node_create(graph, &JIVE_REGCONSTANT, 0, 0);
	
	char * bits_storage = jive_malloc(graph, regcls->nbits);
		
	node->bits = bits_storage;
	memcpy(bits_storage, bits, regcls->nbits);
	
	jive_value_bits_init(&node->output, (jive_node *)node, regcls->nbits);
	jive_value_set_cpureg_class((jive_value *) &node->output, regcls);
	
	return (jive_node *)node;
}

jive_value *
jive_regconstant_create(jive_graph * graph, jive_cpureg_class_t regcls, const char * bits)
{
	jive_output_edge_iterator i;
	JIVE_ITERATE_TOP(i, graph)
		if (jive_regconstant_match(i->target.node, regcls, bits)) {
			return (jive_value *)&((struct jive_regconstant *) i->target.node)->output;
		}
	
	jive_node * node = jive_regconstant_rawcreate(graph, regcls, bits);
	
	return (jive_value *)&((struct jive_regconstant *) node)->output;
}

bool
jive_regconstant_match(const jive_node * _node, jive_cpureg_class_t regcls, const char * bits)
{
	if (_node->type != &JIVE_REGCONSTANT) return false;
	const struct jive_regconstant * node;
	node = (const struct jive_regconstant *)_node;
	
	if (jive_value_get_cpureg_class((jive_value *) &node->output) != regcls)
		return false;
	
	return memcmp(node->bits, bits, regcls->nbits) == 0;
}


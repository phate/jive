/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/symbolic-expression.h>

#include <jive/context.h>
#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/symbolic-constant.h>

static char *
unop_symbolic_expression(jive_context * context, jive_node * node, const char * operator_string)
{
	char * tmp = jive_bitstring_symbolic_expression(context, node->inputs[0]->origin);
	char * res = jive_context_strjoin(context, "(", operator_string, tmp, ")", NULL);
	jive_context_free(context, tmp);
	return res;
}

static char *
binop_symbolic_expression(jive_context * context, jive_node * node, const char * operator_string)
{
	char * acc = jive_bitstring_symbolic_expression(context, node->inputs[0]->origin);
	
	size_t n;
	for (n = 1; n < node->ninputs; n++) {
		char * op = jive_bitstring_symbolic_expression(context, node->inputs[n]->origin);
		
		char * tmp = jive_context_strjoin(context, acc, operator_string, op, NULL);
		jive_context_free(context, op);
		jive_context_free(context, acc);
		acc = tmp;
	}
	
	char * res = jive_context_strjoin(context, "(", acc, ")", NULL);
	jive_context_free(context, acc);
	return res;
}

static char *
bitsymbolicconstant_symbolic_expression(jive_context * context, const jive_bitsymbolicconstant_node * node)
{
	return jive_context_strdup(context, node->attrs.name);
}

static char *
bitconstant_symbolic_expression(jive_context * context, const jive_bitconstant_node * node)
{
	static const char hexdigit[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	size_t ndigits = (node->attrs.nbits + 3) / 4;
	size_t nchars = ndigits + 2;
	
	char * buffer = jive_context_malloc(context, nchars + 1);
	buffer[0] = '0';
	buffer[1] = 'x';
	buffer[nchars] = 0;
	
	size_t p = nchars - 1;
	size_t n, nibble = 0, nibble_bit = 0;
	for (n = 0; n < node->attrs.nbits; n++) {
		char bit = node->attrs.bits[n];
		if (bit != '0' && bit != '1')
			bit = '0';
		bit = bit - '0';
		nibble |= bit << nibble_bit;
		nibble_bit ++;
		if (nibble_bit == 4) {
			buffer[p--] = hexdigit[nibble];
			nibble = 0;
			nibble_bit = 0;
		}
	}
	
	if (nibble_bit == 4) {
		buffer[p--] = hexdigit[nibble];
		nibble = 0;
		nibble_bit = 0;
	}
	
	return buffer;
}

char *
jive_bitstring_symbolic_expression(jive_context * context, jive_output * output)
{
	jive_node * node = output->node;
	if (jive_node_isinstance(node, &JIVE_BITCONSTANT_NODE))
		return bitconstant_symbolic_expression(context, (jive_bitconstant_node *) node);
	else if (jive_node_isinstance(node, &JIVE_BITSYMBOLICCONSTANT_NODE))
		return bitsymbolicconstant_symbolic_expression(context, (jive_bitsymbolicconstant_node *) node);
	else if (jive_node_isinstance(node, &JIVE_BITSUM_NODE))
		return binop_symbolic_expression(context, node, " + ");
	else if (jive_node_isinstance(node, &JIVE_BITPRODUCT_NODE))
		return binop_symbolic_expression(context, node, " * ");
	else if (jive_node_isinstance(node, &JIVE_BITAND_NODE))
		return binop_symbolic_expression(context, node, " & ");
	else if (jive_node_isinstance(node, &JIVE_BITOR_NODE))
		return binop_symbolic_expression(context, node, " | ");
	else if (jive_node_isinstance(node, &JIVE_BITXOR_NODE))
		return binop_symbolic_expression(context, node, " ^ ");
	else if (jive_node_isinstance(node, &JIVE_BITNEGATE_NODE))
		return unop_symbolic_expression(context, node, "- ");
	else if (jive_node_isinstance(node, &JIVE_BITNOT_NODE))
		return unop_symbolic_expression(context, node, "~ ");
	
	return NULL;
}

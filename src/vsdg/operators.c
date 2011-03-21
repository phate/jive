#include <jive/vsdg/operators.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static bool
jive_node_cse_test(
	jive_node * node,
	const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	if (node->class_ != cls) return false;
	if (node->ninputs != noperands) return false;
	if (!jive_node_match_attrs(node, attrs)) return false;
	size_t n;
	for(n = 0; n < noperands; n++)
		if (node->inputs[n]->origin != operands[n]) return false;
	
	return true;
}

static jive_node *
jive_node_cse(
	jive_graph * graph,
	const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	if (noperands) {
		jive_input * input;
		JIVE_LIST_ITERATE(operands[0]->users, input, output_users_list) {
			jive_node * node = input->node;
			if (jive_node_cse_test(node, cls, attrs, noperands, operands))
				return node;
		}
	} else {
		jive_node * node;
		JIVE_LIST_ITERATE(graph->top, node, graph_top_list) {
			if (jive_node_cse_test(node, cls, attrs, noperands, operands))
				return node;
		}
	}
	return NULL;
}

jive_node *
jive_nullary_operation_normalized_create(
	const jive_node_class * cls,
	struct jive_region * region,
	const jive_node_attrs * attrs)
{
	jive_node * node = jive_node_cse(region->graph, cls, attrs, 0, 0);
	if (node)
		return node;

	return cls->create(region, attrs, 0, 0);
}

jive_node *
jive_unary_operation_normalized_create(
	const jive_node_class * cls_,
	struct jive_region * region,
	const jive_node_attrs * attrs,
	jive_output * operand)
{
	/* FIXME: unconditionally performs normalization, make dependent on
	selected normal form per graph */
	const jive_unary_operation_class * cls = (const jive_unary_operation_class *) cls_;
	
	if (cls->reduce_operand(cls_, attrs, &operand))
		return operand->node;
	
	jive_node * node = jive_node_cse(region->graph, cls_, attrs, 1, &operand);
	if (node)
		return node;
	
	return cls_->create(region, attrs, 1, &operand);
}

static inline size_t
reduce_operands(const jive_node_class * cls_, const jive_node_attrs * attrs, size_t noperands, jive_output * operands[])
{
	const jive_binary_operation_class * cls = (const jive_binary_operation_class *) cls_;
	
	size_t n = 0;
	/* pair-wise reduce */
	if (cls->flags & jive_binary_operation_commutative) {
		while (n < noperands) {
			size_t k = n + 1;
			while (k < noperands) {
				jive_output * op1 = operands[n];
				jive_output * op2 = operands[k];
				if (cls->reduce_operand_pair(cls_, attrs, &op1, &op2)) {
					size_t j;
					for(j = k + 2; j < noperands; j++)
						operands[j-1] = operands[j];
					operands[n] = op1;
					noperands --;
					n --;
					break;
				}
				k = k + 1;
			}
			n = n + 1;
		}
	} else {
		while (n + 1 < noperands) {
			if (cls->reduce_operand_pair(cls_, attrs, &operands[n], &operands[n + 1])) {
				size_t k = 0;
				for(k = n + 2; k < noperands; k++)
					operands[k-1] = operands[k];
				noperands --;
				if (n > 0) n--;
			} else
				n++;
		}
	}
	
	return noperands;
}

jive_node *
jive_binary_operation_normalized_create(
	const jive_node_class * cls_,
	struct jive_region * region,
	const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands_[])
{
	/* FIXME: unconditionally performs normalization, make dependent on
	selected normal form per graph */
	const jive_binary_operation_class * cls = (const jive_binary_operation_class *) cls_;
	
	jive_output ** operands = NULL;
	
	/* expand associative */
	if (cls->flags & jive_binary_operation_associative) {
		size_t count = 0, n;
		for(n = 0; n<noperands; n++) {
			if (operands_[n]->node->class_ == cls_)
				count += operands_[n]->node->ninputs;
			else
				count ++;
		}
		
		operands = alloca(sizeof(operands[0]) * count);
		count = 0;
		for(n = 0; n<noperands; n++) {
			if (operands_[n]->node->class_ == cls_) {
				size_t k;
				for(k = 0; k<operands_[n]->node->ninputs; k++)
					operands[count++] = operands_[n]->node->inputs[k]->origin;
			} else operands[count++] = operands_[n];
		}
		noperands = count;
	} else {
		operands = alloca(sizeof(operands[0]) * noperands);
		size_t n;
		for(n = 0; n<noperands; n++)
			operands[n] = operands_[n];
	}
	
	noperands = reduce_operands(cls_, attrs, noperands, operands);
	
	if ((cls->flags & jive_binary_operation_associative) && (noperands == 1))
		return operands[0]->node;
	
	jive_node * node = jive_node_cse(region->graph, cls_, attrs, noperands, operands);
	if (node)
		return node;
	
	return cls_->create(region, attrs, noperands, operands);
}

bool
jive_unary_operation_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * operand)
{
	return false;
}

bool
jive_unary_operation_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** operand)
{
	return false;
}

bool
jive_binary_operation_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2)
{
	return false;
}

bool
jive_binary_operation_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
{
	return false;
}

const jive_node_class JIVE_NULLARY_OPERATION = {
	.parent = &JIVE_NODE,
	.fini = _jive_node_fini,
	.get_default_normal_form = _jive_node_get_default_normal_form,
	.get_label = _jive_node_get_label,
	.get_attrs = _jive_node_get_attrs,
	.match_attrs = _jive_node_match_attrs,
	.create = _jive_node_create,
	.get_aux_rescls = _jive_node_get_aux_rescls
};

const jive_unary_operation_class JIVE_UNARY_OPERATION_ = {
	.base = { /* jive_node_class */
		.parent = &JIVE_UNARY_OPERATION,
		.fini = _jive_node_fini, /* inherit */
		.get_label = _jive_node_get_label, /* inherit */
		.get_attrs = _jive_node_get_attrs, /* inherit */
		.match_attrs = _jive_node_match_attrs, /* inherit */
		.create = _jive_node_create, /* inherit */
		.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
	},
	
	.single_apply_over = NULL,
	.multi_apply_over = NULL,
	
	.can_reduce_operand = jive_unary_operation_can_reduce_operand_,
	.reduce_operand = jive_unary_operation_reduce_operand_
};

const jive_binary_operation_class JIVE_BINARY_OPERATION_ = {
	.base = { /* jive_node_class */
		.parent = &JIVE_BINARY_OPERATION,
		.fini = _jive_node_fini, /* inherit */
		.get_label = _jive_node_get_label, /* inherit */
		.get_attrs = _jive_node_get_attrs, /* inherit */
		.match_attrs = _jive_node_match_attrs, /* inherit */
		.create = _jive_node_create, /* inherit */
		.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
	},
	
	.flags = 0,
	.single_apply_under = NULL,
	.multi_apply_under = NULL,
	.distributive_over = NULL,
	.distributive_under = NULL,
	
	.can_reduce_operand_pair = jive_binary_operation_can_reduce_operand_pair_,
	.reduce_operand_pair = jive_binary_operation_reduce_operand_pair
};

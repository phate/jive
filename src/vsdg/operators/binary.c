/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/operators/binary.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace base {

binary_op::~binary_op() noexcept {}

jive_binary_operation_flags
binary_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

}
}

static inline size_t
reduce_operands(
	const jive::base::binary_op & op,
	size_t noperands,
	jive::output * operands[])
{
	size_t n = 0;
	/* pair-wise reduce */
	if (op.is_commutative()) {
		while (n < noperands) {
			size_t k = n + 1;
			while (k < noperands) {
				jive::output * op1 = operands[n];
				jive::output * op2 = operands[k];
				jive_binop_reduction_path_t reduction = op.can_reduce_operand_pair(op1, op2);
				if (reduction != jive_binop_reduction_none) {
					size_t j;
					for(j = k + 1; j < noperands; j++)
						operands[j-1] = operands[j];
					operands[n] = op.reduce_operand_pair(reduction, op1, op2);
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
			jive_binop_reduction_path_t reduction =
				op.can_reduce_operand_pair(operands[n], operands[n+1]);
			if (reduction != jive_binop_reduction_none) {
				operands[n] = op.reduce_operand_pair(reduction, operands[n], operands[n+1]);
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

/* node class */

const jive_node_class JIVE_BINARY_OPERATION = {
	parent : &JIVE_NODE,
	name : "BINARY",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* override */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

/* node class inheritable methods */

jive_node_normal_form *
jive_binary_operation_get_default_normal_form_(
	const jive_node_class * cls,
	jive_node_normal_form * parent_,
	jive_graph * graph)
{
	jive_context * context = graph->context;
	jive_binary_operation_normal_form * nf = jive_context_malloc(context, sizeof(*nf));
	
	jive_node_normal_form_init_(&nf->base, cls, parent_, graph);
	nf->base.class_ = &JIVE_BINARY_OPERATION_NORMAL_FORM;
	
	jive_binary_operation_normal_form * parent = jive_binary_operation_normal_form_cast(parent_);
	
	if (parent) {
		nf->enable_reducible = parent->enable_reducible;
		nf->enable_flatten = parent->enable_flatten;
		nf->enable_reorder = parent->enable_reorder;
		nf->enable_distribute = parent->enable_distribute;
		nf->enable_factorize = parent->enable_factorize;
	} else {
		nf->enable_reducible = true;
		nf->enable_flatten = true;
		nf->enable_reorder = true;
		nf->enable_distribute = true;
		nf->enable_factorize = true;
	}
	
	return &nf->base;
}

/* normal form class */

/* FIXME: rename to jive_binary_operation_normal_form_normalized_create_ after we removed
	the old interface
*/
void
jive_binary_operation_normalized_create_new_(const jive_node_normal_form * self_,
	jive_graph * graph, const jive::operation * base_op,
	size_t noperands_, jive::output * const operands_[], jive::output * results[])
{
	const jive::base::binary_op& op =
		*static_cast<const jive::base::binary_op *>(base_op);
	const jive_binary_operation_normal_form * self = (const jive_binary_operation_normal_form *)self_;

	jive::output ** operands = NULL;
	size_t noperands = 0;

	/* possibly expand associative */
	if (self->base.enable_mutable && self->enable_flatten && op.is_associative()) {
		size_t count = 0, n;
		for (n = 0; n < noperands_; n++) {
			if (operands_[n]->node()->class_ == self_->node_class)
				count += operands_[n]->node()->noperands;
			else
				count ++;
		}

		operands = alloca(sizeof(operands[0]) * count);
		count = 0;
		for (n = 0; n < noperands_; n++) {
			if (operands_[n]->node()->class_ == self_->node_class) {
				size_t k;
				for(k = 0; k < operands_[n]->node()->noperands; k++)
					operands[count++] = operands_[n]->node()->inputs[k]->origin();
			} else operands[count++] = operands_[n];
		}
		noperands = count;
	} else {
		operands = alloca(sizeof(operands[0]) * noperands_);
		size_t n;
		for (n = 0; n < noperands_; n++)
			operands[n] = operands_[n];
		noperands = noperands_;
	}

	if (self->base.enable_mutable && self->enable_reducible) {
		noperands = reduce_operands(op, noperands, operands);

		if (noperands == 1){
			results[0] = operands[0];
			return;
		}
	}

	/* FIXME: reorder for commutative operation */

	/* FIXME: attempt distributive transform */

	return jive_node_normal_form_normalized_create_(self_, graph, &op, noperands, operands,
		results);
}

const jive_binary_operation_normal_form_class JIVE_BINARY_OPERATION_NORMAL_FORM_ = {
	base : {
		parent : &JIVE_NODE_NORMAL_FORM,
		fini : jive_node_normal_form_fini_, /* inherit */
		normalize_node : jive_binary_operation_normalize_node_, /* override */
		operands_are_normalized : jive_binary_operation_operands_are_normalized_, /* inherit */
		normalized_create : jive_binary_operation_normalized_create_new_, /* override */
		set_mutable : jive_node_normal_form_set_mutable_, /* inherit */
		set_cse : jive_node_normal_form_set_cse_ /* inherit */
	},
	set_reducible : jive_binary_operation_set_reducible_,
	set_flatten : jive_binary_operation_set_flatten_,
	set_reorder : jive_binary_operation_set_reorder_,
	set_distribute : jive_binary_operation_set_distribute_,
	set_factorize : jive_binary_operation_set_factorize_,
	normalized_create : nullptr
};

/* normal form inheritable methods */

bool
jive_binary_operation_normalize_node_(const jive_node_normal_form * self_, jive_node * node)
{
	const jive_binary_operation_normal_form * self = (const jive_binary_operation_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;
	
	const jive::operation & base_op = node->operation();
	const jive::base::binary_op & op = *static_cast<const jive::base::binary_op *>(&base_op);
	
	jive::output ** operands = NULL;
	size_t noperands = 0;
	
	/* possibly expand associative */
	if (self->enable_flatten && op.is_associative()) {
		size_t count = 0, n;
		for (n = 0; n < node->noperands; n++) {
			if (node->producer(n)->class_ == self_->node_class)
				count += node->producer(n)->noperands;
			else
				count ++;
		}
		
		operands = alloca(sizeof(operands[0]) * count);
		count = 0;
		for (n = 0; n < node->noperands; n++) {
			if (node->producer(n)->class_ == self_->node_class) {
				size_t k;
				for(k = 0; k < node->producer(n)->noperands; k++)
					operands[count++] = node->producer(n)->inputs[k]->origin();
			} else operands[count++] = node->inputs[n]->origin();
		}
		noperands = count;
	} else {
		operands = alloca(sizeof(operands[0]) * node->noperands);
		size_t n;
		for (n = 0; n < node->noperands; n++)
			operands[n] = node->inputs[n]->origin();
		noperands = node->noperands;
	}
	
	if (self->enable_reducible) {
		noperands = reduce_operands(op, noperands, operands);
		
		if (noperands == 1) {
			jive_output_replace(node->outputs[0], operands[0]);
			/* FIXME: not sure whether "destroy" is really appropriate? */
			jive_node_destroy(node);
			return false;
		}
	}
	
	/* FIXME: reorder for commutative operation */
	
	/* FIXME: attempt distributive transform */
	
	bool changes = false;
	if (noperands != node->noperands) {
		changes = true;
	} else {
		size_t n;
		for (n = 0; n < node->noperands; n++)
			if (node->inputs[n]->origin() != operands[n])
				changes = true;
	}
	
	if (changes) {
		jive_node * new_node = 0;
		
		if (self->base.enable_cse)
			jive_node_cse(node->region, self_->node_class, &op, noperands, operands);
		
		if (!new_node) {
			new_node = jive_node_create(self_->node_class, op, node->region, noperands, operands);
		}
		
		if (new_node != node) {
			jive_output_replace(node->outputs[0], new_node->outputs[0]);
			jive_node_destroy(node);
			return false;
		}
	}
	
	return true;
}

bool
jive_binary_operation_operands_are_normalized_(
	const jive_node_normal_form * self_,
	size_t noperands,
	jive::output * const operands[],
	const jive::operation * base_op)
{
	const jive::base::binary_op& op = *static_cast<const jive::base::binary_op*>(base_op);
	const jive_binary_operation_normal_form * self = (const jive_binary_operation_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;
	
	/* possibly expand associative */
	if (self->enable_flatten && op.is_associative()) {
		size_t n;
		for (n = 0; n < noperands; n++) {
			if (operands[n]->node()->class_ == self_->node_class)
				return false;
		}
	}
	
	if (self->enable_reducible) {
		jive::output * tmp_operands[noperands];
		size_t n;
		for (n = 0; n < noperands; n++)
			tmp_operands[n] = operands[n];
		
		size_t new_noperands = reduce_operands(op, noperands, tmp_operands);
		
		if (new_noperands != noperands)
			return false;
		if (noperands == 1)
			return false;
	}
	
	/* FIXME: reorder for commutative operation */
	
	/* FIXME: attempt distributive transform */
	
	return true;
}

void
jive_binary_operation_set_reducible_(jive_binary_operation_normal_form * self_, bool enable)
{
	jive_binary_operation_normal_form * self = (jive_binary_operation_normal_form *) self_;
	if (self->enable_reducible == enable)
		return;
	
	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->base.subclasses, child, normal_form_subclass_list) {
		jive_binary_operation_set_reducible((jive_binary_operation_normal_form *)child, enable);
	}
	
	self->enable_reducible = enable;
	if (self->base.enable_mutable && self->enable_reducible)
		jive_graph_mark_denormalized(self->base.graph);
}

void
jive_binary_operation_set_flatten_(jive_binary_operation_normal_form * self_, bool enable)
{
	jive_binary_operation_normal_form * self = (jive_binary_operation_normal_form *) self_;
	if (self->enable_flatten == enable)
		return;
	
	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->base.subclasses, child, normal_form_subclass_list) {
		jive_binary_operation_set_flatten((jive_binary_operation_normal_form *)child, enable);
	}
	
	self->enable_flatten = enable;
	if (self->base.enable_mutable && self->enable_flatten)
		jive_graph_mark_denormalized(self->base.graph);
}

void
jive_binary_operation_set_reorder_(jive_binary_operation_normal_form * self_, bool enable)
{
	jive_binary_operation_normal_form * self = (jive_binary_operation_normal_form *) self_;
	if (self->enable_reorder == enable)
		return;
	
	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->base.subclasses, child, normal_form_subclass_list) {
		jive_binary_operation_set_reorder((jive_binary_operation_normal_form *)child, enable);
	}
	
	self->enable_reorder = enable;
	if (self->base.enable_mutable && self->enable_reorder)
		jive_graph_mark_denormalized(self->base.graph);
}

void
jive_binary_operation_set_distribute_(jive_binary_operation_normal_form * self_, bool enable)
{
	jive_binary_operation_normal_form * self = (jive_binary_operation_normal_form *) self_;
	if (self->enable_distribute == enable)
		return;
	
	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->base.subclasses, child, normal_form_subclass_list) {
		jive_binary_operation_set_distribute((jive_binary_operation_normal_form *)child, enable);
	}
	
	self->enable_distribute = enable;
	if (self->base.enable_mutable && self->enable_distribute)
		jive_graph_mark_denormalized(self->base.graph);
}

void
jive_binary_operation_set_factorize_(jive_binary_operation_normal_form * self_, bool enable)
{
	jive_binary_operation_normal_form * self = (jive_binary_operation_normal_form *) self_;
	if (self->enable_factorize == enable)
		return;
	
	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->base.subclasses, child, normal_form_subclass_list) {
		jive_binary_operation_set_factorize((jive_binary_operation_normal_form *)child, enable);
	}
	
	self->enable_factorize = enable;
	if (self->base.enable_mutable && self->enable_factorize)
		jive_graph_mark_denormalized(self->base.graph);
}

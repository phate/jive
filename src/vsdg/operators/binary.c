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

binary_operation::~binary_operation() noexcept {}

jive_binop_reduction_path_t
binary_operation::can_reduce_operand_pair(
	const jive_output * op1,
	const jive_output * op2) const noexcept
{
	return jive_binop_reduction_none;
}

jive_output *
binary_operation::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive_output * op1,
	jive_output * op2) const
{
	return nullptr;
}

jive_binary_operation_flags
binary_operation::flags() const noexcept
{
	return jive_binary_operation_none;
}

}

static inline jive_binop_reduction_path_t
can_reduce_operand_pair(
	const jive_binary_operation_class * cls,
	const jive_node_attrs * attrs,
	jive_output * arg1,
	jive_output * arg2)
{
	if (cls->can_reduce_operand_pair) {
		return cls->can_reduce_operand_pair(&cls->base, attrs, arg1, arg2);
	} else {
		const jive::binary_operation * op =
			static_cast<const jive::binary_operation *>(attrs);
		return op->can_reduce_operand_pair(arg1, arg2);
	}
}

static inline jive_output *
reduce_operand_pair(
	const jive_binary_operation_class * cls,
	const jive_node_attrs * attrs,
	jive_binop_reduction_path_t path,
	jive_output * arg1,
	jive_output * arg2)
{
	if (cls->reduce_operand_pair) {
		return cls->reduce_operand_pair(path, &cls->base, attrs, arg1, arg2);
	} else {
		const jive::binary_operation * op =
			static_cast<const jive::binary_operation *>(attrs);
		return op->reduce_operand_pair(path, arg1, arg2);
	}
}

static inline size_t
reduce_operands(
	const jive_node_class * cls_,
	const jive_node_attrs * attrs,
	size_t noperands,
	jive_output * operands[])
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
				jive_binop_reduction_path_t reduction = can_reduce_operand_pair(cls, attrs, op1, op2);
				if (reduction != jive_binop_reduction_none) {
					size_t j;
					for(j = k + 1; j < noperands; j++)
						operands[j-1] = operands[j];
					operands[n] = reduce_operand_pair(cls, attrs, reduction, op1, op2);
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
				can_reduce_operand_pair(cls, attrs, operands[n], operands[n+1]);
			if (reduction != jive_binop_reduction_none) {
				operands[n] = reduce_operand_pair(cls, attrs, reduction, operands[n], operands[n+1]);
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

const jive_binary_operation_class JIVE_BINARY_OPERATION_ = {
	base : { /* jive_node_class */
		parent : &JIVE_NODE,
		name : "BINARY",
		fini : jive_node_fini_, /* inherit */
		get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* override */
		get_label : jive_node_get_label_, /* inherit */
		match_attrs : jive_node_match_attrs_, /* inherit */
		check_operands : NULL,
		create : nullptr,
	},
	
	flags : 0,
	single_apply_under : NULL,
	multi_apply_under : NULL,
	distributive_over : NULL,
	distributive_under : NULL,
	
	can_reduce_operand_pair : jive_binary_operation_can_reduce_operand_pair_,
	reduce_operand_pair : jive_binary_operation_reduce_operand_pair_
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

jive_binop_reduction_path_t
jive_binary_operation_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2)
{
	return jive_binop_reduction_none;
}

jive_output *
jive_binary_operation_reduce_operand_pair_(
	jive_binop_reduction_path_t path,
	const jive_node_class * cls,
	const jive_node_attrs * attrs,
	jive_output * op1,
	jive_output * op2)
{
	return NULL;
}

/* normal form class */

/* FIXME: rename to jive_binary_operation_normal_form_normalized_create_ after we removed
	the old interface
*/
void
jive_binary_operation_normalized_create_new_(const jive_node_normal_form * self_,
	jive_graph * graph, const jive_node_attrs * attrs,
	size_t noperands_, jive_output * const operands_[], jive_output * results[])
{
	const jive_binary_operation_normal_form * self = (const jive_binary_operation_normal_form *)self_;
	const jive_binary_operation_class * cls = (const jive_binary_operation_class *) self_->node_class;

	jive_output ** operands = NULL;
	size_t noperands = 0;

	/* possibly expand associative */
	if (self->base.enable_mutable && self->enable_flatten &&
		(cls->flags & jive_binary_operation_associative)) {
		size_t count = 0, n;
		for (n = 0; n < noperands_; n++) {
			if (operands_[n]->node->class_ == &cls->base)
				count += operands_[n]->node->noperands;
			else
				count ++;
		}

		operands = alloca(sizeof(operands[0]) * count);
		count = 0;
		for (n = 0; n < noperands_; n++) {
			if (operands_[n]->node->class_ == &cls->base) {
				size_t k;
				for(k = 0; k < operands_[n]->node->noperands; k++)
					operands[count++] = operands_[n]->node->inputs[k]->origin();
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
		noperands = reduce_operands(&cls->base, attrs, noperands, operands);

		if (noperands == 1){
			results[0] = operands[0];
			return;
		}
	}

	/* FIXME: reorder for commutative operation */

	/* FIXME: attempt distributive transform */

	return jive_node_normal_form_normalized_create_(self_, graph, attrs, noperands, operands,
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
	normalized_create : jive_binary_operation_normalized_create_
};

jive_output *
jive_binary_operation_normalized_create(
	const jive_node_class * cls_,
	struct jive_region * region,
	const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands_[])
{
	jive_binary_operation_normal_form * nf = (jive_binary_operation_normal_form *)
		jive_graph_get_nodeclass_form(region->graph, cls_);
	/* FIXME: unconditionally performs normalization, make dependent on
	selected normal form per graph */
	const jive_binary_operation_class * cls = (const jive_binary_operation_class *) cls_;
	
	jive_output ** operands = NULL;
	
	bool expand_associative =
		nf->base.enable_mutable &&
		nf->enable_flatten &&
		(cls->flags & jive_binary_operation_associative) != 0;
	
	/* expand associative */
	if (expand_associative) {
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
					operands[count++] = operands_[n]->node->inputs[k]->origin();
			} else operands[count++] = operands_[n];
		}
		noperands = count;
	} else {
		operands = alloca(sizeof(operands[0]) * noperands);
		size_t n;
		for(n = 0; n<noperands; n++)
			operands[n] = operands_[n];
	}
	
	if (nf->base.enable_mutable && nf->enable_reducible)
		noperands = reduce_operands(cls_, attrs, noperands, operands);
	
	if (noperands == 1)
		return operands[0];
	
	jive_node * node = 0;
	if (nf->base.enable_mutable && nf->base.enable_cse)
		node = jive_node_cse(region, cls_, attrs, noperands, operands);
	if (node)
		return node->outputs[0];
	
	return jive_node_create(cls_, *attrs, region, noperands, operands)->outputs[0];
}


/* normal form inheritable methods */

bool
jive_binary_operation_normalize_node_(const jive_node_normal_form * self_, jive_node * node)
{
	const jive_binary_operation_normal_form * self = (const jive_binary_operation_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;
	
	const jive_node_attrs * attrs = jive_node_get_attrs(node);
	
	const jive_binary_operation_class * cls =
		(const jive_binary_operation_class *) self->base.node_class;
	
	jive_output ** operands = NULL;
	size_t noperands = 0;
	
	/* possibly expand associative */
	if (self->enable_flatten && (cls->flags & jive_binary_operation_associative)) {
		size_t count = 0, n;
		for (n = 0; n < node->noperands; n++) {
			if (node->inputs[n]->origin()->node->class_ == &cls->base)
				count += node->inputs[n]->origin()->node->noperands;
			else
				count ++;
		}
		
		operands = alloca(sizeof(operands[0]) * count);
		count = 0;
		for (n = 0; n < node->noperands; n++) {
			if (node->inputs[n]->origin()->node->class_ == &cls->base) {
				size_t k;
				for(k = 0; k < node->inputs[n]->origin()->node->noperands; k++)
					operands[count++] = node->inputs[n]->origin()->node->inputs[k]->origin();
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
		noperands = reduce_operands(&cls->base, attrs, noperands, operands);
		
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
			jive_node_cse(node->region, &cls->base, attrs, noperands, operands);
		
		if (!new_node) {
			new_node = jive_node_create(&cls->base, *attrs, node->region, noperands, operands);
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
	jive_output * const operands[],
	const jive_node_attrs * attrs)
{
	const jive_binary_operation_normal_form * self = (const jive_binary_operation_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;
	
	const jive_binary_operation_class * cls =
		(const jive_binary_operation_class *) self->base.node_class;
	
	/* possibly expand associative */
	if (self->enable_flatten && (cls->flags & jive_binary_operation_associative)) {
		size_t n;
		for (n = 0; n < noperands; n++) {
			if (operands[n]->node->class_ == &cls->base)
				return false;
		}
	}
	
	if (self->enable_reducible) {
		jive_output * tmp_operands[noperands];
		size_t n;
		for (n = 0; n < noperands; n++)
			tmp_operands[n] = operands[n];
		
		size_t new_noperands = reduce_operands(&cls->base, attrs, noperands, tmp_operands);
		
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

jive_output *
jive_binary_operation_normalized_create_(
	const jive_binary_operation_normal_form * self,
	struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands_, jive_output * const operands_[])
{
	const jive_binary_operation_class * cls =
		(const jive_binary_operation_class *) self->base.node_class;
	
	jive_output ** operands = NULL;
	size_t noperands = 0;
	
	/* possibly expand associative */
	bool associative = (cls->flags & jive_binary_operation_associative);
	if (self->base.enable_mutable && self->enable_flatten && associative) {
		size_t count = 0, n;
		for (n = 0; n < noperands_; n++) {
			if (operands_[n]->node->class_ == &cls->base)
				count += operands_[n]->node->noperands;
			else
				count ++;
		}
		
		operands = alloca(sizeof(operands[0]) * count);
		count = 0;
		for (n = 0; n < noperands_; n++) {
			if (operands_[n]->node->class_ == &cls->base) {
				size_t k;
				for(k = 0; k < operands_[n]->node->noperands; k++)
					operands[count++] = operands_[n]->node->inputs[k]->origin();
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
		noperands = reduce_operands(&cls->base, attrs, noperands, operands);
		
		if (noperands == 1)
			return operands[0];
	}
	
	/* FIXME: reorder for commutative operation */
	
	/* FIXME: attempt distributive transform */
	
	if (self->base.enable_mutable && self->base.enable_cse) {
		jive_node * new_node = jive_node_cse(region, &cls->base, attrs, noperands, operands);
		if (new_node) {
			return new_node->outputs[0];
		}
	}
	
	return cls->base.create(region, attrs, noperands, operands)->outputs[0];
}

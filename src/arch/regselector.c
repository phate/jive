#include <jive/arch/regselector.h>

#include <stdint.h>

#include <jive/arch/regvalue.h>
#include <jive/common.h>
#include <jive/context.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/traverser.h>
#include <jive/types/bitstring/arithmetic.h>

typedef struct jive_regselector_option jive_regselector_option;

struct jive_regselector_option {
	jive_negotiator_option base;
	uint32_t mask;
};

static void
jive_regselector_option_fini_(const jive_negotiator * self, jive_negotiator_option * option)
{
}

static jive_negotiator_option *
jive_regselector_option_create_(const jive_negotiator * self)
{
	jive_regselector_option * option = jive_context_malloc(self->context, sizeof(*option));
	option->mask = 0;
	return &option->base;
}

static bool
jive_regselector_option_equals_(const jive_negotiator * self, const jive_negotiator_option * o1_, const jive_negotiator_option * o2_)
{
	const jive_regselector_option * o1 = (const jive_regselector_option *) o1_;
	const jive_regselector_option * o2 = (const jive_regselector_option *) o2_;
	return o1->mask == o2->mask;
}

static bool
jive_regselector_option_specialize_(const jive_negotiator * self, jive_negotiator_option * option_)
{
	jive_regselector_option * option = (jive_regselector_option *) option_;
	
	JIVE_DEBUG_ASSERT(option->mask < 65535);
	/* if this is a power of two already, nothing to do */
	if ( (option->mask & (option->mask -1)) == 0 )
		return false;
	
	uint32_t x = 0x80000000;
	while ( !(x & option->mask) )
		x >>= 1;
	option->mask = x;
	return true;
}

static bool
jive_regselector_option_intersect_(const jive_negotiator * self, jive_negotiator_option * dst_, const jive_negotiator_option * src_)
{
	jive_regselector_option * dst = (jive_regselector_option *) dst_;
	const jive_regselector_option * src = (const jive_regselector_option *) src_;
	
	uint32_t tmp = dst->mask & src->mask;
	if (tmp == 0)
		return false;
	dst->mask = tmp;
	return true;
}

static bool
jive_regselector_option_assign_(const jive_negotiator * self, jive_negotiator_option * dst_, const jive_negotiator_option * src_)
{
	jive_regselector_option * dst = (jive_regselector_option *) dst_;
	const jive_regselector_option * src = (const jive_regselector_option *) src_;
	
	if (src->mask == dst->mask)
		return false;
	dst->mask = src->mask;
	return true;
}

static uint32_t
jive_regselector_classify_regcls(const jive_regselector * self, const jive_register_class * regcls)
{
	uint32_t option_mask = 0;
	size_t n = 0;
	for (n = 0; n < self->classifier->nclasses; ++n) {
		if (regcls == self->classifier->classes[n])
			option_mask |= (1 << n);
	}
	return option_mask;
}

static void
jive_regselector_annotate_node_proper_(jive_negotiator * self_, jive_node * node)
{
	jive_regselector * self = (jive_regselector *) self_;
	
	if (node->class_ == &JIVE_REGVALUE_NODE) {
		const jive_register_class * regcls = ((jive_regvalue_node *) node)->attrs.regcls;
		jive_regselector_option option;
		option.mask = jive_regselector_classify_regcls(self, regcls);
		jive_negotiator_annotate_simple_output(&self->base, node->outputs[0], &option.base);
	} else if (jive_node_isinstance(node, &JIVE_BITBINARY_NODE) || (jive_node_isinstance(node, &JIVE_BITUNARY_NODE))) {
		jive_regselector_option option;
		const jive_bitstring_type * type;
		type = (const jive_bitstring_type *) jive_output_get_type(node->outputs[0]);
		
		const jive_bitbinary_operation_class * cls;
		cls = (const jive_bitbinary_operation_class *) node->class_;
		
		option.mask = jive_reg_classifier_classify_fixed_arithmetic(self->classifier, cls->type, type->nbits);
		
		jive_negotiator_annotate_identity_node(&self->base, node, &option.base);
	}
}

static bool
jive_regselector_option_gate_default_(const jive_negotiator * self_, jive_negotiator_option * dst, const jive_gate * gate)
{
	jive_regselector * self = (jive_regselector *) self_;
	jive_regselector_option * option = (jive_regselector_option *) dst;
	option->mask = jive_reg_classifier_classify_type(self->classifier, jive_gate_get_type(gate), gate->required_rescls);
	return !!option->mask;
}

static const jive_negotiator_class JIVE_REGSELECTOR_CLASS = {
	.option_fini = jive_regselector_option_fini_,
	.option_create = jive_regselector_option_create_,
	.option_equals = jive_regselector_option_equals_,
	.option_specialize = jive_regselector_option_specialize_,
	.option_intersect = jive_regselector_option_intersect_,
	.option_assign = jive_regselector_option_assign_,
	.option_gate_default = jive_regselector_option_gate_default_,
	.annotate_node_proper = jive_regselector_annotate_node_proper_,
	.annotate_node = jive_negotiator_annotate_node_,
	.process_region = jive_negotiator_process_region_
};

static const jive_register_class *
jive_regselector_map_port(const jive_regselector * self, const jive_negotiator_port * port)
{
	if (!port) return NULL;
	const jive_regselector_option * option;
	option = (const jive_regselector_option *) port->option;
	if (!option->mask) return NULL;
	
	jive_regselect_index index = 0;
	while (! (option->mask & (1 << index) ) )
		index ++;
	return self->classifier->classes[index];
}

static void
jive_regselector_pull_node(jive_regselector * self, jive_node * node)
{
	jive_region * root_region = self->base.graph->root_region;
	
	if (node->region == root_region)
		return;
	
	/* determine function region */
	jive_region * region = node->region;
	while (region->parent != root_region)
		region = region->parent;
	jive_node * top = region->top;
	if (!top || top->noutputs < 1)
		return;
	jive_output * ctl = top->outputs[0];
	if (ctl->class_ != &JIVE_CONTROL_OUTPUT)
		return;
	
	if (node->class_ == &JIVE_REGVALUE_NODE) {
		const jive_register_class * regcls = ((jive_regvalue_node *) node)->attrs.regcls;
		jive_node * origin = node->inputs[1]->origin->node;
		JIVE_DEBUG_ASSERT(origin->region == root_region);
		
		if (jive_node_isinstance(origin, &JIVE_BITBINARY_NODE)) {
			jive_output * operands[origin->noperands];
			size_t n;
			for (n = 0; n < origin->noperands; n++) {
				jive_output * operand = origin->inputs[n]->origin;
				jive_output * regvalue = jive_regvalue(ctl, regcls, operand);
				jive_negotiator_port * reg_port = jive_negotiator_map_output(&self->base, regvalue);
				if (!reg_port)
					self->base.class_->annotate_node(&self->base, regvalue->node);
				operands[n] = regvalue;
			}
			
			jive_output * subst = jive_binary_operation_normalized_create(
				origin->class_, region, jive_node_get_attrs(origin),
				origin->noperands, operands);
			
			jive_negotiator_port_split(jive_negotiator_map_output(&self->base, node->outputs[0]));
			jive_output_replace(node->outputs[0], subst);
			
			if (!jive_negotiator_map_output(&self->base, subst))
				self->base.class_->annotate_node(&self->base, subst->node);
			jive_negotiator_fully_specialize(&self->base);
		} else if (jive_node_isinstance(origin, &JIVE_BITUNARY_NODE)) {
			jive_output * operands[origin->noperands];
			size_t n;
			for (n = 0; n < origin->noperands; n++) {
				jive_output * operand = origin->inputs[n]->origin;
				jive_output * regvalue = jive_regvalue(ctl, regcls, operand);
				jive_negotiator_port * reg_port = jive_negotiator_map_output(&self->base, regvalue);
				if (!reg_port)
					self->base.class_->annotate_node(&self->base, regvalue->node);
				operands[n] = regvalue;
			}
			
			const jive_unary_operation_normal_form * nf =
				(const jive_unary_operation_normal_form *)
				jive_graph_get_nodeclass_form(self->base.graph, origin->class_);

			jive_output * subst = jive_unary_operation_normalized_create(nf,
				region, jive_node_get_attrs(origin), operands[0]);
			
			jive_negotiator_port_split(jive_negotiator_map_output(&self->base, node->outputs[0]));
			jive_output_replace(node->outputs[0], subst);
			
			if (!jive_negotiator_map_output(&self->base, subst))
				self->base.class_->annotate_node(&self->base, subst->node);
			jive_negotiator_fully_specialize(&self->base);
		}
		return;
	}
	
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		if (input->origin->node->region != root_region)
			continue;
		jive_negotiator_port * port = jive_negotiator_map_input(&self->base, input);
		if (!port)
			continue;
		const jive_register_class * regcls = jive_regselector_map_port(self, port);
		if (!regcls)
			continue;
		
		jive_output * regvalue = jive_regvalue(ctl, regcls, input->origin);
		jive_negotiator_port * reg_port = jive_negotiator_map_output(&self->base, regvalue);
		if (!reg_port) {
			self->base.class_->annotate_node(&self->base, regvalue->node);
			reg_port = jive_negotiator_map_output(&self->base, regvalue);
		}
		
		jive_negotiator_port_divert(port, reg_port->connection);
		jive_input_divert_origin(input, regvalue);
		jive_negotiator_fully_specialize(&self->base);
	}
}

void
jive_regselector_pull(jive_regselector * self)
{
	jive_traverser * trav;
	trav = jive_bottomup_revisit_traverser_create(self->base.graph);
	
	jive_node * node = jive_traverser_next(trav);
	for (; node; node = jive_traverser_next(trav)) {
		jive_regselector_pull_node(self, node);
	}
	
	jive_traverser_destroy(trav);
}

void
jive_regselector_init(jive_regselector * self, struct jive_graph * graph, const jive_reg_classifier * classifier)
{
	jive_negotiator_init_(&self->base, &JIVE_REGSELECTOR_CLASS, graph);
	self->classifier = classifier;
}

void
jive_regselector_process(jive_regselector * self)
{
	jive_negotiator_process(&self->base);
	jive_negotiator_insert_split_nodes(&self->base);
	jive_regselector_pull(self);
	jive_negotiator_process(&self->base);
	jive_negotiator_remove_split_nodes(&self->base);
}

const jive_register_class *
jive_regselector_map_output(const jive_regselector * self, jive_output * output)
{
	jive_negotiator_port * port = jive_negotiator_map_output(&self->base, output);
	return jive_regselector_map_port(self, port);
}

const jive_register_class *
jive_regselector_map_input(const jive_regselector * self, jive_input * input)
{
	jive_negotiator_port * port = jive_negotiator_map_input(&self->base, input);
	return jive_regselector_map_port(self, port);
}

void
jive_regselector_fini(jive_regselector * self)
{
	jive_negotiator_fini_(&self->base);
}


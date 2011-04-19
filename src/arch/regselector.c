#include <jive/arch/regselector.h>

#include <stdint.h>

#include <jive/common.h>

#include <jive/context.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/node.h>
#include <jive/bitstring/arithmetic.h>

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
	if (tmp == dst->mask)
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

static void
jive_regselector_annotate_node_proper_(jive_negotiator * self_, jive_node * node)
{
	jive_regselector * self = (jive_regselector *) self_;
	if (jive_node_isinstance(node, &JIVE_BITBINARY_NODE)) {
		jive_regselector_option option;
		const jive_bitstring_type * type;
		type = (const jive_bitstring_type *) jive_output_get_type(node->outputs[0]);
		
		const jive_bitbinary_operation_class * cls;
		cls = (const jive_bitbinary_operation_class *) node->class_;
		
		option.mask = self->classifier->classify_fixed_arithmetic(cls->type, type->nbits);
		
		jive_negotiator_annotate_identity_node(&self->base, node, &option.base);
	}
}

static bool
jive_regselector_option_gate_default_(const jive_negotiator * self_, jive_negotiator_option * dst, const jive_gate * gate)
{
	jive_regselector * self = (jive_regselector *) self_;
	jive_regselector_option * option = (jive_regselector_option *) dst;
	option->mask = self->classifier->classify_type(jive_gate_get_type(gate), gate->required_rescls);
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
}

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


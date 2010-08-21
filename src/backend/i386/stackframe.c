#include <jive/backend/i386/stackframe.h>
#include <jive/arch/stackframe-private.h>
#include <jive/arch/instruction.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/graph.h>

static inline void
_jive_i386_stackframe_init(jive_i386_stackframe * self, jive_region * region, struct jive_output * stackptr)
{
	_jive_stackframe_init(&self->base, region, stackptr);
	self->size = 0;
}

static void
_jive_i386_stackframe_layout(jive_stackframe * self_)
{
	jive_i386_stackframe * self = (jive_i386_stackframe *) self_;
	
	jive_stackslot_resource * slot;
	JIVE_LIST_ITERATE(self->base.slots, slot, stackframe_slots_list) {
		self->size += 4;
		slot->offset = -self->size;
		
		jive_input * input;
		JIVE_LIST_ITERATE(slot->base.base.inputs, input, resource_input_list) {
			jive_instruction_node * node = (jive_instruction_node *) input->node;
			node->immediates[0] = slot->offset;
		}
		
		jive_output * output;
		JIVE_LIST_ITERATE(slot->base.base.outputs, output, resource_output_list) {
			jive_instruction_node * node = (jive_instruction_node *) output->node;
			node->immediates[0] = slot->offset;
		}
	}
	
	/* TODO: move stack pointer */
}

const jive_stackframe_class JIVE_I386_STACKFRAME_CLASS = {
	.fini = _jive_stackframe_fini,
	.layout = _jive_i386_stackframe_layout
};

jive_stackframe *
jive_i386_stackframe_create(jive_region * region, jive_output * stackptr)
{
	jive_i386_stackframe * stackframe = jive_context_malloc(region->graph->context, sizeof(*stackframe));
	_jive_i386_stackframe_init(stackframe, region, stackptr);
	stackframe->base.class_ = &JIVE_I386_STACKFRAME_CLASS;
	
	return &stackframe->base;
}

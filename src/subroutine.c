#include <jive/machine.h>
#include <jive/internal/subroutinestr.h>

/** \brief Get entry node of subroutine */
jive_node *
jive_subroutine_enter(jive_subroutine * sub)
{
	return sub->enter;
}

/** \brief Get exit node of subroutine */
jive_node *
jive_subroutine_leave(jive_subroutine * sub)
{
	return sub->leave;
}

/** \brief Get stackframe associated with subroutine */
jive_stackframe *
jive_subroutine_stackframe(jive_subroutine * sub)
{
	return sub->stackframe;
}

/** \brief Get parameter value */
jive_value *
jive_subroutine_parameter(jive_subroutine * sub, size_t index)
{
	return sub->vmt->get_parameter(sub, index);
}

/** \brief Set return value of subroutine */
void
jive_subroutine_return_value(jive_subroutine * sub, jive_value * value)
{
	sub->vmt->return_value(sub, value);
}

jive_stackslot *
jive_stackframe_allocate_slot(jive_stackframe * frame, jive_cpureg_class_t regcls)
{
	return frame->vmt->allocate_slot(frame, regcls);
}

static jive_stackslot *
jive_def_stackframe_allocate_slot(jive_stackframe * frame, jive_cpureg_class_t regcls)
{
	jive_stackslot * slot = jive_malloc(frame->graph, sizeof(*slot));
	
	slot->size = regcls->nbits/8;
	
	slot->prev = frame->last;
	slot->next = 0;
	if (frame->last) frame->last->next = slot;
	else frame->first =slot;
	frame->last = slot;
	
	return slot;
}

static const jive_stackframe_vmt jive_def_stackframe_vmt = {
	.allocate_slot = &jive_def_stackframe_allocate_slot,
	.finalize = 0
};

jive_stackframe *
jive_default_stackframe_create(jive_graph * graph)
{
	jive_stackframe * frame = jive_malloc(graph, sizeof(*frame));
	frame->vmt = &jive_def_stackframe_vmt;
	frame->graph = graph;
	frame->first = frame->last = 0;
	
	return frame;
}

void
jive_stackframe_finalize(jive_stackframe * frame)
{
	frame->vmt->finalize(frame);
}

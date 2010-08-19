#include <jive/context.h>
#include <jive/arch/stackframe-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/basetype.h>

void
_jive_stackframe_fini(jive_stackframe * self)
{
}

const jive_stackframe_class JIVE_STACKFRAME_CLASS = {
	.parent = 0,
	.fini = _jive_stackframe_fini
};

jive_stackframe *
jive_stackframe_create(jive_region * region, jive_output * stackptr)
{
	jive_stackframe * stackframe = jive_context_malloc(region->graph->context, sizeof(*stackframe));
	_jive_stackframe_init(stackframe, region, stackptr);
	stackframe->class_ = &JIVE_STACKFRAME_CLASS;
	return stackframe;
}

void
jive_stackframe_destroy(jive_stackframe * self)
{
	self->class_->fini(self);
	jive_context_free(self->region->graph->context, self);
}

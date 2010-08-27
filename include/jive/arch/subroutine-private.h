#ifndef JIVE_ARCH_SUBROUTINE_PRIVATE_H
#define JIVE_ARCH_SUBROUTINE_PRIVATE_H

#include <jive/arch/subroutine.h>
#include <jive/vsdg/controltype.h>

void
_jive_subroutine_node_init(
	jive_subroutine_node * self,
	jive_subroutine * subroutine);

void
_jive_subroutine_node_fini(jive_node * self);

char *
_jive_subroutine_node_get_label(const jive_node * self);

jive_node *
_jive_subroutine_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * operands[]);

bool
_jive_subroutine_node_equiv(const jive_node_attrs * first, const jive_node_attrs * second);

static inline void
_jive_subroutine_init(jive_subroutine * self, jive_node * enter, jive_node * leave)
{
	self->enter = enter;
	self->leave = leave;
	self->region = enter->region;
	
	self->subroutine_node = jive_context_malloc(enter->graph->context, sizeof(*self->subroutine_node));
	_jive_subroutine_node_init(self->subroutine_node, self);
	
	JIVE_DECLARE_CONTROL_TYPE(type);
	
	jive_output * output = jive_node_add_output(leave, type);
	jive_node_add_input(&self->subroutine_node->base, type, output);
}

#endif

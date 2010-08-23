#include <jive/arch/subroutine-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <string.h>

const jive_node_class JIVE_SUBROUTINE_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_subroutine_node_fini, /* override */
	.get_label = _jive_subroutine_node_get_label, /* override */
	.copy = _jive_subroutine_node_copy, /* override */
	.equiv = _jive_subroutine_node_equiv, /* override */
	.get_aux_regcls = _jive_node_get_aux_regcls, /* inherit */
};

void
_jive_subroutine_node_init(
	jive_subroutine_node * self,
	jive_subroutine * subroutine)
{
	self->base.class_ = &JIVE_SUBROUTINE_NODE;
	_jive_node_init(&self->base, subroutine->enter->region->parent,
		0, NULL, NULL,
		0, NULL);
	self->subroutine = subroutine;
}

void
_jive_subroutine_node_fini(jive_node * self_)
{
	jive_subroutine_node * self = (jive_subroutine_node *) self_;
	jive_subroutine_destroy(self->subroutine);
	_jive_node_fini(&self->base);
}

char *
_jive_subroutine_node_get_label(const jive_node * self_)
{
	return strdup("SUBROUTINE");
}
	
jive_node *
_jive_subroutine_node_copy(const jive_node * self_,
	struct jive_region * region,
	struct jive_output * operands[])
{
	return 0;
}

bool
_jive_subroutine_node_equiv(const jive_node * self_, const jive_node * other_)
{
	return false;
}

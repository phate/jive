#ifndef JIVE_BACKEND_I386_SUBROUTINE_H
#define JIVE_BACKEND_I386_SUBROUTINE_H

#include <jive/arch/subroutine.h>

struct jive_region;
struct jive_node;

/* convert according to "default" ABI */
jive_node *
jive_i386_subroutine_convert(struct jive_region * target_parent, struct jive_node * lambda_node);

#endif

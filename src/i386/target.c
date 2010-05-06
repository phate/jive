#include <jive/i386/machine.h>
#include <jive/i386/abi.h>
#include <jive/target.h>

const jive_target jive_i386_target = {
	.machine = &jive_i386_machine,
	.create_subroutine = &jive_i386_subroutine_create
};

const jive_target *
jive_get_default_target(void)
{
	return &jive_i386_target;
}

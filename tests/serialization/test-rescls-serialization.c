#include <stdio.h>
#include <string.h>

#include <jive/context.h>
#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/serialization/grammar.h>
#include <jive/serialization/token-stream.h>
#include <jive/util/buffer.h>

static void
my_handle_error(jive_serialization_driver * ctx, const char msg[])
{
	fputs(msg, stderr);
	abort();
}

static void
verify_serialize(const jive_resource_class * rescls, const char * expect_repr)
{
	jive_context * ctx = jive_context_create();
	jive_buffer buf;
	jive_buffer_init(&buf, ctx);
	
	jive_token_ostream * os = jive_token_ostream_simple_create(&buf);
	
	jive_serialization_driver drv;
	jive_serialization_driver_init(&drv, ctx);
	drv.error = my_handle_error;
	jive_serialize_rescls(&drv, rescls, os);
	jive_serialization_driver_fini(&drv);
	
	jive_token_ostream_destroy(os);
	
	assert(strncmp((char *)buf.data, expect_repr, buf.size) == 0);
	
	jive_buffer_fini(&buf);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
}

static void
verify_deserialize(const char * repr, const jive_resource_class * expect_rescls)
{
	jive_context * ctx = jive_context_create();
	jive_token_istream * is = jive_token_istream_simple_create(
		ctx, repr, repr + strlen(repr));
	
	const jive_resource_class * rescls;
	
	jive_serialization_driver drv;
	jive_serialization_driver_init(&drv, ctx);
	drv.error = my_handle_error;
	assert(jive_deserialize_rescls(&drv, is, &rescls));
	jive_serialization_driver_fini(&drv);
	
	jive_token_istream_destroy(is);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	
	assert(rescls == expect_rescls);
}

int
main()
{
	verify_serialize(&jive_root_resource_class, "root<>");
	verify_serialize(&jive_root_register_class, "register<>");
	verify_serialize(jive_stackslot_size_class_get(4, 4), "stackslot<4,4>");
	verify_serialize(jive_fixed_stackslot_class_get(4, 4, -12), "stack_frameslot<4,4,-12>");
	verify_serialize(jive_callslot_class_get(4, 4, 12), "stack_callslot<4,4,12>");
	
	verify_deserialize("root<>", &jive_root_resource_class);
	verify_deserialize("register<>", &jive_root_register_class);
	verify_deserialize("stackslot<4,32>", jive_stackslot_size_class_get(4, 32));
	verify_deserialize("stack_frameslot<4,4,-32>", jive_fixed_stackslot_class_get(4, 4, -32));
	verify_deserialize("stack_callslot<4,4,32>", jive_callslot_class_get(4, 4, 32));
	verify_deserialize("stack_callslot<4,4,-32>", jive_callslot_class_get(4, 4, -32));
	
	return 0;
}

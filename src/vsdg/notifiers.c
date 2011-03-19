#include <jive/vsdg/notifiers.h>
#include <jive/util/list.h>

#define MAKE_NOTIFIER(name, NAME, parameters, arguments) \
struct jive_##name##_notifier { \
	jive_notifier base; \
	jive_##name##_notifier_function function; \
	void * closure; \
	jive_##name##_notifier_slot * slot; \
	jive_context * context; \
	struct { \
		jive_##name##_notifier * prev; \
		jive_##name##_notifier * next; \
	} notifier_list; \
}; \
 \
static inline void \
jive_##name##_notifier_unlink(jive_##name##_notifier * self) \
{ \
	if (!self->slot) return; \
	 \
	JIVE_LIST_REMOVE(self->slot->notifiers, self, notifier_list); \
	self->slot = 0; \
} \
 \
static void \
_jive_##name##_notifier_disconnect(jive_notifier * _self) \
{ \
	jive_##name##_notifier * self = (jive_##name##_notifier *) _self; \
	jive_##name##_notifier_unlink(self); \
	jive_context_free(self->context, self); \
} \
 \
static const jive_notifier_class JIVE_##NAME##_NOTIFIER = { \
	.disconnect = _jive_##name##_notifier_disconnect \
}; \
 \
void \
jive_##name##_notifier_slot_fini(jive_##name##_notifier_slot * self) \
{ \
	while(self->notifiers.first) jive_##name##_notifier_unlink(self->notifiers.first); \
} \
 \
jive_notifier * \
jive_##name##_notifier_slot_connect(jive_##name##_notifier_slot * self, jive_##name##_notifier_function function, void * closure) \
{ \
	jive_##name##_notifier * notifier = jive_context_malloc(self->context, sizeof(*notifier)); \
	notifier->base.class_ = &JIVE_##NAME##_NOTIFIER; \
	notifier->slot = self; \
	notifier->context = self->context; \
	notifier->function = function; \
	notifier->closure = closure; \
	JIVE_LIST_PUSH_BACK(self->notifiers, notifier, notifier_list); \
	 \
	return &notifier->base; \
} \
 \
void \
jive_##name##_notifier_slot_call(const jive_##name##_notifier_slot * self, parameters) \
{ \
	jive_##name##_notifier * notifier; \
	JIVE_LIST_ITERATE(self->notifiers, notifier, notifier_list) \
		notifier->function(notifier->closure, arguments); \
} \
 \

#define WRAP(args...) args

/* node notifiers */

MAKE_NOTIFIER(node, NODE, struct jive_node * node, node)
MAKE_NOTIFIER(input, INPUT, struct jive_input * input, input)
MAKE_NOTIFIER(input_change, INPUT_CHANGE, WRAP(struct jive_input * input, struct jive_output * old_origin, struct jive_output * new_origin), WRAP(input, old_origin, new_origin))
MAKE_NOTIFIER(output, OUTPUT, struct jive_output * output, output)

MAKE_NOTIFIER(region, REGION, struct jive_region * region, region)
MAKE_NOTIFIER(region_ssavar, REGION_SSAVAR, WRAP(struct jive_region * region, struct jive_ssavar * ssavar), WRAP(region, ssavar))

MAKE_NOTIFIER(variable, VARIABLE, struct jive_variable * variable, variable)
MAKE_NOTIFIER(variable_resource_class, VARIABLE_RESOURCE_CLASS, WRAP(struct jive_variable * variable, const struct jive_resource_class * old_rescls, const struct jive_resource_class * new_rescls), WRAP(variable, old_rescls, new_rescls))
MAKE_NOTIFIER(variable_resource_name, VARIABLE_RESOURCE_NAME, WRAP(struct jive_variable * variable, const struct jive_resource_name * old_resname, const struct jive_resource_name * new_resname), WRAP(variable, old_resname, new_resname))
MAKE_NOTIFIER(variable_gate, VARIABLE_GATE, WRAP(struct jive_variable * variable, struct jive_gate * gate), WRAP(variable, gate))

MAKE_NOTIFIER(ssavar, SSAVAR, struct jive_ssavar * ssavar, ssavar)
MAKE_NOTIFIER(ssavar_variable, SSAVAR_VARIABLE, WRAP(struct jive_ssavar * ssavar, struct jive_variable * old_var, struct jive_variable * new_var), WRAP(ssavar, old_var, new_var))
MAKE_NOTIFIER(ssavar_divert, SSAVAR_DIVERT, WRAP(struct jive_ssavar * ssavar, struct jive_output * old_origin, struct jive_output * new_origin), WRAP(ssavar, old_origin, new_origin))
MAKE_NOTIFIER(ssavar_output, SSAVAR_OUTPUT, WRAP(struct jive_ssavar * ssavar, struct jive_output * output), WRAP(ssavar, output))
MAKE_NOTIFIER(ssavar_input, SSAVAR_INPUT, WRAP(struct jive_ssavar * ssavar, struct jive_input * input), WRAP(ssavar, input))

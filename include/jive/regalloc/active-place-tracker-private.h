#ifndef JIVE_REGALLOC_ACTIVE_PLACE_TRACKER_PRIVATE_H
#define JIVE_REGALLOC_ACTIVE_PLACE_TRACKER_PRIVATE_H

#include <jive/regalloc/active-place-tracker.h>
#include <jive/vsdg/regcls-count-private.h>
#include <jive/vsdg/basetype.h>

JIVE_DEFINE_HASH_TYPE(jive_active_resource_map, jive_active_place, struct jive_resource *, resource, hash_by_resource);
JIVE_DEFINE_HASH_TYPE(jive_active_origin_map, jive_active_place, struct jive_output *, origin, hash_by_origin);

jive_active_place_tracker *
jive_active_place_tracker_create(jive_context * context);

void
jive_active_place_tracker_destroy(jive_active_place_tracker * self);

jive_active_place_tracker *
jive_active_place_tracker_copy(const jive_active_place_tracker * self);

void
jive_active_place_tracker_unlock(jive_active_place_tracker * self);

void
jive_active_place_tracker_divert_origin(jive_active_place_tracker * self, jive_output * old_origin, jive_output * new_origin);

jive_active_place *
jive_active_place_tracker_add_place(jive_active_place_tracker * self, jive_output * origin, jive_resource * resource);

jive_active_place *
jive_active_place_tracker_activate_input(jive_active_place_tracker * self, jive_input * input);

jive_active_place *
jive_active_place_tracker_activate_output(jive_active_place_tracker * self, jive_input * input);

void
jive_active_place_tracker_deactivate_place(jive_active_place_tracker * self, jive_active_place * place);

void
jive_active_place_tracker_deactivate_output(jive_active_place_tracker * self, jive_output * output);

jive_active_place *
jive_active_place_tracker_get_output_place(const jive_active_place_tracker * self, jive_output * output);

jive_active_place *
jive_active_place_tracker_get_input_place(const jive_active_place_tracker * self, jive_input * input);

jive_active_place *
jive_active_place_tracker_get_resource_place(const jive_active_place_tracker * self, jive_resource * resource);

jive_conflict
jive_active_place_tracker_check_replacement_conflict(const jive_active_place_tracker * self, jive_active_place * place, jive_resource * new_constraint);

bool
jive_active_place_tracker_check_merge_conflict(const jive_active_place_tracker * self, jive_active_place * place, jive_resource * new_constraint);

jive_conflict
jive_active_place_tracker_check_aux_regcls_conflict(const jive_active_place_tracker * self, const struct jive_regcls * regcls);

jive_active_place *
jive_active_place_tracker_merge_input_constraint(jive_active_place_tracker * self, jive_active_place * place, jive_input * input, jive_resource * new_constraint);

jive_active_place *
jive_active_place_tracker_merge_output_constraint(jive_active_place_tracker * self, jive_active_place * place, jive_output * output, jive_resource * new_constraint);

#endif

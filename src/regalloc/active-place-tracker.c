#include <jive/regalloc/active-place-tracker-private.h>
#include <jive/vsdg/basetype.h>
#include <jive/context.h>

static jive_active_place *
jive_active_place_create(jive_output * origin, jive_resource * resource, jive_context * context)
{
	jive_active_place * self = jive_context_malloc(context, sizeof(*self));
	self->origin = origin;
	self->resource = resource;
	self->locked = 0;
	
	return self;
}

static inline void
jive_active_place_tracker_init(jive_active_place_tracker * self, jive_context * context)
{
	self->context = context;
	jive_active_resource_map_init(&self->resource_map, context);
	jive_active_origin_map_init(&self->origin_map, context);
	jive_regcls_count_init(&self->use_count);
}

static inline void
jive_active_place_tracker_fini(jive_active_place_tracker * self)
{
	struct jive_active_resource_map_iterator i;
	i = jive_active_resource_map_begin(&self->resource_map);
	
	while(i.entry) {
		jive_active_place * place = i.entry;
		jive_active_place_tracker_deactivate_place(self, place);
		jive_active_resource_map_iterator_next(&i);
	}
	jive_active_resource_map_fini(&self->resource_map);
	jive_active_origin_map_fini(&self->origin_map);
	jive_regcls_count_fini(&self->use_count, self->context);
}

jive_active_place_tracker *
jive_active_place_tracker_create(jive_context * context)
{
	jive_active_place_tracker * self = jive_context_malloc(context, sizeof(*self));
	jive_active_place_tracker_init(self, context);
	return self;
}

void
jive_active_place_tracker_destroy(jive_active_place_tracker * self)
{
	jive_active_place_tracker_fini(self);
	jive_context_free(self->context, self);
}

jive_active_place_tracker *
jive_active_place_tracker_copy(const jive_active_place_tracker * self)
{
	jive_active_place_tracker * tmp = jive_active_place_tracker_create(self->context);
	
	struct jive_active_resource_map_iterator i;
	JIVE_HASH_ITERATE(jive_active_resource_map, self->resource_map, i) {
		jive_active_place * new_place = jive_active_place_create(i.entry->origin, i.entry->resource, self->context);
		jive_active_resource_map_insert(&tmp->resource_map, new_place);
		jive_active_origin_map_insert(&tmp->origin_map, new_place);
	}
	
	jive_regcls_count_copy(&tmp->use_count, self->context, &self->use_count);
	
	return tmp;
}

void
jive_active_place_tracker_unlock(jive_active_place_tracker * self)
{
	struct jive_active_resource_map_iterator i;
	JIVE_HASH_ITERATE(jive_active_resource_map, self->resource_map, i)
		i.entry->locked = 0;
}

void
jive_active_place_tracker_divert_origin(jive_active_place_tracker * self, jive_output * old_origin, jive_output * new_origin)
{
	jive_active_place * place = jive_active_origin_map_lookup(&self->origin_map, old_origin);
	if (!place) return;
	
	jive_active_origin_map_remove(&self->origin_map, place);
	place->origin = new_origin;
	jive_active_origin_map_insert(&self->origin_map, place);
}

jive_active_place *
jive_active_place_tracker_add_place(jive_active_place_tracker * self, jive_output * origin, jive_resource * resource)
{
	jive_active_place * place = jive_active_place_create(origin, resource, self->context);
	jive_active_origin_map_insert(&self->origin_map, place);
	jive_active_resource_map_insert(&self->resource_map, place);
	jive_regcls_count_add(&self->use_count, self->context, jive_resource_get_real_regcls(resource));
	
	return place;
}

jive_active_place *
jive_active_place_tracker_activate_input(jive_active_place_tracker * self, jive_input * input)
{
	jive_active_place * place = jive_active_resource_map_lookup(&self->resource_map, input->resource);
	if (place) return place;
	return jive_active_place_tracker_add_place(self, input->origin, input->resource);
}

jive_active_place *
jive_active_place_tracker_activate_output(jive_active_place_tracker * self, jive_input * input)
{
	jive_active_place * place = jive_active_resource_map_lookup(&self->resource_map, input->resource);
	if (place) return place;
	return jive_active_place_tracker_add_place(self, input->origin, input->resource);
}

void
jive_active_place_tracker_deactivate_place(jive_active_place_tracker * self, jive_active_place * place)
{
	jive_regcls_count_sub(&self->use_count, self->context, jive_resource_get_real_regcls(place->resource));
	jive_active_origin_map_remove(&self->origin_map, place);
	jive_active_resource_map_remove(&self->resource_map, place);
	jive_context_free(self->context, place);
}

void
jive_active_place_tracker_deactivate_output(jive_active_place_tracker * self, jive_output * output)
{
	jive_active_place * place = jive_active_origin_map_lookup(&self->origin_map, output);
	jive_active_place_tracker_deactivate_place(self, place);
}

jive_active_place *
jive_active_place_tracker_get_output_place(const jive_active_place_tracker * self, jive_output * output)
{
	return jive_active_origin_map_lookup(&self->origin_map, output);
}

jive_active_place *
jive_active_place_tracker_get_input_place(const jive_active_place_tracker * self, jive_input * input)
{
	return jive_active_origin_map_lookup(&self->origin_map, input->origin);
}

jive_active_place *
jive_active_place_tracker_get_resource_place(const jive_active_place_tracker * self, jive_resource * resource)
{
	return jive_active_resource_map_lookup(&self->resource_map, resource);
}

jive_conflict
jive_active_place_tracker_check_replacement_conflict(const jive_active_place_tracker * self, jive_active_place * place, jive_resource * new_constraint)
{
	struct jive_active_resource_map_iterator i;
	JIVE_HASH_ITERATE(jive_active_resource_map, self->resource_map, i) {
		if (i.entry == place) continue;
		if (jive_resource_conflicts_with(i.entry->resource, new_constraint)) {
			jive_conflict conflict;
			conflict.type = jive_conflict_resource_name;
			conflict.place = i.entry;
			return conflict;
		}
	}
	
	const struct jive_regcls * old_regcls = 0;
	if (place) old_regcls = jive_resource_get_real_regcls(place->resource);
	
	const struct jive_regcls * overflow;
	overflow = jive_regcls_count_check_change(&self->use_count, old_regcls, jive_resource_get_real_regcls(new_constraint));
	if (overflow) {
		jive_conflict conflict;
		conflict.type = jive_conflict_register_class;
		conflict.regcls = overflow;
		return conflict;
	}
	
	jive_conflict conflict;
	conflict.type = jive_conflict_none;
	
	return conflict;
}

bool
jive_active_place_tracker_check_merge_conflict(const jive_active_place_tracker * self, jive_active_place * place, jive_resource * new_constraint)
{
	if (!place) return false;
	return !jive_resource_can_merge(place->resource, new_constraint);
}

jive_conflict
jive_active_place_tracker_check_aux_regcls_conflict(const jive_active_place_tracker * self, const struct jive_regcls * regcls)
{
	jive_conflict conflict;
	if (regcls) {
		const struct jive_regcls * overflow = jive_regcls_count_check_add(&self->use_count, regcls);
		if (overflow) {
			conflict.type = jive_conflict_register_class;
			conflict.regcls = overflow;
		}
	}
	conflict.type = jive_conflict_none;
	return conflict;
}

jive_active_place *
jive_active_place_tracker_merge_input_constraint(jive_active_place_tracker * self, jive_active_place * place, jive_input * input, jive_resource * new_constraint)
{
	if (!place) {
		place = jive_active_place_tracker_add_place(self, input->origin, new_constraint);
		jive_resource_assign_input(new_constraint, input);
		return place;
	}
	
	jive_resource_assign_input(place->resource, input);
	const struct jive_regcls * old_regcls = jive_resource_get_real_regcls(place->resource);
	jive_resource_merge(place->resource, new_constraint);
	const struct jive_regcls * new_regcls = jive_resource_get_real_regcls(place->resource);
	jive_regcls_count_change(&self->use_count, self->context, old_regcls, new_regcls);
	
	return place;
}

jive_active_place *
jive_active_place_tracker_merge_output_constraint(jive_active_place_tracker * self, jive_active_place * place, jive_output * output, jive_resource * new_constraint)
{
	if (!place) {
		place = jive_active_place_tracker_add_place(self, output, new_constraint);
		jive_resource_assign_output(new_constraint, output);
		return place;
	}
	
	jive_resource_assign_output(place->resource, output);
	const struct jive_regcls * old_regcls = jive_resource_get_real_regcls(place->resource);
	jive_resource_merge(place->resource, new_constraint);
	const struct jive_regcls * new_regcls = jive_resource_get_real_regcls(place->resource);
	jive_regcls_count_change(&self->use_count, self->context, old_regcls, new_regcls);
	
	return place;
}

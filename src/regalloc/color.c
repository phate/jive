#include <jive/regalloc/color.h>
#include <jive/vsdg/valuetype-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/resource-interference-private.h>
#include <jive/debug-private.h>

static const jive_cpureg *
find_allowed_register(jive_value_resource * regcand)
{
	const jive_cpureg * best_reg = 0;
	size_t best_pressure = regcand->base.interference.nitems + 1;
	
	struct jive_allowed_registers_hash_iterator i;
	JIVE_HASH_ITERATE(jive_allowed_registers_hash, regcand->allowed_registers, i) {
		const jive_cpureg * reg = i.entry->reg;
		
		/* TODO: test for overflow */
		
		size_t pressure = 0;
		
		/* count the neighbours who could legally also be assigned this register */
		struct jive_resource_interference_hash_iterator j;
		JIVE_HASH_ITERATE(jive_resource_interference_hash, regcand->base.interference, j) {
			jive_resource * resource = j.entry->resource;
			if (!jive_resource_isinstance(resource, &JIVE_VALUE_RESOURCE)) continue;
			
			jive_value_resource * vresource = (jive_value_resource *) resource;
			if (jive_allowed_registers_hash_lookup(&vresource->allowed_registers, reg))
				pressure ++;
		}
		
		/* pick the one that is least constraining for the neighbours */
		if (pressure < best_pressure) {
			best_pressure = pressure;
			best_reg = reg;
		}
	}
	
	DEBUG_ASSERT(best_reg);
	
	return best_reg;
}

static void
color_single(jive_graph * graph, jive_value_resource * regcand)
{
	const jive_cpureg * reg = find_allowed_register(regcand);
	/* TODO: implement conflict resolution */
	DEBUG_ASSERT(reg);
	jive_value_resource_set_cpureg(regcand, reg);
}

static jive_value_resource *
find_next_uncolored(jive_graph * graph)
{
	if (graph->valueres.max_pressure)
		return graph->valueres.pressured[graph->valueres.max_pressure - 1].first;
	if (graph->valueres.trivial.first)
		return graph->valueres.trivial.first;
	return 0;
}

void
jive_regalloc_color(jive_graph * graph)
{
	for(;;) {
		jive_value_resource * regcand = find_next_uncolored(graph);
		if (!regcand) return;
		if (!jive_resource_used(&regcand->base)) {
			jive_resource_destroy(&regcand->base);
			continue;
		}
		color_single(graph, regcand);
	}
}

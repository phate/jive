/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SELECTOR_H
#define JIVE_REGALLOC_SELECTOR_H

struct jive_node;
struct jive_region;
struct jive_resource_class;
struct jive_ssavar;

namespace jive {
namespace regalloc {

/* select next nodes to be shaped in region */
class region_selector {
public:
	virtual
	~region_selector() noexcept;

	/* select next node for shaping */
	virtual
	jive_node *
	select_node() = 0;

	/* select variable suitable for spilling */
	virtual
	jive_ssavar *
	select_spill(
		const jive_resource_class * rescls,
		jive_node * disallow_origins) const = 0;

	/* push onto priority node selection stack -- this node will be
	 * preferred for selection over all other currently selectable nodes */
	virtual
	void
	push_node_stack(
		jive_node * node) = 0;
};

/* master tying the selectors for indvidual regions together; also provides
 * common support infrastructure */
class master_selector {
public:
	virtual
	~master_selector() noexcept;

	/* lookup or create shaper selector for region */
	virtual
	region_selector *
	map_region(const jive_region * region) = 0;
};

}
}

#endif

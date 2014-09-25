/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_REGION_SSAVAR_USE_H
#define JIVE_VSDG_REGION_SSAVAR_USE_H

#include <stddef.h>

#include <jive/util/intrusive-hash.h>

struct jive_region_ssavar_use {
	struct jive_region * region;
	struct jive_ssavar * ssavar;
	size_t count;

private:
	jive::detail::intrusive_hash_anchor<jive_region_ssavar_use> region_hash_chain;
	jive::detail::intrusive_hash_anchor<jive_region_ssavar_use> ssavar_hash_chain;

public:
	typedef jive::detail::intrusive_hash_accessor<
		struct jive_ssavar *,
		jive_region_ssavar_use,
		&jive_region_ssavar_use::ssavar,
		&jive_region_ssavar_use::region_hash_chain
	> region_hash_chain_accessor;

	typedef jive::detail::intrusive_hash_accessor<
		struct jive_region *,
		jive_region_ssavar_use,
		&jive_region_ssavar_use::region,
		&jive_region_ssavar_use::ssavar_hash_chain
	> ssavar_hash_chain_accessor;
};

typedef jive::detail::intrusive_hash<
	const jive_ssavar *,
	jive_region_ssavar_use,
	jive_region_ssavar_use::region_hash_chain_accessor
> jive_region_ssavar_hash;

typedef jive::detail::intrusive_hash<
	const jive_region *,
	jive_region_ssavar_use,
	jive_region_ssavar_use::ssavar_hash_chain_accessor
> jive_ssavar_region_hash;

#endif

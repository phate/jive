/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_BASIC_BLOCK_H
#define JIVE_FRONTEND_BASIC_BLOCK_H

#include <jive/frontend/cfg_node.h>

namespace jive {
namespace frontend {
	class three_address_code;
}
}

class jive_basic_block final : public jive_cfg_node {
public:
	virtual ~jive_basic_block();

	jive_basic_block(struct jive_cfg * cfg) noexcept;

	virtual std::string debug_string() const override;

	struct {
		jive::frontend::three_address_code * first;
		jive::frontend::three_address_code * last;
	} three_address_codes;
};

struct jive_cfg_node *
jive_basic_block_create(struct jive_cfg * cfg);

#endif

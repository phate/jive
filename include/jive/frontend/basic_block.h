/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_BASIC_BLOCK_H
#define JIVE_FRONTEND_BASIC_BLOCK_H

#include <jive/frontend/cfg_node.h>

namespace jive {
namespace frontend {

class basic_block final : public cfg_node {
public:
	virtual ~basic_block();

	basic_block(jive::frontend::cfg & cfg) noexcept;

	virtual std::string debug_string() const override;
};

}
}

#endif

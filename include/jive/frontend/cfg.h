/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CFG_H
#define JIVE_FRONTEND_CFG_H

#include <jive/frontend/cfg_node.h>

namespace jive {
	class buffer;

namespace frontend {
	class clg_node;
	class basic_block;

class cfg final {
	class enter_node;
	class exit_node;
public:
	~cfg() {}

	cfg();

	cfg(jive::frontend::clg_node  & clg_node);

	std::vector<std::unordered_set<cfg_node*>> find_sccs() const;

	void convert_to_dot(jive::buffer & buffer) const;

	jive::frontend::clg_node * clg_node;

	inline cfg::enter_node * enter() const noexcept { return enter_; }
	inline cfg::exit_node * exit() const noexcept { return exit_; }

	jive::frontend::basic_block * create_basic_block();

private:
	class enter_node final : public cfg_node {
	public:
		virtual ~enter_node() noexcept;

		enter_node(jive::frontend::cfg & cfg) noexcept;

		virtual std::string debug_string() const override;
	};

	class exit_node final : public cfg_node {
	public:
		virtual ~exit_node() noexcept;

		exit_node(jive::frontend::cfg & cfg) noexcept;

		virtual std::string debug_string() const override;
	};

	jive::frontend::cfg::enter_node * enter_;
	jive::frontend::cfg::exit_node * exit_;
	std::unordered_set<std::unique_ptr<cfg_node>> nodes_;
};

}
}

void
jive_cfg_view(const jive::frontend::cfg & self);

#endif

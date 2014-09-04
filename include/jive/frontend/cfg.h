/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CFG_H
#define JIVE_FRONTEND_CFG_H

#include <jive/frontend/cfg_node.h>

namespace jive {
namespace util {
	class buffer;
}

namespace frontend {
	class clg_node;

class cfg final {
	class enter_node;
	class exit_node;
public:
	~cfg();
	cfg();
	cfg(jive::frontend::clg_node  & clg_node);

	std::vector<std::unordered_set<cfg_node*>> find_sccs() const;

	void convert_to_dot(jive::buffer & buffer) const;

	jive::frontend::clg_node * clg_node;

	jive::frontend::cfg::enter_node * enter;
	jive::frontend::cfg::exit_node * exit;

	struct {
		jive::frontend::cfg_node * first;
		jive::frontend::cfg_node * last;
	} nodes;

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
};

}
}

void
jive_cfg_view(const jive::frontend::cfg & self);

void
jive_cfg_validate(const jive::frontend::cfg & self);

#endif

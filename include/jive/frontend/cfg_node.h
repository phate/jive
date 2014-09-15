/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CFG_NODE_H
#define JIVE_FRONTEND_CFG_NODE_H

#include <jive/common.h>

#include <memory>
#include <string>
#include <vector>


namespace jive {
namespace frontend {

class cfg;
class cfg_node;

class cfg_edge final {
public:
	~cfg_edge() noexcept {};

	cfg_edge(cfg_node * source, cfg_node * sink, size_t index) noexcept;

	void divert(cfg_node * new_sink, size_t new_index) noexcept;

	inline cfg_node * source() const noexcept { return source_; }
	inline cfg_node * sink() const noexcept { return sink_; }
	inline size_t index() const noexcept { return index_; }

private:
	cfg_node * source_;
	cfg_node * sink_;
	size_t index_;
};

class cfg_node {
public:
	virtual ~cfg_node();

protected:
	cfg_node(jive::frontend::cfg & cfg);

public:
	virtual std::string debug_string() const = 0;

	inline jive::frontend::cfg * cfg() const noexcept { return cfg_; }

	void add_taken_successor(cfg_node * successor);
	void add_nottaken_successor(cfg_node * successor);

	void remove_taken_successor();
	void remove_nottaken_successor();
	inline void remove_successors() { remove_taken_successor(); remove_nottaken_successor(); }

	void remove_predecessors();

	inline void divert_taken_successor(cfg_node * successor) {
		remove_taken_successor();
		add_taken_successor(successor);
	}

	inline void divert_nottaken_successor(cfg_node * successor) {
		remove_nottaken_successor();
		add_nottaken_successor(successor);
	}

	void divert_predecessors(cfg_node * node);

	inline jive::frontend::cfg_edge * taken_edge() const noexcept { return taken_edge_.get(); }
	inline jive::frontend::cfg_edge * nottaken_edge() const noexcept { return nottaken_edge_.get(); }
	cfg_node * taken_successor() const noexcept;
	cfg_node * nottaken_successor() const noexcept;

	inline size_t npredecessors() const noexcept { return predecessors_.size(); }
	inline std::vector<jive::frontend::cfg_edge *> predecessors() const noexcept {
		return predecessors_; }

	struct {
		jive::frontend::cfg_node * prev;
		jive::frontend::cfg_node * next;
	} cfg_node_list;

private:
	std::unique_ptr<jive::frontend::cfg_edge> taken_edge_;
	std::unique_ptr<jive::frontend::cfg_edge> nottaken_edge_;
	std::vector<jive::frontend::cfg_edge *> predecessors_;
	jive::frontend::cfg * cfg_;
};

}
}

#endif

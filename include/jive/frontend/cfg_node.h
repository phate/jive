/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CFG_NODE_H
#define JIVE_FRONTEND_CFG_NODE_H

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>


namespace jive {
namespace frontend {

class cfg;
class cfg_node;

class cfg_edge final {
public:
	~cfg_edge() noexcept {};

	cfg_edge(cfg_node * source, cfg_node * sink, size_t index) noexcept;

	inline void divert(cfg_node * new_sink) noexcept { sink_ = new_sink; }

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
	cfg_node(jive::frontend::cfg & cfg) : cfg_(&cfg) {}

public:
	virtual std::string debug_string() const = 0;

	inline jive::frontend::cfg * cfg() const noexcept { return cfg_; }

	jive::frontend::cfg_edge * add_outedge(jive::frontend::cfg_node * successor, size_t index);

	void remove_outedge(jive::frontend::cfg_edge * edge);

	void remove_outedges();

	size_t noutedges() const noexcept;

	std::vector<jive::frontend::cfg_edge*> outedges() const;

	void divert_inedges(jive::frontend::cfg_node * new_successor);

	void remove_inedges();

	size_t ninedges() const noexcept;

	std::vector<jive::frontend::cfg_edge*> inedges() const;

	bool no_predecessor() const noexcept;

	bool single_predecessor() const noexcept;

	bool no_successor() const noexcept;

	bool single_successor() const noexcept;


private:
	std::unordered_set<std::unique_ptr<jive::frontend::cfg_edge>> outedges_;
	std::unordered_set<jive::frontend::cfg_edge*> inedges_;
	jive::frontend::cfg * cfg_;
};

}
}

#endif

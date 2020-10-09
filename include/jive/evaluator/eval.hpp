/*
 * Copyright 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_EVALUATOR_EVAL_HPP
#define JIVE_EVALUATOR_EVAL_HPP

#include <memory>
#include <vector>

namespace jive {
namespace eval {

class literal;

const std::unique_ptr<const literal>
eval(
	const jive::graph * graph,
	const std::string & name,
	const std::vector<const literal*> & literals);

}
}

#endif

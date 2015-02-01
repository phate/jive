/*
 * Copyright 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SEQTYPE_H
#define JIVE_VSDG_SEQTYPE_H

#include <jive/vsdg/statetype.h>

namespace jive {
namespace seq {

class type final : public jive::state::type {
public:
	virtual ~type() noexcept;

	inline constexpr type() noexcept : jive::state::type() {};

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive::seq::type * copy() const override;
};

const type seqtype;

}
}

#endif

/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_SUBROUTINE_NODES_H
#define JIVE_ARCH_SUBROUTINE_NODES_H

#include <jive/arch/subroutine/signature.h>
#include <jive/vsdg/anchor.h>
#include <jive/vsdg/node.h>

namespace jive {

class subroutine_head_op final : public region_head_op {
public:
	virtual
	~subroutine_head_op() noexcept;

	virtual size_t
	nresults() const noexcept override;

	virtual const base::type &
	result_type(size_t index) const noexcept override;
	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class subroutine_tail_op final : public region_tail_op {
public:
	virtual
	~subroutine_tail_op() noexcept;

	virtual size_t
	narguments() const noexcept override;

	virtual const base::type &
	argument_type(size_t index) const noexcept override;
	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class subroutine_op final : public region_anchor_op {
public:
	virtual
	~subroutine_op() noexcept;

	inline subroutine_op(
		subroutine_machine_signature signature) noexcept
		: signature_(std::move(signature))
	{
	}

	subroutine_op(const subroutine_op & other) = default;

	subroutine_op(subroutine_op && other) noexcept = default;

	virtual size_t
	nresults() const noexcept override;

	virtual const base::type &
	result_type(size_t index) const noexcept override;
	virtual std::string
	debug_string() const override;

	inline const jive::subroutine_machine_signature &
	signature() const noexcept
	{
		return signature_;
	}

	output *
	get_passthrough_enter_by_name(jive::region * region, const char * name) const noexcept;

	output *
	get_passthrough_enter_by_index(jive::region * region, size_t index) const noexcept;

	input *
	get_passthrough_leave_by_name(jive::region * region, const char * name) const noexcept;

	input *
	get_passthrough_leave_by_index(jive::region * region, size_t index) const noexcept;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	subroutine_machine_signature signature_;
};

}

#endif

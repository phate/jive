/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/fltconstant.h>
#include <jive/types/float/fltoperation-classes-private.h>
#include <jive/types/float/fltoperation-classes.h>
#include <jive/types/float/flttype.h>

#include <jive/vsdg/control.h>
#include <jive/vsdg/node-private.h>

namespace jive {

flt_unary_operation::~flt_unary_operation() noexcept
{
}

flt_binary_operation::~flt_binary_operation() noexcept
{
}

size_t
flt_binary_operation::narguments() const noexcept
{
	return 2;
}

const jive::base::type &
flt_binary_operation::argument_type(size_t index) const noexcept
{
	static const jive::flt::type flt;
	return flt;
}

size_t
flt_binary_operation::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
flt_binary_operation::result_type(size_t index) const noexcept
{
	static const jive::flt::type flt;
	return flt;
}

/* reduction methods */
jive_binop_reduction_path_t
flt_binary_operation::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	bool arg1_is_constant =
		dynamic_cast<const flt::constant_operation *>(&arg1->node()->operation());
	bool arg2_is_constant =
		dynamic_cast<const flt::constant_operation *>(&arg2->node()->operation());
	
	if (arg1_is_constant && arg2_is_constant) {
		return jive_binop_reduction_constants;
	}

	return jive_binop_reduction_none;
}

jive::output *
flt_binary_operation::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::output * arg1,
	jive::output * arg2) const
{
	jive_graph * graph = arg1->node()->graph;

	if (path == jive_binop_reduction_constants) {
		const flt::constant_operation & c1 =
			static_cast<const flt::constant_operation&>(arg1->node()->operation());
		const flt::constant_operation & c2 =
			static_cast<const flt::constant_operation&>(arg2->node()->operation());
		flt::value_repr result = reduce_constants(c1.value(), c2.value());
		return jive_fltconstant(graph, result);
	}

	return nullptr;
}


flt_compare_operation::~flt_compare_operation() noexcept
{
}

flt_compare_operation::~flt_compare_operation() noexcept;

size_t
flt_compare_operation::narguments() const noexcept
{
	return 2;
}

const jive::base::type &
flt_compare_operation::argument_type(size_t index) const noexcept
{
	static const jive::flt::type flt;
	return flt;
}

size_t
flt_compare_operation::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
flt_compare_operation::result_type(size_t index) const noexcept
{
	static const jive::ctl::type ctl;
	return ctl;
}

jive_binop_reduction_path_t
flt_compare_operation::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	bool arg1_is_constant =
		dynamic_cast<const flt::constant_operation *>(&arg1->node()->operation());
	bool arg2_is_constant =
		dynamic_cast<const flt::constant_operation *>(&arg2->node()->operation());
	
	if (arg1_is_constant && arg2_is_constant) {
		return jive_binop_reduction_constants;
	}

	return jive_binop_reduction_none;
}

jive::output *
flt_compare_operation::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::output * arg1,
	jive::output * arg2) const
{
	jive_graph * graph = arg1->node()->graph;

	if (path == jive_binop_reduction_constants) {
		const flt::constant_operation & c1 =
			static_cast<const flt::constant_operation&>(arg1->node()->operation());
		const flt::constant_operation & c2 =
			static_cast<const flt::constant_operation&>(arg2->node()->operation());
		bool result = reduce_constants(c1.value(), c2.value());
		if (result) {
			return jive_control_true(graph);
		} else {
			return jive_control_false(graph);
		}
	}

	return nullptr;
}

}


static void
jive_fltoperation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[], jive_context * context)
{
	if (noperands == 0)
		return;

	size_t n;
	jive::flt::type flttype;
	for (n = 0; n < noperands; n++) {
		if (!dynamic_cast<const jive::flt::output*>(operands[n]))
			jive_raise_type_error(&flttype, &operands[n]->type(), context);
	}
}

/* fltbinary operation class */

void
jive_fltbinary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[], jive_context * context)
{
	jive_fltoperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_fltbinary_operation_class JIVE_FLTBINARY_NODE_ = {
	base : {	/* jive_binary_operation_class */
		base : {	/* jive_node_class */
			parent : &JIVE_BINARY_OPERATION,
			name : "FLTBINARY",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_fltbinary_operation_check_operands_, /* override */
			create : nullptr,
		},
	
		flags : jive_binary_operation_none,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : jive_binary_operation_can_reduce_operand_pair_, /* inherit */
		reduce_operand_pair : jive_binary_operation_reduce_operand_pair_ /* inherit */
	},
	type : jive_fltop_code_invalid
};

/* fltunary operation class */

void
jive_fltunary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[], jive_context * context)
{
	jive_fltoperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_fltunary_operation_class JIVE_FLTUNARY_NODE_ = {
	base : { /* jive_unary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_UNARY_OPERATION,
			name : "FLTUNARY",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_fltunary_operation_check_operands_, /* override */
			create : nullptr,
		},
	
		single_apply_over : NULL,
		multi_apply_over : NULL,

		can_reduce_operand : jive_unary_operation_can_reduce_operand_, /* inherit */
		reduce_operand : jive_unary_operation_reduce_operand_ /* inherit */
	},
	type : jive_fltop_code_invalid
};

/* fltcomparison operation class */

void
jive_fltcomparison_operation_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive::output * const operands[],
	jive_context * context)
{
	jive_fltoperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_fltcomparison_operation_class JIVE_FLTCOMPARISON_NODE_ = {
	base : { /* jive_binary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BINARY_OPERATION,
			name : "FLTCOMPARISON",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_fltcomparison_operation_check_operands_, /* override */
			create : nullptr,
		},

		flags : jive_binary_operation_none,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : jive_binary_operation_can_reduce_operand_pair_, /* inherit */
		reduce_operand_pair : jive_binary_operation_reduce_operand_pair_ /* inherit */
	},
	type : jive_fltcmp_code_invalid
};

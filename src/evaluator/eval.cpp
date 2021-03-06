/*
 * Copyright 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/load.hpp>
#include <jive/arch/store.hpp>
#include <jive/evaluator/context.hpp>
#include <jive/evaluator/eval.hpp>
#include <jive/evaluator/literal.hpp>
#include <jive/rvsdg/control.hpp>
#include <jive/rvsdg/gamma.hpp>
#include <jive/rvsdg/phi.hpp>
#include <jive/rvsdg/theta.hpp>
#include <jive/types/bitstring.hpp>
#include <jive/types/function.hpp>

#include <typeindex>
#include <unordered_map>

typedef std::unordered_map<
		std::type_index,
		std::vector<std::unique_ptr<const jive::eval::literal>>(*)(
			const jive::operation & operation,
			const std::vector<std::unique_ptr<const jive::eval::literal>> & operands)
	> operation_map;

typedef std::unordered_map<
	std::type_index,
	const std::unique_ptr<const jive::eval::literal>(*)(
		const jive::node * node,
		size_t index,
		jive::eval::context & ctx)
	> eval_map;

namespace jive {
namespace eval {

/* computation */

static std::vector<std::unique_ptr<const literal>>
compute_bitconstant_op(
	const jive::operation & operation,
	const std::vector<std::unique_ptr<const literal>> & operands)
{
	JIVE_DEBUG_ASSERT(operands.size() == 0);
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::bitconstant_op*>(&operation));

	auto op = static_cast<const jive::bitconstant_op*>(&operation);

	std::vector<std::unique_ptr<const literal>> results;
	results.emplace_back(bitliteral(op->value()).copy());
	return results;
}

static std::vector<std::unique_ptr<const literal>>
compute_bitconcat_op(
	const jive::operation & operation,
	const std::vector<std::unique_ptr<const literal>> & operands)
{
	JIVE_DEBUG_ASSERT(operands.size() != 0);
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::bitconcat_op*>(&operation));

	jive::bitvalue_repr result(static_cast<const bitliteral*>(operands[0].get())->value_repr());
	for (size_t n = 1; n < operands.size(); n++)
		result = result.concat(static_cast<const bitliteral*>(operands[n].get())->value_repr());

	std::vector<std::unique_ptr<const literal>> results;
	results.emplace_back(bitliteral(result).copy());
	return results;
}

static std::vector<std::unique_ptr<const literal>>
compute_bitslice_op(
	const jive::operation & operation,
	const std::vector<std::unique_ptr<const literal>> & operands)
{
	JIVE_DEBUG_ASSERT(operands.size() == 1);
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::bitslice_op*>(&operation));

	auto op = static_cast<const jive::bitslice_op*>(&operation);

	const bitliteral * operand = static_cast<const bitliteral*>(operands[0].get());
	jive::bitvalue_repr result = operand->value_repr().slice(op->low(), op->high());

	std::vector<std::unique_ptr<const literal>> results;
	results.emplace_back(bitliteral(result).copy());
	return results;
}

static std::vector<std::unique_ptr<const literal>>
compute_bitunary_op(
	const jive::operation & operation,
	const std::vector<std::unique_ptr<const literal>> & operands)
{
	JIVE_DEBUG_ASSERT(operands.size() == 1);
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::bitunary_op*>(&operation));

	auto op = static_cast<const jive::bitunary_op*>(&operation);

	const bitliteral * operand = static_cast<const bitliteral*>(operands[0].get());
	jive::bitvalue_repr result = op->reduce_constant(operand->value_repr());

	std::vector<std::unique_ptr<const literal>> results;
	results.emplace_back(bitliteral(result).copy());
	return results;
}

static std::vector<std::unique_ptr<const literal>>
compute_bitbinary_op(
	const jive::operation & operation,
	const std::vector<std::unique_ptr<const literal>> & operands)
{
	JIVE_DEBUG_ASSERT(operands.size() == 2);
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::bitbinary_op*>(&operation));

	auto op = static_cast<const jive::bitbinary_op*>(&operation);

	const bitliteral * op1 = static_cast<const bitliteral*>(operands[0].get());
	const bitliteral * op2 = static_cast<const bitliteral*>(operands[1].get());

	jive::bitvalue_repr result = op->reduce_constants(op1->value_repr(), op2->value_repr());

	std::vector<std::unique_ptr<const literal>> results;
	results.emplace_back(bitliteral(result).copy());
	return results;
}

static std::vector<std::unique_ptr<const literal>>
compute_bitcompare_op(
	const jive::operation & operation,
	const std::vector<std::unique_ptr<const literal>> & operands)
{
	JIVE_DEBUG_ASSERT(operands.size() == 2);
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::bitcompare_op*>(&operation));

	auto op = static_cast<const jive::bitcompare_op*>(&operation);

	const bitliteral * operand1 = static_cast<const bitliteral*>(operands[0].get());
	const bitliteral * operand2 = static_cast<const bitliteral*>(operands[1].get());

	std::vector<std::unique_ptr<const literal>> results;
	switch (op->reduce_constants(operand1->value_repr(), operand2->value_repr())) {
		case compare_result::static_true:
			results.emplace_back(bitliteral(bitvalue_repr(1, 1)).copy());
			break;
		case compare_result::static_false:
			results.emplace_back(bitliteral(bitvalue_repr(1, 0)).copy());
			break;
		default:
			throw compiler_error("Comparison is undecidable.");
	}

	return results;
}

static std::vector<std::unique_ptr<const literal>>
compute_bitload_op(
	const jive::operation & operation,
	const std::vector<std::unique_ptr<const literal>> & operands)
{
	JIVE_DEBUG_ASSERT(operands.size() > 1);
	JIVE_DEBUG_ASSERT(is<load_op>(operation));
	JIVE_DEBUG_ASSERT(dynamic_cast<const bitliteral*>(operands[0].get()));

	auto op = static_cast<const bitload_op*>(&operation);
	auto vtype = dynamic_cast<const bittype*>(&op->valuetype());

	const bitliteral * address = static_cast<const bitliteral*>(operands[0].get());
	JIVE_DEBUG_ASSERT(address->value_repr().nbits() <= 64);
	uint64_t value = *((uint64_t*)address->value_repr().to_uint());
	value = value & ((uint64_t)-1) >> (64 - vtype->nbits());

	bitliteral result(bitvalue_repr(vtype->nbits(), value));
	std::vector<std::unique_ptr<const literal>> results;
	results.emplace_back(result.copy());
	return results;
}

static std::vector<std::unique_ptr<const literal>>
compute_bitstore_op(
	const jive::operation & operation,
	const std::vector<std::unique_ptr<const literal>> & operands)
{
	JIVE_DEBUG_ASSERT(operands.size() > 2);
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::store_op*>(&operation));
	JIVE_DEBUG_ASSERT(dynamic_cast<const bitliteral*>(operands[0].get()));
	JIVE_DEBUG_ASSERT(dynamic_cast<const bitliteral*>(operands[1].get()));

	auto & av = static_cast<const bitliteral*>(operands[0].get())->value_repr();
	auto & dv = static_cast<const bitliteral*>(operands[1].get())->value_repr();
	JIVE_DEBUG_ASSERT(av.nbits() <= 64);
	JIVE_DEBUG_ASSERT(dv.nbits() <= 64);

	uint64_t * address = (uint64_t *)av.to_uint();
	address = (uint64_t*) (((uint64_t)address) & (((uint64_t)-1) >> (64 - av.nbits())));

	uint64_t data = dv.to_uint();
	data = data & (((uint64_t)-1) >> (64 - dv.nbits()));

	*address = dv.nbits() == 64 ? data : ((*address >> dv.nbits()) << dv.nbits()) | data;

	std::vector<std::unique_ptr<const literal>> results;
	for (size_t n = 2; n < operands.size(); n++)
		results.emplace_back(operands[n]->copy());

	return results;
}

static std::vector<std::unique_ptr<const literal>>
compute_ctlconstant_op(
	const jive::operation & operation,
	const std::vector<std::unique_ptr<const literal>> & operands)
{
	JIVE_DEBUG_ASSERT(operands.size() == 0);
	JIVE_DEBUG_ASSERT(is_ctlconstant_op(operation));
	auto & op = to_ctlconstant_op(operation);

	std::vector<std::unique_ptr<const literal>> results;
	results.emplace_back(ctlliteral(op.value()).copy());
	return results;
}

static std::vector<std::unique_ptr<const literal>>
compute_match_op(
	const jive::operation & operation,
	const std::vector<std::unique_ptr<const literal>> & operands)
{
	JIVE_DEBUG_ASSERT(operands.size() == 1);
	JIVE_DEBUG_ASSERT(dynamic_cast<const bitliteral*>(operands[0].get()));
	JIVE_DEBUG_ASSERT(is<match_op>(operation));

	auto op = static_cast<const match_op*>(&operation);
	const bitliteral * cmp = static_cast<const bitliteral*>(operands[0].get());

	ctlvalue_repr vr(op->alternative(cmp->value_repr().to_uint()), op->nalternatives());

	std::vector<std::unique_ptr<const literal>> results;
	results.emplace_back(ctlliteral(vr).copy());
	return results;
}

static operation_map opmap({
	{std::type_index(typeid(jive::bitconstant_op)), compute_bitconstant_op},
	{std::type_index(typeid(jive::bitconcat_op)), compute_bitconcat_op},
	{std::type_index(typeid(jive::bitslice_op)), compute_bitslice_op},
	{std::type_index(typeid(jive::bitnot_op)), compute_bitunary_op},
	{std::type_index(typeid(jive::bitneg_op)), compute_bitunary_op},
	{std::type_index(typeid(jive::bitand_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitadd_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitashr_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitsub_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitor_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitmul_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitumulh_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitsmulh_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitshl_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitshr_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitsmod_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitumod_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitsdiv_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitudiv_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::bitxor_op)), compute_bitbinary_op},
	{std::type_index(typeid(jive::biteq_op)), compute_bitcompare_op},
	{std::type_index(typeid(jive::bitne_op)), compute_bitcompare_op},
	{std::type_index(typeid(jive::bitult_op)), compute_bitcompare_op},
	{std::type_index(typeid(jive::bitslt_op)), compute_bitcompare_op},
	{std::type_index(typeid(jive::bitule_op)), compute_bitcompare_op},
	{std::type_index(typeid(jive::bitsle_op)), compute_bitcompare_op},
	{std::type_index(typeid(jive::bitsgt_op)), compute_bitcompare_op},
	{std::type_index(typeid(jive::bitugt_op)), compute_bitcompare_op},
	{std::type_index(typeid(jive::bitsge_op)), compute_bitcompare_op},
	{std::type_index(typeid(jive::bituge_op)), compute_bitcompare_op},
	{std::type_index(typeid(jive::bitload_op)), compute_bitload_op},
	{std::type_index(typeid(jive::bitstore_op)), compute_bitstore_op},
	{std::type_index(typeid(jive::ctlconstant_op)), compute_ctlconstant_op},
	{std::type_index(typeid(jive::match_op)), compute_match_op}
});

static std::vector<std::unique_ptr<const literal>>
compute_operation(
	const jive::operation & operation,
	const std::vector<std::unique_ptr<const literal>> & operands)
{
	if (opmap.find(typeid(operation)) == opmap.end())
		throw compiler_error("Unknown operation.");

	return opmap[typeid(operation)](operation, operands);
}

/* evaluation */

static std::unique_ptr<const literal>
eval_input(const jive::input * input, context & ctx);

static const std::unique_ptr<const literal>
eval_apply_node(const jive::node * node, size_t index, context & ctx)
{
	JIVE_DEBUG_ASSERT(jive::is<jive::apply_op>(node));
	JIVE_DEBUG_ASSERT(index < node->noutputs());

	std::vector<std::unique_ptr<const literal>> arguments;
	for (size_t n = 1; n < node->ninputs(); n++)
			arguments.emplace_back(eval_input(node->input(n), ctx));

	ctx.push_arguments(arguments);
	auto fct = eval_input(node->input(0), ctx);
	ctx.pop_arguments();

	const fctliteral * fctv = static_cast<const fctliteral*>(fct.get());

	JIVE_DEBUG_ASSERT(node->noutputs() == fctv->nresults());
	for (size_t n = 0; n < fctv->nresults(); n++)
		ctx.insert(node->output(n), &fctv->result(n));

	return fctv->result(index).copy();
}

static const std::unique_ptr<const literal>
eval_lambda_node(const jive::node * node, size_t index, context & ctx)
{
	JIVE_DEBUG_ASSERT(jive::is<jive::lambda_op>(node));
	auto lambda = static_cast<const jive::structural_node*>(node);
	auto region = lambda->subregion(0);

	ctx.push_frame(region);

	std::vector<std::unique_ptr<const literal>> results;
	for (size_t n = 0; n < region->nresults(); n++)
		results.emplace_back(eval_input(region->result(n), ctx));

	std::unique_ptr<const literal> fct(new fctliteral(ctx.top_arguments(), results));
	ctx.pop_frame(region);

	return fct;
}

static const std::unique_ptr<const literal>
eval_gamma_node(const jive::node * node, size_t index, context & ctx)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::gamma_op*>(&node->operation()));
	auto gamma = static_cast<const jive::structural_node*>(node);

	auto predicate = node->input(0);
	JIVE_DEBUG_ASSERT(is_ctltype(predicate->type()));

	size_t alt = static_cast<const ctlliteral*>(eval_input(predicate, ctx).get())->alternative();
	auto region = gamma->subregion(alt);

	ctx.push_frame(region);

	std::vector<std::unique_ptr<const literal>> results;
	for (size_t n = 0; n < region->nresults(); n++)
		results.emplace_back(eval_input(region->result(n), ctx));

	ctx.pop_frame(region);

	JIVE_DEBUG_ASSERT(node->noutputs() == results.size());
	for (size_t n = 0; n < node->noutputs(); n++)
		ctx.insert(node->output(n), results[n].get());

	return results[index]->copy();
}

static const std::unique_ptr<const literal>
eval_theta_node(const jive::node * node, size_t index, context & ctx)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::theta_op*>(&node->operation()));
	auto theta = static_cast<const jive::structural_node*>(node);

	std::vector<std::unique_ptr<const literal>> results;
	do {
		auto subregion = theta->subregion(0);
		ctx.push_frame(subregion);

		if (!results.empty()) {
			JIVE_DEBUG_ASSERT(results.size() == subregion->narguments()+1);
			for (size_t n = 0; n < subregion->narguments(); n++)
				ctx.insert(subregion->argument(n), results[n+1].get());
			results.clear();
		}

		for (size_t n = 0; n < subregion->nresults(); n++)
			results.emplace_back(eval_input(subregion->result(n), ctx));
		ctx.pop_frame(subregion);

		JIVE_DEBUG_ASSERT(is_ctltype(results[0]->type()));
	} while (static_cast<const ctlliteral*>(results[0].get())->alternative());


	JIVE_DEBUG_ASSERT(node->noutputs() == results.size()-1);
	for (size_t n = 0; n < node->noutputs(); n++)
		ctx.insert(node->output(n), results[n+1].get());

	return std::move(results[index+1]);
}

static const std::unique_ptr<const literal>
eval_phi_node(const jive::node * node, size_t index, context & ctx)
{
	JIVE_DEBUG_ASSERT(is<phi::operation>(node));

	auto phi = static_cast<const phi::node*>(node);
	auto phi_region = phi->subregion();

	ctx.push_frame(phi_region);
	auto result = eval_input(phi_region->result(index), ctx);
	ctx.pop_frame(phi_region);

	return result;
}

static eval_map evlmap
({
	{typeid(jive::apply_op), eval_apply_node},
	{typeid(jive::lambda_op), eval_lambda_node},
	{std::type_index(typeid(jive::gamma_op)), eval_gamma_node},
	{std::type_index(typeid(jive::theta_op)), eval_theta_node},
	{std::type_index(typeid(phi::operation)), eval_phi_node}
});

static const std::unique_ptr<const literal>
eval_node(const jive::node * node, size_t index, context & ctx)
{
	JIVE_DEBUG_ASSERT(index < node->noutputs());

	/* check for special nodes and evaluate them */
	const auto & op = node->operation();
	if (evlmap.find(typeid(op)) != evlmap.end()) {
		std::unique_ptr<const literal> result;
		result = evlmap[typeid(op)](node, index, ctx)->copy();
		return result;
	}

	/* evaluate all other nodes */
	std::vector<std::unique_ptr<const literal>> operands;
	for (size_t n = 0; n < node->ninputs(); n++)
		operands.emplace_back(eval_input(node->input(n), ctx));

	std::vector<std::unique_ptr<const literal>> results;
	results = compute_operation(op, operands);

	JIVE_DEBUG_ASSERT(results.size() == node->noutputs());
	for (size_t n = 0; n < node->noutputs(); n++)
		ctx.insert(node->output(n), results[n].get());

	std::unique_ptr<const literal> result = results[index]->copy();
	return result;
}

static std::unique_ptr<const literal>
eval_argument(const jive::argument * argument, context & ctx)
{
	if (argument->region() == argument->region()->graph()->root())
		throw compiler_error("Cannot evaluate external entity.");

	std::unique_ptr<const literal> result;
	if (is<phi::operation>(argument->region()->node())) {
		ctx.push_frame(argument->region());
		result = eval_input(argument->region()->result(argument->index()), ctx);
		ctx.pop_frame(argument->region());
	} else if (is<lambda_op>(argument->region()->node())) {
		if (argument->input()) {
			/* it is an external dependency */
			result = eval_input(argument->input(), ctx);
		} else {
			/* it is a lambda argument */
			JIVE_DEBUG_ASSERT(!ctx.exists(argument));
			auto v = ctx.top_arguments()[argument->index()].get();
			ctx.insert(argument, v);
			result = v->copy();
		}
	} else {
		result = eval_input(argument->input(), ctx);
		ctx.insert(argument, result.get());
	}

	return result;
}

static std::unique_ptr<const literal>
eval_output(const jive::output * output, context & ctx)
{
	if (ctx.exists(output))
		return ctx.lookup(output)->copy();

	if (auto arg = dynamic_cast<const jive::argument*>(output))
		return eval_argument(arg, ctx);

	return eval_node(node_output::node(output), output->index(), ctx)->copy();
}

static std::unique_ptr<const literal>
eval_input(const jive::input * input, context & ctx)
{
	return eval_output(input->origin(), ctx);
}

const std::unique_ptr<const literal>
eval(
	const jive::graph * graph,
	const std::string & name,
	const std::vector<const literal*> & arguments)
{
	const jive::input * port = nullptr;
	for (size_t n = 0; n < graph->root()->nresults(); n++) {
		auto result = graph->root()->result(n);
		auto ep = dynamic_cast<const expport*>(&result->port());
		if (ep && ep->name() == name) {
			port = result;
			break;
		}
	}
	if (!port)
		throw compiler_error("Export not found.");

	context ctx;
	ctx.push_frame(graph->root());

	auto fcttype = dynamic_cast<const jive::fcttype*>(&port->type());
	if (fcttype) {
		if (fcttype->narguments() != arguments.size())
			throw compiler_error("Number of arguments does not coincide with function arguments.");

		for (size_t n = 0; n < fcttype->narguments(); n++) {
			if (fcttype->argument_type(n) != arguments[n]->type())
				throw type_error(fcttype->argument_type(n).debug_string(),
					arguments[n]->type().debug_string());
		}

		ctx.push_arguments(arguments);
	}

	auto result = eval_input(port, ctx);

	if (fcttype)
		ctx.pop_arguments();

	ctx.pop_frame(graph->root());
	JIVE_DEBUG_ASSERT(ctx.nframes(graph->root()) == 0);
	JIVE_DEBUG_ASSERT(ctx.narguments() == 0);

	return result;
}

}
}

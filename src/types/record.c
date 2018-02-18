/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/address-transform.h>
#include <jive/arch/load.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/record.h>

static constexpr jive_unop_reduction_path_t jive_select_reduction_load = 128;

namespace {

typedef std::unordered_set<std::unique_ptr<jive::rcddeclaration>> declarationset;
typedef std::unordered_map<const jive::graph*, declarationset> declarationmap;

declarationmap &
map()
{
	static declarationmap map;
	return map;
}

void
register_declaration(
	const jive::graph * graph,
	std::unique_ptr<jive::rcddeclaration> dcl)
{
	auto & m = map();
	if (m.find(graph) == m.end())
		m[graph] = declarationset();

	m[graph].insert(std::move(dcl));
}

}

namespace jive {

/* record declaration */

void
unregister_rcddeclarations(const jive::graph * graph)
{
	map().erase(graph);
}

rcddeclaration *
rcddeclaration::create(const jive::graph * graph)
{
	std::unique_ptr<jive::rcddeclaration> dcl(new jive::rcddeclaration());
	auto ptr = dcl.get();

	register_declaration(graph, std::move(dcl));
	return ptr;
}

/* record type */

rcdtype::~rcdtype() noexcept
{}

std::string
rcdtype::debug_string() const
{
	return "rcd";
}

bool
rcdtype::operator==(const jive::type & other) const noexcept
{
	auto type = dynamic_cast<const rcdtype*>(&other);
	return type != nullptr
	    && declaration() == type->declaration();
}

std::unique_ptr<jive::type>
rcdtype::copy() const
{
	return std::unique_ptr<jive::type>(new rcdtype(*this));
}

/* group operator */

group_op::~group_op() noexcept
{}

bool
group_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const group_op*>(&other);
	if (!op || op->narguments() != narguments())
		return false;

	for (size_t n = 0; n < narguments(); n++) {
		if (op->argument(n) != argument(n))
			return false;
	}

	return op->result(0) == result(0);
}

std::string
group_op::debug_string() const
{
	return "GROUP";
}

std::unique_ptr<jive::operation>
group_op::copy() const
{
	return std::unique_ptr<jive::operation>(new group_op(*this));
}

std::vector<jive::port>
group_op::create_operands(const rcddeclaration * dcl)
{
	std::vector<jive::port> operands;
	for (size_t n = 0; n < dcl->nelements(); n++)
		operands.push_back({dcl->element(n)});

	return operands;
}

/* select operator */

select_op::~select_op() noexcept
{}

bool
select_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const select_op*>(&other);
	return op
	    && op->index() == index()
	    && op->argument(0) == argument(0);
}

std::string
select_op::debug_string() const
{
	return detail::strfmt("SELECT(", index(), ")");
}

jive_unop_reduction_path_t
select_op::can_reduce_operand(const jive::output * arg) const noexcept
{
	if (arg->node() && dynamic_cast<const group_op *>(&arg->node()->operation()))
		return jive_unop_reduction_inverse;

	if (arg->node() && dynamic_cast<const load_op *>(&arg->node()->operation()))
		return jive_select_reduction_load;

	return jive_unop_reduction_none;
}

jive::output *
select_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse)
		return arg->node()->input(index())->origin();

	if (path == jive_select_reduction_load) {
		auto address = arg->node()->input(0)->origin();

		size_t nbits = 0;
		if (auto bt = dynamic_cast<const bittype*>(&address->type())) {
			nbits = bt->nbits();
			address = bit2addr_op::create(address, bt->nbits(), address->type());
		}

		auto decl = static_cast<const rcdtype*>(&arg->node()->output(0)->type())->declaration();

		size_t nstates = arg->node()->ninputs()-1;
		std::vector<jive::output*> states;
		for (size_t n = 0; n < nstates; n++)
			states.push_back(arg->node()->input(n+1)->origin());

		auto element_address = memberof_op::create(address, decl, index());
		if (dynamic_cast<const jive::addrtype*>(&address->type())) {
			return addrload_op::create(element_address, states);
		} else {
			return bitload_op::create(element_address, nbits, decl->element(index()), states);
		}
	}

	return nullptr;
}

std::unique_ptr<jive::operation>
select_op::copy() const
{
	return std::unique_ptr<jive::operation>(new select_op(*this));
}

}

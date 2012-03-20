#include <assert.h>
#include <locale.h>

#include <jive/arch/regvalue.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include "testarch.h"

int main()
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	jive_graph * graph = jive_graph_create(context);
	
	jive_subroutine * subroutine = jive_testarch_subroutine_create(graph->root_region,
		4, (jive_argument_type []){jive_argument_long, jive_argument_long, jive_argument_long, jive_argument_long},
		1, (jive_argument_type []){jive_argument_long});
	
	jive_node_reserve(&subroutine->subroutine_node->base);
	
	jive_output * arg1 = jive_subroutine_value_parameter(subroutine, 0);
	
	jive_output * lit = jive_bitconstant_unsigned(graph, 32, 42);
	jive_output * sym = jive_bitsymbolicconstant(graph, 32, "symbol");
	jive_output * not = jive_bitnot(sym);
	jive_output * sum1 = jive_bitsum(2, (jive_output*[]){arg1, lit});
	jive_output * sum2 = jive_bitsum(2, (jive_output*[]){lit, not});
	jive_output * res = jive_bituquotient(sum1, sum2);
	jive_subroutine_value_return(subroutine, 0, res);
	
	jive_view(graph, stdout);
	
	jive_regselector regselect;
	jive_regselector_init(&regselect, graph, &jive_testarch_reg_classifier);
	jive_regselector_process(&regselect);
	jive_regselector_fini(&regselect);
	
	jive_node * n1 = sum1->node->inputs[0]->origin->node;
	jive_node * n2 = sum1->node->inputs[1]->origin->node;
	
	jive_regvalue_node * rv = jive_regvalue_node_cast(n1);
	if (!rv)
		rv = jive_regvalue_node_cast(n2);
	assert(rv);
	assert(rv->attrs.regcls == &jive_testarch_regcls[cls_gpr]);
	
	sum2 = res->node->inputs[1]->origin;
	n1 = sum2->node->inputs[0]->origin->node;
	n2 = sum2->node->inputs[1]->origin->node;
	assert(n1->class_ == &JIVE_REGVALUE_NODE);
	assert(n2->class_ == &JIVE_BITNOT_NODE);
	n2 = n2->inputs[0]->origin->node;
	assert(n2->class_ == &JIVE_REGVALUE_NODE);
	jive_output * o1 = n1->inputs[1]->origin;
	jive_output * o2 = n2->inputs[1]->origin;
	assert(o1 == lit);
	assert(o2 == sym);
	
	jive_graph_prune(graph);
	jive_view(graph, stdout);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	return 0;
}
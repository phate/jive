#ifndef JIVE_VSDG_NORMALIZATION_PRIVATE_H
#define JIVE_VSDG_NORMALIZATION_PRIVATE_H

#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/basetype.h>

static inline size_t
max_expanded_operands(const jive_node_class * cls, size_t noperands, jive_output * operands[const])
{
	if (!(cls->flags & jive_node_class_associative)) return noperands;
	size_t n, total = 0;
	for(n=0; n<noperands; n++) {
		if (operands[n]->node->class_ == cls) total += operands[n]->node->ninputs;
		else total ++;
	}
	return total;
}

static inline size_t
normalize_operands(const jive_node_class * cls, jive_output * expanded_operands[], size_t noperands, jive_output * operands[const])
{
	size_t nexpanded_operands = 0;
	
	if (cls->flags & jive_node_class_associative) {
		size_t n;
		for(n=0; n<noperands; n++) {
			if (operands[n]->node->class_ == cls) {
				size_t k;
				for(k=0; k<operands[n]->node->ninputs; k++)
					expanded_operands[nexpanded_operands++] = operands[n]->node->inputs[k]->origin;
			} else expanded_operands[nexpanded_operands++] = operands[n];
		}
	} else {
		size_t n;
		for(n=0; n<noperands; n++)
			expanded_operands[nexpanded_operands++] = operands[n];
	}
	
	if (cls->flags & jive_node_class_commutative) {
		size_t n = 0;
		while(n < nexpanded_operands) {
			size_t k = n + 1;
			while(k < nexpanded_operands) {
				jive_output * out = cls->reduce(expanded_operands[n], expanded_operands[k]);
				if (out) {
					expanded_operands[n] = out;
					size_t j = k + 1;
					while(j<nexpanded_operands) expanded_operands[j-1] = expanded_operands[j];
					nexpanded_operands --;
				} else k ++;
			}
			n ++;
		}
		/* sort */
		for(n=0; n<nexpanded_operands; n++) {
			size_t k;
			for(k=nexpanded_operands; k>n; k--) {
			}
		}
	} else {
		size_t n = 1;
		while(n < nexpanded_operands) {
			jive_output * out = cls->reduce(expanded_operands[n-1], expanded_operands[n]);
			if (out) {
				expanded_operands[n-1] = out;
				size_t j = n + 1;
				while(j<nexpanded_operands) expanded_operands[j-1] = expanded_operands[j];
				nexpanded_operands --;
			} else n++;
		}
	}
	
	return nexpanded_operands;
}

static inline bool
jive_node_cse_test(jive_node * node, const jive_node_class * cls, const jive_node_attrs * attrs, size_t noperands, jive_output * operands[const])
{
	if (node->class_ != cls) return false;
	if (node->ninputs != noperands) return false;
	size_t n;
	for(n=0; n<node->ninputs; n++)
		if (!(node->inputs[n]->origin == operands[n])) return false;
	const jive_node_attrs * match = jive_node_get_attrs(node);
	return cls->equiv(attrs, match);
}

static inline jive_node *
jive_node_cse(const jive_node_class * cls, jive_graph * graph, const jive_node_attrs * attrs, size_t noperands, jive_output * operands[const])
{
	if (noperands) {
		jive_input * user;
		JIVE_LIST_ITERATE(operands[0]->users, user, output_users_list) {
			if (user->index != 0) continue;
			if (jive_node_cse_test(user->node, cls, attrs, noperands, operands)) return user->node;
		}
	} else {
		jive_node * node;
		JIVE_LIST_ITERATE(graph->top, node, graph_top_list) {
			if (jive_node_cse_test(node, cls, attrs, noperands, operands)) return node;
		}
	}
	
	return 0;
}

static inline jive_region *
jive_deepest_region(size_t noperands, jive_output * operands[const])
{
	jive_region * region = operands[0]->node->region;
	size_t n;
	for(n=1; n<noperands; n++) {
		if (operands[n]->node->region->depth > region->depth)
			region = operands[n]->node->region;
	}
	
	return region;
}

static inline jive_node *
jive_node_normalized_create(const jive_node_class * cls, const jive_node_attrs * attrs, size_t noperands, jive_output * operands[const])
{
	jive_output * expanded_operands[max_expanded_operands(cls, noperands, operands)];
	size_t nexpanded_operands = normalize_operands(cls, expanded_operands, noperands, operands);
	if ((cls->flags & jive_node_class_associative) && (nexpanded_operands == 1))
		return expanded_operands[0]->node;
	jive_node * node = jive_node_cse(cls, operands[0]->node->graph, attrs, nexpanded_operands, expanded_operands);
	if (!node) {
		jive_region * region = jive_deepest_region(nexpanded_operands, expanded_operands);
		node = cls->create(region, attrs, nexpanded_operands, operands);
	}
	return node;
}

static inline jive_node *
jive_node_normalize(jive_node * node)
{
	/* TODO: the following two restrictions could conceivably be relaxed */
	if (node->noperands != node->ninputs) return node;
	if (node->noutputs != 1) return node;
	
	jive_output * operands[node->ninputs];
	size_t n;
	for(n=0; n<node->ninputs; n++) operands[n] = node->inputs[n]->origin;
	
	jive_output * expanded_operands[max_expanded_operands(node->class_, node->ninputs, operands)];
	size_t nexpanded_operands = normalize_operands(node->class_, expanded_operands, node->ninputs, operands);
	
	if (nexpanded_operands == node->ninputs) {
		for(n=0; n<nexpanded_operands; n++)
			if (node->inputs[n]->origin != expanded_operands[n]) break;
	} else n = 0;
	
	if (n == nexpanded_operands) return node;
	
	jive_output * replacement;
	if (nexpanded_operands == 1) replacement = expanded_operands[0];
	else replacement = node->class_->create(node->region, jive_node_get_attrs(node), nexpanded_operands, expanded_operands)->outputs[0];
	
	jive_output_replace(node->outputs[0], replacement);
	
	return replacement->node;
}

#endif

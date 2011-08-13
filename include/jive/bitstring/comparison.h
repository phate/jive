#ifndef JIVE_BITSTRING_COMPARISON_H
#define JIVE_BITSTRING_COMPARISON_H

#include <jive/bitstring/type.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

typedef struct jive_bitcomparison_operation_class jive_bitcomparison_operation_class;

typedef enum jive_bitcmp_code {
	jive_bitcmp_code_invalid = 0,
	jive_bitcmp_code_equal = 1,
	jive_bitcmp_code_notequal = 2,
	jive_bitcmp_code_sless = 3,
	jive_bitcmp_code_uless = 4,
	jive_bitcmp_code_slesseq = 5,
	jive_bitcmp_code_ulesseq = 6,
	jive_bitcmp_code_sgreater = 7,
	jive_bitcmp_code_ugreater = 8,
	jive_bitcmp_code_sgreatereq = 9,
	jive_bitcmp_code_ugreatereq = 10
} jive_bitcmp_code;

struct jive_bitcomparison_operation_class {
	jive_binary_operation_class base;
	jive_bitcmp_code type;
	char (*compare_constants)(const char * c1, const char * c2, size_t nbits);
};

extern const jive_bitcomparison_operation_class JIVE_BITCOMPARISON_NODE_;
#define JIVE_BITCOMPARISON_NODE (JIVE_BITCOMPARISON_NODE_.base.base)


extern const jive_bitcomparison_operation_class JIVE_BITEQUAL_NODE_;
#define JIVE_BITEQUAL_NODE (JIVE_BITEQUAL_NODE_.base.base)

jive_node *
jive_bitequal_create(
	struct jive_region * region,
	struct jive_output * x, struct jive_output * y);

jive_output *
jive_bitequal(struct jive_output * x, struct jive_output * y);

JIVE_EXPORTED_INLINE jive_node *
jive_bitequal_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITEQUAL_NODE) return node;
	else return 0;
}


extern const jive_bitcomparison_operation_class JIVE_BITNOTEQUAL_NODE_;
#define JIVE_BITNOTEQUAL_NODE (JIVE_BITNOTEQUAL_NODE_.base.base)

jive_node *
jive_bitnotequal_create(
	struct jive_region * region,
	struct jive_output * x, struct jive_output * y);

jive_output *
jive_bitnotequal(struct jive_output * x, struct jive_output * y);

JIVE_EXPORTED_INLINE jive_node *
jive_bitnotequal_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITNOTEQUAL_NODE) return node;
	else return 0;
}


extern const jive_bitcomparison_operation_class JIVE_BITSLESS_NODE_;
#define JIVE_BITSLESS_NODE (JIVE_BITSLESS_NODE_.base.base)

jive_node *
jive_bitsless_create(
	struct jive_region * region,
	struct jive_output * x, struct jive_output * y);

jive_output *
jive_bitsless(struct jive_output * x, struct jive_output * y);

JIVE_EXPORTED_INLINE jive_node *
jive_bitsless_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITSLESS_NODE) return node;
	else return 0;
}


extern const jive_bitcomparison_operation_class JIVE_BITULESS_NODE_;
#define JIVE_BITULESS_NODE (JIVE_BITULESS_NODE_.base.base)

jive_node *
jive_bituless_create(
	struct jive_region * region,
	struct jive_output * x, struct jive_output * y);

jive_output *
jive_bituless(struct jive_output * x, struct jive_output * y);

JIVE_EXPORTED_INLINE jive_node *
jive_bituless_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITULESS_NODE) return node;
	else return 0;
}


extern const jive_bitcomparison_operation_class JIVE_BITSLESSEQ_NODE_;
#define JIVE_BITSLESSEQ_NODE (JIVE_BITSLESSEQ_NODE_.base.base)

jive_node *
jive_bitslesseq_create(
	struct jive_region * region,
	struct jive_output * x, struct jive_output * y);

jive_output *
jive_bitslesseq(struct jive_output * x, struct jive_output * y);

JIVE_EXPORTED_INLINE jive_node *
jive_bitslesseq_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITSLESSEQ_NODE) return node;
	else return 0;
}


extern const jive_bitcomparison_operation_class JIVE_BITULESSEQ_NODE_;
#define JIVE_BITULESSEQ_NODE (JIVE_BITULESSEQ_NODE_.base.base)

jive_node *
jive_bitulesseq_create(
	struct jive_region * region,
	struct jive_output * x, struct jive_output * y);

jive_output *
jive_bitulesseq(struct jive_output * x, struct jive_output * y);

JIVE_EXPORTED_INLINE jive_node *
jive_bitulesseq_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITULESSEQ_NODE) return node;
	else return 0;
}


extern const jive_bitcomparison_operation_class JIVE_BITSGREATER_NODE_;
#define JIVE_BITSGREATER_NODE (JIVE_BITSGREATER_NODE_.base.base)

jive_node *
jive_bitsgreater_create(
	struct jive_region * region,
	struct jive_output * x, struct jive_output * y);

jive_output *
jive_bitsgreater(struct jive_output * x, struct jive_output * y);

JIVE_EXPORTED_INLINE jive_node *
jive_bitsgreater_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITSGREATER_NODE) return node;
	else return 0;
}


extern const jive_bitcomparison_operation_class JIVE_BITUGREATER_NODE_;
#define JIVE_BITUGREATER_NODE (JIVE_BITUGREATER_NODE_.base.base)

jive_node *
jive_bitugreater_create(
	struct jive_region * region,
	struct jive_output * x, struct jive_output * y);

jive_output *
jive_bitugreater(struct jive_output * x, struct jive_output * y);

JIVE_EXPORTED_INLINE jive_node *
jive_bitugreater_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITUGREATER_NODE) return node;
	else return 0;
}


extern const jive_bitcomparison_operation_class JIVE_BITSGREATEREQ_NODE_;
#define JIVE_BITSGREATEREQ_NODE (JIVE_BITSGREATEREQ_NODE_.base.base)

jive_node *
jive_bitsgreatereq_create(
	struct jive_region * region,
	struct jive_output * x, struct jive_output * y);

jive_output *
jive_bitsgreatereq(struct jive_output * x, struct jive_output * y);

JIVE_EXPORTED_INLINE jive_node *
jive_bitsgreatereq_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITSGREATEREQ_NODE) return node;
	else return 0;
}


extern const jive_bitcomparison_operation_class JIVE_BITUGREATEREQ_NODE_;
#define JIVE_BITUGREATEREQ_NODE (JIVE_BITUGREATEREQ_NODE_.base.base)

jive_node *
jive_bitugreatereq_create(
	struct jive_region * region,
	struct jive_output * x, struct jive_output * y);

jive_output *
jive_bitugreatereq(struct jive_output * x, struct jive_output * y);

JIVE_EXPORTED_INLINE jive_node *
jive_bitugreatereq_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITUGREATEREQ_NODE) return node;
	else return 0;
}


#endif

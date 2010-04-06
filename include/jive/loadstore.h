#ifndef JIVE_LOADSTORE_H
#define JIVE_LOADSTORE_H

#include <jive/bitstring.h>

extern const jive_node_class JIVE_MEMACCESS;

/* abstract read from memory */

extern const jive_node_class JIVE_MEMREAD;

jive_node *
jive_memread_rawcreate(jive_value_bits * address, size_t size);

jive_value_bits *
jive_memread(jive_value_bits * input, size_t size);

/* abstract write to memory */

extern const jive_node_class JIVE_MEMWRITE;

jive_node *
jive_memwrite(jive_value_bits * address, jive_value_bits * data);

/* load of "word" from memory */

extern const jive_node_class JIVE_LOAD;

jive_node *
jive_load_rawcreate(jive_value_bits * address, size_t size, size_t ext_size, bool sign_extend);

jive_value_bits *
jive_load(jive_value_bits * address, size_t size, size_t ext_size, bool sign_extend);

/* store of "word" to memory */

extern const jive_node_class JIVE_STORE;

jive_node *
jive_storewrite(jive_value_bits * address, jive_value_bits * data);

#endif

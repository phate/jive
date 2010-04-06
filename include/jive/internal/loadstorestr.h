#ifndef JIVE_INTERNAL_LOADSTORESTR_H
#define JIVE_INTERNAL_LOADSTORESTR_H

typedef struct _jive_memaccess_node jive_memaccess_node;
typedef struct _jive_memread_node jive_memread_node;
typedef struct _jive_load_node jive_load_node;
typedef struct _jive_store_node jive_store_node;

struct _jive_memaccess_node {
	jive_node base;
};

struct _jive_memread_node {
	jive_memaccess_node base;
	jive_value_bits output;
};

struct _jive_load_node {
	jive_memaccess_node base;
	jive_value_bits output;
	bool sign_extend;
};

#endif

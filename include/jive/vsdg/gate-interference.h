#ifndef JIVE_GATE_INTERFERENCE_H
#define JIVE_GATE_INTERFERENCE_H

typedef struct jive_gate_interference jive_gate_interference;
typedef struct jive_gate_interference_part jive_gate_interference_part;
typedef struct jive_gate_interference_hash jive_gate_interference_hash;
typedef struct jive_gate_interference_hash_bucket jive_gate_interference_hash_bucket;

struct jive_gate_interference_hash_bucket {
	jive_gate_interference_part * first;
	jive_gate_interference_part * last;
};

struct jive_gate_interference_hash {
	size_t nitems, nbuckets;
	jive_gate_interference_hash_bucket * buckets;
};

#endif

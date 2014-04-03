/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_SERIALIZATION_TOKEN_STREAM_H
#define JIVE_SERIALIZATION_TOKEN_STREAM_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <jive/common.h>

struct jive_buffer;
struct jive_context;

typedef enum jive_token_type {
	jive_token_error = -1,
	jive_token_end = 0,
	jive_token_identifier = 1,
	jive_token_string = 2,
	jive_token_integral = 3,
	jive_token_colon = ':',
	jive_token_semicolon = ';',
	jive_token_openbrace = '{',
	jive_token_closebrace = '}',
	jive_token_openparen = '(',
	jive_token_closeparen = ')',
	jive_token_lt = '<',
	jive_token_gt = '>',
	jive_token_comma = ',',
	jive_token_equal = '=',
	jive_token_minus = '-',
	jive_token_plus = '+',
	jive_token_dot = '.',
	
	jive_token_keyword_first = 256,
	
	jive_token_gate = 256,
	jive_token_node = 257,
	jive_token_region = 258,
	jive_token_label = 259,
	jive_token_stackptr = 260,
	jive_token_frameptr = 261,
	
	jive_token_keyword_last_plus1
} jive_token_type;

typedef struct jive_token jive_token;
struct jive_token {
	jive_token_type type;
	union {
		const char * identifier;
		struct {
			const char * str;
			size_t len;
		} string;
		int64_t integral;
	} v;
};

bool
jive_token_equals(const jive_token * self, const jive_token * other);

typedef struct jive_token_ostream jive_token_ostream;
typedef struct jive_token_ostream_class jive_token_ostream_class;

struct jive_token_ostream_class {
	void (*destroy)(jive_token_ostream * self);
	void (*put)(jive_token_ostream * self, const jive_token * token);
};

struct jive_token_ostream {
	const jive_token_ostream_class * class_;
};

JIVE_EXPORTED_INLINE void
jive_token_ostream_destroy(jive_token_ostream * self)
{
	self->class_->destroy(self);
}

JIVE_EXPORTED_INLINE void
jive_token_ostream_put(jive_token_ostream * self, const jive_token * token)
{
	self->class_->put(self, token);
}

JIVE_EXPORTED_INLINE void
jive_token_ostream_identifier(jive_token_ostream * self, const char * identifier)
{
	jive_token token;
	token.type = jive_token_identifier;
	token.v.identifier = identifier;
	jive_token_ostream_put(self, &token);
}

JIVE_EXPORTED_INLINE void
jive_token_ostream_string(jive_token_ostream * self, const char * string, size_t size)
{
	jive_token token;
	token.type = jive_token_string;
	token.v.string.str = string;
	token.v.string.len = size;
	jive_token_ostream_put(self, &token);
}

JIVE_EXPORTED_INLINE void
jive_token_ostream_integral(jive_token_ostream * self, uint64_t value)
{
	jive_token token;
	token.type = jive_token_integral;
	token.v.integral = value;
	jive_token_ostream_put(self, &token);
}

JIVE_EXPORTED_INLINE void
jive_token_ostream_char(jive_token_ostream * self, char what)
{
	jive_token token;
	token.type = (jive_token_type) what;
	jive_token_ostream_put(self, &token);
}

jive_token_ostream *
jive_token_ostream_simple_create(struct jive_buffer * buffer);

typedef struct jive_token_istream jive_token_istream;
typedef struct jive_token_istream_class jive_token_istream_class;

struct jive_token_istream_class {
	void (*destroy)(jive_token_istream * self);
	void (*advance)(jive_token_istream * self);
};

struct jive_token_istream {
	const jive_token_istream_class * class_;
	jive_token current;
	jive_token next;
};

JIVE_EXPORTED_INLINE void
jive_token_istream_destroy(jive_token_istream * self)
{
	self->class_->destroy(self);
}

JIVE_EXPORTED_INLINE void
jive_token_istream_advance(jive_token_istream * self)
{
	self->class_->advance(self);
}

JIVE_EXPORTED_INLINE const jive_token *
jive_token_istream_current(const jive_token_istream * self)
{
	return &self->current;
}

JIVE_EXPORTED_INLINE const jive_token *
jive_token_istream_next(const jive_token_istream * self)
{
	return &self->next;
}

jive_token_istream *
jive_token_istream_simple_create(struct jive_context * context,
	const char * begin, const char * end);

JIVE_EXPORTED_INLINE bool
jive_token_istream_signed_integral(jive_token_istream * is, int64_t * value)
{
	const jive_token * current = jive_token_istream_current(is);
	const jive_token * next = jive_token_istream_next(is);
	if (current->type == jive_token_integral) {
		if (current->v.integral <= (uint64_t) 0x7fffffffffffffffLL) {
			*value = current->v.integral;
			jive_token_istream_advance(is);
			return true;
		} else {
			return false;
		}
	} else if (current->type == jive_token_minus && next->type == jive_token_integral) {
		if (current->v.integral <= 1 + ~ (uint64_t) (-0x7fffffffffffffffLL-1)) {
			*value = - next->v.integral;
			jive_token_istream_advance(is);
			jive_token_istream_advance(is);
			return true;
		} else {
			return false;
		}
	}
	return false;
}

#endif

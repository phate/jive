/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_UTIL_BUFFER_H
#define JIVE_UTIL_BUFFER_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <jive/common.h>

#include <stdint.h>
#include <vector>

/** \file jive/buffer.h */

namespace jive {

class buffer final {
public:
	inline
	~buffer()
	{}

	inline
	buffer()
	{}

	buffer(const buffer &) = delete;

	buffer(buffer &&) = delete;

	buffer &
	operator=(const buffer &) = delete;

	buffer &
	operator=(buffer &&) = delete;

	inline void
	push_back(const void * data, size_t nbytes)
	{
		auto d = static_cast<const uint8_t*>(data);
		for (size_t n = 0; n < nbytes; n++)
			data_.push_back(d[n]);
	}

	inline void
	push_back(const std::string & s)
	{
		push_back(s.c_str(), s.size());
	}

	inline void
	push_back(uint8_t byte)
	{
		data_.push_back(byte);
	}

	inline size_t
	size() const noexcept
	{
		return data_.size();
	}

	inline const uint8_t *
	data()
	{
		return &data_[0];
	}

	inline const char *
	c_str()
	{
		push_back('\0');
		return (const char *)(&data_[0]);
	}

	inline void
	clear()
	{
		data_.clear();
	}

private:
	std::vector<uint8_t> data_;
};

}

/**
	\defgroup jive_buffer Expandable buffers
	
	Management of expandable buffers
	
	@{
*/

/** \brief Expandable buffer */
typedef struct jive_buffer jive_buffer;

/** \brief Expandable buffer */
struct jive_buffer {
	std::vector<char> data;
};

static inline bool
jive_buffer_put(jive_buffer * self, const void * data, size_t size);

static inline bool
jive_buffer_putbyte(jive_buffer * self, unsigned char byte);

void *
jive_buffer_executable(const jive_buffer * self);

/* implementation */

static inline const char *
jive_buffer_to_string(struct jive_buffer * self)
{
	jive_buffer_putbyte(self, '\0');
	return static_cast<const char*>(&self->data[0]);
}

static inline void
jive_buffer_clear(struct jive_buffer * self)
{
	self->data.clear();
}

/**
	\brief Resize buffer
	
	\param self Buffer to be resized
	\param size New size of buffer in bytes
	
	Resizes the given buffer to the desired size. The data will be
	truncated or expanded with undefined values.
*/
static inline void
jive_buffer_resize(jive_buffer * self, size_t new_size)
{
	self->data.resize(new_size);
}

/**
	\brief Append to buffer
	
	\param buffer Buffer to append to
	\param data Data to append to buffer
	\param size Number of bytes to append
	\returns Whether adding data succeeded
	
	Appends @c data to @c buffer. Returns @c true if data was added
	to buffer, or @c false if no data could be added due to an out-of-memory
	condition.
*/
static inline bool
jive_buffer_put(jive_buffer * self, const void * data, size_t size)
{
	for (size_t n = 0; n < size; n++)
		self->data.push_back(static_cast<const char*>(data)[n]);
	return true;
}

static inline bool
jive_buffer_putstr(jive_buffer * self, const char * str)
{
	return jive_buffer_put(self, str, strlen(str));
}

/**
	\brief Append to buffer
	
	\param buffer Buffer to append to
	\param byte Data to append to buffer
	\returns Whether adding data succeeded
	
	Appends @c byte to @c buffer. Returns @c true if data was added
	to buffer, or @c false if no data could be added due to an out-of-memory
	condition.
*/
static inline bool
jive_buffer_putbyte(jive_buffer * self, unsigned char byte)
{
	return jive_buffer_put(self, &byte, 1);
}

/** @} */

#endif

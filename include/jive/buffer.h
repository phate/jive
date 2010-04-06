#ifndef JIVE_BUFFER_H
#define JIVE_BUFFER_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/** \file rtdt/buffer.h */

/**
	\defgroup jive_buffer Expandable buffers 
	
	Management of expandable buffers
	
	@{
*/

/** \brief Expandable buffer */
typedef struct _jive_buffer jive_buffer;

/** \brief Expandable buffer */
struct _jive_buffer {
	/** \brief Data stored in buffer */
	void * data;
	/** \brief Number of bytes stored in buffer */
	size_t size;
	/** \brief Available storage space before buffer must be expanded */
	size_t available;
};

static inline void
jive_buffer_init(jive_buffer * buffer);

static inline void
jive_buffer_destroy(jive_buffer * buffer);

static inline void *
jive_buffer_reserve(jive_buffer * buffer, size_t size);

static inline bool
jive_buffer_put(jive_buffer * buffer, void * data, size_t size);

static inline bool
jive_buffer_putbyte(jive_buffer * buffer, unsigned char byte);

void *
jive_buffer_executable(const jive_buffer * buffer);

/* implementation */

void *
jive_buffer_reserve_slow(jive_buffer * buffer, size_t size);

/**
	\brief Initialize expandable buffer
	
	\param buffer Buffer to initialize
*/
static inline void
jive_buffer_init(jive_buffer * buffer)
{
	buffer->data = 0;
	buffer->size = buffer->available = 0;
}

/**
	\brief Destroy expandable buffer
	\param buffer Buffer to destroy
	
	Frees the memory stored in the buffer
*/
static inline void
jive_buffer_destroy(jive_buffer * buffer)
{
	if (buffer->data) free(buffer->data);
}

/**
	\brief Reserve space in buffer
	
	\param buffer Buffer to append to
	\param size Number of bytes to append
	\returns Pointer to first byte to be written, or NULL if resizing failed
	
	Resizes @c buffer such that at least @c size additional bytes
	fit in. The newly-reserved space is uninitialized and should
	immediately be filled with data.
	
	The returned pointer is only valid until the next call to
	\ref jive_buffer_append
*/
static inline void *
jive_buffer_reserve(jive_buffer * buffer, size_t size)
{
	if (buffer->size + size > buffer->available)
		return jive_buffer_reserve_slow(buffer, size);
	
	void * tmp = buffer->size + (char *)(buffer->data);
	buffer->size += size;
	return tmp;
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
jive_buffer_put(jive_buffer * buffer, void * data, size_t size)
{
	void * ptr = jive_buffer_reserve(buffer, size);
	if (!ptr) return false;
	memcpy(ptr, data, size);
	return true;
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
jive_buffer_putbyte(jive_buffer * buffer, unsigned char byte)
{
	return jive_buffer_put(buffer, &byte, 1);
}

/** @} */

#endif

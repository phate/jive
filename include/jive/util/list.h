/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_UTIL_LIST_H
#define JIVE_UTIL_LIST_H

#define JIVE_LIST_PUSH_BACK(list_head, object, anchor) \
do { \
	if ((list_head).last) (list_head).last->anchor.next = (object); \
	else (list_head).first = (object); \
	(object)->anchor.prev = (list_head).last; \
	(object)->anchor.next = 0; \
	(list_head).last = object; \
} while(0)

#define JIVE_LIST_PUSH_FRONT(list_head, object, anchor) \
do { \
	if ((list_head).first) (list_head).first->anchor.prev = (object); \
	else (list_head).last = object; \
	(object)->anchor.prev = 0; \
	(object)->anchor.next = (list_head).first; \
	(list_head).first = object; \
} while(0)

#define JIVE_LIST_INSERT(list_head, at, object, anchor) \
do { \
	if ((at)) { \
		(object)->anchor.prev = (at)->anchor.prev; \
		(object)->anchor.next = (at); \
		if ((at)->anchor.prev) (at)->anchor.prev->anchor.next = (object); \
		else (list_head).first = (object); \
		(at)->anchor.prev = (object); \
	} else { \
		(object)->anchor.prev = (list_head).last; \
		(object)->anchor.next = 0; \
		if ((list_head).last) (list_head).last->anchor.next = (object); \
		else (list_head).first = (object); \
		(list_head).last = (object); \
	} \
} while(0)

#define JIVE_LIST_REMOVE(list_head, object, anchor) \
do { \
	if ((object)->anchor.prev) (object)->anchor.prev->anchor.next = (object)->anchor.next; \
	else (list_head).first = (object)->anchor.next; \
	if ((object)->anchor.next) (object)->anchor.next->anchor.prev = (object)->anchor.prev; \
	else (list_head).last = (object)->anchor.prev; \
} while(0) \

/* iterate through linked list of objects; the list may not be modified during
iteration */
#define JIVE_LIST_ITERATE(list_head, object, anchor) \
	for(object = (list_head).first; object; object = object->anchor.next)

/* iterate through linked list of objects; the list may not be modified during
iteration, except for the current element which may be removed from the list */
#define JIVE_LIST_ITERATE_SAFE(list_head, object, nextobj, anchor) \
	for(object = (list_head).first, nextobj = object ? object->anchor.next : 0; object; (object = nextobj), (nextobj = nextobj ? nextobj->anchor.next : 0))

#endif

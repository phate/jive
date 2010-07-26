#ifndef JIVE_UTIL_LIST_H
#define JIVE_UTIL_LIST_H

#define JIVE_LIST_PUSHBACK(list_head, object, anchor) \
do { \
	if ((list_head).last) (list_head).last->anchor.next = object; \
	else (list_head).first = object; \
	object->anchor.prev = (list_head).last; \
	object->anchor.next = 0; \
	(list_head).last = object; \
} while(0)

#define JIVE_LIST_REMOVE(list_head, object, anchor) \
do { \
	if ((object)->anchor.prev) (object)->anchor.prev->anchor.next = (object)->anchor.next; \
	else (list_head).first = (object)->anchor.next; \
	if ((object)->anchor.next) (object)->anchor.next->anchor.prev = (object)->anchor.prev; \
	else (list_head).last = (object)->anchor.prev; \
} while(0) \

#define JIVE_LIST_ITERATE(list_head, object, anchor) \
	for(object = list_head.first; object; object = object->anchor.next)

#endif

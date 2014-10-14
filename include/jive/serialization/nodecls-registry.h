/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_SERIALIZATION_NODECLS_REGISTRY_H
#define JIVE_SERIALIZATION_NODECLS_REGISTRY_H

#include <stdbool.h>
#include <stddef.h>

#include <string>
#include <typeindex>
#include <typeinfo>

#include <jive/common.h>
#include <jive/util/intrusive-hash.h>
#include <jive/vsdg/node.h>

struct jive_region;
struct jive_serialization_driver;
struct jive_token_istream;
struct jive_token_ostream;

namespace jive {

class operation;
class output;

namespace serialization {

class parser_driver;
class output_driver;

class opcls_registry;

class opcls_handler {
private:
	jive::detail::intrusive_hash_anchor<opcls_handler> tag_anchor_;
	jive::detail::intrusive_hash_anchor<opcls_handler> cls_chain_;

	std::string tag_;
	const std::type_info & cls_;
	opcls_registry & registry_;

	typedef jive::detail::intrusive_hash_accessor<
		std::string, opcls_handler,
		&opcls_handler::tag_, &opcls_handler::tag_anchor_
	> tag_hash_accessor;

	class cls_hash_accessor {
	public:
		const std::type_index
		get_key(const opcls_handler * obj) const noexcept
		{
			return std::type_index(obj->cls_);
		}
		opcls_handler *
		get_prev(const opcls_handler * obj) const noexcept
		{
			return obj->cls_chain_.prev;
		}
		void
		set_prev(opcls_handler * obj, opcls_handler * prev) const noexcept
		{
			obj->cls_chain_.prev = prev;
		}
		opcls_handler *
		get_next(const opcls_handler * obj) const noexcept
		{
			return obj->cls_chain_.next;
		}
		void
		set_next(opcls_handler * obj, opcls_handler * next) const noexcept
		{
			obj->cls_chain_.next = next;
		}
	};

	friend class opcls_registry;
public:
	virtual
	~opcls_handler() noexcept;

	virtual void
	serialize(
		const operation & op,
		output_driver & out) const = 0;

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const = 0;

	inline const std::string &
	tag() const noexcept
	{
		return tag_;
	}

protected:
	inline opcls_handler(
		std::string tag,
		const std::type_info & cls,
		opcls_registry & registry);
};

class opcls_registry {
public:
	typedef jive::detail::intrusive_hash<
		std::string, opcls_handler, opcls_handler::tag_hash_accessor
	> tag_hash_type;
	typedef jive::detail::intrusive_hash<
		std::type_index, opcls_handler, opcls_handler::cls_hash_accessor
	> cls_hash_type;

	inline const opcls_handler *
	lookup(std::type_index cls) const noexcept
	{
		auto i = cls_hash_.find(cls);
		return i != cls_hash_.end() ? i.ptr() : nullptr;
	}

	inline const opcls_handler *
	lookup(const std::string& tag) const noexcept
	{
		auto i = tag_hash_.find(tag);
		return i != tag_hash_.end() ? i.ptr() : nullptr;
	}

	static const opcls_registry &
	instance();

	static opcls_registry &
	mutable_instance();

private:
	tag_hash_type tag_hash_;
	cls_hash_type cls_hash_;

	friend class opcls_handler;
};

inline
opcls_handler::opcls_handler(
	std::string tag,
	const std::type_info & cls,
	opcls_registry & registry)
	: tag_(tag)
	, cls_(cls)
	, registry_(registry)
{
	registry.tag_hash_.insert(this);
	registry.cls_hash_.insert(this);
}

template<typename Opcls>
class simple_opcls_handler : public opcls_handler {
public:
	inline simple_opcls_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(Opcls), registry)
	{
	}

	virtual void
	serialize(
		const operation & op,
		output_driver & out) const override
	{
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		return std::unique_ptr<operation>(new Opcls());
	}
};
#define JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(tag, opcls) \
	jive::serialization::simple_opcls_handler<opcls> registerer_##tag( \
		#tag, jive::serialization::opcls_registry::mutable_instance());

}

}

#endif

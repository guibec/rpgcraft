#pragma once

#include "x-types.h"
#include "x-stl.h"

#include <set>
#include <unordered_set>
#include <queue>

class	ITickableEntity;
class	IDrawableEntity;
struct	EntityContainerEvent;


enum EntityContainerEvent_t
{
	ECEvt_EntityAdd,
	ECEvt_EntityRemove,
};

union SaltedOrderId
{
	struct {
		u32		m_salt;
		u32		m_order;
	};

	u64		m_fullSortOrder;

	SaltedOrderId() {
		m_fullSortOrder = 0;
	}

	u32 Salt() const {
		return m_fullSortOrder & ((1ULL<<32)-1);
	}

	u32 Order() const {
		return m_fullSortOrder >> 32;
	}

	SaltedOrderId& SetSalt(u32 salt) {
		//m_fullSortOrder = (m_fullSortOrder & (u64(UINT32_MAX)<<32)) | salt;
		m_salt = salt;
		return *this;
	}

	SaltedOrderId& SetOrder(u32 order) {
		//m_fullSortOrder = (u64(order) << 32) | (m_fullSortOrder & UINT32_MAX);
		m_order = order;
		return *this;
	}

	bool operator< (const SaltedOrderId& right) const { return m_fullSortOrder <  right.m_fullSortOrder; }
	bool operator<=(const SaltedOrderId& right) const { return m_fullSortOrder <= right.m_fullSortOrder; }
	bool operator> (const SaltedOrderId& right) const { return m_fullSortOrder >  right.m_fullSortOrder; }
	bool operator>=(const SaltedOrderId& right) const { return m_fullSortOrder >= right.m_fullSortOrder; }
};

extern SaltedOrderId MakeSaltedOrder(const ITickableEntity* entity, u32 order);
extern SaltedOrderId MakeSaltedOrder(const IDrawableEntity* entity, u32 order);

struct EntityUIDItem
{
	u32					uid;
	SaltedOrderId		saltedOrder;
};

struct TickableEntityItem
{
	ITickableEntity*	entity;
	SaltedOrderId		tickOrderSalted;
};

struct DrawableEntityItem
{
	const IDrawableEntity*	entity;
	SaltedOrderId			drawOrderSalted;
};

// Needed only for TickableEntityContainer, due to DrawableEntityContainer being const iterator at all times.
// (modification of drawable entity list during draw is invalid.  It can only be modified from tick context).
struct EntityContainerEvent
{
	EntityContainerEvent_t		evtId;
	TickableEntityItem			target;
};

typedef std::queue<EntityContainerEvent> EntityContainerEventQueue;

class ISpawnId
{
public:
	virtual u32		GetSpawnId() const=0;
};

class ITickableEntity : public virtual ISpawnId
{
public:
	virtual void	Tick()=0;
};

class IDrawableEntity : public virtual ISpawnId
{
public:
	virtual void	Draw() const=0;
};

struct CompareTickableEntity_Greater {
	bool operator()(const TickableEntityItem& _Left, const TickableEntityItem& _Right) const {
		return (_Left.tickOrderSalted > _Right.tickOrderSalted);
	}
};

struct CompareTickableEntity_Less {
	bool operator()(const TickableEntityItem& _Left, const TickableEntityItem& _Right) const {
		return (_Left.tickOrderSalted < _Right.tickOrderSalted);
	}
};

struct CompareDrawableEntity_Greater {
	bool operator()(const DrawableEntityItem& _Left, const DrawableEntityItem& _Right) const {
		return (_Left.drawOrderSalted > _Right.drawOrderSalted);
	}
};

struct CompareDrawableEntity_Less {
	bool operator()(const DrawableEntityItem& _Left, const DrawableEntityItem& _Right) const {
		return (_Left.drawOrderSalted < _Right.drawOrderSalted);
	}
};

class FunctHashEntityItem {
public:
	__xi size_t operator()(const TickableEntityItem& input) const {
		bug_on(uptr(input.entity) & 15, "Unaligned entity pointer detected.");
		return uptr(input.entity) >> 4;
	}

	__xi size_t operator()(const DrawableEntityItem& input) const {
		return uptr(input.entity) >> 4;
	}

	__xi size_t operator()(const SaltedOrderId& input) const {
		return input.Salt();
	}
};

class FunctEqualEntityItem {
public:
	__xi bool operator()(const SaltedOrderId& left, const SaltedOrderId& right) const {
		return left.Salt() == right.Salt();
	}

	__xi bool operator()(const DrawableEntityItem& left, const DrawableEntityItem& right) const {
		return left.entity == right.entity;
	}
};

// --------------------------------------------------------------------------------------
//  TickableEntityContainer
// --------------------------------------------------------------------------------------
struct TickableEntityContainer {
	typedef std::unordered_set<SaltedOrderId, FunctHashEntityItem, FunctEqualEntityItem>			HashedContainerType;
	typedef std::set<TickableEntityItem, CompareTickableEntity_Less>								OrderedContainerType;

	HashedContainerType			m_hashed;
	OrderedContainerType		m_ordered;			// ordered by priority
	EntityContainerEventQueue	m_evt_queue;
	int							m_max_container_size;
	int							m_iterator_mode;

	void		_Add			(const TickableEntityItem& entityInfo);
	void		Remove			(ITickableEntity* entity);
	void		ExecEventQueue	();

	__ai void Add(ITickableEntity* entity, int orderId) {
		_Add( { entity, MakeSaltedOrder(entity, orderId) } );
	}

	auto	ForEachForward		();
	auto	ForEachReverse		();

	void EnterIteratorMode() {
		m_iterator_mode += 1;
	}

	void LeaveIteratorMode() {
		bug_on (m_iterator_mode <= 0);
		m_iterator_mode -= 1;

		if (!m_iterator_mode) {
			ExecEventQueue();
		}
	}
};

// --------------------------------------------------------------------------------------
//  DrawableEntityContainer
// --------------------------------------------------------------------------------------
struct DrawableEntityContainer {
	typedef std::unordered_set<SaltedOrderId, FunctHashEntityItem, FunctEqualEntityItem>			HashedContainerType;
	typedef std::set<DrawableEntityItem, CompareDrawableEntity_Less>								OrderedContainerType;

	HashedContainerType				m_hashed;
	OrderedContainerType			m_ordered;

	// Add/Remove note: 
	//  * modification of drawable entity list during draw is invalid.  It can only be modified from tick context/

	void	_Add		(const DrawableEntityItem& entity);
	void	Remove		(const IDrawableEntity* entity);

	__ai void Add(const IDrawableEntity* entity, u32 order) {
		_Add( { static_cast<const IDrawableEntity*>(entity), MakeSaltedOrder(entity, order) } );
	}

	auto ForEachOpaque	() const;
	auto ForEachAlpha	() const;
};

struct TickableEntityForeachIfc_Forward
{
	TickableEntityContainer*	m_entityList;

	TickableEntityForeachIfc_Forward(TickableEntityContainer& src) {
		m_entityList = &src;
		m_entityList->EnterIteratorMode();
	}

	~TickableEntityForeachIfc_Forward() throw()
	{
		if (m_entityList) {
			m_entityList->LeaveIteratorMode();
			m_entityList = nullptr;
		}
	}

	__ai auto begin		()	const { return m_entityList->m_ordered.begin();		}
	__ai auto end		()	const { return m_entityList->m_ordered.end();		}
	__ai auto cbegin	()	const { return m_entityList->m_ordered.begin();		}
	__ai auto cend		()	const { return m_entityList->m_ordered.end();		}

};

struct TickableEntityForeachIfc_Reverse
{
	TickableEntityContainer*	m_entityList;

	TickableEntityForeachIfc_Reverse(TickableEntityContainer& src) {
		m_entityList = &src;
		m_entityList->EnterIteratorMode();
	}

	TickableEntityForeachIfc_Reverse(TickableEntityForeachIfc_Reverse&& rvalue)
	{
		m_entityList		= rvalue.m_entityList;
		rvalue.m_entityList = nullptr;
	}

	TickableEntityForeachIfc_Reverse& operator=(TickableEntityForeachIfc_Reverse&& rvalue)
	{
		std::swap(m_entityList, rvalue.m_entityList);
		return *this;
	}

	~TickableEntityForeachIfc_Reverse() throw()
	{
		if (m_entityList) {
			m_entityList->LeaveIteratorMode();
			m_entityList = nullptr;
		}
	}

	__ai auto begin		()	const { return m_entityList->m_ordered.rbegin();	}
	__ai auto end		()	const { return m_entityList->m_ordered.rend();		}
	__ai auto cbegin	()	const { return m_entityList->m_ordered.rbegin();	}
	__ai auto cend		()	const { return m_entityList->m_ordered.rend();		}

};

struct DrawableEntityForeachIfc_OpaqueOrder
{
	const DrawableEntityContainer*	m_entityList;

	DrawableEntityForeachIfc_OpaqueOrder(const DrawableEntityContainer& src) {
		m_entityList = &src;
	}

	__ai auto begin		()	const { return m_entityList->m_ordered.cbegin();	}
	__ai auto end		()	const { return m_entityList->m_ordered.cend();		}
	__ai auto cbegin	()	const { return m_entityList->m_ordered.cbegin();	}
	__ai auto cend		()	const { return m_entityList->m_ordered.cend();		}

};

struct DrawableEntityForeachIfc_AlphaOrder
{
	const DrawableEntityContainer*	m_entityList;

	DrawableEntityForeachIfc_AlphaOrder(const DrawableEntityContainer& src) {
		m_entityList = &src;
	}

	__ai auto begin		()	const { return m_entityList->m_ordered.crbegin();	}
	__ai auto end		()	const { return m_entityList->m_ordered.crend();		}
	__ai auto cbegin	()	const { return m_entityList->m_ordered.crbegin();	}
	__ai auto cend		()	const { return m_entityList->m_ordered.crend();		}

};

// ------------------------------------------------------------------------------------------------

inline auto TickableEntityContainer::ForEachForward()
{
	return TickableEntityForeachIfc_Forward(*this);
}

inline auto TickableEntityContainer::ForEachReverse()
{
	return TickableEntityForeachIfc_Reverse(*this);
}

inline auto DrawableEntityContainer::ForEachOpaque() const {
	return DrawableEntityForeachIfc_OpaqueOrder(*this);
}

inline auto DrawableEntityContainer::ForEachAlpha() const {
	return DrawableEntityForeachIfc_AlphaOrder(*this);
}

inline SaltedOrderId MakeSaltedOrder(const ITickableEntity* entity, u32 order)
{
	return SaltedOrderId().SetOrder(order).SetSalt(entity->GetSpawnId());
}

inline SaltedOrderId MakeSaltedOrder(const IDrawableEntity* entity, u32 order)
{
	return SaltedOrderId().SetOrder(order).SetSalt(entity->GetSpawnId());
}

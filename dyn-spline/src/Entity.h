#pragma once

#include "x-types.h"
#include "x-stl.h"

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <queue>

struct	EntityContainerEvent;


enum EntityContainerEvent_t
{
	ECEvt_EntityAdd,
	ECEvt_EntityRemove,
};

using EntityGid_t = u32;

struct EntityPointerContainerItem
{
	EntityGid_t		gid;
	void*			entityPtr;
	const char*		classname;		// alloc'd in same heap as entity
};

union EntityGidOrderPair
{
	struct {
		EntityGid_t		m_gid;
		u32				m_order;
	};

	u64		m_fullSortOrder;

	EntityGidOrderPair() {
		m_fullSortOrder = 0;
	}

	EntityGid_t Gid() const {
		return m_gid;
	}

	u32 Order() const {
		return m_order;
	}

	EntityGidOrderPair& SetGid(EntityGid_t gid) {
		//m_fullSortOrder = (m_fullSortOrder & (u64(UINT32_MAX)<<32)) | salt;
		m_gid = gid;
		return *this;
	}

	EntityGidOrderPair& SetOrder(u32 order) {
		//m_fullSortOrder = (u64(order) << 32) | (m_fullSortOrder & UINT32_MAX);
		m_order = order;
		return *this;
	}

	bool operator< (const EntityGidOrderPair& right) const { return m_fullSortOrder <  right.m_fullSortOrder; }
	bool operator<=(const EntityGidOrderPair& right) const { return m_fullSortOrder <= right.m_fullSortOrder; }
	bool operator> (const EntityGidOrderPair& right) const { return m_fullSortOrder >  right.m_fullSortOrder; }
	bool operator>=(const EntityGidOrderPair& right) const { return m_fullSortOrder >= right.m_fullSortOrder; }
};

// Global hash registry can be optimized later using hash table LUT:
//  - Define set maximum such as 20 bits of entity GIDs (1 million).
//  - Pick new IDs sequentually.
//  - If an entity exists in an ID slot, keep trying until an empty slot is realized.
//
// Or --
//  - Define a smaller set maximum, eg. 14 bits (16k entities)
//  - Perform a "cache check" when grabbing an entity to ensure that the full GID matches the cached one.
//  - On mismatch, perform cache eviction -- reload correct entity from global hash into "fast lut"
//

extern EntityGidOrderPair MakeGidOrder(const EntityGid_t gid, u32 order);

typedef void (EntityFn_LogicTick)	(		void* objdata);
typedef void (EntityFn_Draw)		(const	void* objdata);

struct TickableEntity
{
	EntityFn_LogicTick*		Tick;
	EntityGid_t				entityGid;
};

struct DrawableEntity
{
	EntityFn_Draw*			Draw;
	EntityGid_t				entityGid;
};

struct TickableEntityItem
{
	EntityGidOrderPair		orderId;
	EntityFn_LogicTick*		Tick;
};

struct DrawableEntityItem
{
	EntityGidOrderPair		orderId;
	EntityFn_Draw*			Draw;
};

// Needed only for TickableEntityContainer, due to DrawableEntityContainer being const iterator at all times.
// (modification of drawable entity list during draw is invalid.  It can only be modified from tick context).
struct EntityContainerEvent
{
	EntityContainerEvent_t		evtId;
	TickableEntityItem			entityInfo;
};

struct CompareEntityGid {
	bool operator()(const EntityPointerContainerItem& _Left, const EntityPointerContainerItem& _Right) const {
		return (_Left.gid == _Right.gid);
	}
};

struct CompareTickableEntity_Greater {
	bool operator()(const TickableEntityItem& _Left, const TickableEntityItem& _Right) const {
		return (_Left.orderId > _Right.orderId);
	}
};

struct CompareTickableEntity_Less {
	bool operator()(const TickableEntityItem& _Left, const TickableEntityItem& _Right) const {
		return (_Left.orderId < _Right.orderId);
	}
};

struct CompareDrawableEntity_Greater {
	bool operator()(const DrawableEntityItem& _Left, const DrawableEntityItem& _Right) const {
		return (_Left.orderId > _Right.orderId);
	}
};

struct CompareDrawableEntity_Less {
	bool operator()(const DrawableEntityItem& _Left, const DrawableEntityItem& _Right) const {
		return (_Left.orderId < _Right.orderId);
	}
};

class FunctHashEntityItem {
public:
	__xi size_t operator()(const EntityGid_t& input) const {
		return input;
	}

	__xi size_t operator()(const EntityPointerContainerItem& input) const {
		return input.gid;
	}

	__xi size_t operator()(const TickableEntityItem& input) const {
		return input.orderId.Order();
		//bug_on(uptr(input.orderId.Order()) & 15, "Unaligned entity pointer detected.");
		//return uptr(input.orderId.Order()) >> 4;
	}

	__xi size_t operator()(const DrawableEntityItem& input) const {
		return input.orderId.Order();
	}

	__xi size_t operator()(const EntityGidOrderPair& input) const {
		return input.m_fullSortOrder;
	}
};

typedef std::queue<EntityContainerEvent> EntityContainerEventQueue;

// Could also be defined with a 32-bit displacement from the base heap pointer, if entities
// are given a custom heap.
typedef std::unordered_set<EntityPointerContainerItem, FunctHashEntityItem, CompareEntityGid> EntityPointerContainer;


// --------------------------------------------------------------------------------------
//  TickableEntityContainer
// --------------------------------------------------------------------------------------
struct TickableEntityContainer {

	// HashedContainerType
	//   Resolves entity GID into the full "sorted gid" which is used to create the set.
	//   A single entity may have multiple draw or logic stages in a list (each must have
	//   a unique sort order), and so a multiset is used.

	typedef std::unordered_multimap<EntityGid_t, EntityGidOrderPair, FunctHashEntityItem>			HashedContainerType;
	typedef std::set<TickableEntityItem, CompareTickableEntity_Less>								OrderedContainerType;

	HashedContainerType			m_hashed;
//	OrderedContainerType		m_ordered;			// ordered by priority
	OrderedContainerType		m_ordered;			// ordered by priority
	EntityContainerEventQueue	m_evt_queue;
	int							m_max_container_size;
	int							m_iterator_mode;

	void		_Add			(const TickableEntityItem& entityInfo);
	void		Remove			(EntityGid_t entityGid, u32 order = (u32)-1);
	void		ExecEventQueue	();

	__ai void Add(int orderId, EntityGid_t entityGid, EntityFn_LogicTick* logic) {
		_Add( { MakeGidOrder(entityGid, orderId), logic } );
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
	typedef std::unordered_multimap<EntityGid_t, EntityGidOrderPair, FunctHashEntityItem>			HashedContainerType;
	typedef std::set<DrawableEntityItem, CompareDrawableEntity_Less>								OrderedContainerType;

	HashedContainerType				m_hashed;
	OrderedContainerType			m_ordered;

	// Add/Remove note:
	//  * modification of drawable entity list during draw is invalid.  It can only be modified from tick context/

	void	_Add		(const DrawableEntityItem& entity);
	void	Remove		(EntityGid_t entityGid, u32 order  = (u32)-1);

	__ai void Add(int orderId, EntityGid_t entityGid, EntityFn_Draw* draw) {
		_Add( { MakeGidOrder(entityGid, orderId), draw } );
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

inline EntityGidOrderPair MakeGidOrder(const EntityGid_t gid, u32 order)
{
	return EntityGidOrderPair().SetOrder(order).SetGid(gid);
}


extern void*			Entity_GlobalLookup		(EntityGid_t gid);
extern EntityGid_t		Entity_GlobalSpawn		(void* entity, const char* classname=nullptr);
extern void*			Entity_Malloc			(int size);

template< typename T >
inline T* Entity_PlacementNew(const char* classname)
{
	T* entity = new (Entity_Malloc(sizeof(T))) T;
	entity->m_gid = Entity_GlobalSpawn(entity, classname);
	return entity;
}

#define PlaceEntity(instance, ...)	\
	bug_on(instance.m_gid, "Entity has already been added to global spawn manager.");	\
	instance.m_gid = Entity_GlobalSpawn(&instance, #instance __VA_ARGS__)

#define NewEntity(type, ...)		Entity_PlacementNew<type>( #type __VA_ARGS__)

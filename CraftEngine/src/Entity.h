#pragma once

#include "x-types.h"
#include "x-stl.h"

#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <queue>

struct	EntityContainerEvent;

enum EntityContainerEvent_t
{
	ECEvt_EntityAdd,
	ECEvt_EntityRemove,
};

// EntityGid_t - Opaque type to avoid accidents during function overloading.
struct EntityGid_t
{
	u32		val;

	bool operator==(const EntityGid_t& right) const { return  val == right.val; }
	bool operator!=(const EntityGid_t& right) const { return  val != right.val; }
};

enum EntitySystemGID_t : u32 {
	ESGID_Empty		= 0,
};

struct EntityPointerContainerItem
{
	EntityGid_t		gid;
	void*			objectptr;
	char*			classname;		// alloc'd in same heap as entity
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

typedef void (EntityFn_LogicTick)	(		void* objdata, int order);
typedef void (EntityFn_Draw)		(const	void* objdata, float zorder);

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
	EntityGidOrderPair		orderGidPair;
	EntityFn_LogicTick*		Tick;
};

struct DrawableEntityItem
{
	EntityGidOrderPair		orderGidPair;
	EntityFn_Draw*			Draw;
};

// Needed only for TickableEntityContainer, due to OrderedDrawList being const iterator at all times.
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
		return (_Left.orderGidPair > _Right.orderGidPair);
	}
};

struct CompareTickableEntity_Less {
	bool operator()(const TickableEntityItem& _Left, const TickableEntityItem& _Right) const {
		return (_Left.orderGidPair < _Right.orderGidPair);
	}
};

struct CompareDrawableEntity_Greater {
	bool operator()(const DrawableEntityItem& _Left, const DrawableEntityItem& _Right) const {
		return (_Left.orderGidPair > _Right.orderGidPair);
	}
};

struct CompareDrawableEntity_Less {
	bool operator()(const DrawableEntityItem& _Left, const DrawableEntityItem& _Right) const {
		return (_Left.orderGidPair < _Right.orderGidPair);
	}
};

class FunctHashEntityItem {
public:
	__xi size_t operator()(const EntityGid_t& input) const {
		return input.val;
	}

	__xi size_t operator()(const EntityPointerContainerItem& input) const {
		return input.gid.val;
	}

	__xi size_t operator()(const TickableEntityItem& input) const {
		return input.orderGidPair.Order();
		//bug_on(uptr(input.orderGidPair.Order()) & 15, "Unaligned entity pointer detected.");
		//return uptr(input.orderGidPair.Order()) >> 4;
	}

	__xi size_t operator()(const DrawableEntityItem& input) const {
		return input.orderGidPair.Order();
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
	void		Clear			();
	void		ExecEventQueue	();

	__ai void Add(int orderGidPair, EntityGid_t entityGid, EntityFn_LogicTick* logic) {
		_Add( { MakeGidOrder(entityGid, orderGidPair), logic } );
	}

	template< typename T >
	__ai void Add(T* entity, int order) {
		Add(order, entity->m_gid, [](void* entity, int order) { ((T*)entity)->Tick(order); } );
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

	struct ForeachIfcForward
	{
		TickableEntityContainer*	m_entityList;

		ForeachIfcForward(TickableEntityContainer& src) {
			m_entityList = &src;
			m_entityList->EnterIteratorMode();
		}

		~ForeachIfcForward() throw()
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

	struct ForeachIfcReverse
	{
		TickableEntityContainer*	m_entityList;

		ForeachIfcReverse(TickableEntityContainer& src) {
			m_entityList = &src;
			m_entityList->EnterIteratorMode();
		}

		ForeachIfcReverse(ForeachIfcReverse&& rvalue)
		{
			m_entityList		= rvalue.m_entityList;
			rvalue.m_entityList = nullptr;
		}

		ForeachIfcReverse& operator=(ForeachIfcReverse&& rvalue)
		{
			std::swap(m_entityList, rvalue.m_entityList);
			return *this;
		}

		~ForeachIfcReverse() throw()
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

};


// --------------------------------------------------------------------------------------
//  OrderedDrawList
// --------------------------------------------------------------------------------------
// Draw are designed to be non-persistent: wiped before each Logic() update and then re-populated
// with new scene entities.  Lists are built for fast insert and fast ordered traversal.  Item
// removal is extremely slow.  Use drawing masking variables embedded into specific item data if
// you encounter situations where it's useful to remove objects in a draw list.
//
struct OrderedDrawList {
	struct DrawListItem
	{
		const void*			ObjectData;
		EntityFn_Draw*		DrawFunc;
	};

	typedef std::multimap<float, DrawListItem>	OrderedContainerType;

	// Intentionally lacks a hashed container.  Element removal is extremely slow for this reason.
	// If logic demands that an item in the draw list be removed after it has been added for some
	// reason, then the best strategy is to add a "drawing mask" value to the object that allows
	// it to skip drawing -- and then modify that.

	OrderedContainerType			m_ordered;
	float4							m_visibleArea;			// visible area/frustrum - in tile coords - for culling

	void		_Add			(const DrawListItem& entity, float zorder);
	void		Add				(EntityGid_t entityGid, float zorder, EntityFn_Draw* draw);
	void		Remove			(EntityGid_t entityGid, float order);
	void		Remove			(EntityGid_t entityGid);
	void		Remove			(void* objectData, float order);
	void		Remove			(void* objectData);
	void		Clear			();

	template< typename T >
	__ai void Add(const T* anyobj, float zorder, EntityFn_Draw* draw) {
		_Add( { anyobj, draw }, zorder );
	}

	// Use to bind any object, either managed by entity system or static/dynamic object managed by C++
	// (ideally use only for entities or static items, to avoid accidental delete-before-frameout problem)
	template< typename T >
	__ai void Add(const T* anyobj, float zorder) {
		Add( anyobj, zorder, [](const void* anyobj, float zorder) { ((T*)anyobj)->Draw(zorder); } );
	}

	auto ForEachOpaque	() const;
	auto ForEachAlpha	() const;

	struct ForeachIfcOpaque
	{
		const OrderedDrawList*	m_drawList;

		ForeachIfcOpaque(const OrderedDrawList& src) {
			m_drawList = &src;
		}

		__ai auto begin		()	const { return m_drawList->m_ordered.cbegin();	}
		__ai auto end		()	const { return m_drawList->m_ordered.cend();	}
		__ai auto cbegin	()	const { return m_drawList->m_ordered.cbegin();	}
		__ai auto cend		()	const { return m_drawList->m_ordered.cend();	}

	};

	struct ForeachIfcAlpha
	{
		const OrderedDrawList*	m_drawList;

		ForeachIfcAlpha(const OrderedDrawList& src) {
			m_drawList = &src;
		}

		__ai auto begin		()	const { return m_drawList->m_ordered.crbegin();	}
		__ai auto end		()	const { return m_drawList->m_ordered.crend();	}
		__ai auto cbegin	()	const { return m_drawList->m_ordered.crbegin();	}
		__ai auto cend		()	const { return m_drawList->m_ordered.crend();	}

	};
};

// ------------------------------------------------------------------------------------------------

inline auto TickableEntityContainer::ForEachForward()
{
	return ForeachIfcForward(*this);
}

inline auto TickableEntityContainer::ForEachReverse()
{
	return ForeachIfcReverse(*this);
}

inline auto OrderedDrawList::ForEachOpaque() const {
	return ForeachIfcOpaque(*this);
}

inline auto OrderedDrawList::ForEachAlpha() const {
	return ForeachIfcAlpha(*this);
}

inline EntityGidOrderPair MakeGidOrder(const EntityGid_t gid, u32 order)
{
	return EntityGidOrderPair().SetOrder(order).SetGid(gid);
}


extern const EntityPointerContainerItem*	Entity_TryLookup		(EntityGid_t gid);
extern const EntityPointerContainerItem&	Entity_Lookup			(EntityGid_t gid);
extern const char*							Entity_LookupName		(EntityGid_t gid);
extern void*								Entity_Remove			(EntityGid_t gid);
extern EntityGid_t							Entity_Spawn			(void* entity, const char* classname=nullptr);
extern void*								Entity_Malloc			(int size);

extern void									EntityManager_Reset		();

template< typename T >
inline T* NewEntityT(const char* classname)
{
	T* entity = new (Entity_Malloc(sizeof(T))) T;
	entity->m_gid = Entity_Spawn(entity, classname);
	return entity;
}

template<typename T>
T* Entity_LookupAs(EntityGid_t gid) {
	return (T*)Entity_Lookup(gid).objectptr;
}

#define PlaceEntity(instance, ...)	\
	Entity_Remove(instance.m_gid); instance.m_gid = { 0 };			\
	instance.m_gid = Entity_Spawn(&instance, #instance __VA_ARGS__)

// Notice:  heap-allocated entities from C++ are strongly discouraged, as a great deal
// of manual resource management is required.  And no, there's no magic-bullet fixfor that using
// shared_ptr<> or CComPtr<> or whatever else.  Just don't do it, folks.
//
// There will be LUA-GC based dynamic entity management which is more ideal to the on-the-fly
// spawner paradigm.

#define NewEntity(type, ...)		NewEntityT<type>( #type ## __VA_ARGS__)


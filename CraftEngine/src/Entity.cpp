#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"
#include "x-thread.h"
#include "x-atomic.h"

#include "Entity.h"

#include <unordered_set>

DECLARE_MODULE_NAME("Entity");


#define EntityLog(...)		log_host( __VA_ARGS__ )

static u32 s_entity_spawn_id = 1;

EntityPointerContainer	g_GlobalEntities;
EntityPointerContainer	g_GlobalEntityAlloc;

static EntityGid_t getNextSpawnId()
{
	u32 result = AtomicInc((s32&)s_entity_spawn_id);
	if (result == 0) {
		// wrapped around the world.  impressive.
		log_host("super-secret impossible easter egg found!");
		// but zero is an invalid spawn ID, so discard it:
		result = AtomicInc((s32&)s_entity_spawn_id);
	}
	return { result };
}


void* Entity_Malloc(int size)
{
	return xMalloc(size);
}

void EntityManager_Reset()
{
	// Remove this once global entity MSPACE is provided...
	for(auto& item : g_GlobalEntityAlloc) {
		xFree(item.objectptr);
	}

	// Remove this once global entity MSPACE is provided...
	for(auto& item : g_GlobalEntities) {
		xFree(item.classname);
	}

	g_GlobalEntities	.clear();
	g_GlobalEntityAlloc	.clear();

	// Wipe mspace here...
}

const EntityPointerContainerItem s_missing =
{
	ESGID_Empty,
	nullptr,
	"Missing"
};

const EntityPointerContainerItem* Entity_TryLookup(EntityGid_t gid)
{
	auto it = g_GlobalEntities.find( {gid, nullptr} );
	if (it == g_GlobalEntities.end()) return nullptr;
	return &(*it);
}

const EntityPointerContainerItem& Entity_Lookup(EntityGid_t gid)
{
	auto it = g_GlobalEntities.find( {gid, nullptr} );
	if (it == g_GlobalEntities.end()) return s_missing;
	return *it;
}

const char* Entity_LookupName(EntityGid_t gid)
{
	return Entity_Lookup(gid).classname;
}

void* Entity_Remove(EntityGid_t gid)
{
	auto it = g_GlobalEntities.find( {gid, nullptr} );
	if (it == g_GlobalEntities.end()) return nullptr;
	void* retval = it->objectptr;
	g_GlobalEntities.erase(it);
	return retval;
}

EntityGid_t Entity_Spawn(void* entity, const char* classname)
{
	char* namedup = nullptr;
	auto  namelen = strlen(classname);
	if (namelen) {
		namedup = (char*)xMalloc(namelen+1);
		strcpy(namedup, classname);
	}

	EntityPointerContainerItem item = {0, entity, namedup};
	for (;;) {
		item.gid = getNextSpawnId();
		if (g_GlobalEntities.find(item) != g_GlobalEntities.end()) {
			continue;
		}
		g_GlobalEntities.insert(item);
		return item.gid;
	}
}

void TickableEntityContainer::ExecEventQueue()
{
	//todo( "implement me!" );
	//m_evt_queue
}

void TickableEntityContainer::Clear()
{
	m_hashed	.clear();
	m_ordered	.clear();

	// also clears memory which I'm not too thrilled aobut, but std::queue has no clear..
	m_evt_queue	= {};
}

void TickableEntityContainer::_Add(const TickableEntityItem& entityInfo)
{
	if (!m_iterator_mode) {
		m_hashed	.insert( { entityInfo.orderGidPair.Gid(), entityInfo.orderGidPair } );
		m_ordered	.insert(entityInfo);
	}
	else {
		m_evt_queue.push( { ECEvt_EntityAdd, entityInfo } );
	}
}

void TickableEntityContainer::Remove(EntityGid_t entityGid, u32 order)
{
	auto hashit = m_hashed.equal_range( entityGid );

	if (hashit.first == m_hashed.end()) {
		// item not found... ?
		return;
	}

	if (!m_iterator_mode) {
		for (auto it=hashit.first; it!=hashit.second; ++it) {
			if (order != (u32)-1) {
				if (it->second.Order() != order) {
					continue;
				}
			}
			// don't care about function ptr, it's not checked as part of set comparison operation.
			m_ordered.erase({ it->second, nullptr });
			m_hashed.erase(it);
		}
	}
	else {
		m_evt_queue.push({ ECEvt_EntityRemove, { MakeGidOrder(entityGid, order), nullptr } });
	}
}

// todo: m_hashed is only needed for persistent-style entity lists, where quick removal is important.
// Draw lists will instead be non-persistent, wiped before each Logic() update.  Therefore this container
// doesn't need fast removal capability.  Using a hashed set should still be fine for performance.
// On theory a linked list might be slightly faster for sorted insertion... but they have a lot of other
// drawbacks so let's stick to the set unless it becomes a problem.  --jstine

void OrderedDrawList::_Add(const DrawListItem& entityInfo, float zorder)
{
	m_ordered.insert({ zorder, entityInfo });
}

void OrderedDrawList::Add(EntityGid_t entityGid, float zorder, EntityFn_Draw* draw) {
	auto entityPtr = Entity_Lookup(entityGid).objectptr;
	if (entityPtr) {
		_Add( { entityPtr, draw }, zorder );
	}
	else {
		EntityLog("DrawList: ignoring nil entity, gid=0x%08x.");
	}
}

// OrderedDrawList::Remove (all variants):
//   Impl dependent erase, should be logarithmic (bisect search).

// SLOW!  Do not use, except for rapid iteration or debugging purpose.
void OrderedDrawList::Remove(EntityGid_t entityGid, float order)
{
	auto entityPtr = Entity_Lookup(entityGid).objectptr;
	if (entityPtr) {
		Remove(entityPtr, order);
	}
}

void OrderedDrawList::Remove(void* objectData, float order)
{
	auto itpair = m_ordered.equal_range(order);
	auto it = itpair.first;
	while (it != m_ordered.end() && it != itpair.second) {
		if (it->second.ObjectData == objectData) {
			it = m_ordered.erase(it);
		}
		else {
			++it;
		}
	}
}

void OrderedDrawList::Remove(EntityGid_t entityGid)
{
	auto entityPtr = Entity_Lookup(entityGid).objectptr;
	if (entityPtr) {
		Remove(entityPtr);
	}
}

void OrderedDrawList::Remove(void* objectData)
{
	auto it = m_ordered.begin();
	while (it != m_ordered.end()) {
		if (it->second.ObjectData == objectData) {
			it = m_ordered.erase(it);
		}
		else {
			++it;
		}
	}
}


void OrderedDrawList::Clear()
{
	m_ordered	.clear();
}
#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"
#include "x-thread.h"
#include "x-atomic.h"

#include "Entity.h"

#include <unordered_set>

// Entity Engineering Thoughts:
//   Currently supporting managed and unmanaged entities for sake of completeness.  Unmnaged entities
//   are typically static (persisitent) objects within C++ for which pointers are always valid.  There's
//   actually no good motivation currently to even have these as part of the "entity system."  I'm going
//   to leave the managed flag in for now tho, in case some use case crops up.  --jstine
//
//   Entity lookup by gid_t can be optimized considerably using an associative cache.
//      * create a fixed-sized array large enough to handle most entities in a scene (64k), containing:
//               gid_t, pointer to entity
//      * do lookup using gid_t & (cache_size-1)
//               if (cache[gid_masked] == gid) { return result; } else { do_full_lookup(); }

#define EntityLog(...)      log_host( __VA_ARGS__ )

static u32 s_entity_spawn_id = 1;

EntityPointerContainer  g_GlobalEntities;
EntityNameAssociator    g_EntitiesByName;


static std::vector<EntityGid_t> s_DeletedEntities;

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

static void freeManagedEntity(EntityPointerContainerItem& item)
{
    xFree(item.classname);
    if (item.managed) {
        xFree(item.objectptr);
    }
}

void EntityManager_Reset()
{
    // Remove this once global entity MSPACE is provided...
    for(auto& item : g_GlobalEntities) {
        freeManagedEntity(item.second);
    }

    g_GlobalEntities.clear();
    g_EntitiesByName.clear();
    // Wipe mspace here...
}

void EntityManager_CollectGarbage()
{
    for (const auto& gid : s_DeletedEntities)
    {
        auto it = g_GlobalEntities.find( gid );
        if (it == g_GlobalEntities.end()) continue;
        freeManagedEntity(it->second);
        g_GlobalEntities.erase(it);
    }
    s_DeletedEntities.clear();
}

const EntityPointerContainerItem s_missing =
{
    ESGID_Empty,
    nullptr,
    "Missing",
    false,
    false
};

u32 _getStringHash(const char* name, int length, u32 curhash=0)
{
    u32 result = curhash;
    const char* c = name;
    int i=0;
    for(; i<length/8; i += 4) {
        result = i_crc32(result, (u32&)c[i]);
    }

    for(; i<length; i += 1) {
        result = i_crc32(result, (u8&)c[i]);
    }

    return result;
}

__xi const EntityPointerContainerItem* _impl_entity_TryLookup(const char* name, int length)
{
    auto result = _getStringHash(name, length);
    auto it1 = g_EntitiesByName.find(result);
    auto it2 = g_GlobalEntities.find(it1->second);

    if (it2 == g_GlobalEntities.end()) return nullptr;
    return &it2->second;
}

const EntityPointerContainerItem& _impl_entity_Lookup(const char* name, int length)
{
    auto* result = _impl_entity_TryLookup(name, length);
    if (!result) return s_missing;
    return *result;
}

const EntityPointerContainerItem* Entity_TryLookup(EntityGid_t gid)
{
    auto it = g_GlobalEntities.find( gid );
    if (it == g_GlobalEntities.end()) return nullptr;
    return &it->second;
}

const EntityPointerContainerItem& Entity_Lookup(const xString& name)
{
    return _impl_entity_Lookup(name, name.GetLength());
}

const EntityPointerContainerItem* Entity_TryLookup(const xString& name)
{
    return _impl_entity_TryLookup(name, name.GetLength());
}

const EntityPointerContainerItem& Entity_Lookup(EntityGid_t gid)
{
    auto it = g_GlobalEntities.find( gid );
    if (it == g_GlobalEntities.end()) return s_missing;
    return it->second;
}

const char* Entity_LookupName(EntityGid_t gid)
{
    return Entity_Lookup(gid).classname;
}

void Entity_Remove(EntityGid_t gid)
{
    auto it = g_GlobalEntities.find( gid );
    if (it == g_GlobalEntities.end()) return;
    if (it->second.managed) {
        s_DeletedEntities.push_back(gid);
        it->second.deleted = 1;
    }
}

EntityGid_t _impl_Entity_Spawn(EntityPointerContainerItem& item, const char* classname)
{
    // TODO - change asManaged to a proper enum type.

    char* namedup = nullptr;
    auto  namelen = strlen(classname);
    if (namelen) {
        namedup = (char*)xMalloc(namelen+1);
        strcpy(namedup, classname);
    }

    item.classname = namedup;

    for (;;) {
        item.gid = getNextSpawnId();
        if (g_GlobalEntities.find(item.gid) != g_GlobalEntities.end()) {
            continue;
        }
        auto namehash = _getStringHash(classname, namelen);
        g_GlobalEntities.insert({item.gid, item});
        g_EntitiesByName.insert({namehash, item.gid});
        return item.gid;
    }
}

void Entity_AddUnmanaged(EntityGid_t& gid, void* entity, const char* classname)
{
    Entity_Remove(gid); gid = { 0 };
    EntityPointerContainerItem item;
    item.gid            = { 0 };
    item.objectptr      = entity;
    item.managed        = 0;
    item.deleted        = 0;
    gid = _impl_Entity_Spawn(item, classname);
}

EntityGid_t Entity_AddManaged(void* entity, const char* classname)
{
    EntityPointerContainerItem item;
    item.gid            = { 0 };
    item.objectptr      = entity;
    item.managed        = 1;
    item.deleted        = 0;
    return _impl_Entity_Spawn(item, classname);
}

void TickableEntityContainer::ExecEventQueue()
{
    //todo( "implement me!" );
    //m_evt_queue
}

void TickableEntityContainer::Clear()
{
    m_hashed    .clear();
    m_ordered   .clear();

    // also clears memory which I'm not too thrilled aobut, but std::queue has no clear..
    m_evt_queue = {};
}

void TickableEntityContainer::_Add(const TickableEntityItem& entityInfo)
{
    if (!m_iterator_mode) {
        m_hashed    .insert( { entityInfo.orderGidPair.Gid(), entityInfo.orderGidPair } );
        m_ordered   .insert(entityInfo);
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
        EntityLog("DrawList: ignoring nil entity, gid=0x%08x.", entityGid.val);
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
    m_ordered   .clear();
}

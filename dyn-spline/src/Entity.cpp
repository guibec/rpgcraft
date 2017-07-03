
#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"
#include "x-thread.h"

#include "Entity.h"


void TickableEntityContainer::ExecEventQueue()
{
	//todo( "implement me!" );
	//m_evt_queue
}

void TickableEntityContainer::_Add(const TickableEntityItem& entityInfo)
{
	if (!m_iterator_mode) {
		m_hashed	.insert(entityInfo);
		m_ordered	.insert(entityInfo);
	}
	else {
		m_evt_queue.push( { ECEvt_EntityAdd, entityInfo } );
	}
}

void TickableEntityContainer::Remove(ITickableEntity* entity)
{
	auto hashit = m_hashed.find({ entity, SaltedOrderId() });

	if (hashit == m_hashed.end()) {
		// item not found... ?
		return;
	}

	if (!m_iterator_mode) {

		m_ordered.erase(*hashit);
		m_hashed.erase(hashit);
	}
	else {
		m_evt_queue.push( { ECEvt_EntityRemove, TickableEntityItem { entity, hashit->tickOrderSalted } } );
	}
}

void DrawableEntityContainer::_Add(const DrawableEntityItem& entityInfo)
{
	m_hashed	.insert(entityInfo);
	m_ordered	.insert(entityInfo);
}

void DrawableEntityContainer::Remove(const IDrawableEntity* entity)
{
	auto hashit = m_hashed.find({ entity, SaltedOrderId() });

	if (hashit == m_hashed.end()) {
		// item not found... ?
		return;
	}

	m_ordered.erase(*hashit);
	m_hashed.erase(hashit);
}

#pragma once

#include "x-types.h"
#include "x-stl.h"

#include <set>

class ITickableEntity
{
public:
	float			TickOrder;

public:
	virtual void	Tick()=0;
};

class IDrawableEntity
{
public:
	float			DrawOrder;

public:
	virtual void	Draw() const=0;
};

struct CompareTickableEntity_Greater {
	bool operator()(const ITickableEntity* const& _Left, const ITickableEntity* const& _Right) const {
		return (_Left->TickOrder > _Right->TickOrder);
	}
};

struct CompareTickableEntity_Less {
	bool operator()(const ITickableEntity* const& _Left, const ITickableEntity* const& _Right) const {
		return (_Left->TickOrder < _Right->TickOrder);
	}
};

struct CompareDrawableEntity_Greater {
	bool operator()(const IDrawableEntity* const& _Left, const IDrawableEntity* const& _Right) const {
		return (_Left->DrawOrder > _Right->DrawOrder);
	}
};

struct CompareDrawableEntity_Less {
	bool operator()(const IDrawableEntity* const& _Left, const IDrawableEntity* const& _Right) const {
		return (_Left->DrawOrder < _Right->DrawOrder);
	}
};

// Sorted by TickOrder
template< typename SortPred >
struct TickableEntityContainerTmpl {
	typedef std::multiset<ITickableEntity*, SortPred> StlContainerType;

	StlContainerType	m_entityList;

	pragma_todo("Add priority_queue to proxy addition and removal of entities during container list processing")

	auto	Add			(ITickableEntity*		entity)	{ return m_entityList.insert(entity); }
	auto	Remove		(const ITickableEntity* entity) { return m_entityList.erase (entity); }

	// -----------------------------------------------------------------------
	// `for each()` interface implementation
	//
	// DevNote: If we hide these behind a scoped interface, then we can use the scoped ifc to automatically
	// invoke usage of the queue proxy for list modifications and/or bug on other bad behavior during list
	// processing (via checking list for consistency when the object leaves scope).

	__ai auto		begin	()			{ return m_entityList.begin();	}
	__ai auto		end		()			{ return m_entityList.end();	}

	__ai auto		cbegin	()	const	{ return m_entityList.cbegin();	}
	__ai auto		cend	()	const	{ return m_entityList.cend();	}
	// -----------------------------------------------------------------------
};

// Sorted by DrawOrder
template< typename SortPred >
struct DrawableEntityContainerTmpl {
	typedef std::multiset<const IDrawableEntity*, SortPred> StlContainerType;

	StlContainerType	m_entityList;

	auto	Add			(const IDrawableEntity* entity)	{ return m_entityList.insert(entity); }
	auto	Remove		(const IDrawableEntity* entity) { return m_entityList.erase (entity); }

	// -----------------------------------------------------------------------
	// `for each()` interface implementation

	__ai auto		begin		()			{ return m_entityList.begin();		}
	__ai auto		end			()			{ return m_entityList.end();		}
	__ai auto		rbegin		()			{ return m_entityList.rbegin();		}
	__ai auto		rend		()			{ return m_entityList.rend();		}

	__ai auto		cbegin		()	const	{ return m_entityList.cbegin();		}
	__ai auto		cend		()	const	{ return m_entityList.cend();		}

	__ai auto		crbegin		()	const	{ return m_entityList.crbegin();	}
	__ai auto		crend		()	const	{ return m_entityList.crend();		}
	// -----------------------------------------------------------------------

	auto ForEachAlpha() const;
};

typedef TickableEntityContainerTmpl<CompareTickableEntity_Less>		TickableEntityContainer;
typedef DrawableEntityContainerTmpl<CompareDrawableEntity_Less>		DrawableEntityContainer;


struct DrawableEntityForeachIfc
{
	const DrawableEntityContainer*	m_entityList;

	DrawableEntityForeachIfc(const DrawableEntityContainer& src) {
		m_entityList = &src;
	}

	__ai auto begin		()	const { return m_entityList->crbegin();		}
	__ai auto end		()	const { return m_entityList->crend();		}
	__ai auto cbegin	()	const { return m_entityList->crbegin();		}
	__ai auto cend		()	const { return m_entityList->crend();		}

};

template< typename SortPred >
auto DrawableEntityContainerTmpl<SortPred>::ForEachAlpha() const {
	return DrawableEntityForeachIfc(*this);
}

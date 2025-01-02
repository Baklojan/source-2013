//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Filters are outboard entities that hold a set of rules that other
//			entities can use to determine behaviors.
//			
//			For example, triggers can use an activator filter to determine who
//			activates them. NPCs and breakables can use a damage filter to
//			determine what can damage them.
//
//			Current filter criteria are:
//
//				Activator name
//				Activator class
//				Activator team
//				Damage type (for damage filters only)
//
//			More than one filter can be combined to create a more complex boolean
//			expression by using filter_multi.
//
//=============================================================================//

#ifndef FILTERS_H
#define FILTERS_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "entityoutput.h"

// ###################################################################
//	> BaseFilter
// ###################################################################
class CBaseFilter : public CLogicalEntity
{
	DECLARE_CLASS( CBaseFilter, CLogicalEntity );

public:

	DECLARE_DATADESC();

	bool PassesFilter( CBaseEntity *pCaller, CBaseEntity *pEntity );
	bool PassesDamageFilter( const CTakeDamageInfo &info );

	bool m_bNegated;

	// Inputs
	void InputTestActivator( inputdata_t &inputdata );

	// Outputs
	COutputEvent	m_OnPass;		// Fired when filter is passed
	COutputEvent	m_OnFail;		// Fired when filter is failed

protected:

	virtual bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity );
	virtual bool PassesDamageFilterImpl( const CTakeDamageInfo &info );
};


// ###################################################################
//	> FilterMultiple
//
//   Allows one to filter through mutiple filters
// ###################################################################
#define MAX_FILTERS 5
enum filter_t
{
	FILTER_AND,
	FILTER_OR,
};

class CFilterMultiple : public CBaseFilter
{
	DECLARE_CLASS( CFilterMultiple, CBaseFilter );
	DECLARE_DATADESC();

	filter_t	m_nFilterType;
	string_t	m_iFilterName[MAX_FILTERS];
	EHANDLE		m_hFilter[MAX_FILTERS];

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity );
	bool PassesDamageFilterImpl( const CTakeDamageInfo &info );
	void Activate( void );
};


// ###################################################################
//	> FilterName
// ###################################################################
class CFilterName : public CBaseFilter
{
	DECLARE_CLASS( CFilterName, CBaseFilter );
	DECLARE_DATADESC();

public:
	string_t m_iFilterName;

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		// special check for !player as GetEntityName for player won't return "!player" as a name
		if ( FStrEq( STRING( m_iFilterName ), "!player" ) )
		{
			return pEntity->IsPlayer();
		}
		else
		{
			return pEntity->NameMatches( STRING( m_iFilterName ) );
		}
	}
};


// ###################################################################
//	> FilterClass
// ###################################################################
class CFilterClass : public CBaseFilter
{
	DECLARE_CLASS( CFilterClass, CBaseFilter );
	DECLARE_DATADESC();

public:
	string_t m_iFilterClass;

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		return pEntity->ClassMatches( STRING( m_iFilterClass ) );
	}
};


// ###################################################################
//	> FilterTeam
// ###################################################################
class FilterTeam : public CBaseFilter
{
	DECLARE_CLASS( FilterTeam, CBaseFilter );
	DECLARE_DATADESC();

public:
	int		m_iFilterTeam;

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		return ( pEntity->GetTeamNumber() == m_iFilterTeam );
	}
};


// ###################################################################
//	> FilterMassGreater
// ###################################################################
class CFilterMassGreater : public CBaseFilter
{
	DECLARE_CLASS( CFilterMassGreater, CBaseFilter );
	DECLARE_DATADESC();

public:
	float m_fFilterMass;

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		if ( pEntity->VPhysicsGetObject() == NULL )
			return false;

		return ( pEntity->VPhysicsGetObject()->GetMass() > m_fFilterMass );
	}
};


// ###################################################################
//	> FilterDamageType
// ###################################################################
class FilterDamageType : public CBaseFilter
{
	DECLARE_CLASS( FilterDamageType, CBaseFilter );
	DECLARE_DATADESC();

protected:

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		ASSERT( false );
		return true;
	}

	bool PassesDamageFilterImpl( const CTakeDamageInfo &info )
	{
		//Tony; these are bitflags. check them as so.
		return ( ( info.GetDamageType() & m_iDamageType ) == m_iDamageType );
	}

	int m_iDamageType;
};


// ###################################################################
//	> CFilterEnemy
// ###################################################################

#define SF_FILTER_ENEMY_NO_LOSE_AQUIRED	(1<<0)

class CFilterEnemy : public CBaseFilter
{
	DECLARE_CLASS( CFilterEnemy, CBaseFilter );
	// NOT SAVED	
	// m_iszPlayerName
	DECLARE_DATADESC();

public:

	virtual bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity );
	virtual bool PassesDamageFilterImpl( const CTakeDamageInfo &info );

private:

	bool	PassesNameFilter( CBaseEntity *pCaller );
	bool	PassesProximityFilter( CBaseEntity *pCaller, CBaseEntity *pEnemy );
	bool	PassesMobbedFilter( CBaseEntity *pCaller, CBaseEntity *pEnemy );

	string_t	m_iszEnemyName;				// Name or classname
	float		m_flRadius;					// Radius (enemies are acquired at this range)
	float		m_flOuterRadius;			// Outer radius (enemies are LOST at this range)
	int		m_nMaxSquadmatesPerEnemy;	// Maximum number of squadmates who may share the same enemy
	string_t	m_iszPlayerName;			// "!player"
};

#endif // FILTERS_H
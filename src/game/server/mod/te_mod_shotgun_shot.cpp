//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "basetempentity.h"


#define NUM_BULLET_SEED_BITS 8


//-----------------------------------------------------------------------------
// Purpose: Display's a blood sprite
//-----------------------------------------------------------------------------
class CTEModFireBullets : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEModFireBullets, CBaseTempEntity );
	DECLARE_SERVERCLASS();

					CTEModFireBullets( const char *name );
	virtual			~CTEModFireBullets( void );

public:
	CNetworkVar( int, m_iPlayer );
	CNetworkVector( m_vecOrigin );
	CNetworkVector( m_vecDir );
	CNetworkVar( int, m_iAmmoID );
	CNetworkVar( int, m_iSeed );
	CNetworkVar( int, m_iShots );
	CNetworkVar( float, m_flSpread );
	CNetworkVar( bool, m_bDoImpacts );
	CNetworkVar( bool, m_bDoTracers );
};

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
//-----------------------------------------------------------------------------
CTEModFireBullets::CTEModFireBullets( const char *name ) :
	CBaseTempEntity( name )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTEModFireBullets::~CTEModFireBullets( void )
{
}

IMPLEMENT_SERVERCLASS_ST_NOBASE(CTEModFireBullets, DT_TEModFireBullets)
	SendPropVector( SENDINFO(m_vecOrigin), -1, SPROP_COORD ),
	SendPropVector( SENDINFO(m_vecDir), -1 ),
	SendPropInt( SENDINFO( m_iAmmoID ), 5, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iSeed ), NUM_BULLET_SEED_BITS, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iShots ), 5, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iPlayer ), 6, SPROP_UNSIGNED ), 	// max 64 players, see MAX_PLAYERS
	SendPropFloat( SENDINFO( m_flSpread ), 10, 0, 0, 1 ),	
	SendPropBool( SENDINFO( m_bDoImpacts ) ),
	SendPropBool( SENDINFO( m_bDoTracers ) ),
END_SEND_TABLE()


// Singleton
static CTEModFireBullets g_TEModFireBullets( "Shotgun Shot" );


void TE_HL2MPFireBullets( 
	int	iPlayerIndex,
	const Vector &vOrigin,
	const Vector &vDir,
	int	iAmmoID,
	int iSeed,
	int iShots,
	float flSpread,
	bool bDoTracers,
	bool bDoImpacts )
{
	CPASFilter filter( vOrigin );
	filter.UsePredictionRules();

	g_TEModFireBullets.m_iPlayer = iPlayerIndex;
	g_TEModFireBullets.m_vecOrigin = vOrigin;
	g_TEModFireBullets.m_vecDir = vDir;
	g_TEModFireBullets.m_iSeed = iSeed;
	g_TEModFireBullets.m_iShots = iShots;
	g_TEModFireBullets.m_flSpread = flSpread;
	g_TEModFireBullets.m_iAmmoID = iAmmoID;
	g_TEModFireBullets.m_bDoTracers = bDoTracers;
	g_TEModFireBullets.m_bDoImpacts = bDoImpacts;
	
	Assert( iSeed < (1 << NUM_BULLET_SEED_BITS) );
	
	g_TEModFireBullets.Create( filter, 0 );
}

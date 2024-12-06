//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#ifndef BASEMODCOMBATWEAPON_SHARED_H
#define BASEMODCOMBATWEAPON_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
	#include "c_mod_player.h"
#else
	#include "mod_player.h"
#endif

#include "weapon_basemod.h"

#if defined( CLIENT_DLL )
#define CBaseModCombatWeapon C_BaseModCombatWeapon
#endif

class CBaseModCombatWeapon : public CWeaponModBase
{
#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif

	DECLARE_CLASS( CBaseModCombatWeapon, CWeaponModBase );
public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CBaseModCombatWeapon();

	virtual bool	WeaponShouldBeLowered( void );

	virtual bool	Ready( void );
	virtual bool	Lower( void );
	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	WeaponIdle( void );

	virtual void	AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles );
	virtual	float	CalcViewmodelBob( void );

	virtual Vector	GetBulletSpread( WeaponProficiency_t proficiency );
	virtual float	GetSpreadBias( WeaponProficiency_t proficiency );

	virtual const	WeaponProficiencyInfo_t *GetProficiencyValues();
	static const	WeaponProficiencyInfo_t *GetDefaultProficiencyValues();

	virtual void	ItemHolsterFrame( void );

protected:

	bool			m_bLowered;			// Whether the viewmodel is raised or lowered
	float			m_flRaiseTime;		// If lowered, the time we should raise the viewmodel
	float			m_flHolsterTime;	// When the weapon was holstered

private:
	
	CBaseModCombatWeapon( const CBaseModCombatWeapon & );
};

#endif // WEAPON_BASEMODCOMBATWEAPON_SHARED_H

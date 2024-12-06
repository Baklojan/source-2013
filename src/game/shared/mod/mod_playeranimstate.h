//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef mod_playerANIMSTATE_H
#define mod_playerANIMSTATE_H
#ifdef _WIN32
#pragma once
#endif


#include "convar.h"
#include "multiplayer_animstate.h"

#if defined( CLIENT_DLL )
class C_Mod_Player;
#define CMod_Player C_Mod_Player
#else
class CMod_Player;
#endif

// ------------------------------------------------------------------------------------------------ //
// CPlayerAnimState declaration.
// ------------------------------------------------------------------------------------------------ //
class CModPlayerAnimState : public CMultiPlayerAnimState
{
public:
	
	DECLARE_CLASS( CModPlayerAnimState, CMultiPlayerAnimState );

	CModPlayerAnimState();
	CModPlayerAnimState( CBasePlayer *pPlayer, MultiPlayerMovementData_t &movementData );
	~CModPlayerAnimState();

	void InitModAnimState( CMod_Player *pPlayer );
	CMod_Player *GetModPlayer( void )							{ return m_pModPlayer; }

	virtual void ClearAnimationState();
	virtual Activity TranslateActivity( Activity actDesired );
	virtual void Update( float eyeYaw, float eyePitch );

	void	DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );

	bool	HandleMoving( Activity &idealActivity );
	bool	HandleJumping( Activity &idealActivity );
	bool	HandleDucking( Activity &idealActivity );
	bool	HandleSwimming( Activity &idealActivity );

	virtual float GetCurrentMaxGroundSpeed();

private:

	bool						SetupPoseParameters( CStudioHdr *pStudioHdr );
	virtual void				EstimateYaw( void );
	virtual void				ComputePoseParam_MoveYaw( CStudioHdr *pStudioHdr );
	virtual void				ComputePoseParam_AimPitch( CStudioHdr *pStudioHdr );
	virtual void				ComputePoseParam_AimYaw( CStudioHdr *pStudioHdr );
	
	CMod_Player   *m_pModPlayer;
	bool		m_bInAirWalk;
	float		m_flHoldDeployedPoseUntilTime;
};

CModPlayerAnimState *CreateModPlayerAnimState( CMod_Player *pPlayer );



#endif // mod_playerANIMSTATE_H

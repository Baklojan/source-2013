//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "flashlighteffect.h"
#include "dlight.h"
#include "iefx.h"
#include "iviewrender.h"
#include "view.h"
#include "engine/ivdebugoverlay.h"
#include "tier0/vprof.h"
#include "tier1/KeyValues.h"
#include "toolframework_client.h"

#ifdef HL2_CLIENT_DLL
#include "c_basehlplayer.h"
#endif // HL2_CLIENT_DLL

extern ConVar r_flashlightdepthres;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar r_flashlightdepthtexture;

static ConVar r_swingflashlight( "r_swingflashlight", "1", FCVAR_CHEAT );
static ConVar r_flashlightlockposition( "r_flashlightlockposition", "0", FCVAR_CHEAT );
static ConVar r_flashlightfov( "r_flashlightfov", "45.0", FCVAR_CHEAT );
static ConVar r_flashlightoffsetx( "r_flashlightoffsetx", "10.0", FCVAR_CHEAT );
static ConVar r_flashlightoffsety( "r_flashlightoffsety", "-20.0", FCVAR_CHEAT );
static ConVar r_flashlightoffsetz( "r_flashlightoffsetz", "24.0", FCVAR_CHEAT );
static ConVar r_flashlightnear( "r_flashlightnear", "4.0", FCVAR_CHEAT );
static ConVar r_flashlightfar( "r_flashlightfar", "750.0", FCVAR_CHEAT );
static ConVar r_flashlightconstant( "r_flashlightconstant", "0.0", FCVAR_CHEAT );
static ConVar r_flashlightlinear( "r_flashlightlinear", "100.0", FCVAR_CHEAT );
static ConVar r_flashlightquadratic( "r_flashlightquadratic", "0.0", FCVAR_CHEAT );
static ConVar r_flashlightvisualizetrace( "r_flashlightvisualizetrace", "0", FCVAR_CHEAT );
static ConVar r_flashlightambient( "r_flashlightambient", "0.0", FCVAR_CHEAT );
static ConVar r_flashlightshadowatten( "r_flashlightshadowatten", "0.35", FCVAR_CHEAT );
static ConVar r_flashlightladderdist( "r_flashlightladderdist", "40.0", FCVAR_CHEAT );

static ConVar r_flashlightnearoffsetscale( "r_flashlightnearoffsetscale", "1.0", FCVAR_CHEAT );
static ConVar r_flashlighttracedistcutoff( "r_flashlighttracedistcutoff", "128" );
static ConVar r_flashlightbacktraceoffset( "r_flashlightbacktraceoffset", "0.4", FCVAR_CHEAT );

static ConVar mat_slopescaledepthbias_shadowmap( "mat_slopescaledepthbias_shadowmap", "16", FCVAR_CHEAT );
static ConVar mat_depthbias_shadowmap(	"mat_depthbias_shadowmap", "0.0005", FCVAR_CHEAT  );

ClientShadowHandle_t g_hFlashlightHandle = CLIENTSHADOW_INVALID_HANDLE;

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nEntIndex - The m_nEntIndex of the client entity that is creating us.
//			vecPos - The position of the light emitter.
//			vecDir - The direction of the light emission.
//-----------------------------------------------------------------------------
CFlashlightEffect::CFlashlightEffect(int nEntIndex)
{
	m_FlashlightHandle = CLIENTSHADOW_INVALID_HANDLE;
	m_nEntIndex = nEntIndex;

	m_flCurrentPullBackDist = 1.0f;

	m_bIsOn = false;
	m_pPointLight = NULL;

	if ( g_pMaterialSystemHardwareConfig->SupportsBorderColor() )
	{
		m_FlashlightTexture.Init( "effects/flashlight_border", TEXTURE_GROUP_OTHER, true );
	}
	else
	{
		m_FlashlightTexture.Init( "effects/flashlight001", TEXTURE_GROUP_OTHER, true );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFlashlightEffect::~CFlashlightEffect()
{
	LightOff();

	g_hFlashlightHandle = CLIENTSHADOW_INVALID_HANDLE;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFlashlightEffect::TurnOn()
{
	m_bIsOn = true;
	m_flCurrentPullBackDist = 1.0f;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFlashlightEffect::TurnOff()
{
	if (m_bIsOn)
	{
		m_bIsOn = false;
		LightOff();
	}
}

// Custom trace filter that skips the player and the view model.
// If we don't do this, we'll end up having the light right in front of us all
// the time.
class CTraceFilterSkipPlayerAndViewModel : public CTraceFilter
{
public:
	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		// Test against the vehicle too?
		// FLASHLIGHTFIXME: how do you know that you are actually inside of the vehicle?
		C_BaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );
		if ( !pEntity )
			return true;

		if ( ( dynamic_cast<C_BaseViewModel *>( pEntity ) != NULL ) ||
			 ( dynamic_cast<C_BasePlayer *>( pEntity ) != NULL ) ||
			 pEntity->GetCollisionGroup() == COLLISION_GROUP_DEBRIS ||
			 pEntity->GetCollisionGroup() == COLLISION_GROUP_INTERACTIVE_DEBRIS )
		{
			return false;
		}

		return true;
	}
};

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void CFlashlightEffect::UpdateLight( const Vector &vecPos, const Vector &vecForward, const Vector &vecRight, const Vector &vecUp, bool bTracePlayers )
{
	VPROF_BUDGET( __FUNCTION__, VPROF_BUDGETGROUP_SHADOW_DEPTH_TEXTURING );

	FlashlightState_t state;
	
	if ( UpdateDefaultFlashlightState( state, vecPos, vecForward, vecRight, vecUp, bTracePlayers ) == false )
	{
		return;
	}
	/*if (r_swingflashlight.GetBool())
	{
		// This projects the view direction backwards, attempting to raise the vertical
		// offset of the flashlight, but only when the player is looking down.
		Vector vecSwingLight = vecPos + vecForward * -12.0f;
		if( vecSwingLight.z > vecPos.z )
		{
			flOffsetY += (vecSwingLight.z - vecPos.z);
		}
	}*/

	if( m_FlashlightHandle == CLIENTSHADOW_INVALID_HANDLE )
	{
		m_FlashlightHandle = g_pClientShadowMgr->CreateFlashlight( state );
	}
	else
	{
		if( !r_flashlightlockposition.GetBool() )
		{
			g_pClientShadowMgr->UpdateFlashlightState( m_FlashlightHandle, state );
		}
	}
	
	g_pClientShadowMgr->UpdateProjectedTexture( m_FlashlightHandle, true );
	
	g_hFlashlightHandle = m_FlashlightHandle;

#ifndef NO_TOOLFRAMEWORK
	if ( clienttools->IsInRecordingMode() )
	{
		KeyValues *msg = new KeyValues( "FlashlightState" );
		msg->SetFloat( "time", gpGlobals->curtime );
		msg->SetInt( "entindex", m_nEntIndex );
		msg->SetInt( "flashlightHandle", m_FlashlightHandle );
		msg->SetPtr( "flashlightState", &state );
		ToolFramework_PostToolMessage( HTOOLHANDLE_INVALID, msg );
		msg->deleteThis();
	}
#endif
}

bool CFlashlightEffect::UpdateDefaultFlashlightState( FlashlightState_t& state, const Vector &vecPos, const Vector &vecForward, 
														const Vector &vecRight, const Vector &vecUp, bool bTracePlayers )
{
	VPROF_BUDGET( __FUNCTION__, VPROF_BUDGETGROUP_SHADOW_DEPTH_TEXTURING );

	if (!m_bIsOn)
	{
		//		return;
	}

	if ( ComputeLightPosAndOrientation ( vecPos, vecForward, vecRight, vecUp, state.m_vecLightOrigin, state.m_quatOrientation, bTracePlayers ) == false )
	{
		return false;
	}

	state.m_fQuadraticAtten = r_flashlightquadratic.GetFloat();

	bool bFlicker = false;

#ifdef HL2_EPISODIC
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if ( pPlayer )
	{
		float flBatteryPower = ( pPlayer->m_HL2Local.m_flFlashBattery >= 0.0f ) ? ( pPlayer->m_HL2Local.m_flFlashBattery ) : pPlayer->m_HL2Local.m_flSuitPower;
		if ( flBatteryPower <= 10.0f )
		{
			float flScale;
			if ( flBatteryPower >= 0.0f )
			{	
				flScale = ( flBatteryPower <= 4.5f ) ? SimpleSplineRemapVal( flBatteryPower, 4.5f, 0.0f, 1.0f, 0.0f ) : 1.0f;
			}
			else
			{
				flScale = SimpleSplineRemapVal( flBatteryPower, 10.0f, 4.8f, 1.0f, 0.0f );
			}
			
			flScale = clamp( flScale, 0.0f, 1.0f );

			if ( flScale < 0.35f )
			{
				float flFlicker = cosf( gpGlobals->curtime * 6.0f ) * sinf( gpGlobals->curtime * 15.0f );
				
				if ( flFlicker > 0.25f && flFlicker < 0.75f )
				{
					// On
					state.m_fLinearAtten = r_flashlightlinear.GetFloat() * flScale;
				}
				else
				{
					// Off
					state.m_fLinearAtten = 0.0f;
				}
			}
			else
			{
				float flNoise = cosf( gpGlobals->curtime * 7.0f ) * sinf( gpGlobals->curtime * 25.0f );
				state.m_fLinearAtten = r_flashlightlinear.GetFloat() * flScale + 1.5f * flNoise;
			}

			state.m_fHorizontalFOVDegrees = r_flashlightfov.GetFloat() - ( 16.0f * (1.0f-flScale) );
			state.m_fVerticalFOVDegrees = r_flashlightfov.GetFloat() - ( 16.0f * (1.0f-flScale) );
			
			bFlicker = true;
		}
	}
#endif // HL2_EPISODIC

	if ( bFlicker == false )
	{
		state.m_fLinearAtten = r_flashlightlinear.GetFloat();
		state.m_fHorizontalFOVDegrees = r_flashlightfov.GetFloat();
		state.m_fVerticalFOVDegrees = r_flashlightfov.GetFloat();
	}

	state.m_fConstantAtten = r_flashlightconstant.GetFloat();
	state.m_Color[0] = 1.0f;
	state.m_Color[1] = 1.0f;
	state.m_Color[2] = 1.0f;
	state.m_Color[3] = r_flashlightambient.GetFloat();

	state.m_NearZ = r_flashlightnear.GetFloat() + r_flashlightnearoffsetscale.GetFloat() * m_flCurrentPullBackDist;	// Optionally push near plane out so that we don't clip the world when the flashlight pulls back 
	state.m_FarZ = r_flashlightfar.GetFloat();
	state.m_bEnableShadows = r_flashlightdepthtexture.GetBool();
	state.m_flShadowMapResolution = r_flashlightdepthres.GetInt();

	/*if (pFlashlightTexture)
	{
		state.m_pSpotlightTexture = m_FlashlightTexture;
		state.m_pProjectedMaterial = NULL;
	}*/

	state.m_pSpotlightTexture = m_FlashlightTexture;
	//state.m_pProjectedMaterial = NULL;
	

	state.m_nSpotlightTextureFrame = 0;

	state.m_flShadowAtten = r_flashlightshadowatten.GetFloat();
	state.m_flShadowSlopeScaleDepthBias = mat_slopescaledepthbias_shadowmap.GetFloat();
	state.m_flShadowDepthBias = mat_depthbias_shadowmap.GetFloat();

	return true;
}

bool CFlashlightEffect::ComputeLightPosAndOrientation( const Vector &vecPos, const Vector &vecForward, const Vector &vecRight, const Vector &vecUp, 
														Vector& vecFinalPos, Quaternion& quatOrientation, bool bTracePlayers )
{
	const float flEpsilon = 0.1f;			// Offset flashlight position along vecUp
	float flDistCutoff = r_flashlighttracedistcutoff.GetFloat();
	const float flDistDrag = 0.2;
	bool bDebugVis = r_flashlightvisualizetrace.GetBool();

	// We will lock some of the flashlight params if player is on a ladder, to prevent oscillations due to the trace-rays
	bool bPlayerOnLadder = ( C_BasePlayer::GetLocalPlayer()->GetMoveType() == MOVETYPE_LADDER );

	CTraceFilterSkipPlayerAndViewModel traceFilter;

	//	Vector vOrigin = vecPos + r_flashlightoffsety.GetFloat() * vecUp;
	Vector vecOffset;
	Vector vOrigin = vecPos + vecOffset;

	// Not on ladder...trace a hull
	if ( !bPlayerOnLadder ) 
	{
		trace_t pmOriginTrace;
		UTIL_TraceHull( vecPos, vOrigin, Vector(-4, -4, -4), Vector(4, 4, 4), MASK_SOLID & ~(CONTENTS_HITBOX), &traceFilter, &pmOriginTrace );

		if ( pmOriginTrace.DidHit() )
		{
			vOrigin = vecPos;
		}
	}
	else // on ladder...skip the above hull trace
	{
		vOrigin = vecPos;
	}

	// Now do a trace along the flashlight direction to ensure there is nothing within range to pull back from
	int iMask = MASK_OPAQUE_AND_NPCS;
	iMask &= ~CONTENTS_HITBOX;
	iMask |= CONTENTS_WINDOW;

	Vector vTarget = vOrigin + vecForward * r_flashlightfar.GetFloat();

	// Work with these local copies of the basis for the rest of the function
	Vector vDir   = vTarget - vOrigin;
	Vector vRight = vecRight;
	Vector vUp    = vecUp;
	VectorNormalize( vDir   );
	VectorNormalize( vRight );
	VectorNormalize( vUp    );

	// Orthonormalize the basis, since the flashlight texture projection will require this later...
	vUp -= DotProduct( vDir, vUp ) * vDir;
	VectorNormalize( vUp );
	vRight -= DotProduct( vDir, vRight ) * vDir;
	VectorNormalize( vRight );
	vRight -= DotProduct( vUp, vRight ) * vUp;
	VectorNormalize( vRight );

	AssertFloatEquals( DotProduct( vDir, vRight ), 0.0f, 1e-3 );
	AssertFloatEquals( DotProduct( vDir, vUp    ), 0.0f, 1e-3 );
	AssertFloatEquals( DotProduct( vRight, vUp  ), 0.0f, 1e-3 );

	trace_t pmDirectionTrace;
	UTIL_TraceHull( vOrigin, vTarget, Vector( -1.5, -1.5, -1.5 ), Vector( 1.5, 1.5, 1.5 ), iMask, &traceFilter, &pmDirectionTrace );

	//if ( r_flashlightvisualizetrace.GetBool() == true )
	if ( bDebugVis )
	{
		debugoverlay->AddBoxOverlay( pmDirectionTrace.endpos, Vector( -4, -4, -4 ), Vector( 4, 4, 4 ), QAngle( 0, 0, 0 ), 0, 0, 255, 16, 0 );
		debugoverlay->AddLineOverlay( vOrigin, pmDirectionTrace.endpos, 255, 0, 0, false, 0 );
	}

	float flTargetPullBackDist = 0.0f;
	float flDist = (pmDirectionTrace.endpos - vOrigin).Length();

	if ( flDist < flDistCutoff )
	{
		// We have an intersection with our cutoff range
		// Determine how far to pull back, then trace to see if we are clear
		float flPullBackDist = bPlayerOnLadder ? r_flashlightladderdist.GetFloat() : flDistCutoff - flDist;	// Fixed pull-back distance if on ladder
		//m_flDistMod = Lerp( flDistDrag, m_flDistMod, flPullBackDist );
		
		flTargetPullBackDist = flPullBackDist;

		if ( !bPlayerOnLadder )
		{
			trace_t pmBackTrace;
			// start the trace away from the actual trace origin a bit, to avoid getting stuck on small, close "lips"
			UTIL_TraceHull( vOrigin - vDir * ( flDistCutoff * r_flashlightbacktraceoffset.GetFloat() ), vOrigin - vDir * ( flPullBackDist - flEpsilon ), 
				Vector( -1.5f, -1.5f, -1.5f ), Vector( 1.5f, 1.5f, 1.5f ), iMask, &traceFilter, &pmBackTrace );
			
			if ( bDebugVis )
			{
				debugoverlay->AddLineOverlay( pmBackTrace.startpos, pmBackTrace.endpos, 255, 0, 255, true, 0 );
			}

			if( pmBackTrace.DidHit() )
			{
				// We have an intersection behind us as well, so limit our m_flDistMod
				float flMaxDist = (pmBackTrace.endpos - vOrigin).Length() - flEpsilon;
				flTargetPullBackDist = MIN( flMaxDist, flTargetPullBackDist );
				//if( m_flCurrentPullBackDist > flMaxDist )
					//m_flCurrentPullBackDist = flMaxDist;
			}
		}
	}

	if ( bDebugVis )
	{
		// visualize pullback
		debugoverlay->AddBoxOverlay( vOrigin - vDir * m_flCurrentPullBackDist, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), QAngle( 0, 0, 0 ), 255, 255, 0, 16, 0 );
		debugoverlay->AddBoxOverlay( vOrigin - vDir * flTargetPullBackDist, Vector( -1, -1, -1 ), Vector( 1, 1, 1 ), QAngle( 0, 0, 0 ), 128, 128, 0, 16, 0 );
	}

	/*else
	{
		m_flDistMod = Lerp( flDistDrag, m_flDistMod, 0.0f );
	}
	vOrigin = vOrigin - vDir * m_flDistMod; */

	m_flCurrentPullBackDist = Lerp( flDistDrag, m_flCurrentPullBackDist, flTargetPullBackDist );
	m_flCurrentPullBackDist = MIN( m_flCurrentPullBackDist, flDistCutoff );	// clamp to max pullback dist
	vOrigin = vOrigin - vDir * m_flCurrentPullBackDist;

	vecFinalPos = vOrigin;
	BasisToQuaternion( vDir, vRight, vUp, quatOrientation );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFlashlightEffect::LightOff()
{
#ifndef NO_TOOLFRAMEWORK
	if ( clienttools->IsInRecordingMode() )
	{
		KeyValues *msg = new KeyValues( "FlashlightState" );
		msg->SetFloat( "time", gpGlobals->curtime );
		msg->SetInt( "entindex", m_nEntIndex );
		msg->SetInt( "flashlightHandle", m_FlashlightHandle );
		msg->SetPtr( "flashlightState", NULL );
		ToolFramework_PostToolMessage( HTOOLHANDLE_INVALID, msg );
		msg->deleteThis();
	}
#endif

	// Clear out the light
	if( m_FlashlightHandle != CLIENTSHADOW_INVALID_HANDLE )
	{
		g_pClientShadowMgr->DestroyFlashlight( m_FlashlightHandle );
		m_FlashlightHandle = CLIENTSHADOW_INVALID_HANDLE;
	}

	g_hFlashlightHandle = CLIENTSHADOW_INVALID_HANDLE;
}

CHeadlightEffect::CHeadlightEffect() 
{

}

CHeadlightEffect::~CHeadlightEffect()
{
	
}

void CHeadlightEffect::UpdateLight( const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance )
{
	if ( IsOn() == false )
		 return;

	FlashlightState_t state;
	Vector basisX, basisY, basisZ;
	basisX = vecDir;
	basisY = vecRight;
	basisZ = vecUp;
	VectorNormalize(basisX);
	VectorNormalize(basisY);
	VectorNormalize(basisZ);

	BasisToQuaternion( basisX, basisY, basisZ, state.m_quatOrientation );
		
	state.m_vecLightOrigin = vecPos;

	state.m_fHorizontalFOVDegrees = 45.0f;
	state.m_fVerticalFOVDegrees = 30.0f;
	state.m_fQuadraticAtten = r_flashlightquadratic.GetFloat();
	state.m_fLinearAtten = r_flashlightlinear.GetFloat();
	state.m_fConstantAtten = r_flashlightconstant.GetFloat();
	state.m_Color[0] = 1.0f;
	state.m_Color[1] = 1.0f;
	state.m_Color[2] = 1.0f;
	state.m_Color[3] = r_flashlightambient.GetFloat();
	state.m_NearZ = r_flashlightnear.GetFloat();
	state.m_FarZ = r_flashlightfar.GetFloat();
	state.m_bEnableShadows = true;
	state.m_pSpotlightTexture = m_FlashlightTexture;
	state.m_nSpotlightTextureFrame = 0;
	
	if( GetFlashlightHandle() == CLIENTSHADOW_INVALID_HANDLE )
	{
		SetFlashlightHandle( g_pClientShadowMgr->CreateFlashlight( state ) );
	}
	else
	{
		g_pClientShadowMgr->UpdateFlashlightState( GetFlashlightHandle(), state );
	}
	
	g_pClientShadowMgr->UpdateProjectedTexture( GetFlashlightHandle(), true );
}


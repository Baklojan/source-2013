//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FLASHLIGHTEFFECT_H
#define FLASHLIGHTEFFECT_H
#ifdef _WIN32
#pragma once
#endif

struct dlight_t;


class CFlashlightEffect
{
public:

	CFlashlightEffect( int nEntIndex = 0, float flFarZ = 0.0f );
	~CFlashlightEffect();

	void UpdateLight( const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, float flFarZ, bool bTracePlayers );
	
	void TurnOn();
	void TurnOff();
	bool IsOn( void ) { return m_bIsOn;	}

	ClientShadowHandle_t GetFlashlightHandle( void ) { return m_FlashlightHandle; }
	void SetFlashlightHandle( ClientShadowHandle_t Handle ) { m_FlashlightHandle = Handle;	}
	
protected:

	bool UpdateDefaultFlashlightState( FlashlightState_t& state, const Vector& vecPos, const Vector& vecForward,
										const Vector& vecRight, const Vector& vecUp, bool bTracePlayers );
	bool ComputeLightPosAndOrientation( const Vector& vecPos, const Vector& vecForward, const Vector& vecRight, const Vector& vecUp,
										Vector& vecFinalPos, Quaternion& quatOrientation, bool bTracePlayers );
	void LightOff();

	bool m_bIsOn;
	int m_nEntIndex;
	ClientShadowHandle_t m_FlashlightHandle;

	// Vehicle headlight dynamic light pointer
	dlight_t *m_pPointLight;

	float m_flFarZ;
	float m_flLinearAtten;

	float m_flCurrentPullBackDist;

	// Texture for flashlight
	CTextureReference m_FlashlightTexture;
};

class CHeadlightEffect : public CFlashlightEffect
{
public:
	
	CHeadlightEffect();
	~CHeadlightEffect();

	virtual void UpdateLight( const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance);
};



#endif // FLASHLIGHTEFFECT_H

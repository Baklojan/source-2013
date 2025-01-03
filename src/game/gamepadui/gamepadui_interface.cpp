#include "gamepadui_interface.h"
#include "gamepadui_basepanel.h"
#include "gamepadui_mainmenu.h"

#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CDllDemandLoader s_GameUI( "GameUI" );

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( GamepadUI, IGamepadUI, GAMEPADUI_INTERFACE_VERSION, GamepadUI::GetInstance() );

GamepadUI *GamepadUI::s_pGamepadUI = NULL;

GamepadUI& GamepadUI::GetInstance()
{
	if ( !s_pGamepadUI )
		s_pGamepadUI = new GamepadUI;

	return *s_pGamepadUI;
}

void GamepadUI::Initialize( CreateInterfaceFn factory )
{
	ConnectTier1Libraries( &factory, 1 );
	ConnectTier2Libraries( &factory, 1 );
	ConVar_Register( FCVAR_CLIENTDLL );
	ConnectTier3Libraries( &factory, 1 );

	m_pEngineClient           = (IVEngineClient*)         factory( VENGINE_CLIENT_INTERFACE_VERSION, NULL );
	m_pEngineSound            = (IEngineSound*)           factory( IENGINESOUND_CLIENT_INTERFACE_VERSION, NULL );
	m_pEngineVGui             = (IEngineVGui*)            factory( VENGINE_VGUI_VERSION, NULL );
	m_pGameUIFuncs            = (IGameUIFuncs*)           factory( VENGINE_GAMEUIFUNCS_VERSION, NULL );
	m_pMaterialSystem         = (IMaterialSystem*)        factory( MATERIAL_SYSTEM_INTERFACE_VERSION, NULL );
	m_pMaterialSystemSurface  = (IMatSystemSurface*)      factory( MAT_SYSTEM_SURFACE_INTERFACE_VERSION, NULL );
	m_pRenderView             = (IVRenderView*)           factory( VENGINE_RENDERVIEW_INTERFACE_VERSION, NULL );
	m_pSoundEmitterSystemBase = (ISoundEmitterSystemBase*)factory( SOUNDEMITTERSYSTEM_INTERFACE_VERSION, NULL );

	CreateInterfaceFn gameuiFactory = s_GameUI.GetFactory();
	if ( gameuiFactory )
		m_pGameUI = (IGameUI *)gameuiFactory( GAMEUI_INTERFACE_VERSION, NULL );

	//m_pAchievementMgr = (IAchievementMgr *)m_pEngineClient->GetAchievementMgr();

#ifdef DEBUG
	bool bFailed = !m_pEngineClient  ||
		!m_pEngineSound              ||
		!m_pEngineVGui               ||
		!m_pGameUIFuncs              ||
		!m_pMaterialSystem           ||
		!m_pMaterialSystemSurface    ||
		!m_pRenderView               ||
		!m_pSoundEmitterSystemBase   ||
		!m_pGameUI;
	if ( bFailed )
	{
		GamepadUI_Log( "GamepadUI::Initialize() failed to get necessary interfaces.\n" );
		return;
	}
#else
	//m_pEngineClient
	( m_pEngineClient == nullptr ) ? GamepadUI_Log( "m_pEngineClient failed\n" ) : GamepadUI_Log( "m_pEngineClient loaded\n" );
	//m_pEngineSound
	( m_pEngineSound == nullptr ) ? GamepadUI_Log( "m_pEngineSound failed\n" ) : GamepadUI_Log( "m_pEngineSound loaded\n" );
	//m_pEngineVGui
	( m_pEngineVGui == nullptr ) ? GamepadUI_Log( "m_pEngineVGui failed\n" ) : GamepadUI_Log( "m_pEngineVGui loaded\n" );
	//m_pGameUIFuncs
	( m_pGameUIFuncs == nullptr ) ? GamepadUI_Log( "m_pGameUIFuncs failed\n" ) : GamepadUI_Log( "m_pGameUIFuncs loaded\n" );
	//m_pMaterialSystem
	( m_pMaterialSystem == nullptr ) ? GamepadUI_Log( "m_pMaterialSystem failed\n" ) : GamepadUI_Log( "m_pMaterialSystem loaded\n" );
	//m_pMaterialSystemSurface
	( m_pMaterialSystemSurface == nullptr ) ? GamepadUI_Log( "m_pMaterialSystemSurface failed\n" ) : GamepadUI_Log( "m_pMaterialSystemSurface loaded\n" );
	//m_pRenderView
	( m_pRenderView == nullptr ) ? GamepadUI_Log( "m_pRenderView failed\n" ) : GamepadUI_Log( "m_pRenderView loaded\n" );
	//m_pSoundEmitterSystemBase
	( m_pSoundEmitterSystemBase == nullptr ) ? GamepadUI_Log( "m_pSoundEmitterSystemBase failed\n" ) : GamepadUI_Log( "m_pSoundEmitterSystemBase loaded\n" );
	//m_pGameUI
	( m_pGameUI == nullptr ) ? GamepadUI_Log( "m_pGameUI failed\n" ) : GamepadUI_Log( "m_pGameUI loaded\n" );
	//m_pAchievementMgr
	( m_pAchievementMgr == nullptr ) ? GamepadUI_Log( "m_pAchievementMgr failed\n" ) : GamepadUI_Log( "m_pAchievementMgr loaded\n" );
#endif

	g_pVGuiLocalize->AddFile( "resource/gameui_%language%.txt", "GAME", true );

	m_pBasePanel = new GamepadUIBasePanel( GetRootVPanel() );
	if ( !m_pBasePanel )
	{
		GamepadUI_Log( "Failed to create BasePanel.\n" );
		return;
	}

	GamepadUI_Log( "Overriding menu.\n" );

	m_pGameUI->SetMainMenuOverride( GetBaseVPanel() );

	m_pAnimationController = new vgui::AnimationController( m_pBasePanel );
	m_pAnimationController->SetProportional( false );

	GetMainMenu()->Activate();
}

void GamepadUI::Shutdown()
{
	if ( m_pGameUI )
		m_pGameUI->SetMainMenuOverride( NULL );

	if ( m_pBasePanel )
		m_pBasePanel->DeletePanel();

	ConVar_Unregister();
	DisconnectTier3Libraries();
	DisconnectTier2Libraries();
	DisconnectTier1Libraries();
}


void GamepadUI::OnUpdate( float flFrametime )
{
	if ( m_pAnimationController )
		m_pAnimationController->UpdateAnimations( GetTime() );
}

void GamepadUI::OnLevelInitializePreEntity()
{
}

void GamepadUI::OnLevelInitializePostEntity()
{
	m_pBasePanel->OnMenuStateChanged();
	GetMainMenu()->OnMenuStateChanged();
}

void GamepadUI::OnLevelShutdown()
{
	if ( m_pAnimationController )
	{
		m_pAnimationController->UpdateAnimations( GetTime() );
		m_pAnimationController->RunAllAnimationsToCompletion();
	}

	m_pBasePanel->OnMenuStateChanged();
	GetMainMenu()->OnMenuStateChanged();
}

void GamepadUI::VidInit()
{
	int w, h;
	vgui::surface()->GetScreenSize( w, h );

	Assert( w != 0 && h != 0 );

	// Scale elements proportional to the aspect ratio's distance from 16:10
	const float flDefaultInvAspect = 0.625f;
	float flInvAspectRatio = ((float)h) / ((float)w);

	if (flInvAspectRatio != flDefaultInvAspect)
	{
		m_flScreenRatio = 1.0f - (flInvAspectRatio - flDefaultInvAspect);
	}
	else
	{
		m_flScreenRatio = 1.0f;
	}

	m_pBasePanel->InvalidateLayout( false, true );
}


bool GamepadUI::IsInLevel() const
{
	const char *pLevelName = m_pEngineClient->GetLevelName();
	return pLevelName && *pLevelName && !m_pEngineClient->IsLevelMainMenuBackground();
}

bool GamepadUI::IsInBackgroundLevel() const
{
	const char *pLevelName = m_pEngineClient->GetLevelName();
	return pLevelName && *pLevelName && m_pEngineClient->IsLevelMainMenuBackground();
}

bool GamepadUI::IsInMultiplayer() const
{
	return IsInLevel() && m_pEngineClient->GetMaxClients() > 1;
}

bool GamepadUI::IsGamepadUIVisible() const
{
	return !IsInLevel() || IsInBackgroundLevel();
}


void GamepadUI::ResetToMainMenuGradients()
{
	GetMainMenu()->UpdateGradients();
}

vgui::VPANEL GamepadUI::GetRootVPanel() const
{
	return m_pEngineVGui->GetPanel( PANEL_GAMEUIDLL );
}

vgui::Panel *GamepadUI::GetBasePanel() const
{
	return m_pBasePanel;
}

vgui::VPANEL GamepadUI::GetBaseVPanel() const
{
	return m_pBasePanel ? m_pBasePanel->GetVPanel() : 0;
}

vgui::Panel *GamepadUI::GetMainMenuPanel() const
{
	return m_pBasePanel ? m_pBasePanel->GetMainMenuPanel() : NULL;
}

vgui::VPANEL GamepadUI::GetMainMenuVPanel() const
{
	return GetMainMenuPanel() ? GetMainMenuPanel()->GetVPanel() : 0;
}

GamepadUIMainMenu* GamepadUI::GetMainMenu() const
{
	return static_cast<GamepadUIMainMenu*>( GetMainMenuPanel() );
}

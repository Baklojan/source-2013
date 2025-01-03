#ifndef GAMEPADUI_IMAGE_H
#define GAMEPADUI_IMAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Panel.h"
#include "bitmap/tgaloader.h"

class GamepadUIImage
{
public:
    GamepadUIImage()
    {
    }
    GamepadUIImage( const char* pName )
    {
        SetImage( pName );
    }
    ~GamepadUIImage()
    {
        Cleanup();
    }
    void Cleanup()
    {
        if ( IsValid() )
            vgui::surface()->DestroyTextureID( m_nId );

        m_nId = -1;
    }
    void SetImage( const char* pName )
    {
        Cleanup();

        m_nId = vgui::surface()->CreateNewTextureID();
        vgui::surface()->DrawSetTextureFile( m_nId, pName, true, false );
    }
    void SetTGAImage( const char* pName )
    {
        Cleanup();

        CUtlMemory< unsigned char > tga;
        int nWidth, nHeight;
        if ( !TGALoader::LoadRGBA8888( pName, tga, nWidth, nHeight ) )
            return;

        m_nId = vgui::surface()->CreateNewTextureID( true );

        g_pMatSystemSurface->DrawSetTextureRGBAEx( m_nId, tga.Base(), nWidth, nHeight, IMAGE_FORMAT_RGBA8888 );
    }
    bool IsValid()
    {
        return m_nId > 0;
    }
    operator int()
    {
        return m_nId;
    }
private:
    int m_nId = -1;
};

#endif // GAMEPADUI_IMAGE_H

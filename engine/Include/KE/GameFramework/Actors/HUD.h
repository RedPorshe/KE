#pragma once

#include "KE/GameFramework/Actors/Actor.h"

// Forward declarations
class CController;
class CFont;
class CTexture;
class CMaterial;

class KE_API CHUD : public CActor
    {
    CHUDDO_DECLARE_CLASS ( CHUD, CActor )

    public:
        CHUD ( CObject * inOwner = nullptr, const std::string & inDisplayName = "HUD" );
        virtual ~CHUD ();

        // ========== CONTROLLER ==========
        void SetOwnerController ( CController * InController );
        CController * GetOwnerController () const { return OwnerController; }

        // ========== HUD LIFECYCLE ==========
        virtual void InitializeHUD ();
        virtual void DrawHUD ( float DeltaTime );
        virtual void Tick ( float DeltaTime ) override;
        virtual void BeginPlay () override;
        virtual void EndPlay () override;

        // ========== DRAWING PRIMITIVES ==========
        virtual void DrawText ( const std::string & Text, float X, float Y,
                                const FVector & Color = FVector ( 1.0f, 1.0f, 1.0f ),
                                float Scale = 1.0f, bool bCentered = false );

        virtual void DrawLine ( float X1, float Y1, float X2, float Y2,
                                const FVector & Color = FVector ( 1.0f, 1.0f, 1.0f ),
                                float Thickness = 1.0f );

        virtual void DrawRect ( float X, float Y, float Width, float Height,
                                const FVector & Color = FVector ( 1.0f, 1.0f, 1.0f ),
                                bool bFilled = false );

        virtual void DrawTexture ( CTexture * Texture, float X, float Y,
                                   float Width, float Height, float Opacity = 1.0f );

           // ========== UI ELEMENTS ==========
        void ShowMessage ( const std::string & Message, float Duration = 3.0f );
        void ShowDebugInfo ();
        void ShowCrosshair ();

        // ========== STATE ==========
        bool IsVisible () const { return bVisible; }
        void SetVisible ( bool bVisible ) { this->bVisible = bVisible; }

      

    protected:
        // Owner
        CController * OwnerController = nullptr;

        // HUD State
        bool bVisible = true;
        bool bShowDebugInfo = false;
        bool bShowCrosshair = true;

        // Current message
        struct HUDMessage
            {
            std::string Text;
            float RemainingTime = 0.0f;
            FVector Color = FVector ( 1.0f, 1.0f, 1.0f );
            };

        HUDMessage CurrentMessage;

        // Font
        CFont * DefaultFont = nullptr;

        // Crosshair texture
        CTexture * CrosshairTexture = nullptr;

        // Virtual screen dimensions (for resolution-independent UI)
        float VirtualScreenWidth = 1920.0f;
        float VirtualScreenHeight = 1080.0f;

        // Helper methods
        float GetScreenWidth () const;
        float GetScreenHeight () const;
        FVector2D GetVirtualScreenPosition ( float X, float Y ) const;
    };

REGISTER_CLASS_FACTORY ( CHUD );
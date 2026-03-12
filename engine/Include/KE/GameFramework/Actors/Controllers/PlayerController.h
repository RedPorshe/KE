#pragma once

#include "Actors/Controllers/Controller.h"

class KE_API CPlayerController : public CController
    {
    CHUDDO_DECLARE_CLASS ( CPlayerController, CController );

    public:
        CPlayerController ( CObject * inOwner = nullptr, const std::string & inDisplayName = "PlayerController" );
        virtual ~CPlayerController ();

        // ========== INPUT ==========
        virtual void ProcessPlayerInput ( float DeltaTime ) override;

        // ========== MOUSE CONTROL ==========
        void SetShowMouseCursor ( bool bShow ) { bShowMouseCursor = bShow; }
        bool GetShowMouseCursor () const { return bShowMouseCursor; }

        // ========== CAMERA CONTROL ==========
        void SetViewTarget ( CActor * NewViewTarget ) { ViewTarget = NewViewTarget; }
        CActor * GetViewTarget () const { return ViewTarget; }

        // ========== ACTOR OVERRIDES ==========
        virtual void BeginPlay () override;
        virtual void EndPlay () override;

    protected:
        // Mouse state
        bool bShowMouseCursor = false;
        FVector2D MousePosition = FVector2D::Zero ();
    };

REGISTER_CLASS_FACTORY ( CPlayerController );
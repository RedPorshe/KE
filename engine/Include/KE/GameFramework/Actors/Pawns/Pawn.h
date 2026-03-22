#pragma once

#include "KE/GameFramework/Actors/Actor.h"

class CController;
class CInputComponent;
class CMovementComponent;

class KE_API CPawn : public CActor
    {
    CHUDDO_DECLARE_CLASS ( CPawn, CActor );

    public:
        CPawn ( CObject * inOwner = nullptr, const std::string & inDisplayName = "Pawn" );
        virtual ~CPawn ();

        // ========== CONTROLLER ==========
        void SetController ( CController * NewController );
        CController * GetController () const { return Controller; }
        bool IsPlayerControlled () const { return Controller != nullptr; }

        // ========== INPUT ==========
        CMovementComponent * GetMovementComponent () const { return MovementComponent; }

        // Input API (как в UE)
        void AddMovementInput ( const FVector & WorldDirection, float ScaleValue, bool bForce = false );
        void AddControllerYawInput ( float Val );
        void AddControllerPitchInput ( float Val );
        void AddControllerRollInput ( float Val );
        bool IsInputEnabled () const;
        void SetInputEnabled ( bool value ) { bInputEnabled = value; }

        CInputComponent * GetInputComponent () const { return m_InputComponent; }

        // ========== MOVEMENT SETTINGS ==========
        void SetAirControl ( float value );
        float GetAirControl () const;
        void SetMaxAirSpeed ( float value );
        float GetMaxAirSpeed () const;
        void SetGroundSpeed ( float value );
        float GetGroundSpeed () const;

        // ========== ACTOR OVERRIDES ==========
        virtual void Tick ( float DeltaTime ) override;
        virtual void BeginPlay () override;
        virtual void EndPlay () override;

        void OnPossessed ( CController * NewController );
        void OnUnpossessed ( CController * OldController );
        virtual void OnPossess ();

    protected:
        virtual void SetupPlayerInputComponent ( CInputComponent * InputComponent );
        void ProcessPlayerInput ( float DeltaTime );

        // Компоненты
        CMovementComponent * MovementComponent = nullptr;
        CController * Controller = nullptr;
        CInputComponent * m_InputComponent = nullptr;

        bool bUseControllerRotaionYaw = false;
        bool bUseControllerRotaionRoll = false;
        bool bUseControllerRotaionPitch = false;
        // Состояние
        bool bInputEnabled = false;
    };

REGISTER_CLASS_FACTORY ( CPawn );
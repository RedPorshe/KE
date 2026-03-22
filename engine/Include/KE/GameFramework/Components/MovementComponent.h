#pragma once

#include "KE/GameFramework/Components/BaseComponent.h"

class CPawn;
class CController;
class KE_API CMovementComponent : public CBaseComponent
    {
    CHUDDO_DECLARE_CLASS ( CMovementComponent, CBaseComponent );

    public:
        CMovementComponent ( CObject * inOwner = nullptr, const std::string & inName = "MovementComponent" );
        virtual ~CMovementComponent () = default;

        void InitComponent () override;
        void Tick ( float DeltaTime ) override;
        void OnBeginPlay () override;

        CController * GetContoller () const { return Controller; }
        void SetController ( CController * inController ) { Controller = inController; }

        // Input accumulation
        void AddInputVector ( const FVector & WorldDirection, float ScaleValue, bool bForce = false );
        void AddPitchInput ( float Value );
        void AddYawInput ( float Value );
        void AddRollInput ( float Value );

        // Градусы для удобства
        void AddPitchInputDegrees ( float Degrees ) { AddPitchInput ( CEMath::DegreesToRadians ( Degrees ) ); }
        void AddYawInputDegrees ( float Degrees ) { AddYawInput ( CEMath::DegreesToRadians ( Degrees ) ); }
        void AddRollInputDegrees ( float Degrees ) { AddRollInput ( CEMath::DegreesToRadians ( Degrees ) ); }

        // Owner access
        CPawn * GetOwnerPawn () const { return OwnerPawn; }
        void SetOwnerPawn ( CPawn * InPawn ) { OwnerPawn = InPawn; }

        // Movement settings
        void SetMaxWalkSpeed ( float Speed ) { MaxWalkSpeed = Speed; }
        float GetMaxWalkSpeed () const { return MaxWalkSpeed; }

        void SetMaxAirSpeed ( float Speed ) { MaxAirSpeed = Speed; }
        float GetMaxAirSpeed () const { return MaxAirSpeed; }

        void SetAirControl ( float Control ) { AirControl = Control; }
        float GetAirControl () const { return AirControl; }

        float GetAccelerationRate () const { return AccelerationRate; }
        void SetAccelerationRate (const float & value)  {  AccelerationRate = value; }
        float GetDecelerationRate () const { return DecelerationRate; }
        void SetDecelerationRate (const float & value)  { DecelerationRate = value; }
        void SetUseControllRotationYaw ( bool bUse ) { bUseControllerRotaionYaw = bUse; }
        void SetUseControllRotationRoll ( bool bUse ) { bUseControllerRotaionRoll = bUse; }
        void SetUseControllRotationPitch ( bool bUse ) { bUseControllerRotaionPitch = bUse; }
        bool GetUseContollRotaionYaw () const { return bUseControllerRotaionYaw; }
        bool GetUseContollRotaionRoll () const { return bUseControllerRotaionRoll; }
        bool GetUseContollRotaionPitch () const { return bUseControllerRotaionPitch; }

    protected:
        virtual void ProcessMovementInput ( float DeltaTime );
        virtual void ProcessRotationInput ( float DeltaTime );
        virtual bool IsJumping () const { return false; }

        // Input accumulators
        FVector MovementInputAccumulator;
        float YawInputAccumulator = 0.0f;
        float PitchInputAccumulator = 0.0f;
        float RollInputAccumulator = 0.0f;

        bool bHasMovementInput = false;
        bool bHasRotationInput = false;

        // Movement settings
        float MaxWalkSpeed = 600.0f;
        float MaxAirSpeed = 400.0f;
        float AirControl = 0.3f;
        float AccelerationRate = 10.0f;   
        float DecelerationRate = 7.f;

        bool bUseControllerRotaionYaw = false;
        bool bUseControllerRotaionRoll = false;
        bool bUseControllerRotaionPitch = false;

        // State
        bool bIsGrounded = true;
        CController * Controller = nullptr;
        // Owner
        CPawn * OwnerPawn = nullptr;
    };

REGISTER_CLASS_FACTORY ( CMovementComponent );
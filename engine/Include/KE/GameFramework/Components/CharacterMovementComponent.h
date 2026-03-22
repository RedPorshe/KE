#pragma once
#include "KE/GameFramework/Components/MovementComponent.h"

class KE_API CCharacterMovementComponent : public CMovementComponent
    {
    CHUDDO_DECLARE_CLASS ( CCharacterMovementComponent, CMovementComponent );

    public:
        CCharacterMovementComponent ( CObject * inOwner, const std::string & Name );
        virtual ~CCharacterMovementComponent () = default;

        void InitComponent () override;
        void Tick ( float DeltaTime ) override;
        void OnBeginPlay () override;

        // Прыжок
        void Jump ();
        void StopJumping ();
        bool CanJump () const; 


         void ProcessMovementInput ( float DeltaTime ) override;
         void ProcessRotationInput ( float DeltaTime )override;
        
        // Настройки прыжка
        void SetJumpHeight ( float Height ) { JumpHeight = Height; }
        float GetJumpHeight () const { return JumpHeight; }

        void SetMaxJumpCount ( int Count ) { MaxJumpCount = Count; }
        int GetMaxJumpCount () const { return MaxJumpCount; }

        void SetCurrentJumpCount ( int Count ) { CurrentJumpCount = Count; }
        int GetCurrentJumpCount () const { return CurrentJumpCount; }

       

        // Множитель для второго прыжка
        void SetAirJumpMultiplier ( float Multiplier ) { AirJumpMultiplier = Multiplier; }
        float GetAirJumpMultiplier () const { return AirJumpMultiplier; }

        bool IsJumping () const override { return bIsJumping || !bIsGrounded; }
    protected:
      
        float JumpHeight = 10.0f;
        int MaxJumpCount = 1;
        int CurrentJumpCount = 0;
        float AirJumpMultiplier = 0.7f;
        bool bIsJumping = false;
    };

REGISTER_CLASS_FACTORY ( CCharacterMovementComponent );
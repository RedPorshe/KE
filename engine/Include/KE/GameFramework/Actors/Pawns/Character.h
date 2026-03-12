#pragma once
#include "Actors/Pawns/Pawn.h"
#include "Components/InputComponent.h"


class CCapsuleComponent;
class CStaticMeshComponent;
class CCameraComponent;
class CTerrainMeshComponent;
class KE_API CCharacter : public CPawn
    {
    CHUDDO_DECLARE_CLASS ( CCharacter, CPawn );

    public:
        CCharacter ( CObject * inOwner, const std::string & DisplayName );
        virtual ~CCharacter () = default;

        void BeginPlay () override;
        void Tick ( float DeltaTime ) override;
        void EndPlay () override;

       
        bool IsJumping () const;
         void OnComponentBeginOverlap ( CBaseCollisionComponent * other ) override;
         void OnComponentEndOverlap ( CBaseCollisionComponent * other ) override;
         void OnComponentHit ( CBaseCollisionComponent * other )override;

    protected:
        void SetupPlayerInputComponent ( CInputComponent * InputComponent ) override;
      
        void StartJump ();
      
        void DebugInfo ( float dt ) override;
        CStaticMeshComponent/*CSkeletalMeshComponent*/ * Mesh = nullptr; //class for mesh instance
        CCapsuleComponent * Capsule = nullptr;
     
        CTerrainMeshComponent * terrainMesh = nullptr;
    private:
        void CreateCharacterMovementComponent ();
        FQuat StartRotation = FQuat::Identity ();
    };

REGISTER_CLASS_FACTORY ( CCharacter );
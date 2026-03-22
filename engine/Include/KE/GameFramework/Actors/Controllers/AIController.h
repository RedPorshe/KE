#pragma once

#include "KE/GameFramework/Actors/Controllers/Controller.h"

class KE_API CAIController : public CController
    {
    CHUDDO_DECLARE_CLASS ( CAIController, CController );

    public:
        CAIController ( CObject * inOwner = nullptr, const std::string & inDisplayName = "AIController" );
        virtual ~CAIController ();

        // ========== INPUT ==========
        virtual void ProcessPlayerInput ( float DeltaTime ) override;

        // ========== AI BEHAVIOR ==========
        virtual void SetTargetActor ( CActor * InTarget ) { TargetActor = InTarget; }
        CActor * GetTargetActor () const { return TargetActor; }

        virtual void SetBehaviorState ( int State ) { BehaviorState = State; }
        int GetBehaviorState () const { return BehaviorState; }

        // ========== ACTOR OVERRIDES ==========
        virtual void Tick ( float DeltaTime ) override;
        virtual void BeginPlay () override;
        virtual void EndPlay () override;

    protected:
        // AI state
        CActor * TargetActor = nullptr;
        int BehaviorState = 0;  // 0 - idle, 1 - patrolling, 2 - chasing, 3 - attacking, etc.

        // Behavior timers
        float DecisionTimer = 0.0f;
        float DecisionInterval = 1.0f;  // Принимать решения каждую секунду
        void MakeDecision ();
        void ExecuteBehavior ( float DeltaTime );
    };

REGISTER_CLASS_FACTORY ( CAIController );
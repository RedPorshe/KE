#pragma once
#include <KE/GameFramework/Actors/Pawns/Character.h>

void CreateTestWorld ();


class myChar : public CCharacter
	{
	CHUDDO_DECLARE_CLASS(myChar,CCharacter)
	public:
		myChar ( CObject * iowner = nullptr, const std::string & inname = "mychar" );
		virtual ~myChar () = default;
		void BeginPlay ()override;
		void Tick ( float dt ) override;
		void EndPlay () override;
		void OnComponentBeginOverlap ( CBaseCollisionComponent * other ) override;
		void OnComponentEndOverlap ( CBaseCollisionComponent * other ) override;
		void OnComponentHit ( CBaseCollisionComponent * other )override;
	protected:
		void SetupPlayerInputComponent ( CInputComponent * InputComponent ) override;
		class CCameraComponent *Camera = nullptr;
		void DebugInfo ( float dt ) override;
		void MoveForward ( float val );
		void MoveRight ( float val );
		void Jump ();
	};
REGISTER_CLASS_FACTORY ( myChar );


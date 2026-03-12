#include <CoreMinimal.h>

#include <Actors/Pawns/Character.h>

class CCameraComponent;

class MyCharacter : public CCharacter
	{
	CHUDDO_DECLARE_CLASS ( MyCharacter, CCharacter );

	public: 
		MyCharacter ( CObject * owner, const std::string & inName );
		virtual ~MyCharacter () = default;
		void BeginPlay () override;
		void Tick ( float Deltatime ) override;
		void EndPlay () override;
		void SetupPlayerInputComponent ( CInputComponent * InputComponent ) override;
		void DebugInfo ( float dt ) override;
		void MoveForward ( float value );
		void MoveRight ( float value );
		void Jump ();
	private:
		CCameraComponent * Camera = nullptr;
	};
REGISTER_CLASS_FACTORY ( MyCharacter );
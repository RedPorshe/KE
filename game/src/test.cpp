#include "test.h"
#include "Camera/CameraComponent.h"
#include "Components/Collisions/CapsuleComponent.h"
#include <Actors/Controllers/Controller.h>
#include <World/World.h>

MyCharacter::MyCharacter ( CObject * owner, const std::string & inName ) : Super(owner,inName)
	{
	Camera = AddDefaultSubObject<CCameraComponent> ( "camera" );
	if(Capsule)
	Camera->AttachTo ( Capsule );
	Camera->SetRelativeLocation ( { 0.f, 50.f, -20.f } ); // далеко сзади и высоко

   // Наклонить камеру вниз, чтобы видеть террейн
	Camera->SetRelativeRotation ( FQuat::FromEulerAngles ( -30.f, 0.f, 0.f ) );
	}

void MyCharacter::BeginPlay ()
	{
	Super::BeginPlay ();
	}

void MyCharacter::Tick ( float Deltatime )
	{
	Super::Tick ( Deltatime );
	}

void MyCharacter::EndPlay ()
	{
	Super::EndPlay ();
	}

void MyCharacter::SetupPlayerInputComponent ( CInputComponent * InputComponent )
	{
	Super::SetupPlayerInputComponent ( InputComponent );
	LOG_INFO ( GetName (), " was contolled by ", GetController ()->GetName (), " and input component: ", GetInputComponent ()->GetName () );
	}

void MyCharacter::DebugInfo ( float dt )
	{
	static float timer = 0.f;
	timer += dt;
	if (timer >= 1.0f)
		{		
		FRenderInfo info;
		GetWorld ()->CollectRenderInfo ( &info );
		
		timer = 0.f;
		}
	}

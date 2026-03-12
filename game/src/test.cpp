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
	auto input = GetInputComponent ();
	input->BindAction ( "jumping", EKeys::Space, EInputEvent::IE_Pressed, [ this ] () { Jump (); } );
	input->BindAxis ( "moveforward", EKeys::W, EKeys::S, [ this ] ( float value ) { MoveForward ( value ); } );
	input->BindAxis ( "moveright", EKeys::D, EKeys::A, [ this ] ( float value ) { MoveRight ( value ); } );
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

void MyCharacter::MoveForward ( float value )
	{
	FVector forward = GetActorForwardVector ();
	AddMovementInput ( forward, value );
	}

void MyCharacter::MoveRight ( float value )
	{
	FVector Right = GetActorRightVector ();
	AddMovementInput ( Right, value );
	}

void MyCharacter::Jump ()
	{
	StartJump ();
	}

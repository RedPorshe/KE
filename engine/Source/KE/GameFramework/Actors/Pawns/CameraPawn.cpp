#include "KE/GameFramework/Actors/Pawns/CameraPawn.h"
#include "KE/GameFramework/Camera/CameraComponent.h"
#include "KE/GameFramework/Components/InputComponent.h"
#include "KE/GameFramework/World/World.h"


#include "KE/Engine.h"

CameraPawnActor::CameraPawnActor ( CObject * inOwner, const std::string & indisplayName ) :Super(inOwner,indisplayName)
	{
	Camera = AddDefaultSubObject<CCameraComponent> ( "DefaultCamera" );
	if (Camera)
		{
		Camera->SetFOV ( 90.f );
		Camera->SetFarClipPlane (10000.f);
		Camera->SetNearClipPlane ( 0.1f );
		}
	Camera->AttachTo ( GetRootComponent () );
	DestroyGravity ();
	bIsTurning = false;
	bIsLookUp = false;
	}

CameraPawnActor::~CameraPawnActor ()
	{}

void CameraPawnActor::Tick ( float DeltaTime )
	{
	Super::Tick ( DeltaTime );
	}

void CameraPawnActor::BeginPlay ()
	{ Super::BeginPlay (); }

void CameraPawnActor::EndPlay ()
	{ Super::EndPlay (); }

void CameraPawnActor::SetupPlayerInputComponent ( CInputComponent * InputComponent )
	{ 
	Super::SetupPlayerInputComponent ( InputComponent );
	if (auto Input = GetInputComponent ())
		{
		Input->BindAxis ( "moveforward", EKeys::W, EKeys::S, [ this ] ( float val ) { MoveForward ( val ); } );
		Input->BindAxis ( "moveright", EKeys::D, EKeys::A, [ this ] ( float val ) { MoveRight ( val ); } );
		Input->BindAxis ( "moveup", EKeys::Q, EKeys::E, [ this ] ( float val ) { MoveUp ( val ); } );
		Input->BindMouseAxis ( "lookup", EMouseAxis::MouseY, [ this ] ( float val ) { LookUp ( val ); } );
		Input->BindMouseAxis ( "turn", EMouseAxis::MouseX, [ this ] ( float val ) { Turn ( val ); } );
		Input->BindAction ( "Exit", EKeys::Escape, EInputEvent::IE_Pressed, [ this ] () { CEngine::Get ().RequestShutdown (); }	);
		}
	}

void CameraPawnActor::MoveForward ( float value )
	{
	auto Forward = GetActorForwardVector ();
	auto NewDelta = Forward * m_speed*value *GetWorld()->GetDeltaSeconds();
	MoveActor ( NewDelta );	
	}

void CameraPawnActor::MoveRight ( float value ) 
	{
	auto Right = GetActorRightVector ();
	auto NewDelta = Right * m_speed * value * GetWorld ()->GetDeltaSeconds ();
	MoveActor ( NewDelta );
	}

void CameraPawnActor::MoveUp ( float value )
	{
	auto Up = GetActorUpVector ();
	auto NewDelta = Up * m_speed * value * GetWorld ()->GetDeltaSeconds ();
	MoveActor ( NewDelta );
	}

void CameraPawnActor::LookUp ( float value )
	{
	if (bIsTurning) return;
	bIsLookUp = true;
	auto right = GetActorRightVector ();
	float deg = m_mouseSens * value * 1000.f * GetWorld ()->GetDeltaSeconds ();

	// Получаем текущий угол поворота
	auto currentrot = GetActorRotation ();

	// Вычисляем новый угол Pitch
	float newPitch = currentrot.x + deg;

	// Ограничиваем угол от -89 до 89 градусов (чтобы не переворачиваться)
	if (newPitch > 89.0f)
		{
		deg = 89.0f - currentrot.x;  // Сколько ещё можно повернуть вверх
		}
	else if (newPitch < -89.0f)
		{
		deg = -89.0f - currentrot.x; // Сколько ещё можно повернуть вниз
		}

		// Применяем поворот (только если есть что применять)
	if (CEMath::Abs ( deg ) > 0.001f)
		{
		RotateAroundAxis ( right, deg );
		}
	bIsLookUp = false;
	}

void CameraPawnActor::Turn ( float value )
	{
	if (bIsLookUp) return;
	bIsTurning = true;
	auto up = GetActorUpVector ();
	float deg = m_mouseSens * value * 1000.f * GetWorld ()->GetDeltaSeconds ();
	RotateAroundAxis ( up, deg );
	bIsTurning = false;
	}

void CameraPawnActor::DebugInfo ( float dt )
	{
	DebugTimer += dt;
	if (DebugTimer >= 1.f)
		{
		LOG_DEBUG ( "Camera actor location: ", GetActorLocation () );
		DebugTimer = 0.f;
		}
	}
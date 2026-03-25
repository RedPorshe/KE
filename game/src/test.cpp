#include "test.h"
#include <KE/GameFramework/GameInstance.h>
#include <KE/GameFramework/World/World.h>
#include <KE/GameFramework/World/Level.h>
#include <KE/GameFramework/Actors/TerrainActor.h>
#include <KE/GameFramework/Camera/CameraComponent.h>
#include <KE/GameFramework/Components/Collisions/CapsuleComponent.h>
#include <KE/GameFramework/Components/Meshes/StaticMeshComponent.h>
#include <KE/Engine.h>


void CreateTestWorld ()
	{
	CGameInstance * GI;
	CLevel * level = nullptr;
	if (!CGameInstance::IsCreated ())
		{
		if (CGameInstance::Create ())
			{
			GI = &CGameInstance::Get ();
			auto world = GI->CreateWorld ();
			level = world->CreateLevel<CLevel>();
			CEngine::Get ().SetGameInstance ( GI );
			};
		}
	if (level)
		{
		auto terr = level->SpawnActor<CTerrainActor> ();
		
		terr->GenerateNoise ( 50, 50, 50.f );
		terr->SetActorLocation ( FVector::Zero (), true );
		auto startpoint = level->SpawnActorByClass ( "CPlayerStart", "PlayerStart", FVector ( 100.f, 100.f, 100.f ) );
		startpoint->SetActorLocation ( { 100.f, 100.f, 100.f }, true );
		auto Act = level->SpawnActor<CActor> ( "vik Room" );
		Act->SetActorLocation ( { 100.f, 100.f, 100.f },true );
		auto vikModel = Act->AddDefaultSubObject<CStaticMeshComponent> ( "VikRoom" );
		vikModel->SetMesh ( "viking_room.obj" );
		GI->GetWorld ()->CreateGameMode<CGameMode> ();
		auto gamemode = GI->GetWorld ()->GetGameMode ();
		gamemode->SetDefaultPawnClass ( "myChar" );
		terr->SetDrawCollisions ( true );
		}
	}

myChar::myChar ( CObject * iowner, const std::string & inname ) :Super(iowner,inname)
	{
	Camera = AddDefaultSubObject<CCameraComponent> ( "camera" );
	Camera->AttachTo ( Capsule );
	Camera->SetRelativeLocation ( 0.f, 15.f, -20.f );
	if (Mesh)
		{
		//Mesh->ResizeCube(1.f);
		}
	SetDrawCollisions ( true );
	}

void myChar::BeginPlay ()
	{
	Super::BeginPlay ();
	}

void myChar::Tick ( float dt )
	{
	Super::Tick ( dt );
	
	}

void myChar::EndPlay ()
	{
	Super::EndPlay ();
	}

void myChar::OnComponentBeginOverlap ( CBaseCollisionComponent * other )
	{
	Super::OnComponentBeginOverlap ( other );
	}

void myChar::OnComponentEndOverlap ( CBaseCollisionComponent * other )
	{
	Super::OnComponentEndOverlap ( other );
	}

void myChar::OnComponentHit ( CBaseCollisionComponent * other )
	{
	Super::OnComponentHit ( other );
	}

void myChar::SetupPlayerInputComponent ( CInputComponent * InputComponent )
	{
	Super::SetupPlayerInputComponent ( InputComponent );
	auto input = GetInputComponent ();
	if (input)
		{
		input->BindAction ( "Exit", EKeys::Escape, EInputEvent::IE_Pressed, [ this ] () { CEngine::Get ().RequestShutdown (); } );
		input->BindAction ( "Jump", EKeys::Space, EInputEvent::IE_Pressed, [ this ] () { Jump (); } );
		input->BindAxis ( "MoveForward", EKeys::W, EKeys::S, [ this ] ( float val ) { MoveForward ( val ); } );
		input->BindAxis ( "MoveRight", EKeys::D, EKeys::A, [ this ] ( float val ) { MoveRight ( val ); } );
		}
	if (Mesh)
		{		
		Mesh->SetMesh ( "viking_room.obj" );
		}
	}
void myChar::DebugInfo ( float dt )
	{
	DebugTimer += dt;
	if (DebugTimer >=1.f)
		{
		LOG_DEBUG ( GetName (), " in ", GetActorLocation () );
		DebugTimer = 0.f;
		}
	}

void myChar::MoveForward ( float val )
	{
	auto forward = GetActorForwardVector ();
	AddMovementInput ( forward, val );
	}

void myChar::MoveRight ( float val )
	{
	auto right = GetActorRightVector ();
	AddMovementInput ( right, val );
	}

void myChar::Jump ()
	{
	StartJump ();
	}

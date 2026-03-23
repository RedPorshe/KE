#include "test.h"
#include <KE/GameFramework/GameInstance.h>
#include <KE/GameFramework/World/World.h>
#include <KE/GameFramework/World/Level.h>
#include <KE/GameFramework/Actors/TerrainActor.h>
#include <KE/GameFramework/Camera/CameraComponent.h>
#include <KE/GameFramework/Components/Collisions/CapsuleComponent.h>
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
		terr->GenerateHilly ( 25, 45, 5.f );
		terr->SetActorLocation ( FVector::Zero (), true );
		auto startpoint = level->SpawnActorByClass ( "CPlayerStart", "PlayerStart", FVector ( 100.f, 100.f, 100.f ) );
		startpoint->SetActorLocation ( { -50.f, 150.f, -50.f }, true );
		GI->GetWorld ()->CreateGameMode<CGameMode> ();
		auto gamemode = GI->GetWorld ()->GetGameMode ();
		gamemode->SetDefaultPawnClass ( "myChar" );
		}
	}

myChar::myChar ( CObject * iowner, const std::string & inname ) :Super(iowner,inname)
	{
	Camera = AddDefaultSubObject<CCameraComponent> ( "camera" );
	Camera->AttachTo ( Capsule );
	Camera->SetRelativeLocation ( 0.f, 5.f, -10.f );
	
	}

void myChar::BeginPlay ()
	{
	Super::BeginPlay ();
	}

void myChar::Tick ( float dt )
	{
	Super::Tick ( dt );
	LOG_DEBUG (GetName()," in ", GetActorLocation ());
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
	}

#include "KE/GameFramework/Actors/PlayerStart.h"
#include "KE/GameFramework/Components/GravityComponent.h"

CPlayerStart::CPlayerStart ( CObject * inOwner, const std::string & inName )
	: Super ( inOwner, inName )
	{
    if (!RootComponent) LOG_ERROR ("NO ROOT COMPONENT");	
	DestroyGravity ();
	}

CPlayerStart::~CPlayerStart ()
	{}

void CPlayerStart::BeginPlay ()
	{
	Super::BeginPlay ();
	}

void CPlayerStart::Tick ( float DeltaTime )
	{
	Super::Tick ( DeltaTime );
	}

void CPlayerStart::EndPlay ()
	{
	Super::EndPlay ();
	}

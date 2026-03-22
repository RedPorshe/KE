#include "KE/GameFramework/Components/SceneComponent.h"

CSceneComponent::CSceneComponent ( CObject * inOwner, const std::string & inDisplayName ) : Super ( inOwner, inDisplayName )
	{

	}

CSceneComponent::~CSceneComponent ()
	{

	}

void CSceneComponent::InitComponent ()
	{
	Super::InitComponent ();
	}

void CSceneComponent::Tick ( float DeltaTime )
	{
	Super::Tick ( DeltaTime );
	}

void CSceneComponent::OnBeginPlay ()
	{
	Super::OnBeginPlay ();
	}


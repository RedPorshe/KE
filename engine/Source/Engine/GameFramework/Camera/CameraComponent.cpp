#include "Camera/CameraComponent.h"
#include "Components/Collisions/SphereComponent.h"
#include "Actors/Actor.h"
#include "Render/RenderInfo.h"
#include "Core/Engine.h"
#include "Render/Window.h"

CCameraComponent::CCameraComponent ( CObject * inOwner, const std::string & inDisplayName )
	: Super ( inOwner, inDisplayName )
	{
	m_CameraInfo.Clear ();



	}

CCameraComponent::~CCameraComponent ()
	{}

void CCameraComponent::InitComponent ()
	{
	Super::InitComponent ();
	}

void CCameraComponent::Tick ( float DeltaTime )
	{
	Super::Tick ( DeltaTime );

	UpdateInfo ();
	}

void CCameraComponent::OnBeginPlay ()
	{
	Super::OnBeginPlay ();
	}

FMat4 CCameraComponent::GetViewMatrix () const
	{
	FVector location = GetLocation ();
	FQuat rotation = GetRotationQuat ();
	FVector up = FVector::Up ();

	
	FVector forward = rotation * FVector::Forward (); 
	forward.Normalize ();

	float viewDistance = 1000.0f;
	FVector target = location + forward * viewDistance;


	return FMat4::LookAtMatrix ( location, target, up );
	}

FMat4 CCameraComponent::GetProjectionMatrix ( float AspectRatio ) const
	{
	return FMat4::PerspectiveMatrix (
		FieldOfView * CEMath::DEG_TO_RAD, // FOV в радианах
		AspectRatio,
		NearClipPlane,
		FarClipPlane
	);
	}


FCameraInfo CCameraComponent::GetCameraInfo ( float AspectRatio ) 
	{
	UpdateInfo ();	
	return m_CameraInfo;
	}

void CCameraComponent::UpdateInfo ()
	{ 
	m_CameraInfo.Clear ();
	m_CameraInfo.Location = GetLocation ();
	// Направление взгляда
	FQuat rotation = GetRotationQuat ();
	FVector forward = rotation * FVector::Forward ();
	forward.Normalize ();
	float aspectRatio = CEngine::Get ().GetWindow ()->GetAspectRatio ();
	if (aspectRatio <= 0.0f) aspectRatio = 16.0f / 9.0f;
	m_CameraInfo.NearPlane = NearClipPlane;
	m_CameraInfo.FarPlane = FarClipPlane;
	m_CameraInfo.ViewTarget = m_CameraInfo.Location + forward * NearClipPlane;
	m_CameraInfo.FOV = FieldOfView;
	m_CameraInfo.ViewMatrix = GetViewMatrix ();
	m_CameraInfo.ProjectionMatrix = GetProjectionMatrix ( aspectRatio );
	}




#pragma once
#include "KE/GameFramework/Actors/Pawns/Pawn.h"


class CameraPawnActor : public CPawn
	{
	CHUDDO_DECLARE_CLASS ( CameraPawnActor, CPawn );
	public:
	CameraPawnActor ( CObject * inOwner = nullptr, const std::string & indisplayName = "DefaultCameraActor" );
	virtual ~CameraPawnActor ();

	void Tick ( float DeltaTime ) override;
	void BeginPlay () override;
	void EndPlay () override;
	protected:
	void SetupPlayerInputComponent ( CInputComponent * InputComponent ) override;
	class CCameraComponent * Camera = nullptr;
	float m_speed = 200.f;
	float m_mouseSens = 0.5f;
	void MoveForward ( float value );
	void MoveRight ( float value );
	void MoveUp ( float value );
	void LookUp ( float value );
	void Turn ( float Value );
	void DebugInfo ( float dt ) override;
	bool bIsTurning = false;
	bool bIsLookUp = false;

	};
REGISTER_CLASS_FACTORY ( CameraPawnActor );
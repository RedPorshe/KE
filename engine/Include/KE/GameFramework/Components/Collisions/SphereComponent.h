#pragma once

#include "Components/Collisions/BaseCollisionComponent.h"

class KE_API CSphereComponent : public CBaseCollisionComponent
	{
	CHUDDO_DECLARE_CLASS ( CSphereComponent, CBaseCollisionComponent );
	public:
		CSphereComponent(   CObject * inOwner = nullptr,
													const std::string & inDisplayName = "SphereComponent", float InRadius = 25.f);
		virtual ~CSphereComponent ();
		virtual void InitComponent () override;
		virtual void Tick ( float DeltaTime ) override;
		virtual void OnBeginPlay () override;
		virtual bool CheckCollision ( CBaseCollisionComponent * other, FCollisionInfo & outInfo ) const override;
		virtual float GetCollisionRadius () const override { return GetRadius (); }
		float GetRadius () const { return m_Radius; }
		void SetRadius ( float value ) { m_Radius = value; }
		virtual FVector GetExtremePoint ( const FVector & Direction ) const override;
	private:
		float m_Radius = 10.f;
	};
REGISTER_CLASS_FACTORY ( CSphereComponent );
#pragma once

#include "Core/Object.h"

//forward decl
class CWorld;
class CEngine;


class KE_API CGameInstance : public CObject
	{
	CHUDDO_DECLARE_CLASS ( CGameInstance, CObject );

	private:
		static CGameInstance * Instance;
		CWorld * CurrentWorld = nullptr;

	public:
		CGameInstance ( CObject * owner = nullptr, const std::string & displayName = "GameInstance" );
		virtual ~CGameInstance ();

		// ========== SINGLETON INTERFACE ==========
		static CGameInstance & Get ();
		static bool Create ();
		static void Destroy ();
		static bool IsCreated () { return Instance != nullptr; }
		bool IsMustSaveState () const { return bIsMustSaveState; }

		// ========== WORLD MANAGEMENT ==========
		CWorld * GetWorld () const { return CurrentWorld; }
		CWorld * CreateWorld ( const std::string & worldName = "World" );
		bool DestroyWorld ();

		CEngine & GetEngine ();

		// ========== GAME LIFECYCLE ==========
		virtual void Init ();
		virtual void Tick ( float deltaTime );
		virtual void Shutdown ();
		
		void SaveGameInstanceState ();

		// ========== UTILITIES ==========
		float GetGameTime () const { return GameTime; }
		float GetDeltaTime () const { return DeltaTime; }
		virtual void DumpState () const;

	protected:
		float GameTime = 0.0f;
		float DeltaTime = 0.0f;
		bool bIsMustSaveState { true };

	private:
		CGameInstance ( const CGameInstance & ) = delete;
		CGameInstance & operator=( const CGameInstance & ) = delete;

		
	};



REGISTER_CLASS_FACTORY ( CGameInstance );
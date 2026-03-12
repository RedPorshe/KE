#include "Core/Engine.h"
#include "Core/EngineInfo.h"
#include "Core/InputSystem.h"
#include "GameFramework/GameInstance.h"
#include "GameFramework/World/World.h"
#include "GameFramework/World/Level.h"
#include "Render/Window.h"
#include "GameFramework/Actors/Actor.h"
#include "GameFramework/Actors/PlayerStart.h"
#include "GameFramework/GameMode.h"
#include "Components/Collisions/BaseCollisionComponent.h"
#include "Components/Meshes/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Core/CollisionSystem.h"
#include "Core/GLFWDispatcher.h"  
#include "Actors/TerrainActor.h"
#include "Actors/Controllers/PlayerController.h"
#include "Actors/Pawns/Character.h"
#include "Actors/Pawns/Pawn.h"
#include "Render/RenderInfo.h"
#include "Render/Renderer.h"
#include <iostream>
#include <fstream>
#include <thread>

CEngine * CEngine::Instance = nullptr;

CEngine::~CEngine ()
	{
	if (bIsInitialized)
		{
		Shutdown ();
		}
	LOG_INFO ( "Engine destroyed" );
	}

CEngine & CEngine::Get ()
	{
	return *Instance;
	}

bool CEngine::InitializeEngine ( FEngineInfo & EngineInfo )
	{
	if (Instance)
		{
		LOG_ERROR ( "Engine already initialized!" );
		return false;
		}

	Instance = new CEngine ( EngineInfo );
	return Instance->Initialize ();
	}

void CEngine::ShutdownEngine ()
	{
	COLLISION_SYSTEM.Shutdown ();

	if (Instance)
		{
		delete Instance;
		Instance = nullptr;
		}
	}

bool CEngine::Initialize ()
	{
	if (bIsInitialized)
		{
		LOG_WARN ( "Engine already initialized" );
		return true;
		}
	m_LastFrameTime = std::chrono::steady_clock::now ();

	// Инициализируем Window
	Window = std::make_unique<CWindow> ( Info );
	if (!Window->Initialize ())
		{
		LOG_FATAL ( "Failed to initialize Window" );
		return false;
		}

		// ВАЖНО: Info.WindowHandle должен быть установлен в CWindow::Initialize()
	LOG_DEBUG ( "Window handle in Engine::Initialize: ", Info.WindowHandle );

	// Инициализируем InputSystem
	LOG_DEBUG ( "[ENGINE] Initializing InputSystem..." );
	if (!INPUT_SYSTEM->Initialize ( Info ))
		{
		LOG_FATAL ( "Failed to initialize InputSystem" );
		return false;
		}

		// СОЗДАЕМ И НАСТРАИВАЕМ ДИСПЕТЧЕР
	auto * dispatcher = new FGLFWDispatcher ();
	dispatcher->Window = Window.get ();
	dispatcher->InputSystem = INPUT_SYSTEM;

	// Устанавливаем единый user pointer
	glfwSetWindowUserPointer ( Info.WindowHandle, dispatcher );

	// Устанавливаем все колбэки через диспетчер
	glfwSetKeyCallback ( Info.WindowHandle, FGLFWDispatcher::KeyCallback );
	
	glfwSetMouseButtonCallback ( Info.WindowHandle, FGLFWDispatcher::MouseButtonCallback );
	glfwSetCursorPosCallback ( Info.WindowHandle, FGLFWDispatcher::CursorPositionCallback );
	glfwSetScrollCallback ( Info.WindowHandle, FGLFWDispatcher::ScrollCallback );
	glfwSetWindowSizeCallback ( Info.WindowHandle, FGLFWDispatcher::WindowSizeCallback );
	glfwSetWindowCloseCallback ( Info.WindowHandle, FGLFWDispatcher::WindowCloseCallback );
	//Создаем и инициализируем VulkanContext
	Renderer = std::make_unique<CRenderer> ( Info );
	if (!Renderer.get ()->Initialize ())
		{
		LOG_ERROR ( "Failed to Initialize Renderer..." );
		return false;
		}

	// Создаём GameInstance
	if (!CGameInstance::Create ())
		{
		LOG_FATAL ( "Failed to create GameInstance" );
		return false;
		}

		// Инициализируем CollisionSystem
	COLLISION_SYSTEM;

	bIsInitialized = true;
	LOG_INFO ( "Engine initialized successfully" );
	return true;
	}

void CEngine::Shutdown ()
	{
	LOG_INFO ( "Engine shutting down..." );

	// 1. Сначала игровая логика
	auto & GameInstance = CGameInstance::Get ();
	GameInstance.Shutdown ();
	CGameInstance::Destroy ();
	Renderer.reset ();
	// 2. Потом системы, которые используют игровые объекты
	if (INPUT_SYSTEM)
		{
// Важно: сначала обнуляем window handle в InputSystem
		INPUT_SYSTEM->SetWindow ( nullptr );
		INPUT_SYSTEM->ShutdownSystem ();
		}

		// 3. Система коллизий
	COLLISION_SYSTEM.Shutdown ();

	// 4. Очищаем диспетчер
	if (Info.WindowHandle)
		{
		FGLFWDispatcher * dispatcher = static_cast< FGLFWDispatcher * >( glfwGetWindowUserPointer ( Info.WindowHandle ) );
		if (dispatcher)
			{
			delete dispatcher;
			glfwSetWindowUserPointer ( Info.WindowHandle, nullptr );
			}
		}

		// 5. Окно последним
	if (Window)
		{
		Window->Shutdown ();
		Info.WindowHandle = nullptr;  // Важно: обнуляем после уничтожения окна
		}

	bIsInitialized = false;
	bIsRunning = false;

	LOG_DEBUG ( "Engine shutdown complete" );
	}

void CEngine::Start ()
	{
	//CreateTestWorld ();



	MainLoop ();
	}

void CEngine::RequestExit ()
	{
	LOG_DEBUG ( "Exit requested " );
	bIsRunning = false;
	}

CGameInstance & CEngine::GetGameInstance ()
	{
	return CGameInstance::Get ();
	}

void CEngine::MainLoop ()
	{
	bIsRunning = true;	

	auto & GameInstance = CGameInstance::Get ();
	GameInstance.Init ();

	while (bIsRunning && !glfwWindowShouldClose ( Info.WindowHandle ))
		{
		m_RenderInfo->Clear ();
		CalculateDeltaTime ();
		Tick ( m_DeltaTime );

		UpdateRenderInfo ();

		if (!Renderer.get ()->RenderScene ())
			{
			static int errorcount = 0;
			errorcount++;
			LOG_ERROR ( "Failed to render scene" );
			if (errorcount > 10)
				{
				RequestExit ();
				}
			}		
		}
	m_RenderInfo->Clear ();
	}

void CEngine::Tick ( float deltaTime )
	{
	glfwPollEvents ();

	if (glfwGetKey ( Info.WindowHandle, GLFW_KEY_ESCAPE ) == GLFW_PRESS)
		{
		LOG_DEBUG ( "[ENGINE] ESC pressed, exiting..." );
		RequestExit ();
		return;
		}
	INPUT_SYSTEM->Update ( deltaTime );
	CGameInstance::Get ().Tick ( deltaTime );
	CollisionSystem.Update ( deltaTime ); 
	}

void CEngine::CalculateDeltaTime ()
	{
	auto currentTime = std::chrono::steady_clock::now ();

	if (m_LastFrameTime.time_since_epoch ().count () != 0)
		{
		auto delta = std::chrono::duration_cast< std::chrono::microseconds >(
			currentTime - m_LastFrameTime ).count ();

		m_DeltaTime = static_cast< float >( delta ) / 1000000.0f;

		constexpr float MAX_DELTA = 1.0f / 10.0f;
		constexpr float MIN_DELTA = 1.0f / 244.0f;

		m_DeltaTime = std::clamp ( m_DeltaTime, MIN_DELTA, MAX_DELTA );
		}
	else
		{
		m_DeltaTime = 1.0f / 60.0f;
		}

	m_LastFrameTime = currentTime;
	}


CEngine::CEngine ( FEngineInfo & EngineInfo ) :
	CollisionSystem ( COLLISION_SYSTEM ),
	InputSystem ( *INPUT_SYSTEM ),
	Info ( EngineInfo )
	{
	m_LastFrameTime = std::chrono::steady_clock::now ();
	m_RenderInfo = new FRenderInfo ();
	m_RenderInfo->Clear ();
	}

void CEngine::UpdateRenderInfo ()
	{
	m_RenderInfo->Clear ();
	CGameInstance::Get ().GetWorld ()->CollectRenderInfo ( m_RenderInfo );
	static bool bGlobalDrawCollisions = true;
	m_RenderInfo->bDrawCollisions = bGlobalDrawCollisions || m_RenderInfo->HasDebugCollisions ();
	}

#include "Components/Meshes/TerrainMeshComponent.h"

void CEngine::EnableCollisionDebug ( bool bEnable )
	{
	CWorld * World = CGameInstance::Get ().GetWorld ();
	if (!World) return;

	for (CActor * actor : World->GetCurrentLevel ()->GetActors ())
		{
		actor->SetDrawCollisions ( bEnable );
		}
	}

//
//void CEngine::CreateTestWorld ()
//	{
//	auto world = CGameInstance::Get ().CreateWorld ( "Super" );
//	if (world)
//		{
//		auto level = world->CreateLevel<CLevel> ( "Level" );
//		auto start = level->SpawnActor<CPlayerStart> ( "playStart" );
//
//
//		// Создаём террейн - компоненты создадутся внутри GenerateHilly
//		auto Terrain = level->SpawnActor<CTerrainActor> ( "Terrain" );
//
//
//		//Terrain->GenerateHilly ( 45, 45, 15.f );
//		Terrain->GenerateNoise ( 100, 300, 25.f );
//		Terrain->SetActorLocation ( 500.f, 0.f, 500.f, true ); // true явно указываю что нужно телепортировать актора, потому что нету тиков
//		Terrain->SetDrawCollisions ( true );
//		float startHeight = Terrain->GetTerrainMeshComponent ()->GetTerrainComponent ()->GetHeightAtWorld ( FVector { 500.f, 150.3322f, 500.f } );
//		start->SetActorLocation ( FVector { 500.f, startHeight + 50.f, 500.f }, true ); // true явно указываю что нужно телепортировать актора, потому что нету тиков
//
//		LOG_DEBUG ( "Total actors after spawn: ", level->GetNumActors () );
//
//		auto gameMode = world->CreateGameMode<CGameMode> ( "SuperGameMode" );
//		gameMode->SetDefaultPawnClass ( "CCharacter" );
//		world->BeginPlay ();
//		}
//	}
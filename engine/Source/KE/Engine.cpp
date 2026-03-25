#include "KE/Engine.h"
#include "CoreMinimal.h"
#include "KE/Systems/WindowSystem.h"
#include "KE/Systems/RenderSystem.h"
#include "KE/Systems/InputSystem.h"
#include "KE/Systems/ResourceSystem.h"
#include "KE/Systems/CollisionSystem.h"
#include "KE/Systems/GLFWDispatcher.h"
#include <KE/GameFramework/GameInstance.h>
#include <KE/GameFramework/World/World.h>

#include <GLFW/glfw3.h>
#include <chrono>
#include <functional>

CEngine * CEngine::Instance = nullptr;

CEngine & CEngine::Get ()
	{
	return *Instance;
	}

CEngine::CEngine ()
	{
	Instance = this;
	LOG_DEBUG ( "Engine Created" );
	}

CEngine::~CEngine ()
	{
	LOG_DEBUG ( "engine destroyed" );
	}

WindowSystem * CEngine::GetWindow () const
	{
	return static_cast < WindowSystem * >( GetSystem<WindowSystem> ().get () );
	}

RenderSystem * CEngine::GetRenderer () const
	{
	return static_cast < RenderSystem * >( GetSystem<RenderSystem> ().get () );
	}

CInputSystem * CEngine::GetInputSystem () const
	{
	return static_cast < CInputSystem * >( GetSystem<CInputSystem> ().get () );
	}

ResourceSystem * CEngine::GetResourceSystem () const
	{
	return static_cast < ResourceSystem * >( GetSystem<ResourceSystem> ().get () );
	}

CCollisionSystem * CEngine::GetCollisionSystem () const
	{
	return static_cast < CCollisionSystem * >( GetSystem<CCollisionSystem > ().get () );
	}


int CEngine::PreInit ( int argc, char * argv [] )
	{
	if (argc != 0)
		{
		ParseCmdLine ( argc, argv );
		}

	RegisterAllSystems ();

	LOG_DEBUG ( "Exe path: ", ExePath );
	LOG_DEBUG ( "Assets path: ", GetAssetsPath () );
	LOG_DEBUG ( "Shaders path: ", GetShadersPath () );
	LOG_DEBUG ( "Shader Mesh.vert.spv path: ", GetShaderPath ( "Mesh.vert.spv" ) );
	LOG_DEBUG ( "Textures path: ", GetTexturesPath () );
	LOG_DEBUG ( "Models path: ", GetModelsPath () );

	if (!ValidatePaths ())
		{
		return 1;
		}
	if (!PreInitAllSystems ())
		{
		LOG_ERROR ( "Failed preInit systems" );
		return 2;
		}
	return 0;
	}

void CEngine::ParseCmdLine ( int argc, char * argv [] )
	{
	LOG_DEBUG ( "Parsing cmd line ..." );

	for (int i = 0; i < argc; i++)
		{
		if (i == 0)
			{
			std::string Fullpath = argv[ 0 ];
			size_t pos = Fullpath.find_last_of ( "\\/" );
			if (pos != std::string::npos)
				{
				ExePath = Fullpath.substr ( 0, pos + 1 );
				}
			else
				{
				ExePath = "./";
				}
			}
		LOG_DEBUG ( "Arg ", i, ": ", argv[ i ] );

		}
	LOG_DEBUG ( "End Parsing..." );
	}

GLFWwindow * CEngine::GetWindowHandle () const
	{
	auto windowSystem = GetSystem<WindowSystem> ();
	if (!windowSystem)
		{
		LOG_WARN ( "WindowSystem not found" );
		return nullptr;
		}

	GLFWwindow * window = windowSystem->GetWindowHandle ();
	if (!window)
		{
		LOG_WARN ( "Window handle is null" );
		return nullptr;
		}

	return window;
	}

bool CEngine::ValidatePaths () const
	{
	bool allGood = true;
	std::vector<std::string> missingFolders;

	if (!std::filesystem::exists ( GetAssetsPath () ))
		missingFolders.push_back ( GetAssetsPath () );
	if (!std::filesystem::exists ( GetShadersPath () ))
		missingFolders.push_back ( GetShadersPath () );
	if (!std::filesystem::exists ( GetTexturesPath () ))
		missingFolders.push_back ( GetTexturesPath () );
	if (!std::filesystem::exists ( GetModelsPath () ))
		missingFolders.push_back ( GetModelsPath () );

	if (!missingFolders.empty ())
		{
		LOG_ERROR ( "Error: Some required asset directories are missing!" );
		LOG_ERROR ( "Please restore the following folders:" );
		for (const auto & folder : missingFolders)
			{
			LOG_ERROR ( "  - ", folder );
			}
		allGood = false;
		}

	return allGood;
	}

void CEngine::RequestShutdown ()
	{
	bIsRunning = false;
	}

int CEngine::Init ()
	{
	if (!InitAllSystems ())
		{
		LOG_ERROR ( "Failed Init Systems" );
		return 1;
		}
	return 0;
	}

void CEngine::RegisterAllSystems ()
	{
	LOG_DEBUG ( "Register Systems..." );
	auto windowSystem = RegisterSystem<WindowSystem> ();
	if (WindowSystem * win = dynamic_cast< WindowSystem * >( windowSystem.get () ))
		{
		win->SetEnginePtr ( this );
		win->SetTitle ( "Game" );
		win->SetExtent ( 1024, 768 );
		win->OnWindowCreated = [ this ] ( GLFWwindow * window )
			{
			auto inputSystem = GetInputSystem ();
			if (inputSystem)
				{
				inputSystem->SetWindow ( window );
				}
			};

		}

	auto vulkanSystem = RegisterSystem<RenderSystem> ();
	if (RenderSystem * render = dynamic_cast< RenderSystem * >( vulkanSystem.get () ))
		{
		render->SetEnginePtr ( this );
		render->SetAplicationName ( "App" ); // need apply in Another place
		render->SetEngineName ( "KuzzbassEngine" );
		}

	auto InputSystem = RegisterSystem<CInputSystem> ();
	if (CInputSystem * input = dynamic_cast< CInputSystem * >( InputSystem.get () ))
		{
		input->SetEnginePtr ( this );		
		}

	auto CollisionSystem = RegisterSystem<CCollisionSystem> ();
	if (CCollisionSystem * collision = dynamic_cast< CCollisionSystem * >( CollisionSystem.get () ))
		{
		collision->SetEnginePtr ( this );
		}

	auto resourceSystem = RegisterSystem<ResourceSystem> ();
	if (ResourceSystem * resSystem = dynamic_cast< ResourceSystem * >( resourceSystem.get () ))
		{
		resSystem->SetEnginePtr ( this );
		}

	LOG_DEBUG ( "All Systems registered: (", m_systems.size (), ") systems" );
	}

void CEngine::Shutdown ()
	{
	for (int i = m_systems.size () - 1; i >= 0; i--)
		{
		m_systems[ i ]->Shutdown ();  // ShutDown -> Shutdown
		}
	LOG_DEBUG ( " Engine Shutdown complete" );
	}

void CEngine::Run ()
	{
	bIsRunning = true;
	
	auto lastTime = std::chrono::high_resolution_clock::now ();
	if (gameinstance)
		{
		gameinstance->Init ();
		}
	while (bIsRunning)
		{
		
		auto currentTime = std::chrono::high_resolution_clock::now ();
		float deltaTime = std::chrono::duration<float> ( currentTime - lastTime ).count ();
		lastTime = currentTime;
		if (deltaTime < 0.0001f) deltaTime = 0.016f;
		if (gameinstance)
			{
			gameinstance->Tick ( deltaTime );			
			}	
		
		for (auto system : m_systems)
			{
			system->Update ( deltaTime );
			}
		}
	if (gameinstance)
		{
		gameinstance->Shutdown ();
		}
	}
bool CEngine::PreInitAllSystems ()
	{
	for (auto system : m_systems)
		{
		if (!system->PreInit ())
			{
			LOG_ERROR ( "System preInit failed: ", system->GetSystemName () );
			return false;
			}
		}
	return true;
	}
bool CEngine::InitAllSystems ()
	{
	for (auto & system : m_systems)
		{
		if (!system->Init ())
			{
			LOG_ERROR ( "System Init failed: {}", system->GetSystemName () );
			return false;
			}
		}
	return true;
	}
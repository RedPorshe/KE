#pragma once
#include <iostream>
#include <string>
#include <filesystem>
#include "EngineObject.h"
#include "CoreMinimal.h"

struct GLFWwindow;

class KE_API CEngine
    {
    public:
        CEngine ();
        int PreInit ( int argc, char * argv [] );
        int Init ();
        void Run ();                
        void Shutdown ();
        static CEngine & Get ();
        ~CEngine ();
        void RequestShutdown ();
        const std::string & GetExePath () const { return ExePath; }

        std::string GetAssetsPath ( const std::string & relativePath = "" ) const {
            return ExePath + ASSETS_DIR + relativePath;
            }
        std::string GetShadersPath () const { return GetAssetsPath ( SHADERS_DIR ); }
        std::string GetShaderPath ( const std::string & shaderName ) const {
            return GetShadersPath () + shaderName;
            }
        std::string GetTexturesPath () const { return GetAssetsPath ( TEXTURES_DIR ); }
        std::string GetTexturePath ( const std::string & textureName ) const {
            return GetTexturesPath () + textureName;
            }
        std::string GetModelsPath () const { return GetAssetsPath ( MODELS_DIR ); }
        std::string GetModelPath ( const std::string & modelName ) const {
            return GetModelsPath () + modelName;
            }
        GLFWwindow * GetWindowHandle () const;
        template<typename T>
        TSharedPtr<T> GetSystem () const
            {
            for (auto & system : m_systems)
                {
                if (auto casted = std::dynamic_pointer_cast< T >( system ))
                    return casted;
                }
            return nullptr;
            }

        class WindowSystem * GetWindow () const;
        class RenderSystem * GetRenderer () const;
        class CInputSystem * GetInputSystem () const;
        class CCollisionSystem * GetCollisionSystem () const;

        bool ValidatePaths () const;

        template<typename T, typename... Args>
        TSystemPtr<T> RegisterSystem ( Args&&... args ) {
            auto system = MakeShared<T> ( std::forward<Args> ( args )... );
            m_systems.push_back ( system );
            return system;
            }

    private:
        static CEngine * Instance;
        void ParseCmdLine ( int argc, char * argv [] );
        bool InitAllSystems ();
        bool PreInitAllSystems ();
        void RegisterAllSystems ();
        TVector<TSystemPtr<IEngineSystem>> m_systems;
        bool bIsRunning = false;
        bool bIsinitialized { false };

#ifdef WIN32
        static constexpr const char * ASSETS_DIR = "Assets\\";
        static constexpr const char * SHADERS_DIR = "Shaders\\";
        static constexpr const char * TEXTURES_DIR = "Textures\\";
        static constexpr const char * MODELS_DIR = "Models\\";
#else
        static constexpr const char * ASSETS_DIR = "Assets/";
        static constexpr const char * SHADERS_DIR = "Shaders/";
        static constexpr const char * TEXTURES_DIR = "Textures/";
        static constexpr const char * MODELS_DIR = "Models/";
#endif

        std::string ExePath {};
    };
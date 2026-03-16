#pragma once
#include <iostream>
#include <string>
#include <filesystem>
#include "EngineObject.h"
#include "CoreMinimal.h"

class CEngine
    {
    public:
        CEngine ();
        int PreInit ( int argc, char * argv [] );
        int Init ();
        void Run ();                
        void Shutdown ();
        ~CEngine ();

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

        bool ValidatePaths () const;

        template<typename T, typename... Args>
        TSystemPtr<T> RegisterSystem ( Args&&... args ) {
            auto system = MakeShared<T> ( std::forward<Args> ( args )... );
            m_systems.push_back ( system );
            return system;
            }

    private:
        void ParseCmdLine ( int argc, char * argv [] );
        bool InitAllSystems ();
        bool PreInitAllSystems ();
        TVector<TSystemPtr<IEngineSystem>> m_systems;


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
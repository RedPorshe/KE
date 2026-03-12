#pragma once
#include "CoreMinimal.h"

class CActor;
class CCollisionSystem;
class CGameInstance;
class CWindow;
class CInputSystem;

class CRenderer;
struct FRenderInfo;
struct FEngineInfo;

class KE_API CEngine
    {
    private:
        static CEngine * Instance;

    public:
        virtual ~CEngine ();

        // Singleton
        static CEngine & Get ();
        static bool InitializeEngine ( FEngineInfo & EngineInfo );
        static void ShutdownEngine ();

        bool Initialize ();
        void Shutdown ();

        void Start ();
        void RequestExit ();
        bool IsRunning () const { return bIsRunning; }

        FRenderInfo * GetRenderInfo () const { return m_RenderInfo; }
        // GameInstance access
        CRenderer* GetRenderer () { return Renderer.get (); }
        CGameInstance & GetGameInstance (); 
        CWindow* GetWindow () const { return Window.get(); }
    protected:
        void MainLoop ();
        void Tick ( float deltaTime );
        void CalculateDeltaTime ();
      //  void CreateTestWorld ();
        void EnableCollisionDebug ( bool bEnable );
        float m_DeltaTime = 0.f;
        std::chrono::steady_clock::time_point m_LastFrameTime;
        bool bIsInitialized = false;
        bool bIsRunning = false;
        void UpdateRenderInfo ();
        
        std::unique_ptr<CWindow>  Window = nullptr;
        std::unique_ptr<CRenderer>  Renderer = nullptr;
        CCollisionSystem & CollisionSystem;
        CInputSystem & InputSystem;
        FEngineInfo & Info;
        FRenderInfo * m_RenderInfo = nullptr;
        CEngine (FEngineInfo& EngineInfo);
        friend class CActor;
    };
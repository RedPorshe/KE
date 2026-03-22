#include "KE/GameFramework/Actors/HUD.h"
#include "KE/GameFramework/Actors/Controllers/PlayerController.h"
#include "KE/GameFramework/Actors/Pawns/Pawn.h"
#include "KE/GameFramework/World/World.h"
#include "KE/GameFramework/Components/TransformComponent.h"
#include "KE/GameFramework/Components/GravityComponent.h"

CHUD::CHUD ( CObject * inOwner, const std::string & inDisplayName )
    : Super ( inOwner, inDisplayName )
    {
    LOG_DEBUG ( "[HUD] Created: ", GetName () );
    if (m_Gravity)
        {
        auto it = std::find ( ActorComponents.begin (), ActorComponents.end (), m_Gravity );
        if (it != ActorComponents.end ())
            {
            ActorComponents.erase ( it );
            }
        RemoveOwnedObject ( m_Gravity->GetName () );

        m_Gravity = nullptr;
        }
    }

CHUD::~CHUD ()
    {
    OwnerController = nullptr;
    DefaultFont = nullptr;
    CrosshairTexture = nullptr;
    }

void CHUD::SetOwnerController ( CController * InController )
    {
    OwnerController = InController;
    LOG_DEBUG ( "[HUD] Owner controller set: ",
                ( InController ? InController->GetName () : "None" ) );
    }

void CHUD::InitializeHUD ()
    {
    LOG_DEBUG ( "[HUD] Initializing HUD: ", GetName () );

    // Load default font
    // DefaultFont = GetGameInstance()->GetResourceManager()->LoadFont("Default");

    // Load crosshair texture
    // CrosshairTexture = GetGameInstance()->GetResourceManager()->LoadTexture("Crosshair");

    bVisible = true;
    }

void CHUD::DrawHUD ( float DeltaTime )
    {
    if (!bVisible) return;

    // Update current message timer
    if (CurrentMessage.RemainingTime > 0.0f)
        {
        CurrentMessage.RemainingTime -= DeltaTime;

        // Draw current message
        float ScreenWidth = GetScreenWidth ();
        float ScreenHeight = GetScreenHeight ();

        DrawText ( CurrentMessage.Text,
                   ScreenWidth * 0.5f,
                   ScreenHeight * 0.1f,
                   CurrentMessage.Color,
                   1.5f,
                   true ); // Centered
        }

        // Draw crosshair
    if (bShowCrosshair)
        {
        ShowCrosshair ();
        }

        // Draw debug info
    if (bShowDebugInfo)
        {
        ShowDebugInfo ();
        }
    }

void CHUD::Tick ( float DeltaTime )
    {
    Super::Tick ( DeltaTime );

    // Draw HUD every frame
    DrawHUD ( DeltaTime );
    }

void CHUD::BeginPlay ()
    {
    Super::BeginPlay ();
    InitializeHUD ();
    LOG_DEBUG ( "[HUD] BeginPlay: ", GetName () );
    }

void CHUD::EndPlay ()
    {
    Super::EndPlay ();
    LOG_DEBUG ( "[HUD] EndPlay: ", GetName () );
    }

void CHUD::DrawText ( const std::string & Text, float X, float Y,
                      const FVector & Color, float Scale, bool bCentered )
    {
    if (Text.empty ()) return;

    // Get renderer instance
    // CRenderer::Get().DrawText(Text, X, Y, Color, Scale, bCentered, DefaultFont);

    // For now, just log
    LOG_DEBUG ( "[HUD] DrawText: '", Text, "' at (", X, ", ", Y, ")" );
    }

void CHUD::DrawLine ( float X1, float Y1, float X2, float Y2,
                      const FVector & Color, float Thickness )
    {
        // CRenderer::Get().DrawLine(X1, Y1, X2, Y2, Color, Thickness);
    LOG_DEBUG ( "[HUD] DrawLine from (", X1, ", ", Y1, ") to (", X2, ", ", Y2, ")" );
    }

void CHUD::DrawRect ( float X, float Y, float Width, float Height,
                      const FVector & Color, bool bFilled )
    {
        // CRenderer::Get().DrawRect(X, Y, Width, Height, Color, bFilled);
    LOG_DEBUG ( "[HUD] DrawRect at (", X, ", ", Y, ") size (", Width, ", ", Height, ")" );
    }

void CHUD::DrawTexture ( CTexture * Texture, float X, float Y,
                         float Width, float Height, float Opacity )
    {
    if (!Texture) return;

    // CRenderer::Get().DrawTexture(Texture, X, Y, Width, Height, Opacity);
    LOG_DEBUG ( "[HUD] DrawTexture: ", "Texture->GetName ()",
                " at (", X, ", ", Y, ") size (", Width, ", ", Height, ")" );
    }

void CHUD::ShowMessage ( const std::string & Message, float Duration )
    {
    CurrentMessage.Text = Message;
    CurrentMessage.RemainingTime = Duration;
    CurrentMessage.Color = FVector ( 1.0f, 1.0f, 1.0f );

    LOG_DEBUG ( "[HUD] ShowMessage: '", Message, "' for ", Duration, " seconds" );
    }

void CHUD::ShowDebugInfo ()
    {
    if (!OwnerController) return;

    CPawn * ControlledPawn = OwnerController->GetPawn ();
    if (!ControlledPawn) return;

    float ScreenWidth = GetScreenWidth ();
    float ScreenHeight = GetScreenHeight ();

    // FPS Counter
    static float FPSTimer = 0.0f;
    static int32 FrameCount = 0;
    static int32 LastFPS = 0;

    FrameCount++;
    FPSTimer += GetWorld ()->GetDeltaSeconds ();

    if (FPSTimer >= 1.0f)
        {
        LastFPS = FrameCount;
        FrameCount = 0;
        FPSTimer = 0.0f;
        }

        // Draw debug info
    std::stringstream DebugText;
    DebugText << "FPS: " << LastFPS << "\n";
    DebugText << "Position: ("
        << ControlledPawn->GetActorLocation ().x << ", "
        << ControlledPawn->GetActorLocation ().y << ", "
        << ControlledPawn->GetActorLocation ().z << ")\n";
    DebugText << "Rotation: ("
        << ControlledPawn->GetActorRotation ().x << ", "
        << ControlledPawn->GetActorRotation ().y << ", "
        << ControlledPawn->GetActorRotation ().z << ")\n";
    DebugText << "Velocity: TODO\n";

    DrawText ( DebugText.str (), 10.0f, 10.0f, FVector ( 0.0f, 1.0f, 0.0f ), 0.8f, false );
    }

void CHUD::ShowCrosshair ()
    {
    float ScreenWidth = GetScreenWidth ();
    float ScreenHeight = GetScreenHeight ();

    float CenterX = ScreenWidth * 0.5f;
    float CenterY = ScreenHeight * 0.5f;
    float CrosshairSize = 20.0f;
    float Thickness = 2.0f;

    // Draw simple crosshair
    DrawLine ( CenterX - CrosshairSize, CenterY, CenterX - CrosshairSize * 0.5f, CenterY,
               FVector ( 1.0f, 1.0f, 1.0f ), Thickness );
    DrawLine ( CenterX + CrosshairSize * 0.5f, CenterY, CenterX + CrosshairSize, CenterY,
               FVector ( 1.0f, 1.0f, 1.0f ), Thickness );
    DrawLine ( CenterX, CenterY - CrosshairSize, CenterX, CenterY - CrosshairSize * 0.5f,
               FVector ( 1.0f, 1.0f, 1.0f ), Thickness );
    DrawLine ( CenterX, CenterY + CrosshairSize * 0.5f, CenterX, CenterY + CrosshairSize,
               FVector ( 1.0f, 1.0f, 1.0f ), Thickness );

      // Draw center dot
    DrawRect ( CenterX - 2.0f, CenterY - 2.0f, 4.0f, 4.0f, FVector ( 1.0f, 1.0f, 1.0f ), true );
    }

float CHUD::GetScreenWidth () const
    {
        // CRenderer::Get().GetScreenWidth();
    return 1920.0f; // Default
    }

float CHUD::GetScreenHeight () const
    {
        // CRenderer::Get().GetScreenHeight();
    return 1080.0f; // Default
    }

FVector2D CHUD::GetVirtualScreenPosition ( float X, float Y ) const
    {
    float ScreenWidth = GetScreenWidth ();
    float ScreenHeight = GetScreenHeight ();

    // Convert virtual coordinates (0-1920x1080) to actual screen coordinates
    float ActualX = ( X / VirtualScreenWidth ) * ScreenWidth;
    float ActualY = ( Y / VirtualScreenHeight ) * ScreenHeight;

    return FVector2D ( ActualX, ActualY );
    }

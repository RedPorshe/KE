#include "KE/GameFramework/GameMode.h"
#include "KE/GameFramework/World/World.h"
#include "KE/GameFramework/World/Level.h"
#include "KE/GameFramework/Actors/Controllers/PlayerController.h"
#include "KE/GameFramework/Actors/Pawns/Pawn.h"
#include "KE/GameFramework/Actors/HUD.h"
#include "KE/GameFramework/Actors/PlayerStart.h"
#include "KE/GameFramework/Actors/Actor.h"
#include "KE/GameFramework/Components/TransformComponent.h"

CGameMode::CGameMode ( CObject * inOwner, const std::string & inName )
    : Super ( inOwner, inName )
    {
    Settings = new FGameModeSettings ();
    }

CGameMode::~CGameMode ()
    {
    PlayerControllers.clear ();
    delete Settings;
    }

void CGameMode::InitGame ()
    {
    bGameStarted = false;
    bGameOver = false;
    GameTime = 0.0f;
    }

void CGameMode::StartPlay ()
    {
    bGameStarted = true;
    bGameOver = false;

    if (Settings->DefaultPawnClassName.empty ())
        {
        LOG_DEBUG ( "[GAMEMODE] Editor mode: Spawning controller only" );
        CController * Controller = SpawnPlayerController ();

        if (Controller && !Settings->DefaultHUDClassName.empty ())
            {
            SpawnHUDForController ( Controller );
            }
        return;
        }

    if (PlayerControllers.empty ())
        {
        if (ShouldSpawnPlayerAutomatically ())
            {
            SpawnPlayerForNewGame ();
            }
        }
    else
        {
        LOG_DEBUG ( "[GAMEMODE] Players already exist: ", PlayerControllers.size () );
        }
    }

void CGameMode::Tick ( float DeltaTime )
    {
    if (bGameStarted && !bGameOver)
        {
        GameTime += DeltaTime;
        for (auto controller : PlayerControllers)
            {
            controller->ProcessPlayerInput ( DeltaTime );
            }
        }
    }

void CGameMode::EndGame ()
    {
    LOG_DEBUG ( "[GAMEMODE] EndGame: ", GetName () );
    GameOver ();
    bGameStarted = false;
    bGameOver = true;
    }

void CGameMode::SetDefaultPawnClass ( const std::string & className )
    {
    if (className.empty ())
        {
        Settings->SetPawn ( "" );
        LOG_DEBUG ( "[GAMEMODE] Editor Mode enabled - no default pawn" );
        return;
        }

    if (!OBJECT_FACTORY.IsDerivedFrom ( className, "CPawn" ))
        {
        LOG_ERROR ( "[GAMEMODE] Cannot set default pawn class: '", className,
                    "' is not derived from CPawn!" );
        return;
        }

    Settings->SetPawn ( className );
    }

CController * CGameMode::SpawnPlayerController ()
    {
    if (Settings->DefaultPlayerControllerClassName.empty ())
        {
        LOG_ERROR ( "[GAMEMODE] Cannot spawn player controller: No class specified" );
        return nullptr;
        }

    if (!OwningWorld)
        {
        LOG_ERROR ( "[GAMEMODE] Cannot spawn player controller: No world" );
        return nullptr;
        }

    CLevel * CurrentLevel = OwningWorld->GetCurrentLevel ();
    if (!CurrentLevel)
        {
        LOG_ERROR ( "[GAMEMODE] Cannot spawn player controller: No current level" );
        return nullptr;
        }

    if (!PlayerControllers.empty ())
        {
        LOG_DEBUG ( "[GAMEMODE] Player controller already exists, returning existing" );
        return PlayerControllers[ 0 ];
        }
    else
        {
        std::string controllerName = "Controller_" + std::to_string ( PlayerControllers.size () + 1 );

        CObject * NewControllerObj = OBJECT_FACTORY.Create (
            Settings->DefaultPlayerControllerClassName,
            CurrentLevel,
            controllerName
        );

        CController * NewController = dynamic_cast< CController * >( NewControllerObj );

        if (NewController)
            {
            PlayerControllers.push_back ( NewController );
            NewController->SetOwningGameMode ( this );
            LOG_DEBUG ( "[GAMEMODE] Spawned player controller: ", NewController->GetName () );
            }
        else
            {
            LOG_ERROR ( "[GAMEMODE] Failed to spawn player controller of class: ",
                        Settings->DefaultPlayerControllerClassName );

            if (NewControllerObj)
                {
                delete NewControllerObj;
                }
            }

        return NewController;
        }
    }

CPawn * CGameMode::SpawnDefaultPawnForController ( CController * Controller )
    {
        // Валидация
    if (!Controller)
        {
        LOG_ERROR ( "[GAMEMODE] Cannot spawn pawn: Null controller" );
        return nullptr;
        }

    if (!OBJECT_FACTORY.IsClassRegistered ( Settings->DefaultPawnClassName ))
        {
        LOG_ERROR ( "[GAMEMODE] Cannot spawn pawn: Class '",
                    Settings->DefaultPawnClassName,
                    "' is not registered!" );
        LOG_ERROR ( "[GAMEMODE] Did you forget to REGISTER_CLASS_FACTORY?" );
        return nullptr;
        }

    if (!IsValidPawnClass ( Settings->DefaultPawnClassName ))
        {
        LOG_ERROR ( "[GAMEMODE] Cannot spawn pawn - invalid class: ",
                    Settings->DefaultPawnClassName );
        return nullptr;
        }

        // Editor Mode - только контроллер
    if (Settings->DefaultPawnClassName.empty ())
        {
        LOG_DEBUG ( "[GAMEMODE] Editor Mode - spawning controller only" );
        return nullptr;
        }

    if (!OwningWorld)
        {
        LOG_ERROR ( "[GAMEMODE] Cannot spawn pawn: No world" );
        return nullptr;
        }

    CLevel * CurrentLevel = OwningWorld->GetCurrentLevel ();
    if (!CurrentLevel)
        {
        LOG_ERROR ( "[GAMEMODE] Cannot spawn pawn: No current level" );
        return nullptr;
        }

        // Находим стартовую позицию
    FTransform SpawnTransform = FTransform::Identity ();
    CActor * PlayerStart = FindPlayerStart ( Controller );

    if (CPlayerStart * Start = dynamic_cast< CPlayerStart * >( PlayerStart ))
        {
        SpawnTransform.Location = Start->GetActorLocation ();
        SpawnTransform.Rotation = Start->GetActorRotationQuat ();
        SpawnTransform.Scale = Start->GetActorScale ();
        }

        // Генерируем уникальное имя для pawn
    std::string pawnName = Controller->GetName () + "_Pawn";

    // Передаем позицию при спавне
    auto NewPawnObj = CurrentLevel->SpawnActorByClass (
        Settings->DefaultPawnClassName,
        pawnName,
        SpawnTransform.Location
    );

    CPawn * NewPawn = dynamic_cast< CPawn * >( NewPawnObj );

    if (NewPawn)
        {
            // Устанавливаем вращение и масштаб (позиция уже установлена)
        NewPawn->SetActorRotation ( SpawnTransform.Rotation );
        NewPawn->SetActorScale ( SpawnTransform.Scale );

        // Устанавливаем состояние
        NewPawn->SetMovableState ( EMovableState::DYNAMIC );

        LOG_DEBUG ( "[GAMEMODE] Spawned pawn: ", NewPawn->GetName (),
                    " at location: ", SpawnTransform.Location );
        }

    return NewPawn;
    }

void CGameMode::SpawnHUDForController ( CController * Controller )
    {
    if (!Controller || Settings->DefaultHUDClassName.empty ()) return;

    if (!Controller->GetHUD ())
        {
        CHUD * NewHUD = dynamic_cast< CHUD * >(
            OBJECT_FACTORY.Create (
                Settings->DefaultHUDClassName,
                Controller,
                Controller->GetName () + "_HUD"
            )
            );

        if (NewHUD)
            {
            Controller->SetHUD ( NewHUD );
            LOG_DEBUG ( "[GAMEMODE] Spawned HUD: ", NewHUD->GetName () );
            }
        }
    }

void CGameMode::RestartPlayer ( CController * Controller )
    {
    if (!Controller)
        {
        LOG_ERROR ( "[GAMEMODE] Cannot restart player: Null controller" );
        return;
        }

        // Проверяем, есть ли уже Pawn
    CPawn * OldPawn = Controller->GetPawn ();
    if (OldPawn)
        {
        LOG_DEBUG ( "[GAMEMODE] Controller already has pawn: ", OldPawn->GetName () );
        return;
        }

        // Спавним новый pawn
    CPawn * NewPawn = SpawnDefaultPawnForController ( Controller );
    if (NewPawn)
        {
        Controller->Possess ( NewPawn );

        // Спавним HUD если его нет
        SpawnHUDForController ( Controller );

        LOG_DEBUG ( "[GAMEMODE] Restarted player: ", Controller->GetName (),
                    " with pawn: ", NewPawn->GetName () );
        }
    }

void CGameMode::RestartPlayerAtTransform ( CController * Controller, const FTransform & SpawnTransform )
    {
    if (!Controller)
        {
        LOG_ERROR ( "[GAMEMODE] Cannot restart player: Null controller" );
        return;
        }

        // Уничтожаем старый pawn если есть
    CPawn * OldPawn = Controller->GetPawn ();
    if (OldPawn)
        {
        LOG_DEBUG ( "[GAMEMODE] Destroying old pawn: ", OldPawn->GetName () );
        OldPawn->Destroy ();
        Controller->Unpossess ();
        }

        // Спавним новый pawn
    CPawn * NewPawn = SpawnDefaultPawnForController ( Controller );
    if (NewPawn)
        {
            // Если передана трансформация, используем её
        if (!SpawnTransform.IsIdentity ())
            {
            CTransformComponent * PawnTransform = NewPawn->GetRootComponent ();
            if (PawnTransform)
                {
                PawnTransform->SetTransform ( SpawnTransform );
                }
            }

        Controller->Possess ( NewPawn );

        // Спавним HUD если нужно
        SpawnHUDForController ( Controller );

        LOG_DEBUG ( "[GAMEMODE] Restarted player at transform: ", Controller->GetName () );
        }
    }

void CGameMode::RemovePlayerController ( CController * Controller )
    {
    auto it = std::find ( PlayerControllers.begin (), PlayerControllers.end (), Controller );
    if (it != PlayerControllers.end ())
        {
        PlayerControllers.erase ( it );
        LOG_DEBUG ( "[GAMEMODE] Removed player controller: ", Controller->GetName () );
        }
    }

bool CGameMode::IsValidPawnClass ( const std::string & ClassName ) const
    {
    if (ClassName.empty ())
        {
        LOG_DEBUG ( "[GAMEMODE] Empty pawn class - entering Editor Mode" );
        return true;
        }

    if (!OBJECT_FACTORY.IsClassRegistered ( ClassName ))
        {
        LOG_ERROR ( "[GAMEMODE] Cannot spawn pawn: Class '", ClassName, "' is not registered!" );
        LOG_ERROR ( "[GAMEMODE] Did you forget to REGISTER_CLASS_FACTORY?" );
        return false;
        }

    if (!OBJECT_FACTORY.IsDerivedFrom ( ClassName, "CPawn" ))
        {
        LOG_ERROR ( "[GAMEMODE] Cannot spawn pawn: Class '", ClassName,
                    "' is not derived from CPawn!" );
        return false;
        }

    LOG_DEBUG ( "[GAMEMODE] Valid pawn class: ", ClassName );
    return true;
    }

void CGameMode::SpawnPlayerForNewGame ()
    {
    if (!PlayerControllers.empty ())
        {
        LOG_DEBUG ( "[GAMEMODE] Player already exists, skipping auto-spawn" );
        return;
        }

    CController * Controller = SpawnPlayerController ();
    if (Controller)
        {
        RestartPlayer ( Controller );
        if (auto * Pawn = Controller->GetPawn ())
            {
            Pawn->GetRootComponent ()->UpdateTransform ();
            }
        LOG_DEBUG ( "[GAMEMODE] Player auto-spawned at game start! at location: ",
                    ( Controller->GetPawn () ? Controller->GetPawn ()->GetActorLocation () : FVector::Zero () ) );
        }
    }

CActor * CGameMode::FindPlayerStart ( CController * Player )
    {
    if (!OwningWorld)
        {
        LOG_WARN ( "[GAMEMODE] Cannot find player start: No world" );
        return nullptr;
        }

    CLevel * CurrentLevel = OwningWorld->GetCurrentLevel ();
    if (!CurrentLevel)
        {
        LOG_WARN ( "[GAMEMODE] Cannot find player start: No current level" );
        return nullptr;
        }

        // Собираем все PlayerStart акторы из уровня
    std::vector<CActor *> PlayerStarts;

    // Используем рекурсивный поиск CObject
    std::function<void ( CObject * )> CollectPlayerStarts = [ & ] ( CObject * Obj )
        {
        if (!Obj) return;

        CActor * Actor = dynamic_cast< CActor * >( Obj );
        if (Actor)
            {
            if (strcmp ( Actor->GetObjectClassName (), "CPlayerStart" ) == 0)
                {
                PlayerStarts.push_back ( Actor );
                }
            }

        for (const auto & Owned : Obj->GetOwnedObjects ())
            {
            CollectPlayerStarts ( Owned.get () );
            }
        };

    CollectPlayerStarts ( CurrentLevel );

    if (PlayerStarts.empty ())
        {
        LOG_WARN ( "[GAMEMODE] No PlayerStart actors found in level: ", CurrentLevel->GetName () );
        return nullptr;
        }

        // Фильтруем только валидные старты
    std::vector<CActor *> ValidStarts;
    for (CActor * Start : PlayerStarts)
        {
        if (IsValidPlayerStart ( Start ))
            {
            ValidStarts.push_back ( Start );
            }
        }

    if (ValidStarts.empty ())
        {
        LOG_WARN ( "[GAMEMODE] No valid PlayerStart actors found" );
        return nullptr;
        }

        // Простой алгоритм: выбираем первый валидный старт
    return ValidStarts[ 0 ];
    }

bool CGameMode::IsValidPlayerStart ( CActor * PlayerStart )
    {
    if (!PlayerStart) return false;

    if (PlayerStart->IsPendingToDestroy ())
        {
        return false;
        }

    return true;
    }

void CGameMode::GameOver ()
    {
    if (bGameOver) return;

    LOG_DEBUG ( "[GAMEMODE] Game Over! Final time: ", GameTime, " seconds" );
    bGameOver = true;
    bGameStarted = false;
    }
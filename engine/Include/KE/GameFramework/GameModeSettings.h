#pragma once

#include "CoreMinimal.h"

struct FGameModeSettings
    {
    std::string DefaultPawnClassName = "CPawn";
    std::string DefaultPlayerControllerClassName = "CPlayerController";
    std::string DefaultHUDClassName = "CHUD";
    std::string DefaultPlayerStartClassName = "CPlayerStart";

    // С проверкой на пустоту и изменением
    void SetPawn ( const std::string & inClassName )
        {        
            DefaultPawnClassName = inClassName;
        }

    void SetPlayerController ( const std::string & inClassName )
        {
        if (!inClassName.empty () && DefaultPlayerControllerClassName != inClassName)
            DefaultPlayerControllerClassName = inClassName;
        }

    void SetHUD ( const std::string & inClassName )
        {       
            DefaultHUDClassName = inClassName;
        }

    void SetPlayerStart ( const std::string & inClassName )
        {
        if (!inClassName.empty () && DefaultPlayerStartClassName != inClassName)
            DefaultPlayerStartClassName = inClassName;
        }
     
    };
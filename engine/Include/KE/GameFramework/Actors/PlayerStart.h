#pragma once

#include "Actors/Actor.h"

class KE_API CPlayerStart : public CActor
    {
    CHUDDO_DECLARE_CLASS ( CPlayerStart, CActor )

    public:
        CPlayerStart ( CObject * inOwner = nullptr, const std::string & inName = "PlayerStart" );
        virtual ~CPlayerStart ();


        void BeginPlay () override;
        void Tick ( float DeltaTime ) override;
        void EndPlay () override;
        // Индекс для мультиплеера
        void SetPlayerStartIndex ( int32 Index ) { PlayerStartIndex = Index; }
        int32 GetPlayerStartIndex () const { return PlayerStartIndex; }

        // Тег для разных команд/классов
        void SetStartTag ( const std::string & Tag ) { StartTag = Tag; }
        const std::string & GetStartTag () const { return StartTag; }

        // Активен ли старт
        bool IsEnabled () const { return bEnabled; }
        void SetEnabled ( bool bEnabled ) { this->bEnabled = bEnabled; }

        // Занят ли другим игроком
        bool IsOccupied () const { return bOccupied; }
        void SetOccupied ( bool bOccupied ) { this->bOccupied = bOccupied; }

    protected:
        int32 PlayerStartIndex = 0;
        std::string StartTag = "Default";
        bool bEnabled = true;
        bool bOccupied = false;
    };

REGISTER_CLASS_FACTORY ( CPlayerStart );
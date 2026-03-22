#include "Core/Collision.h"
#include <cassert>
#include <algorithm>

// ==================== Определения статических членов FCollisionChannelRegistry ====================

// Определяем статические члены класса
std::unordered_map<std::string, ECollisionChannel> FCollisionChannelRegistry::s_NameToChannel;
std::unordered_map<ECollisionChannel, std::string> FCollisionChannelRegistry::s_ChannelToName;
ECollisionChannel FCollisionChannelRegistry::s_NextCustomChannel = ECollisionChannel::CustomStart;
bool FCollisionChannelRegistry::s_Initialized = false;

// ==================== FCollisionChannelRegistry Implementation ====================

namespace
    {
    const std::vector<std::string> STANDARD_CHANNEL_NAMES = {
        "NoCollision",
        "WorldStatic",
        "WorldDynamic",
        "Pawn",
        "Character",
        "Vehicle",
        "Trigger",
        "Interactable"
        };
    }

void FCollisionChannelRegistry::InitializeStandardChannels ()
    {
    if (s_Initialized) return;

    // Регистрируем стандартные каналы
    for (size_t i = 0; i < STANDARD_CHANNEL_NAMES.size (); ++i)
        {
        ECollisionChannel channel = static_cast< ECollisionChannel > ( i );
        const std::string & name = STANDARD_CHANNEL_NAMES[ i ];

        s_NameToChannel[ name ] = channel;
        s_ChannelToName[ channel ] = name;
        }

    s_Initialized = true;
    }

ECollisionChannel FCollisionChannelRegistry::RegisterChannel ( const std::string & channelName,
                                                               ECollisionResponse defaultResponse )
    {
    InitializeStandardChannels ();

    // Проверяем, не зарегистрирован ли уже канал с таким именем
    auto it = s_NameToChannel.find ( channelName );
    if (it != s_NameToChannel.end ())
        {
        return it->second;
        }

        // Проверяем, есть ли свободные каналы
    if (s_NextCustomChannel >= ECollisionChannel::MaxChannels)
        {
        assert ( !"Too many collision channels registered!" );
        return ECollisionChannel::NoCollision;
        }

        // Регистрируем новый канал
    ECollisionChannel newChannel = s_NextCustomChannel;
    s_NameToChannel[ channelName ] = newChannel;
    s_ChannelToName[ newChannel ] = channelName;

    // Увеличиваем счетчик для следующего канала
    s_NextCustomChannel = static_cast< ECollisionChannel >(
        static_cast< uint16_t >( s_NextCustomChannel ) + 1 );

    return newChannel;
    }

std::optional<ECollisionChannel> FCollisionChannelRegistry::GetChannelByName ( const std::string & channelName )
    {
    InitializeStandardChannels ();

    auto it = s_NameToChannel.find ( channelName );
    if (it != s_NameToChannel.end ())
        {
        return it->second;
        }
    return std::nullopt;
    }

std::string FCollisionChannelRegistry::GetChannelName ( ECollisionChannel channel )
    {
    InitializeStandardChannels ();

    auto it = s_ChannelToName.find ( channel );
    if (it != s_ChannelToName.end ())
        {
        return it->second;
        }
    return "Unknown";
    }

const std::unordered_map<std::string, ECollisionChannel> &
FCollisionChannelRegistry::GetAllRegisteredChannels ()
    {
    InitializeStandardChannels ();
    return s_NameToChannel;
    }

std::vector<std::string> FCollisionChannelRegistry::GetAllChannelNames ()
    {
    InitializeStandardChannels ();

    std::vector<std::string> names;
    names.reserve ( s_NameToChannel.size () );

    for (const auto & pair : s_NameToChannel)
        {
        names.push_back ( pair.first );
        }

    std::sort ( names.begin (), names.end () );
    return names;
    }

bool FCollisionChannelRegistry::IsValidChannel ( ECollisionChannel channel )
    {
    return channel < ECollisionChannel::MaxChannels;
    }

bool FCollisionChannelRegistry::IsCustomChannel ( ECollisionChannel channel )
    {
    return static_cast< uint16_t > ( channel ) >= static_cast< uint16_t > ( ECollisionChannel::CustomStart );
    }

bool FCollisionChannelRegistry::RemoveCustomChannel ( const std::string & channelName )
    {
    InitializeStandardChannels ();

    auto it = s_NameToChannel.find ( channelName );
    if (it != s_NameToChannel.end ())
        {
        ECollisionChannel channel = it->second;

        // Можно удалять только пользовательские каналы
        if (IsCustomChannel ( channel ))
            {
            s_NameToChannel.erase ( it );
            s_ChannelToName.erase ( channel );
            return true;
            }
        }
    return false;
    }

void FCollisionChannelRegistry::ClearCustomChannels ()
    {
    InitializeStandardChannels ();

    // Удаляем только пользовательские каналы
    for (auto it = s_NameToChannel.begin (); it != s_NameToChannel.end ();)
        {
        if (IsCustomChannel ( it->second ))
            {
            s_ChannelToName.erase ( it->second );
            it = s_NameToChannel.erase ( it );
            }
        else
            {
            ++it;
            }
        }
    s_NextCustomChannel = ECollisionChannel::CustomStart;
    }

    // ==================== FCollisionChannel Implementation ====================

FCollisionChannel::FCollisionChannel ()
    : Channel ( ECollisionChannel::WorldStatic )
    , DefaultResponse ( ECollisionResponse::BLOCK )
    {
    InitializeDefaultResponses ();
    }

FCollisionChannel::FCollisionChannel ( ECollisionChannel inChannel,
                                       ECollisionResponse inResponse )
    : Channel ( inChannel )
    , DefaultResponse ( inResponse )
    {
    InitializeDefaultResponses ();
    }

FCollisionChannel::FCollisionChannel ( const std::string & channelName,
                                       ECollisionResponse defaultResponse )
    : DefaultResponse ( defaultResponse )
    {
        // Получаем или регистрируем канал
    auto channelOpt = FCollisionChannelRegistry::GetChannelByName ( channelName );
    if (channelOpt)
        {
        Channel = *channelOpt;
        }
    else
        {
        Channel = FCollisionChannelRegistry::RegisterChannel ( channelName, defaultResponse );
        }

    InitializeDefaultResponses ();
    }

void FCollisionChannel::InitializeDefaultResponses ()
    {
        // Инициализируем все каналы значением по умолчанию
    for (uint16_t i = 0; i < static_cast< uint16_t > ( ECollisionChannel::MaxChannels ); ++i)
        {
        ECollisionChannel otherChannel = static_cast< ECollisionChannel > ( i );
        ResponseChannels[ otherChannel ] = DefaultResponse;
        }
    }

void FCollisionChannel::SetResponseTo ( ECollisionChannel inChannel, ECollisionResponse inResponse )
    {
    ResponseChannels[ inChannel ] = inResponse;
    }

void FCollisionChannel::SetResponseTo ( const std::string & otherChannelName, ECollisionResponse inResponse )
    {
    auto channelOpt = FCollisionChannelRegistry::GetChannelByName ( otherChannelName );
    if (channelOpt)
        {
        ResponseChannels[ *channelOpt ] = inResponse;
        }
    else
        {
            // Автоматически регистрируем канал, если его нет
        ECollisionChannel newChannel = FCollisionChannelRegistry::RegisterChannel ( otherChannelName );
        ResponseChannels[ newChannel ] = inResponse;
        }
    }

ECollisionResponse FCollisionChannel::GetResponseTo ( ECollisionChannel inChannel ) const
    {
    auto it = ResponseChannels.find ( inChannel );
    if (it != ResponseChannels.end ())
        {
        return it->second;
        }
    return DefaultResponse;
    }

ECollisionResponse FCollisionChannel::GetResponseTo ( const std::string & otherChannelName ) const
    {
    auto channelOpt = FCollisionChannelRegistry::GetChannelByName ( otherChannelName );
    if (channelOpt)
        {
        return GetResponseTo ( *channelOpt );
        }
    return DefaultResponse;
    }

void FCollisionChannel::SetIgnoreAll ()
    {
    for (auto & pair : ResponseChannels)
        {
        pair.second = ECollisionResponse::IGNORE;
        }
    }

void FCollisionChannel::SetBlockAll ()
    {
    for (auto & pair : ResponseChannels)
        {
        pair.second = ECollisionResponse::BLOCK;
        }
    }

void FCollisionChannel::SetOverlapAll ()
    {
    for (auto & pair : ResponseChannels)
        {
        pair.second = ECollisionResponse::OVERLAP;
        }
    }

bool FCollisionChannel::CanCollideWith ( ECollisionChannel otherChannel ) const
    {
    ECollisionResponse response = GetResponseTo ( otherChannel );
    return response != ECollisionResponse::IGNORE;
    }

bool FCollisionChannel::CanCollideWith ( const std::string & otherChannelName ) const
    {
    ECollisionResponse response = GetResponseTo ( otherChannelName );
    return response != ECollisionResponse::IGNORE;
    }

    // ========== Быстрые настройки ==========

void FCollisionChannel::SetupAsStatic ()
    {
    Channel = ECollisionChannel::WorldStatic;
    DefaultResponse = ECollisionResponse::BLOCK;
    SetBlockAll ();
    SetResponseTo ( ECollisionChannel::WorldStatic, ECollisionResponse::IGNORE );
    SetResponseTo ( ECollisionChannel::Trigger, ECollisionResponse::IGNORE );
    }

void FCollisionChannel::SetupAsDynamic ()
    {
    Channel = ECollisionChannel::WorldDynamic;
    DefaultResponse = ECollisionResponse::BLOCK;
    SetBlockAll ();
    SetResponseTo ( ECollisionChannel::Trigger, ECollisionResponse::OVERLAP );
    }

void FCollisionChannel::SetupAsCharacter ()
    {
    Channel = ECollisionChannel::Character;
    DefaultResponse = ECollisionResponse::BLOCK;
    SetBlockAll ();
    SetResponseTo ( ECollisionChannel::Trigger, ECollisionResponse::OVERLAP );
    SetResponseTo ( ECollisionChannel::Character, ECollisionResponse::IGNORE );
    }

void FCollisionChannel::SetupAsTrigger ()
    {
    Channel = ECollisionChannel::Trigger;
    DefaultResponse = ECollisionResponse::IGNORE;
    SetIgnoreAll ();
    SetResponseTo ( ECollisionChannel::Character, ECollisionResponse::OVERLAP );
    SetResponseTo ( ECollisionChannel::Pawn, ECollisionResponse::OVERLAP );
    SetResponseTo ( ECollisionChannel::WorldDynamic, ECollisionResponse::OVERLAP );
    }

void FCollisionChannel::SetupAsPawn ()
    {
    Channel = ECollisionChannel::Pawn;
    DefaultResponse = ECollisionResponse::BLOCK;
    SetBlockAll ();
    SetResponseTo ( ECollisionChannel::Trigger, ECollisionResponse::OVERLAP );
    }

void FCollisionChannel::SetupAsVehicle ()
    {
    Channel = ECollisionChannel::Vehicle;
    DefaultResponse = ECollisionResponse::BLOCK;
    SetBlockAll ();
    SetResponseTo ( ECollisionChannel::Trigger, ECollisionResponse::OVERLAP );
    }

void FCollisionChannel::SetupAsInteractable ()
    {
    Channel = ECollisionChannel::Interactable;
    DefaultResponse = ECollisionResponse::IGNORE;
    SetIgnoreAll ();
    SetResponseTo ( ECollisionChannel::Character, ECollisionResponse::OVERLAP );
    SetResponseTo ( ECollisionChannel::Pawn, ECollisionResponse::OVERLAP );
    }

    // ========== Factory Methods ==========

FCollisionChannel FCollisionChannel::CreateStatic ()
    {
    FCollisionChannel chan;
    chan.SetupAsStatic ();
    return chan;
    }

FCollisionChannel FCollisionChannel::CreateDynamic ()
    {
    FCollisionChannel chan;
    chan.SetupAsDynamic ();
    return chan;
    }

FCollisionChannel FCollisionChannel::CreateCharacter ()
    {
    FCollisionChannel chan;
    chan.SetupAsCharacter ();
    return chan;
    }

FCollisionChannel FCollisionChannel::CreateTrigger ()
    {
    FCollisionChannel chan;
    chan.SetupAsTrigger ();
    return chan;
    }

FCollisionChannel FCollisionChannel::CreatePawn ()
    {
    FCollisionChannel chan;
    chan.SetupAsPawn ();
    return chan;
    }

FCollisionChannel FCollisionChannel::CreateVehicle ()
    {
    FCollisionChannel chan;
    chan.SetupAsVehicle ();
    return chan;
    }

FCollisionChannel FCollisionChannel::CreateInteractable ()
    {
    FCollisionChannel chan;
    chan.SetupAsInteractable ();
    return chan;
    }

FCollisionChannel FCollisionChannel::Create ( const std::string & channelName,
                                              ECollisionResponse defaultResponse )
    {
    return FCollisionChannel ( channelName, defaultResponse );
    }

    // ========== Static Getters ==========

FCollisionChannel FCollisionChannel::Static ()
    {
    return CreateStatic ();
    }

FCollisionChannel FCollisionChannel::Dynamic ()
    {
    return CreateDynamic ();
    }

FCollisionChannel FCollisionChannel::Character ()
    {
    return CreateCharacter ();
    }

FCollisionChannel FCollisionChannel::Trigger ()
    {
    return CreateTrigger ();
    }

FCollisionChannel FCollisionChannel::Pawn ()
    {
    return CreatePawn ();
    }

FCollisionChannel FCollisionChannel::Vehicle ()
    {
    return CreateVehicle ();
    }

FCollisionChannel FCollisionChannel::Interactable ()
    {
    return CreateInteractable ();
    }

FCollisionChannel FCollisionChannel::Get ( const std::string & channelName )
    {
    return FCollisionChannel ( channelName );
    }

    // ========== Вспомогательные методы ==========

std::string FCollisionChannel::GetName () const
    {
    return FCollisionChannelRegistry::GetChannelName ( Channel );
    }

    // ==================== Collision Namespace ====================

namespace Collision
    {
    const char * ShapeToString ( ECollisionShape shape )
        {
        if (shape < ECollisionShape::NONE || shape >= ECollisionShape::MAX)
            return "Invalid";
        switch (shape)
            {
                case ECollisionShape::NONE: return "None";
                case ECollisionShape::BOX: return "Box";
                case ECollisionShape::SPHERE: return "Sphere";
                case ECollisionShape::CAPSULE: return "Capsule";
                case ECollisionShape::CYLINDER: return "Cylinder";
                case ECollisionShape::CONE: return "Cone";
                case ECollisionShape::COMPOUND: return "Compound";
                case ECollisionShape::MESH: return "Mesh";
                case ECollisionShape::TERRAIN: return "Terrain";
                case ECollisionShape::RAY: return "Ray";
                case ECollisionShape::PLANE: return "Plane";                
                case ECollisionShape::MAX: 
                default: return "Unknown";
            }
        }

    const char * ResponseToString ( ECollisionResponse response )
        {
        switch (response)
            {
                case ECollisionResponse::IGNORE: return "Ignore";
                case ECollisionResponse::OVERLAP: return "Overlap";
                case ECollisionResponse::BLOCK: return "Block";
                default: return "Unknown";
            }
        }

    std::string ChannelToString ( ECollisionChannel channel )
        {
        return FCollisionChannelRegistry::GetChannelName ( channel );
        }

    FCollisionChannel CreateChannel ( const std::string & name,
                                      ECollisionResponse defaultResponse )
        {
        return FCollisionChannel::Create ( name, defaultResponse );
        }

    void SetChannelResponse ( const std::string & channelA,
                              const std::string & channelB,
                              ECollisionResponse response )
        {
        FCollisionChannel chanA = FCollisionChannel::Get ( channelA );
        FCollisionChannel chanB = FCollisionChannel::Get ( channelB );

        chanA.SetResponseTo ( channelB, response );
        chanB.SetResponseTo ( channelA, response );
        }

    bool CanChannelsCollide ( const std::string & channelA, const std::string & channelB )
        {
        FCollisionChannel chanA = FCollisionChannel::Get ( channelA );
        return chanA.CanCollideWith ( channelB );
        }

    ECollisionResponse GetResponseBetween ( const std::string & channelA, const std::string & channelB )
        {
        FCollisionChannel chanA = FCollisionChannel::Get ( channelA );
        return chanA.GetResponseTo ( channelB );
        }
    }
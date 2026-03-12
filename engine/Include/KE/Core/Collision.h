#pragma once
#include "CoreMinimal.h"
#include <unordered_map>
#include <string>
#include <optional>
#include <vector>
#include "KE/Core/KEExport.h"

enum class ECollisionShape : uint8_t
	{

	NONE = 0,
	SPHERE,
	BOX,
	CAPSULE,
	CYLINDER,
	CONE,
	

	COMPOUND,
	MESH,
	TERRAIN,       


	RAY,
	PLANE,

	MAX
	};

enum class ECollisionChannel : uint16_t
	{
		// Стандартные каналы
	NoCollision = 0,
	WorldStatic = 1,
	WorldDynamic = 2,
	Pawn = 3,
	Character = 4,
	Vehicle = 5,
	Trigger = 6,
	Interactable = 7,
	Camera = 8,
	// Пользовательские каналы начинаются здесь
	CustomStart = 9,

	// Максимальное количество каналов
	MaxChannels = 64
	};

enum class ECollisionResponse : uint8_t
	{
	IGNORE,
	OVERLAP,
	BLOCK
	};

	// ==================== Реестр каналов ====================
class KE_API FCollisionChannelRegistry
	{
	public:
		// Регистрация нового канала по имени
		static ECollisionChannel RegisterChannel ( const std::string & channelName,
												   ECollisionResponse defaultResponse = ECollisionResponse::BLOCK );

		   // Получение канала по имени
		static std::optional<ECollisionChannel> GetChannelByName ( const std::string & channelName );

		// Получение имени канала
		static std::string GetChannelName ( ECollisionChannel channel );

		// Получение всех зарегистрированных каналов
		static const std::unordered_map<std::string, ECollisionChannel> & GetAllRegisteredChannels ();

		// Получение всех имён каналов
		static std::vector<std::string> GetAllChannelNames ();

		// Проверка валидности канала
		static bool IsValidChannel ( ECollisionChannel channel );

		// Проверка, является ли канал пользовательским
		static bool IsCustomChannel ( ECollisionChannel channel );

		// Удаление пользовательского канала
		static bool RemoveCustomChannel ( const std::string & channelName );

		// Очистка всех пользовательских каналов
		static void ClearCustomChannels ();

	private:
		static std::unordered_map<std::string, ECollisionChannel> s_NameToChannel;
		static std::unordered_map<ECollisionChannel, std::string> s_ChannelToName;
		static ECollisionChannel s_NextCustomChannel;
		static bool s_Initialized;

		static void InitializeStandardChannels ();
	};

	// ==================== Основной класс канала ====================
struct KE_API FCollisionChannel
	{
	ECollisionChannel Channel;
	ECollisionResponse DefaultResponse;
	std::unordered_map<ECollisionChannel, ECollisionResponse> ResponseChannels;

	// ========== Конструкторы ==========
	FCollisionChannel ();
	FCollisionChannel ( ECollisionChannel inChannel,
						ECollisionResponse inResponse = ECollisionResponse::BLOCK );
	FCollisionChannel ( const std::string & channelName,
						ECollisionResponse defaultResponse = ECollisionResponse::BLOCK );

	   // ========== Основные методы ==========
	void SetResponseTo ( ECollisionChannel inChannel, ECollisionResponse inResponse );
	void SetResponseTo ( const std::string & otherChannelName, ECollisionResponse inResponse );

	ECollisionResponse GetResponseTo ( ECollisionChannel inChannel ) const;
	ECollisionResponse GetResponseTo ( const std::string & otherChannelName ) const;

	void SetIgnoreAll ();
	void SetBlockAll ();
	void SetOverlapAll ();

	// ========== Проверка коллизии ==========
	bool CanCollideWith ( ECollisionChannel otherChannel ) const;
	bool CanCollideWith ( const std::string & otherChannelName ) const;

	  // ========== Проверка типа реакции ==========
	bool ShouldBlock ( ECollisionChannel otherChannel ) const
		{
		return GetResponseTo ( otherChannel ) == ECollisionResponse::BLOCK;
		}

	bool ShouldBlock ( const std::string & otherChannelName ) const
		{
		return GetResponseTo ( otherChannelName ) == ECollisionResponse::BLOCK;
		}

	bool ShouldOverlap ( ECollisionChannel otherChannel ) const
		{
		return GetResponseTo ( otherChannel ) == ECollisionResponse::OVERLAP;
		}

	bool ShouldOverlap ( const std::string & otherChannelName ) const
		{
		return GetResponseTo ( otherChannelName ) == ECollisionResponse::OVERLAP;
		}

	// ========== Быстрые настройки ==========
	void SetupAsStatic ();
	void SetupAsDynamic ();
	void SetupAsCharacter ();
	void SetupAsTrigger ();
	void SetupAsPawn ();
	void SetupAsVehicle ();
	void SetupAsInteractable ();

	// ========== Статические фабричные методы ==========
	static FCollisionChannel CreateStatic ();
	static FCollisionChannel CreateDynamic ();
	static FCollisionChannel CreateCharacter ();
	static FCollisionChannel CreateTrigger ();
	static FCollisionChannel CreatePawn ();
	static FCollisionChannel CreateVehicle ();
	static FCollisionChannel CreateInteractable ();
	static FCollisionChannel Create ( const std::string & channelName,
									  ECollisionResponse defaultResponse = ECollisionResponse::BLOCK );

	   // ========== Статические геттеры ==========
	static FCollisionChannel Static ();
	static FCollisionChannel Dynamic ();
	static FCollisionChannel Character ();
	static FCollisionChannel Trigger ();
	static FCollisionChannel Pawn ();
	static FCollisionChannel Vehicle ();
	static FCollisionChannel Interactable ();
	static FCollisionChannel Get ( const std::string & channelName );

	// ========== Вспомогательные методы ==========
	std::string GetName () const;
	ECollisionChannel GetChannel () const { return Channel; }
	ECollisionResponse GetDefaultResponse () const { return DefaultResponse; }

	private:
		void InitializeDefaultResponses ();
	};

	// ==================== Вспомогательные функции ====================
namespace Collision
	{
		// Конвертация в строку
	KE_API const char * ShapeToString ( ECollisionShape shape );
	KE_API const char * ResponseToString ( ECollisionResponse response );
	KE_API std::string ChannelToString ( ECollisionChannel channel );

	// Работа с каналами по имени
	KE_API FCollisionChannel CreateChannel ( const std::string & name,
									  ECollisionResponse defaultResponse = ECollisionResponse::BLOCK );

	KE_API void SetChannelResponse ( const std::string & channelA,
							  const std::string & channelB,
							  ECollisionResponse response );

	   // Проверка столкновения между каналами
	KE_API bool CanChannelsCollide ( const std::string & channelA, const std::string & channelB );
	KE_API ECollisionResponse GetResponseBetween ( const std::string & channelA, const std::string & channelB );
	}
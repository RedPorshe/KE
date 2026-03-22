#pragma once
#include "CoreMinimal.h"
#include <atomic>
#include <random>
#include <sstream>
#include <iomanip>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <cctype>


// Forward declarations
class CObjectFactory;
//#include <rapidjson/document.h>
//#include <rapidjson/writer.h>
//#include <rapidjson/stringbuffer.h>
//#include <rapidjson/prettywriter.h>

// Base macros for class declaration - УПРОЩЕННЫЕ
#define CHUDDO_DECLARE_CLASS(ClassName, BaseClassName) \
public: \
    using Super = BaseClassName; \
    static const char* StaticClassName() { return #ClassName; } \
    static const char* StaticBaseClassName() { return #BaseClassName; } \
    virtual const char* GetObjectClassName() const override { return #ClassName; } 
   // virtual void SerializeProperties(rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) const override { Super::SerializeProperties(jsonValue, allocator); } \
    virtual void DeserializeProperties(const rapidjson::Value& jsonValue) override { Super::DeserializeProperties(jsonValue); }

#define CHUDDO_DECLARE_ABSTRACT_CLASS(ClassName, BaseClassName) \
public: \
    using Super = BaseClassName; \
    static const char* StaticClassName() { return #ClassName; } \
    static const char* StaticBaseClassName() { return #BaseClassName; } \
    virtual const char* GetObjectClassName() const override { return #ClassName; } 
  //  virtual void SerializeProperties(rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) const override { Super::SerializeProperties(jsonValue, allocator); } \
  //  virtual void DeserializeProperties(const rapidjson::Value& jsonValue) override { Super::DeserializeProperties(jsonValue); }

class KE_API CObject
    {
    public:
        // Базовые объявления без использования макроса
        using Super = CObject;
        static const char * StaticClassName () { return "CObject"; }
        static const char * StaticBaseClassName () { return ""; }
        virtual const char * GetObjectClassName () const { return "CObject"; }
      //  virtual void SerializeProperties ( rapidjson::Value & jsonValue, rapidjson::Document::AllocatorType & allocator ) const;
     //   virtual void DeserializeProperties ( const rapidjson::Value & jsonValue );

    private:
        // Thread-safe UUID generator
        static std::string GenerateUUID ()
            {
            static std::random_device rd;
            static std::mt19937_64 gen ( rd () );
            static std::uniform_int_distribution<uint64_t> dis;

            uint64_t part1 = dis ( gen );
            uint64_t part2 = dis ( gen );

            std::stringstream ss;
            ss << std::hex << std::setfill ( '0' )
                << std::setw ( 16 ) << part1
                << std::setw ( 16 ) << part2;
            return ss.str ();
            }
        friend class CObjectFactory;
    public:
        CObject ( CObject * inOwner = nullptr, const std::string & inDisplayName = "Object" );
        virtual ~CObject ();

        CObject ( const CObject & ) = delete;
        CObject & operator=( const CObject & ) = delete;

        // Разрешаем перемещение (опционально)
        CObject ( CObject && ) = default;
        CObject & operator=( CObject && ) = default;


        // Basic getters
        CObject * GetOwner () const { return ObjectOwner; }
        bool HasOwner () const { return GetOwner () != nullptr; }

        // Display name (for UI/Editor) - can be changed
        std::string GetName () const { return DisplayName; }

        // Unique persistent identifier (UUID) - never changes
        std::string GetUUID () const { return ObjectUUID; }

        // For backward compatibility
        std::string GetUniqName () const { return DisplayName + "_" + GetShortUUID (); }
        std::string GetFullIdentifier () const { return GetUniqName (); }

        void UpdateDebugIdentifier ();
        std::string GetShortUUID () const
            {
            return ( ObjectUUID.length () > 8 ) ? ObjectUUID.substr ( 0, 8 ) : ObjectUUID;
            }

            // Object search
        CObject * FindOwned ( const std::string & displayName ) const;
        const std::vector<std::unique_ptr<CObject>> & GetOwnedObjects () const { return OwnedObjects; }
        size_t GetNumOwnedObjects () const { return OwnedObjects.size (); }
        bool HasOwnedObjects () const { return !OwnedObjects.empty (); }

        // Recursive search by display name
        bool FindRecursive ( const std::string & displayName );
        CObject * FindObjectByDisplayNameRecursive ( const std::string & displayName );

        // Search by UUID (unique)
        CObject * FindByUUID ( const std::string & uuid ) const;
        CObject * FindByUUIDRecursive ( const std::string & uuid );
     
        std::string CheckNameAndGenerateUniqName ( const std::string & DesiredName );

        // Object management
        bool RemoveOwnedObject ( const std::string & displayName );
        void AddOwnedObject ( std::unique_ptr<CObject> object );
        void AddOwnedObject ( CObject * object );
        bool TransferOwnership ( CObject * obj, CObject * newOwner );

        // Child object creation
        template<typename ClassName, typename... Args>
        ClassName * AddSubObject ( const std::string & desiredDisplayName = "SubObject", Args&&... args )
            {
            static_assert( std::is_base_of<CObject, ClassName>::value,
                           "Class must be derived from CObject" );

            if (desiredDisplayName.empty ())
                {
                LOG_ERROR( "Error: Object display name cannot be empty!" )  ;
                return nullptr;
                }

                // Find hierarchy root for global check
            CObject * root = this;
            while (root->GetOwner ())
                {
                root = root->GetOwner ();
                }            
            std::string finalDisplayName = desiredDisplayName;
            finalDisplayName = CheckNameAndGenerateUniqName ( finalDisplayName );
            // Check if display name already exists globally
            if (root->FindRecursive ( finalDisplayName ))
                {
                    // Display name exists ANYWHERE in hierarchy, generate unique variant
                finalDisplayName = GenerateUniqueDisplayNameVariant ( desiredDisplayName, root );
                LOG_WARN ( "Note: Display name '", desiredDisplayName
                              , "' already exists, using '", finalDisplayName , "' instead" );                
                }

                // Create object (old way - direct construction)
            auto newObj = std::make_unique<ClassName> ( this, finalDisplayName, std::forward<Args> ( args )... );
            ClassName * rawPtr = newObj.get ();

            AddOwnedObject ( std::move ( newObj ) );

            return rawPtr;
            }

            // Version 2: Factory creation (new way) - объявление без реализации
        CObject * AddSubObjectByClass ( const std::string & className,
                                        const std::string & desiredDisplayName = "SubObject" );

        template<typename ObjectType>
        ObjectType * FindObjectByType () const
            {
            for (auto & actor : OwnedObjects)
                {
                ObjectType * founded = actor->FindObjectByType <ObjectType>();
                if (founded)
                    {
                    return founded;
                    }
                }
            return nullptr;
            }

           // Renaming operations
        bool RenameOwnedObject ( const std::string & oldDisplayName, const std::string & newDisplayName );
        bool Rename ( const std::string & newDisplayName );

        // Safe type casting for search
        template<typename T>
        T * FindOwnedAs ( const std::string & displayName ) const
            {
            CObject * obj = FindOwned ( displayName );
            if (obj)
                {
                return dynamic_cast< T * >( obj );
                }
            return nullptr;
            }

            // Cloning
        std::unique_ptr<CObject> Clone () const;

        // Get root object
        CObject * GetRoot () const
            {
            const CObject * root = this;
            while (root->GetOwner ())
                {
                root = root->GetOwner ();
                }
            return const_cast< CObject * >( root );
            }

            // === JSON Serialization Methods ===

            // Main serialization method
      //  virtual void Serialize ( rapidjson::Value & jsonValue, rapidjson::Document::AllocatorType & allocator ) const;

        // Main deserialization method
      //  virtual void Deserialize ( const rapidjson::Value & jsonValue );

        // Convert object to JSON string
      //  std::string ToJSON ( bool pretty = false ) const;

        // Load object from JSON string
     //   bool FromJSON ( const std::string & jsonString );

        // Save object to file
     //   bool SaveToFile ( const std::string & filename, bool pretty = true ) const;

        // Load object from file
      //  bool LoadFromFile ( const std::string & filename );

        // Static methods for creating objects from JSON
       /* static std::unique_ptr<CObject> CreateFromJSON ( const std::string & jsonString );
        static std::unique_ptr<CObject> LoadFromJSONFile ( const std::string & filename );*/

        // Utility to get JSON schema for this object type
     //   std::string GetJSONSchema () const;

    protected:
        CObject * ObjectOwner = nullptr;
        std::string DisplayName {};        // User-friendly name (for display/editor)
        std::string ObjectUUID {};         // Unique immutable identifier
        std::vector<std::unique_ptr<CObject>> OwnedObjects;

        // Generate unique display name variant
        static std::string GenerateUniqueDisplayNameVariant ( const std::string & baseDisplayName, CObject * root );

        // Find all objects with similar display names
        static void CollectSimilarDisplayNames ( CObject * node, const std::string & baseDisplayName,
                                                 std::vector<std::string> & result );

          // Get short version of UUID for display
        static std::string GetShortUUID ( const std::string & uuid )
            {
            if (uuid.length () > 8)
                return uuid.substr ( 0, 8 );
            return uuid;
            }
    };
#include "Core/ObjectFactory.h"

    REGISTER_CLASS_FACTORY ( CObject );



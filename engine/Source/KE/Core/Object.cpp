#include "Core/Object.h"
#include "Core/ObjectFactory.h"
#include <functional>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <fstream>
//#include <rapidjson/error/en.h>



CObject::CObject ( CObject * inOwner, const std::string & inDisplayName )
	: ObjectOwner ( inOwner ), DisplayName ( inDisplayName )
	{		
	ObjectUUID = GenerateUUID ();	
	}

CObject::~CObject ()
	{
	LOG_DEBUG ( GetName (), " destroyed" );
	}

void CObject::UpdateDebugIdentifier ()
	{
		// Already handled in GetUniqName()
	}

CObject * CObject::FindOwned ( const std::string & displayName ) const
	{
	for (const auto & obj : OwnedObjects)
		{
		if (obj->GetName () == displayName)
			{
			return obj.get ();
			}
		}
	return nullptr;
	}

bool CObject::FindRecursive ( const std::string & displayName )
	{
	if (GetName () == displayName)
		{
		return true;
		}

	for (const auto & child : OwnedObjects)
		{
		if (child->FindRecursive ( displayName ))
			{
			return true;
			}
		}

	return false;
	}

CObject * CObject::FindObjectByDisplayNameRecursive ( const std::string & displayName )
	{
	auto FoundChild = FindOwned ( displayName );
	if (FoundChild)
		{
		return FoundChild;
		}

		// Search up the hierarchy
	auto root = this;
	while (root->GetOwner ())
		{
		root = root->GetOwner ();
		FoundChild = root->FindOwned ( displayName );
		if (FoundChild)
			return FoundChild;
		}

	return nullptr;
	}

CObject * CObject::FindByUUID ( const std::string & uuid ) const
	{
	if (ObjectUUID == uuid)
		return const_cast< CObject * >( this );

	return nullptr;
	}

CObject * CObject::FindByUUIDRecursive ( const std::string & uuid ) 
	{
	if (ObjectUUID == uuid)
		return this;

	for (const auto & child : OwnedObjects)
		{
		CObject * found = child->FindByUUIDRecursive ( uuid );
		if (found)
			return found;
		}

	return nullptr;
	}

std::string CObject::CheckNameAndGenerateUniqName ( const std::string & DesiredName )
	{
	if (FindRecursive ( DesiredName ))
		{
		return GenerateUniqueDisplayNameVariant ( DesiredName, GetRoot () );
		}
	return DesiredName;
	}


bool CObject::RemoveOwnedObject ( const std::string & displayName )
	{
	auto it = std::find_if ( OwnedObjects.begin (), OwnedObjects.end (),
							 [ &displayName ] ( const std::unique_ptr<CObject> & obj )
							 {
							 return obj && obj->GetName () == displayName;
							 } );

	if (it != OwnedObjects.end ())
		{
		LOG_DEBUG( ( *it )->GetName () , " [UUID: "
			, GetShortUUID ( ( *it )->ObjectUUID )
			, "] removed from " , GetName ());
		OwnedObjects.erase ( it );
		return true;
		}
	return false;
	}

void CObject::AddOwnedObject ( std::unique_ptr<CObject> object )
	{
	if (!object || object.get () == this)
		return;

	object->ObjectOwner = this;
	OwnedObjects.push_back ( std::move ( object ) );
	}

void CObject::AddOwnedObject ( CObject * object )
	{
	if (!object || object == this)
		return;

	auto uniquePtr = std::unique_ptr<CObject> ( object );
	AddOwnedObject ( std::move ( uniquePtr ) );
	}

CObject * CObject::AddSubObjectByClass ( const std::string & className,
										 const std::string & desiredDisplayName )
	{
	if (desiredDisplayName.empty ())
		{
		LOG_ERROR( "Error: Object display name cannot be empty!");
		return nullptr;
		}

		// Find hierarchy root for global check
	CObject * root = this;
	while (root->GetOwner ())
		{
		root = root->GetOwner ();
		}

	std::string finalDisplayName = desiredDisplayName;

	// Check if display name already exists globally
	if (root->FindRecursive ( desiredDisplayName ))
		{
			// Display name exists in hierarchy, generate unique variant
		finalDisplayName = GenerateUniqueDisplayNameVariant ( desiredDisplayName, root );
		LOG_DEBUG( "Note: Display name '" , desiredDisplayName
			, "' already exists, using '" , finalDisplayName , "' instead");
		}

		// Use factory to create object
	CObject * newObj = OBJECT_FACTORY.Create ( className, this, finalDisplayName );

	return newObj;
	}

bool CObject::TransferOwnership ( CObject * obj, CObject * newOwner )
	{
	if (!obj || !newOwner || obj == newOwner || obj == this)
		{
		return false;
		}
	

	auto it = std::find_if ( OwnedObjects.begin (), OwnedObjects.end (),
							 [ obj ] ( const std::unique_ptr<CObject> & owned )
							 {
							 bool found = owned.get () == obj;							
							 return found;
							 } );

	if (it != OwnedObjects.end ())
		{
		std::unique_ptr<CObject> temp = std::move ( *it );
		OwnedObjects.erase ( it );

		temp->ObjectOwner = newOwner;
		LOG_DEBUG( "New Owner '" , newOwner->GetName () , "'");
		newOwner->AddOwnedObject ( std::move ( temp ) );

		LOG_DEBUG ( "Transferred '", obj->GetName (),"' [UUID: ", GetShortUUID ( obj->ObjectUUID ),"] from '", GetName (),"' to '" , newOwner->GetName (),"'" );
		
		return true;
		}
	return false;
	}

	// Helper function to collect similar display names
void CObject::CollectSimilarDisplayNames ( CObject * node, const std::string & baseDisplayName,
										   std::vector<std::string> & result )
	{
	if (!node) return;

	std::string name = node->GetName ();

	// Check if name starts with baseDisplayName
	if (name.find ( baseDisplayName ) == 0)
		{
			// Exact match
		if (name == baseDisplayName)
			{
			result.push_back ( name );
			}
			// Name in format "baseDisplayName_number"
		else if (name.size () > baseDisplayName.size () && name[ baseDisplayName.size () ] == '_')
			{
			std::string suffix = name.substr ( baseDisplayName.size () + 1 );

			// Check if suffix is a number
			bool isNumber = !suffix.empty ();
			for (char c : suffix)
				{
				if (!std::isdigit ( static_cast< unsigned char >( c ) ))
					{
					isNumber = false;
					break;
					}
				}

			if (isNumber)
				{
				result.push_back ( name );
				}
			}
		}

		// Recursively check children
	for (const auto & child : node->GetOwnedObjects ())
		{
		CollectSimilarDisplayNames ( child.get (), baseDisplayName, result );
		}
	}

	// Generate unique display name variant
std::string CObject::GenerateUniqueDisplayNameVariant ( const std::string & baseDisplayName, CObject * root )
	{
	std::vector<std::string> similarNames;
	CollectSimilarDisplayNames ( root, baseDisplayName, similarNames );

	// If no similar names, baseDisplayName is available
	if (similarNames.empty ())
		{
		return baseDisplayName;
		}

		// Extract numbers from names
	std::vector<int> usedNumbers;
	bool baseNameExists = false;

	for (const auto & name : similarNames)
		{
		if (name == baseDisplayName)
			{
			baseNameExists = true;
			usedNumbers.push_back ( 0 ); // Base name is number 0
			}
		else
			{
			std::string suffix = name.substr ( baseDisplayName.size () +1 );
			try
				{
				int num = std::stoi ( suffix );
				usedNumbers.push_back ( num );
				}
				catch (...)
					{
						// Not a number, ignore
					}
			}
		}

		// Sort numbers
	std::sort ( usedNumbers.begin (), usedNumbers.end () );

	// Find first available number
	int nextNumber = baseNameExists ? 1 : 0;
	for (int num : usedNumbers)
		{
		if (num == nextNumber)
			{
			nextNumber++;
			}
		else if (num > nextNumber)
			{
			break; // Found a gap
			}
		}

		// If nextNumber is 0, return baseDisplayName (shouldn't happen here)
	if (nextNumber == 0)
		{
		return baseDisplayName;
		}

	return baseDisplayName + "_" + std::to_string ( nextNumber  );
	}

bool CObject::RenameOwnedObject ( const std::string & oldDisplayName, const std::string & newDisplayName )
	{
	if (oldDisplayName.empty ())
		{
		LOG_ERROR( "Error: Old display name cannot be empty!");
		return false;
		}

	if (newDisplayName.empty ())
		{
		LOG_ERROR( "Error: New display name cannot be empty!");
		return false;
		}

		// Check if new display name is already used among children
	if (FindOwned ( newDisplayName ))
		{
		LOG_ERROR( "Error: Display name '" , newDisplayName , "' already in use among children!");
		return false;
		}

	CObject * obj = FindOwned ( oldDisplayName );
	if (obj)
		{
		return obj->Rename ( newDisplayName );
		}

	LOG_ERROR( "Error: Object '" , oldDisplayName , "' not found!");
	return false;
	}

std::unique_ptr<CObject> CObject::Clone () const
	{
	auto clone = std::make_unique<CObject> ( nullptr, GetName () + "_Copy" );

	for (const auto & child : OwnedObjects)
		{
		auto childClone = child->Clone ();
		clone->AddOwnedObject ( std::move ( childClone ) );
		}

	return clone;
	}

bool CObject::Rename ( const std::string & newDisplayName )
	{
	if (newDisplayName.empty ())
		{
		LOG_ERROR( "Error: New display name cannot be empty!");
		return false;
		}

		// If display name hasn't changed
	if (newDisplayName == DisplayName)
		{
		return true;
		}

		// Get hierarchy root
	CObject * root = GetRoot ();

	// Global uniqueness check
	if (root->FindRecursive ( newDisplayName ))
		{
		LOG_ERROR( "Error: can't Rename to '" , newDisplayName
			, "'. This display name already exists in world.");

		auto existingObj = FindObjectByDisplayNameRecursive ( newDisplayName );
		if (existingObj && typeid( *existingObj ) == typeid( *this ))
			{
			LOG_DEBUG( "Objects have same type, generating unique display name...");
			LOG_DEBUG( typeid( *existingObj ).name () , " found '"
				, existingObj->GetUniqName () , "' -> this '"
				, typeid( *this ).name () , "' " , this->GetUniqName ()) ;

	  // Generate unique display name
			std::string uniqueDisplayName = GenerateUniqueDisplayNameVariant ( newDisplayName, root );
			LOG_DEBUG( "Auto-generating unique display name: '" , uniqueDisplayName , "'");

			// Apply new display name
			std::string oldDisplayName = DisplayName;
			DisplayName = uniqueDisplayName;
			LOG_DEBUG( "Renamed '" , oldDisplayName , "' to '" , uniqueDisplayName
				, "' new unique name '" , GetUniqName () , "'");
			return true;
			}
		return false;
		}

		// Display name is unique, rename
	std::string oldDisplayName = DisplayName;
	DisplayName = newDisplayName;
	LOG_DEBUG( "Renamed '" , oldDisplayName , "' to '" , newDisplayName
		, "' new unique name '" , GetUniqName () , "'");
	return true;
	}

	// ========== JSON Serialization Implementation ==========

//void CObject::Serialize ( rapidjson::Value & jsonValue, rapidjson::Document::AllocatorType & allocator ) const
//	{
//		// Basic object properties
//	jsonValue.AddMember ( "ClassName", rapidjson::StringRef ( GetObjectClassName () ), allocator );
//	jsonValue.AddMember ( "DisplayName", rapidjson::StringRef ( DisplayName.c_str () ), allocator );
//	jsonValue.AddMember ( "UUID", rapidjson::StringRef ( ObjectUUID.c_str () ), allocator );
//
//	// Serialize custom properties from derived classes
//	SerializeProperties ( jsonValue, allocator );
//
//	// Serialize children recursively
//	if (!OwnedObjects.empty ())
//		{
//		rapidjson::Value childrenArray ( rapidjson::kArrayType );
//		for (const auto & child : OwnedObjects)
//			{
//			rapidjson::Value childValue ( rapidjson::kObjectType );
//			child->Serialize ( childValue, allocator );
//			childrenArray.PushBack ( childValue, allocator );
//			}
//		jsonValue.AddMember ( "Children", childrenArray, allocator );
//		}
//	}
//
//void CObject::Deserialize ( const rapidjson::Value & jsonValue )
//	{
//		// Load basic properties
//	if (jsonValue.HasMember ( "DisplayName" ) && jsonValue[ "DisplayName" ].IsString ())
//		{
//		DisplayName = jsonValue[ "DisplayName" ].GetString ();
//		}
//
//	if (jsonValue.HasMember ( "UUID" ) && jsonValue[ "UUID" ].IsString ())
//		{
//		ObjectUUID = jsonValue[ "UUID" ].GetString ();
//		}
//
//		// Deserialize custom properties
//	DeserializeProperties ( jsonValue );
//
//	// Deserialize children
//	if (jsonValue.HasMember ( "Children" ) && jsonValue[ "Children" ].IsArray ())
//		{
//		const rapidjson::Value & childrenArray = jsonValue[ "Children" ];
//		for (rapidjson::SizeType i = 0; i < childrenArray.Size (); i++)
//			{
//			const rapidjson::Value & childValue = childrenArray[ i ];
//			if (childValue.HasMember ( "ClassName" ) && childValue[ "ClassName" ].IsString ())
//				{
//				std::string className = childValue[ "ClassName" ].GetString ();
//				std::string displayName = "LoadedObject";
//
//				if (childValue.HasMember ( "DisplayName" ) && childValue[ "DisplayName" ].IsString ())
//					{
//					displayName = childValue[ "DisplayName" ].GetString ();
//					}
//
//					// Create object through factory
//				CObject * childObj = AddSubObjectByClass ( className, displayName );
//				if (childObj)
//					{
//					childObj->Deserialize ( childValue );
//					}
//				}
//			}
//		}
//	}
//
//void CObject::SerializeProperties ( rapidjson::Value & jsonValue, rapidjson::Document::AllocatorType & allocator ) const
//	{
//		// Base class has no custom properties
//		// Override in derived classes
//	}
//
//void CObject::DeserializeProperties ( const rapidjson::Value & jsonValue )
//	{
//		// Base class has no custom properties
//		// Override in derived classes
//	}

//std::string CObject::ToJSON ( bool pretty ) const
//	{
//	rapidjson::Document doc;
//	doc.SetObject ();
//	rapidjson::Document::AllocatorType & allocator = doc.GetAllocator ();
//
//	// Serialize to root object
//	Serialize ( doc, allocator );
//
//	// Convert to string
//	rapidjson::StringBuffer buffer;
//	if (pretty)
//		{
//		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer ( buffer );
//		doc.Accept ( writer );
//		}
//	else
//		{
//		rapidjson::Writer<rapidjson::StringBuffer> writer ( buffer );
//		doc.Accept ( writer );
//		}
//
//	return buffer.GetString ();
//	}

//bool CObject::FromJSON ( const std::string & jsonString )
//	{
//	rapidjson::Document doc;
//	doc.Parse ( jsonString.c_str () );
//
//	if (doc.HasParseError ())
//		{
//		LOG_ERROR( "JSON parse error: " , rapidjson::GetParseError_En ( doc.GetParseError () )
//			, " at offset " , doc.GetErrorOffset () );
//		return false;
//		}
//
//	Deserialize ( doc );
//	return true;
//	}
//
//bool CObject::SaveToFile ( const std::string & filename, bool pretty ) const
//	{
//	std::ofstream file ( filename );
//	if (!file.is_open ())
//		{
//		LOG_ERROR( "Failed to open file for writing: " , filename );
//		return false;
//		}
//
//	std::string json = ToJSON ( pretty );
//	file << json;
//	file.close ();
//
//	LOG_DEBUG( "Saved object '" , DisplayName , "' to file: " , filename);
//	return true;
//	}
//
//bool CObject::LoadFromFile ( const std::string & filename )
//	{
//	std::ifstream file ( filename );
//	if (!file.is_open ())
//		{
//		LOG_ERROR( "Failed to open file for reading: " , filename);
//		return false;
//		}
//
//		// Read entire file
//	std::stringstream buffer;
//	buffer << file.rdbuf ();
//	file.close ();
//
//	return FromJSON ( buffer.str () );
//	}
//
//std::unique_ptr<CObject> CObject::CreateFromJSON ( const std::string & jsonString )
//	{
//	rapidjson::Document doc;
//	doc.Parse ( jsonString.c_str () );
//
//	if (doc.HasParseError ())
//		{
//		LOG_ERROR( "JSON parse error: " , rapidjson::GetParseError_En ( doc.GetParseError () )
//			, " at offset " , doc.GetErrorOffset () );
//		return nullptr;
//		}
//
//	if (!doc.HasMember ( "ClassName" ) || !doc[ "ClassName" ].IsString ())
//		{
//		LOG_ERROR( "JSON missing ClassName field") ;
//		return nullptr;
//		}
//
//	std::string className = doc[ "ClassName" ].GetString ();
//	std::string displayName = "LoadedObject";
//
//	if (doc.HasMember ( "DisplayName" ) && doc[ "DisplayName" ].IsString ())
//		{
//		displayName = doc[ "DisplayName" ].GetString ();
//		}
//
//		// Create through factory
//	CObject * obj = OBJECT_FACTORY.Create ( className, nullptr, displayName );
//	if (!obj)
//		{
//		LOG_ERROR( "Failed to create object of type: " , className) ;
//		return nullptr;
//		}
//
//		// Deserialize properties
//	obj->Deserialize ( doc );
//
//	return std::unique_ptr<CObject> ( obj );
//	}

//std::unique_ptr<CObject> CObject::LoadFromJSONFile ( const std::string & filename )
//	{
//	std::ifstream file ( filename );
//	if (!file.is_open ())
//		{
//		LOG_ERROR( "Failed to open file: " , filename) ;
//		return nullptr;
//		}
//
//	std::stringstream buffer;
//	buffer << file.rdbuf ();
//	file.close ();
//
//	return CreateFromJSON ( buffer.str () );
//	}

//std::string CObject::GetJSONSchema () const
//	{
//	rapidjson::Document doc;
//	doc.SetObject ();
//	rapidjson::Document::AllocatorType & allocator = doc.GetAllocator ();
//
//	rapidjson::Value schema ( rapidjson::kObjectType );
//	schema.AddMember ( "type", "object", allocator );
//
//	rapidjson::Value properties ( rapidjson::kObjectType );
//
//	// Required properties
//	rapidjson::Value required ( rapidjson::kArrayType );
//	required.PushBack ( "ClassName", allocator );
//	required.PushBack ( "DisplayName", allocator );
//	required.PushBack ( "UUID", allocator );
//
//	// ClassName property
//	rapidjson::Value classNameProp ( rapidjson::kObjectType );
//	classNameProp.AddMember ( "type", "string", allocator );
//	classNameProp.AddMember ( "description", "Object class name", allocator );
//	properties.AddMember ( "ClassName", classNameProp, allocator );
//
//	// DisplayName property
//	rapidjson::Value displayNameProp ( rapidjson::kObjectType );
//	displayNameProp.AddMember ( "type", "string", allocator );
//	displayNameProp.AddMember ( "description", "User-friendly display name", allocator );
//	properties.AddMember ( "DisplayName", displayNameProp, allocator );
//
//	// UUID property
//	rapidjson::Value uuidProp ( rapidjson::kObjectType );
//	uuidProp.AddMember ( "type", "string", allocator );
//	uuidProp.AddMember ( "description", "Unique identifier", allocator );
//	uuidProp.AddMember ( "pattern", "^[0-9a-f]{32}$", allocator );
//	properties.AddMember ( "UUID", uuidProp, allocator );
//
//	// Children property
//	rapidjson::Value childrenProp ( rapidjson::kObjectType );
//	childrenProp.AddMember ( "type", "array", allocator );
//	childrenProp.AddMember ( "description", "Child objects", allocator );
//
//	rapidjson::Value items ( rapidjson::kObjectType );
//	items.AddMember ( "$ref", "#", allocator ); // Self-reference
//	childrenProp.AddMember ( "items", items, allocator );
//
//	properties.AddMember ( "Children", childrenProp, allocator );
//
//	schema.AddMember ( "properties", properties, allocator );
//	schema.AddMember ( "required", required, allocator );
//	schema.AddMember ( "additionalProperties", true, allocator );
//
//	doc.AddMember ( "$schema", "http://json-schema.org/draft-07/schema#", allocator );
//	doc.AddMember ( "$id", rapidjson::StringRef ( GetObjectClassName () ), allocator );
//	doc.AddMember ( "title", rapidjson::StringRef ( GetObjectClassName () ), allocator );
//	doc.AddMember ( "description", "JSON schema for CObject hierarchy", allocator );
//	doc.AddMember ( "definitions", schema, allocator );
//
//	rapidjson::StringBuffer buffer;
//	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer ( buffer );
//	doc.Accept ( writer );
//
//	return buffer.GetString ();
//	}

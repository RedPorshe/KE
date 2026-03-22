#include "Core/ObjectFactory.h"
#include "Core/Object.h"
#include <set>

bool CObjectFactory::IsDerivedFrom ( const std::string & ClassName, const std::string & BaseClassName ) const
    {
     // Базовая валидация
    if (ClassName.empty () || BaseClassName.empty ())
        {
        LOG_DEBUG ( "[FACTORY] IsDerivedFrom: Empty class name" );
        return false;
        }

    LOG_DEBUG ( "[FACTORY] IsDerivedFrom? ", ClassName, " -> ", BaseClassName );

    auto it = ClassHierarchy.find ( ClassName );
    if (it == ClassHierarchy.end ())
        {
        LOG_DEBUG ( "[FACTORY] Class not found: ", ClassName );
        return false;
        }

        // ✅ ВАЖНО! Класс является наследником самого себя
    if (ClassName == BaseClassName)
        {
        LOG_DEBUG ( "[FACTORY] ✓ Same class - derived from itself" );
        return true;
        }

    std::string current = it->second;

    // Защита от циклов
    std::set<std::string> visited;
    visited.insert ( ClassName );

    while (!current.empty ())
        {
            // Проверка на цикл
        if (visited.find ( current ) != visited.end ())
            {
            LOG_ERROR ( "[FACTORY] CYCLE DETECTED! ", ClassName, " -> ", current );
            return false;
            }
        visited.insert ( current );

        LOG_DEBUG ( "[FACTORY] Checking parent: ", current );

        if (current == BaseClassName)
            {
            LOG_DEBUG ( "[FACTORY] ✓ Found inheritance: ", ClassName, " is derived from ", BaseClassName );
            return true;
            }

        auto parentIt = ClassHierarchy.find ( current );
        if (parentIt != ClassHierarchy.end ())
            {
            current = parentIt->second;
            }
        else
            {
            current.clear ();
            }
        }

    LOG_DEBUG ( "[FACTORY] ✗ No inheritance: ", ClassName, " is not derived from ", BaseClassName );
    return false;
    }

CObject * CObjectFactory::Create ( const std::string & className,
                                   CObject * owner,
                                   const std::string & displayName )
    {
        // Проверка валидности имени
    if (displayName.empty ())
        {
        LOG_ERROR ( "[FACTORY] ERROR: Display name cannot be empty!" );
        return nullptr;
        }

        // Проверить уникальность имени в иерархии owner
    std::string finalDisplayName = displayName;
    if (owner)
        {
        CObject * root = owner->GetRoot ();
        while (root->GetOwner () != nullptr)
            {
            root = root->GetOwner ();
            }
        if (root->FindRecursive ( finalDisplayName ))
            {
                // Генерируем уникальное имя
            finalDisplayName = CObject::GenerateUniqueDisplayNameVariant ( finalDisplayName, root );
            LOG_DEBUG ( "[FACTORY] Note: Display name '", displayName,
                        "' already exists, using '", finalDisplayName, "' instead" );
            }
        }

    auto it = ClassCreators.find ( className );
    if (it != ClassCreators.end ())
        {
        CObject * obj = it->second ( owner, finalDisplayName );
        if (obj)
            {
                // АВТОМАТИЧЕСКИ добавляем к владельцу, если он указан
            if (owner)
                {
                owner->AddOwnedObject ( obj );
                }

            LOG_DEBUG ( "[FACTORY] Created '", finalDisplayName,
                        "' of type '", className, "'" );
            }
        return obj;
        }

        // Try to find in parent classes (for backward compatibility)
    std::string parentClass = FindParentClass ( className );
    if (!parentClass.empty ())
        {
        LOG_DEBUG ( "[FACTORY] Class '", className,
                    "' not found, using parent '", parentClass, "'" );
        return Create ( parentClass, owner, finalDisplayName );
        }

    LOG_ERROR ( "[FACTORY] ERROR - Unknown class type: '", className, "'" );
    LOG_ERROR ( "[FACTORY] Available classes:" );
    for (const auto & [name, _] : ClassCreators)
        LOG_ERROR ( "  ", name );
    return nullptr;
    }
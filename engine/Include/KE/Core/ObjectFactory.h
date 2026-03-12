#pragma once
#include "CoreMinimal.h"
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <type_traits>
#include <mutex>
#include <unordered_set>
#include <stack>

// Forward declaration
class CObject;

// Simple registration macro
#define REGISTER_CLASS_FACTORY(ClassName) \
    static bool Register##ClassName() { \
        static bool bRegistered = false; \
        if (!bRegistered) { \
            CObjectFactory::GetInstance().RegisterClass<ClassName>(); \
            bRegistered = true; \
        } \
        return bRegistered; \
    } \
    static bool ClassName##_Registered = Register##ClassName();

class KE_API CObjectFactory
    {
    public:
        using CreatorFunc = std::function<CObject * ( CObject * owner, const std::string & displayName )>;

        // Singleton pattern
        static CObjectFactory & GetInstance ()
            {
            static CObjectFactory instance;
            return instance;
            }

            // Проверка наследования с защитой от циклов
        bool IsDerivedFrom ( const std::string & ClassName, const std::string & BaseClassName ) const;

        // Disable copying
        CObjectFactory ( const CObjectFactory & ) = delete;
        CObjectFactory & operator=( const CObjectFactory & ) = delete;

        // Register a class with automatic name detection
        template<typename T>
        void RegisterClass ()
            {
            static_assert( std::is_base_of<CObject, T>::value,
                           "CObjectFactory: Class must derive from CObject" );

            std::string className = T::StaticClassName ();

            // Используем статическую локальную переменную для потокобезопасной однократной регистрации
            static std::mutex registrationMutex;
            std::lock_guard<std::mutex> lock ( registrationMutex );

            static std::unordered_set<std::string> registeredClasses;

            if (registeredClasses.find ( className ) != registeredClasses.end ())
                {
                return; // Уже зарегистрирован в этой сессии
                }

            if (ClassCreators.find ( className ) != ClassCreators.end ())
                {
                    // Уже зарегистрирован глобально, просто запоминаем
                registeredClasses.insert ( className );
                return;
                }

                // Creator lambda
            CreatorFunc creator = [] ( CObject * owner, const std::string & displayName ) -> CObject *
                {
                return new T ( owner, displayName );
                };

                // Store creator
            ClassCreators[ className ] = creator;

            // Store class hierarchy info
            std::string baseClassName = T::StaticBaseClassName ();

            // ВАЖНО: Для CObject базовый класс должен быть пустой строкой!
            if (className == "CObject")
                {
                ClassHierarchy[ className ] = "";  // Нет базового класса!
                }
            else
                {
                ClassHierarchy[ className ] = baseClassName;
                }

                // Запоминаем
            registeredClasses.insert ( className );

#ifdef _DEBUG
            std::cout << "[FACTORY] Registered class '" << className
                << "' (base: '" << ( baseClassName.empty () ? "None" : baseClassName ) << "')\n";
#endif
            }

            // Register a class with custom name
        template<typename T>
        void RegisterClass ( const std::string & className )
            {
            static_assert( std::is_base_of<CObject, T>::value,
                           "CObjectFactory: Class must derive from CObject" );

            static std::mutex registrationMutex;
            std::lock_guard<std::mutex> lock ( registrationMutex );

            static std::unordered_set<std::string> registeredClasses;

            if (registeredClasses.find ( className ) != registeredClasses.end ())
                {
                return;
                }

            if (ClassCreators.find ( className ) != ClassCreators.end ())
                {
                registeredClasses.insert ( className );
                return;
                }

            ClassCreators[ className ] = [] ( CObject * owner, const std::string & displayName ) -> CObject *
                {
                return new T ( owner, displayName );
                };

                // Для кастомного имени тоже сохраняем иерархию
            if (className == "CObject")
                {
                ClassHierarchy[ className ] = "";
                }
            else
                {
                ClassHierarchy[ className ] = T::StaticBaseClassName ();
                }

            registeredClasses.insert ( className );

#ifdef _DEBUG
            std::cout << "[FACTORY] Registered class '" << className << "'\n";
#endif
            }

            // Create object by class name and automatically add to owner
        CObject * Create ( const std::string & className,
                           CObject * owner = nullptr,
                           const std::string & displayName = "Object" );

           // Check if class is registered
        bool IsClassRegistered ( const std::string & className ) const
            {
            return ClassCreators.find ( className ) != ClassCreators.end ();
            }

            // Get all registered class names
        std::vector<std::string> GetRegisteredClasses () const
            {
            std::vector<std::string> classes;
            for (const auto & [name, _] : ClassCreators)
                classes.push_back ( name );
            return classes;
            }

            // Get class hierarchy
        std::string GetClassHierarchy () const
            {
            std::stringstream ss;
            ss << "Class Hierarchy:\n";
            for (const auto & [className, baseClass] : ClassHierarchy)
                {
                ss << "  " << className << " -> "
                    << ( baseClass.empty () ? "None" : baseClass ) << "\n";
                }
            return ss.str ();
            }

    private:
        CObjectFactory () = default;
        ~CObjectFactory () = default;

        std::unordered_map<std::string, CreatorFunc> ClassCreators;
        std::unordered_map<std::string, std::string> ClassHierarchy;

        // Find parent class for a given class name
        std::string FindParentClass ( const std::string & className ) const
            {
            auto it = ClassHierarchy.find ( className );
            if (it != ClassHierarchy.end ())
                return it->second;
            return "";
            }
    };

    // Singleton access macro
#define OBJECT_FACTORY CObjectFactory::GetInstance()
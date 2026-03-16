#pragma once

#include <memory>
#include <vector>

// Базовые алиасы
template <typename T>
using TSharedPtr = std::shared_ptr<T>;

template <typename T>
using TUniquePtr = std::unique_ptr<T>;

template <typename T>
using TWeakPtr = std::weak_ptr<T>;

template <typename T>
using TVector = std::vector<T>;

// Фабричные функции
template <typename T, typename... Args>
TSharedPtr<T> MakeShared ( Args&&... args ) {
    return std::make_shared<T> ( std::forward<Args> ( args )... );
    }

template <typename T, typename... Args>
TUniquePtr<T> MakeUnique ( Args&&... args ) {
    return std::make_unique<T> ( std::forward<Args> ( args )... );
    }

    // Для систем движка
template <typename T>
using TSystemPtr = TSharedPtr<T>;

template <typename T>
using TSystemUniquePtr = TUniquePtr<T>;
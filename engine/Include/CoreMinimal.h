#pragma once
//system

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <locale>
#include < chrono >
#define GLM_ENABLE_EXPERIMENTAL
//engine
#include "Utils/Math/MathTypes.h"
#include "Utils/Logger.h"
#include "Utils/MathGLMConverters.h"
#include "Core/Collision.h"
#include "KE/Core/KEExport.h"

// Forward declarations
class CObject;
class CObjectFactory;

// Global factory access
extern CObjectFactory & OBJECT_FACTORY;

#define EDITOR_MODE 0

#ifdef EDITOR_MODE
#define IN_EDITOR 1
#else
#define IN_EDITOR 0
#endif // EDITOR_MODE
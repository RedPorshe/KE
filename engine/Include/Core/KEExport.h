#pragma once

#ifdef KE_ENGINE_BUILD
    #define KE_API __declspec(dllexport)
#else
    #define KE_API __declspec(dllimport)
#endif

// Для не-Windows платформ
#ifndef KE_API
    #define KE_API
#endif
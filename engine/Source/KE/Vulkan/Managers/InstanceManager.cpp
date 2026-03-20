#include "KE/Vulkan/Managers/InstanceManager.h"
#include "KE/Vulkan/VKinfo.h"
#include <GLFW/glfw3.h>
#include <vector>

// Функция обратного вызова для debug messenger
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback (
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData,
    void * pUserData )
    {
        // Получаем указатель на менеджер
    InstanceManager * manager = static_cast< InstanceManager * >( pUserData );

    if (!manager) return VK_FALSE;

    // Фильтруем сообщения
    switch (messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                manager->LogTrace ( "Vulkan: ", pCallbackData->pMessage );
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                manager->LogInfo ( "Vulkan: ", pCallbackData->pMessage );
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                manager->LogWarn ( "Vulkan: ", pCallbackData->pMessage );
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                manager->LogError ( "Vulkan: ", pCallbackData->pMessage );
                break;
            default:
                break;
        }

    return VK_FALSE;
    }

InstanceManager::InstanceManager ()
    : IVKManager ()
    {
    LogDebug ( "Created" );
    }

InstanceManager::~InstanceManager ()
    {
    Shutdown ();
    }

bool InstanceManager::Init ()
    {
    LogDebug ( "Initializing Vulkan instance" );

    if (!m_info || !m_info->Window)
        {
        LogError ( "Invalid VkInfo or Window" );
        return false;
        }

    if (!CreateInstance ())
        {
        LogError ( "Failed to create instance" );
        return false;
        }

#ifdef _DEBUG
    // Создаем debug messenger если валидация включена
    if (bIsValidationEnabled)
        {
        if (!CreateDebugMessenger ())
            {
            LogError ( "Failed to create debug messenger" );
            return false;
            }
        }
#endif
    if (!CreateSurface ())
        {
        LogError ( "Failed to create Surface" );
        return false;
        }
    bIsInitialized = true;
    LogInfo ( "Vulkan instance created successfully" );
    return true;
    }


void InstanceManager::Shutdown ()
    {
    

    LogDebug ( "Shutting down" );

    // Уничтожаем Surface (должен быть до Instance)
    if (surface != VK_NULL_HANDLE)
        {
        vkDestroySurfaceKHR ( m_Instance, surface, nullptr );
        surface = VK_NULL_HANDLE;
        LogDebug ( "Surface destroyed" );
        }

#ifdef _DEBUG
    // Уничтожаем Debug Messenger
    if (m_debugMessenger != VK_NULL_HANDLE)
        {
        auto func = ( PFN_vkDestroyDebugUtilsMessengerEXT ) vkGetInstanceProcAddr (
            m_Instance, "vkDestroyDebugUtilsMessengerEXT" );

        if (func)
            {
            func ( m_Instance, m_debugMessenger, nullptr );
            }
        m_debugMessenger = VK_NULL_HANDLE;
        LogDebug ( "Debug messenger destroyed" );
        }
#endif

    // Уничтожаем Instance последним
    if (m_Instance != VK_NULL_HANDLE)
        {
        vkDestroyInstance ( m_Instance, nullptr );
        m_Instance = VK_NULL_HANDLE;
        LogDebug ( "Instance destroyed" );
        }

    bIsInitialized = false;
    }

const std::string & InstanceManager::GetManagerName () const
    {
    static const std::string name = "Instance Manager";
    return name;
    }

bool InstanceManager::CreateInstance ()
    {
        // Application info
    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = m_info->ApplicationName.c_str ();
    appInfo.applicationVersion = VK_MAKE_VERSION ( 1, 0, 0 );
    appInfo.pEngineName = m_info->EngineName.c_str ();
    appInfo.engineVersion = VK_MAKE_VERSION ( 1, 0, 0 );
    appInfo.apiVersion = VK_API_VERSION_1_3;

    // Получаем необходимые расширения от GLFW
    uint32_t glfwExtensionCount = 0;
    const char ** glfwExtensions = glfwGetRequiredInstanceExtensions ( &glfwExtensionCount );

    std::vector<const char *> extensions ( glfwExtensions, glfwExtensions + glfwExtensionCount );

#ifdef _DEBUG
    // Добавляем расширение для отладки
    extensions.push_back ( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
    LogDebug ( "Extensions count: ", extensions.size () );
    for (const auto & ext : extensions)
        {
        LogTrace ( "  - ", ext );
        }
#endif

    // Слои валидации для debug
    std::vector<const char *> validationLayers;
#ifdef _DEBUG
    if (bIsValidationEnabled)
        {
        validationLayers.push_back ( "VK_LAYER_KHRONOS_validation" );
        LogDebug ( "Validation layers enabled" );

        // Проверяем поддержку слоев
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties ( &layerCount, nullptr );
        std::vector<VkLayerProperties> availableLayers ( layerCount );
        vkEnumerateInstanceLayerProperties ( &layerCount, availableLayers.data () );

        bool layerFound = false;
        for (const auto & layer : availableLayers)
            {
            if (strcmp ( "VK_LAYER_KHRONOS_validation", layer.layerName ) == 0)
                {
                layerFound = true;
                break;
                }
            }

        if (!layerFound)
            {
            LogWarn ( "VK_LAYER_KHRONOS_validation not available" );
            bIsValidationEnabled = false;
            validationLayers.clear ();
            }
        }
#endif

    // Instance creation info
    VkInstanceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast< uint32_t >( extensions.size () );
    createInfo.ppEnabledExtensionNames = extensions.data ();
    createInfo.enabledLayerCount = static_cast< uint32_t >( validationLayers.size () );
    createInfo.ppEnabledLayerNames = validationLayers.data ();

    // Создаем instance
    VkResult result = vkCreateInstance ( &createInfo, nullptr, &m_Instance );

    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create Vulkan instance, error code: ", result );
        return false;
        }

    LogDebug ( "Vulkan instance created" );
    return true;
    }

bool InstanceManager::CreateSurface ()
    {
    if (!m_info || !m_info->Window)
        {
        LogError ( "Cannot create surface: invalid window" );
        return false;
        }

    VkResult result = glfwCreateWindowSurface ( m_Instance, m_info->Window, nullptr, &surface );

    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create window surface, error code: ", result );
        return false;
        }

    LogDebug ( "Surface created successfully" );
    return true;
    }

#ifdef _DEBUG
bool InstanceManager::CreateDebugMessenger ()
    {
    VkDebugUtilsMessengerCreateInfoEXT createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData = this;

    // Получаем функцию для создания debug messenger
    auto func = ( PFN_vkCreateDebugUtilsMessengerEXT ) vkGetInstanceProcAddr (
        m_Instance, "vkCreateDebugUtilsMessengerEXT" );

    if (!func)
        {
        LogError ( "Failed to get vkCreateDebugUtilsMessengerEXT function" );
        return false;
        }

    VkResult result = func ( m_Instance, &createInfo, nullptr, &m_debugMessenger );

    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create debug messenger, error code: ", result );
        return false;
        }

    LogDebug ( "Debug messenger created" );
    return true;
    }
#endif
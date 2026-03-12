#include "Render/Vulkan/VulkanContext.h"
#include "Render/Vulkan/Managers/DeviceManager.h"
#include "Render/Vulkan/Managers/SwapChainManager.h"
#include "Render/Vulkan/Managers/CommandManager.h"
#include "Render/Vulkan/Managers/SyncManager.h"
#include "Render/Vulkan/Managers/PipelineManager.h"
#include "Render/Vulkan/Managers/DescriptorManager.h"
#include "Render/Vulkan/Managers/BufferManager.h"
#include "Render/Vulkan/Managers/RenderPassManager.h"
#include "Render/Window.h"
#include "Core/Engine.h"
#include <vector>
#include <cstring>

// Debug callback
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback (
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData,
    void * pUserData )
    {
    // Фильтруем слишком частые сообщения (опционально)
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
        LOG_ERROR ( "[Vulkan Validation] ", pCallbackData->pMessage );
        }
    else
        {
        LOG_DEBUG ( "[Vulkan Validation] ", pCallbackData->pMessage );
        }

    return VK_FALSE;
    }

CVulkanContext::CVulkanContext ( FEngineInfo & Info )
    : IVulkanManager ( Info )
    {}

CVulkanContext::~CVulkanContext ()
    {
    Shutdown ();
    }

bool CVulkanContext::Initialize ()
    {
    LogDebug ( "Initializing VulkanContext..." );

    if (!CreateInstance ())
        {
        LogError ( "Failed to create Vulkan Instance" );
        return false;
        }

    if (!CreateSurface ())
        {
        LogError ( "Failed to create Surface" );
        return false;
        }

    // Создаем все менеджеры
    LogDebug ( "Creating Vulkan managers..." );

    m_Info.Vulkan.DeviceManager = std::make_unique<CDeviceManager> ( m_Info );
    m_Info.Vulkan.SwapChainManager = std::make_unique<CSwapChainManager> ( m_Info );
    m_Info.Vulkan.CommandManager = std::make_unique<CCommandManager> ( m_Info );
    m_Info.Vulkan.SyncManager = std::make_unique<CSyncManager> ( m_Info );
    m_Info.Vulkan.RenderPassManager = std::make_unique<CRenderPassManager> ( m_Info );
    m_Info.Vulkan.PipelineManager = std::make_unique<CPipelineManager> ( m_Info );
    m_Info.Vulkan.BufferManager = std::make_unique<CBufferManager> ( m_Info );
    m_Info.Vulkan.DescriptorManager = std::make_unique<CDescriptorManager> ( m_Info );

    // Инициализируем в правильном порядке
    if (!m_Info.Vulkan.DeviceManager->Initialize ())
        {
        LogError ( "Failed to initialize DeviceManager" );
        return false;
        }

    if (!m_Info.Vulkan.SwapChainManager->Initialize ())
        {
        LogError ( "Failed to initialize SwapChainManager" );
        return false;
        }

    if (!m_Info.Vulkan.CommandManager->Initialize ())
        {
        LogError ( "Failed to initialize CommandManager" );
        return false;
        }

    if (!m_Info.Vulkan.SyncManager->Initialize ())
        {
        LogError ( "Failed to initialize SyncManager" );
        return false;
        }

    if (!m_Info.Vulkan.RenderPassManager->Initialize ())
        {
        LogError ( "Failed to initialize RenderPassManager" );
        return false;
        }

    if (!m_Info.Vulkan.PipelineManager->Initialize ())
        {
        LogError ( "Failed to initialize PipelineManager" );
        return false;
        }

    if (!m_Info.Vulkan.BufferManager->Initialize ())
        {
        LogError ( "Failed to initialize BufferManager" );
        return false;
        }
    if (!m_Info.Vulkan.DescriptorManager->Initialize ())
        {
        LogError ( "Failed to initialize DescriptorManager" );
        return false;
        }

    auto * pipelineMgr = static_cast< CPipelineManager * >( m_Info.Vulkan.PipelineManager.get () );
    auto * descMgr = static_cast< CDescriptorManager * >( m_Info.Vulkan.DescriptorManager.get () );
    pipelineMgr->SetDescriptorManager ( descMgr );

    LogDebug ( "VulkanContext initialized successfully" );
    m_bInitialized = true;
    return true;
    }

void CVulkanContext::Shutdown ()
    {
    if (!m_bInitialized) return;

    LogDebug ( "Shutting down VulkanContext..." );

    // Очищаем менеджеры в обратном порядке
    m_Info.Vulkan.BufferManager.reset ();
    m_Info.Vulkan.PipelineManager.reset ();
    m_Info.Vulkan.RenderPassManager.reset ();
    m_Info.Vulkan.SyncManager.reset ();
    m_Info.Vulkan.CommandManager.reset ();
    m_Info.Vulkan.SwapChainManager.reset ();
    m_Info.Vulkan.DeviceManager.reset ();

    // Удаляем debug messenger
    if (m_DebugMessenger != VK_NULL_HANDLE)
        {
        auto func = ( PFN_vkDestroyDebugUtilsMessengerEXT ) vkGetInstanceProcAddr (
            m_Info.Vulkan.Instance, "vkDestroyDebugUtilsMessengerEXT" );
        if (func)
            {
            func ( m_Info.Vulkan.Instance, m_DebugMessenger, nullptr );
            }
        m_DebugMessenger = VK_NULL_HANDLE;
        }

    // Surface должна быть удалена до Instance
    if (m_Info.Vulkan.Surface != VK_NULL_HANDLE)
        {
        vkDestroySurfaceKHR ( m_Info.Vulkan.Instance, m_Info.Vulkan.Surface, nullptr );
        m_Info.Vulkan.Surface = VK_NULL_HANDLE;
        }

    // Instance удаляем последним
    if (m_Info.Vulkan.Instance != VK_NULL_HANDLE)
        {
        vkDestroyInstance ( m_Info.Vulkan.Instance, nullptr );
        m_Info.Vulkan.Instance = VK_NULL_HANDLE;
        }

    m_bInitialized = false;
    LogDebug ( "VulkanContext shutdown complete" );
    }

const char * CVulkanContext::GetManagerName () const
    {
    return "VulkanContext";
    }

VkInstance CVulkanContext::GetInstance () const
    { return m_Info.Vulkan.Instance; }

VkSurfaceKHR CVulkanContext::GetSurface () const
    { return m_Info.Vulkan.Surface; }

bool CVulkanContext::CheckValidationLayerSupport () const
    {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties ( &layerCount, nullptr );

    std::vector<VkLayerProperties> availableLayers ( layerCount );
    vkEnumerateInstanceLayerProperties ( &layerCount, availableLayers.data () );

    for (const char * layerName : m_Info.Vulkan.InstanceLayers)
        {
        bool layerFound = false;

        for (const auto & layerProperties : availableLayers)
            {
            if (strcmp ( layerName, layerProperties.layerName ) == 0)
                {
                layerFound = true;
                break;
                }
            }

        if (!layerFound)
            {
            LogError ( "Validation layer not found: ", layerName );
            return false;
            }
        }

    return true;
    }

bool CVulkanContext::CreateInstance ()
    {
    LogDebug ( "Creating Vulkan Instance..." );

    // Получаем необходимые расширения от GLFW
    uint32_t glfwExtensionCount = 0;
    const char ** glfwExtensions = glfwGetRequiredInstanceExtensions ( &glfwExtensionCount );

    if (glfwExtensions == nullptr)
        {
        LogError ( "Failed to get GLFW required extensions" );
        return false;
        }

    std::vector<const char *> extensions ( glfwExtensions, glfwExtensions + glfwExtensionCount );

    // Добавляем debug расширение если нужны validation layers
    if (m_Info.VulkanCreateInfo.bEnableValidationLayers)
        {
        extensions.push_back ( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

        // Проверяем доступность слоев валидации
        if (!CheckValidationLayerSupport ())
            {
            LogError ( "Validation layers requested but not available" );
            return false;
            }

        LogDebug ( "Validation layers enabled" );
        }

    m_Info.Vulkan.InstanceExtensions = extensions;

    LogDebug ( "Required extensions:" );
    for (const auto & ext : extensions)
        {
        LogDebug ( "  - ", ext );
        }

    VkApplicationInfo AppInfo {};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pNext = nullptr;
    AppInfo.pEngineName = m_Info.EngineName.c_str ();
    AppInfo.engineVersion = VK_MAKE_VERSION ( 1, 0, 0 );
    AppInfo.pApplicationName = m_Info.WindowInfo.Title.c_str ();
    AppInfo.applicationVersion = VK_MAKE_VERSION ( 1, 0, 0 );
    AppInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo InstanceInfo {};
    InstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    InstanceInfo.pApplicationInfo = &AppInfo;
    InstanceInfo.enabledExtensionCount = static_cast< uint32_t >( extensions.size () );
    InstanceInfo.ppEnabledExtensionNames = extensions.data ();

    // Validation layers setup - ИСПРАВЛЕНО: используем InstanceLayers, а не InstanceExtensions
    VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo {};
    if (m_Info.VulkanCreateInfo.bEnableValidationLayers)
        {
        InstanceInfo.enabledLayerCount = static_cast< uint32_t >( m_Info.Vulkan.InstanceLayers.size () );
        InstanceInfo.ppEnabledLayerNames = m_Info.Vulkan.InstanceLayers.data ();

        DebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        DebugCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        DebugCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        DebugCreateInfo.pfnUserCallback = DebugCallback;

        InstanceInfo.pNext = &DebugCreateInfo;
        }
    else
        {
        InstanceInfo.enabledLayerCount = 0;
        InstanceInfo.pNext = nullptr;
        }

    VkResult result = vkCreateInstance ( &InstanceInfo, nullptr, &m_Info.Vulkan.Instance );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create VkInstance: ", static_cast< int >( result ) );
        return false;
        }

    // Создаем debug messenger после создания instance
    if (m_Info.VulkanCreateInfo.bEnableValidationLayers)
        {
        auto func = ( PFN_vkCreateDebugUtilsMessengerEXT ) vkGetInstanceProcAddr (
            m_Info.Vulkan.Instance, "vkCreateDebugUtilsMessengerEXT" );
        if (func)
            {
            func ( m_Info.Vulkan.Instance, &DebugCreateInfo, nullptr, &m_DebugMessenger );
            LogDebug ( "Debug messenger created" );
            }
        else
            {
            LogError ( "Failed to get vkCreateDebugUtilsMessengerEXT function" );
            }
        }

    LogDebug ( "Successfully created VkInstance" );
    return true;
    }

bool CVulkanContext::CreateSurface ()
    {
    LogDebug ( "Creating Vulkan Surface..." );

    // Получаем окно из Engine и создаем поверхность
    CWindow * Window = CEngine::Get ().GetWindow ();
    if (!Window)
        {
        LogError ( "Window is null" );
        return false;
        }

    m_Info.Vulkan.Surface = Window->CreateVulkanSurface ();
    if (m_Info.Vulkan.Surface == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create vulkan surface" );
        return false;
        }

    LogDebug ( "Successfully created Surface: ", reinterpret_cast< void * >( m_Info.Vulkan.Surface ) );
    return true;
    }
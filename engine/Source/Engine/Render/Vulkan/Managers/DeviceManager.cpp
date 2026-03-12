#include "Render/Vulkan/Managers/DeviceManager.h"
#include "Core/EngineInfo.h"
#include <vector>
#include <set>
#include <string>

CDeviceManager::CDeviceManager ( FEngineInfo & info )
    : IVulkanManager ( info )
    {}

CDeviceManager::~CDeviceManager ()
    {
    Shutdown ();
    }

bool CDeviceManager::Initialize ()
    {
    LogDebug ( "Initializing DeviceManager..." );

    // Проверяем, что Instance и Surface созданы
    if (m_Info.Vulkan.Instance == VK_NULL_HANDLE)
        {
        LogError ( "Vulkan Instance is not created" );
        return false;
        }

    if (m_Info.Vulkan.Surface == VK_NULL_HANDLE)
        {
        LogError ( "Vulkan Surface is not created" );
        return false;
        }

    // Выбираем физическое устройство
    if (!PickPhysicalDevice ())
        {
        LogError ( "Failed to pick physical device" );
        return false;
        }

    // Создаем логическое устройство
    if (!CreateLogicalDevice ())
        {
        LogError ( "Failed to create logical device" );
        return false;
        }

    // Получаем очереди
    vkGetDeviceQueue ( m_Device, m_GraphicsFamily, 0, &m_GraphicsQueue );
    vkGetDeviceQueue ( m_Device, m_PresentFamily, 0, &m_PresentQueue );

    LogDebug ( "DeviceManager initialized successfully" );
    m_bInitialized = true;
    return true;
    }

void CDeviceManager::Shutdown ()
    {
    if (!m_bInitialized) return;

    LogDebug ( "Shutting down DeviceManager..." );

    if (m_Device != VK_NULL_HANDLE)
        {
        vkDeviceWaitIdle ( m_Device );
        vkDestroyDevice ( m_Device, nullptr );
        m_Device = VK_NULL_HANDLE;
        }

    // Сбрасываем все указатели
    m_PhysicalDevice = VK_NULL_HANDLE;
    m_GraphicsFamily = UINT32_MAX;
    m_PresentFamily = UINT32_MAX;
    m_GraphicsQueue = VK_NULL_HANDLE;
    m_PresentQueue = VK_NULL_HANDLE;

    m_bInitialized = false;
    }

const char * CDeviceManager::GetManagerName () const
    {
    return "DeviceManager";
    }

bool CDeviceManager::PickPhysicalDevice ()
    {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices ( m_Info.Vulkan.Instance, &deviceCount, nullptr );

    if (deviceCount == 0)
        {
        LogError ( "No Vulkan compatible GPUs found" );
        return false;
        }

    std::vector<VkPhysicalDevice> devices ( deviceCount );
    vkEnumeratePhysicalDevices ( m_Info.Vulkan.Instance, &deviceCount, devices.data () );

    // Сначала ищем дискретную графику
    for (const auto & device : devices)
        {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties ( device, &deviceProperties );

        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
            if (IsDeviceSuitable ( device ) && FindQueueFamilies ( device ))
                {
                m_PhysicalDevice = device;
                LogDebug ( "Selected discrete GPU: ", deviceProperties.deviceName );
                break;
                }
            }
        }

    // Если не нашли дискретную, ищем интегрированную
    if (m_PhysicalDevice == VK_NULL_HANDLE)
        {
        for (const auto & device : devices)
            {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties ( device, &deviceProperties );

            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                {
                if (IsDeviceSuitable ( device ) && FindQueueFamilies ( device ))
                    {
                    m_PhysicalDevice = device;
                    LogDebug ( "Selected integrated GPU: ", deviceProperties.deviceName );
                    break;
                    }
                }
            }
        }

    // Если все еще не нашли, берем любую подходящую
    if (m_PhysicalDevice == VK_NULL_HANDLE)
        {
        for (const auto & device : devices)
            {
            if (IsDeviceSuitable ( device ) && FindQueueFamilies ( device ))
                {
                m_PhysicalDevice = device;

                VkPhysicalDeviceProperties deviceProperties;
                vkGetPhysicalDeviceProperties ( device, &deviceProperties );
                LogDebug ( "Selected GPU (other type): ", deviceProperties.deviceName );
                break;
                }
            }
        }

    if (m_PhysicalDevice == VK_NULL_HANDLE)
        {
        LogError ( "No suitable physical device found" );
        return false;
        }

    return true;
    }

bool CDeviceManager::IsDeviceSuitable ( VkPhysicalDevice device ) const
    {
    // Проверяем поддержку необходимых расширений
    if (!CheckDeviceExtensionSupport ( device ))
        {
        return false;
        }

    // Проверяем поддержку swapchain
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( device, m_Info.Vulkan.Surface, &capabilities );

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR ( device, m_Info.Vulkan.Surface, &formatCount, nullptr );

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR ( device, m_Info.Vulkan.Surface, &presentModeCount, nullptr );

    if (formatCount == 0 || presentModeCount == 0)
        {
        return false;
        }

    // Дополнительные проверки (опционально)
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures ( device, &supportedFeatures );

    // Проверяем наличие необходимых фич (если нужны)
    // if (!supportedFeatures.samplerAnisotropy) return false;

    return true;
    }

bool CDeviceManager::CheckDeviceExtensionSupport ( VkPhysicalDevice device ) const
    {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties ( device, nullptr, &extensionCount, nullptr );

    std::vector<VkExtensionProperties> availableExtensions ( extensionCount );
    vkEnumerateDeviceExtensionProperties ( device, nullptr, &extensionCount, availableExtensions.data () );

    std::set<std::string> requiredExtensions (
        m_Info.VulkanCreateInfo.DeviceExtensions.begin (),
        m_Info.VulkanCreateInfo.DeviceExtensions.end () );

    for (const auto & extension : availableExtensions)
        {
        requiredExtensions.erase ( extension.extensionName );
        }

    if (!requiredExtensions.empty ())
        {
        LogDebug ( "Missing required extensions:" );
        for (const auto & ext : requiredExtensions)
            {
            LogDebug ( "  - ", ext.c_str () );
            }
        }

    return requiredExtensions.empty ();
    }

bool CDeviceManager::FindQueueFamilies ( VkPhysicalDevice device )
    {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, nullptr );

    std::vector<VkQueueFamilyProperties> queueFamilies ( queueFamilyCount );
    vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, queueFamilies.data () );

    // Временные переменные для поиска
    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t presentFamily = UINT32_MAX;

    for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
        // Ищем семью с графической очередью
        if (queueFamilies[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
            graphicsFamily = i;
            }

        // Ищем семью с поддержкой презентации
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR ( device, i, m_Info.Vulkan.Surface, &presentSupport );
        if (presentSupport)
            {
            presentFamily = i;
            }

        // Если нашли обе семьи, можно прервать цикл
        if (graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX)
            {
            break;
            }
        }

    // Проверяем, что нашли обе семьи
    if (graphicsFamily == UINT32_MAX || presentFamily == UINT32_MAX)
        {
        LogError ( "Failed to find required queue families" );
        return false;
        }

    // Сохраняем найденные семьи
    m_GraphicsFamily = graphicsFamily;
    m_PresentFamily = presentFamily;

    LogDebug ( "Found queue families - Graphics: ", m_GraphicsFamily,
               ", Present: ", m_PresentFamily );

    return true;
    }

bool CDeviceManager::CreateLogicalDevice ()
    {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    // Уникальные семьи для создания очередей
    std::set<uint32_t> uniqueQueueFamilies = { m_GraphicsFamily, m_PresentFamily };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
        {
        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back ( queueCreateInfo );
        }

    // Фичи устройства (можно расширить по необходимости)
    VkPhysicalDeviceFeatures deviceFeatures {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;  // Для текстур
    deviceFeatures.fillModeNonSolid = VK_TRUE;   // Для wireframe режима
    deviceFeatures.wideLines = VK_TRUE;          // Для толстых линий

    VkDeviceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast< uint32_t >( queueCreateInfos.size () );
    createInfo.pQueueCreateInfos = queueCreateInfos.data ();
    createInfo.pEnabledFeatures = &deviceFeatures;

    // Расширения устройства
    createInfo.enabledExtensionCount = static_cast< uint32_t >( m_Info.VulkanCreateInfo.DeviceExtensions.size () );
    createInfo.ppEnabledExtensionNames = m_Info.VulkanCreateInfo.DeviceExtensions.data ();

    // Слои (для совместимости - deprecated, но оставляем)
    if (m_Info.VulkanCreateInfo.bEnableValidationLayers)
        {
        createInfo.enabledLayerCount = static_cast< uint32_t >( m_Info.Vulkan.InstanceLayers.size () );
        createInfo.ppEnabledLayerNames = m_Info.Vulkan.InstanceLayers.data ();
        }
    else
        {
        createInfo.enabledLayerCount = 0;
        }

    VkResult result = vkCreateDevice ( m_PhysicalDevice, &createInfo, nullptr, &m_Device );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create logical device: ", static_cast< int >( result ) );
        return false;
        }

    LogDebug ( "Logical device created successfully" );
    return true;
    }

uint32_t CDeviceManager::FindMemoryType ( uint32_t TypeFilter, VkMemoryPropertyFlags Properties ) const
    {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties ( m_PhysicalDevice, &memProperties );

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
        if (( TypeFilter & ( 1 << i ) ) &&
             ( memProperties.memoryTypes[ i ].propertyFlags & Properties ) == Properties)
            {
            return i;
            }
        }

    LogError ( "Failed to find suitable memory type" );
    return UINT32_MAX;
    }
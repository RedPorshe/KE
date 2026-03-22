#include "KE/Vulkan/Managers/DeviceManager.h"
#include "KE/Vulkan/VKinfo.h"
#include "KE/Vulkan/Managers/InstanceManager.h"
#include <set>

DeviceManager::DeviceManager () : IVKManager ()
    {
    DeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };
    }

DeviceManager::~DeviceManager ()
    {
    Shutdown ();
    }

bool DeviceManager::Init ()
    {
    if (!PickPhysicalDevice ())
        {
        LogError ( "Failed to Pick Physical Device" );
        return false;
        }
    if (!CreateLogicalDevice ())
        {
        LogError ( "Failed to create logical Device" );
        return false;
        }
    if (!GetQueues ())
        {
        LogError ( "Failed to get Queues" );
        return false;
        }
    bIsInitialized = true;
    return true;
    }

void DeviceManager::Shutdown ()
    {
    LogDebug ( "Shutting down device manager" );

    if (m_device != VK_NULL_HANDLE)
        {
        vkDestroyDevice ( m_device, nullptr );
        m_device = VK_NULL_HANDLE;
        LogDebug ( "Logical device destroyed" );
        }

    m_physicalDevice = VK_NULL_HANDLE;
    m_graphicsQueue = VK_NULL_HANDLE;
    m_presentQueue = VK_NULL_HANDLE;
    m_graphicsQueueFamily = 0;
    m_presentQueueFamily = 0;

    bIsInitialized = false;
    }

const std::string & DeviceManager::GetManagerName () const
    {
    static const std::string name = "Device Manager";
    return name;
    }

bool DeviceManager::PickPhysicalDevice ()
    {
    uint32_t deviceCount = 0;
    VkInstance instance = VK_NULL_HANDLE;

    if (InstanceManager * mgr = dynamic_cast< InstanceManager * >( m_info->Managers.InstanceManager.get () ))
        {
        instance = mgr->GetInstance ();
        }
    else
        {
        LogError ( "Instance not created" );
        return false;
        }

    vkEnumeratePhysicalDevices ( instance, &deviceCount, nullptr );

    if (deviceCount == 0)
        {
        LogError ( "No Vulkan compatible GPUs found" );
        return false;
        }

    std::vector<VkPhysicalDevice> devices ( deviceCount );
    vkEnumeratePhysicalDevices ( instance, &deviceCount, devices.data () );

    for (const auto & device : devices)
        {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties ( device, &deviceProperties );

        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
            if (IsDeviceSuitable ( device ) && FindQueueFamilies ( device ))
                {
                m_physicalDevice = device;
                LogDebug ( "Selected discrete GPU: ", deviceProperties.deviceName );
                break;
                }
            }
        }

    if (m_physicalDevice == VK_NULL_HANDLE)
        {
        for (const auto & device : devices)
            {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties ( device, &deviceProperties );

            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                {
                if (IsDeviceSuitable ( device ) && FindQueueFamilies ( device ))
                    {
                    m_physicalDevice = device;
                    LogDebug ( "Selected integrated GPU: ", deviceProperties.deviceName );
                    break;
                    }
                }
            }
        }

    if (m_physicalDevice == VK_NULL_HANDLE)
        {
        for (const auto & device : devices)
            {
            if (IsDeviceSuitable ( device ) && FindQueueFamilies ( device ))
                {
                m_physicalDevice = device;
                VkPhysicalDeviceProperties deviceProperties;
                vkGetPhysicalDeviceProperties ( device, &deviceProperties );
                LogDebug ( "Selected GPU (other type): ", deviceProperties.deviceName );
                break;
                }
            }
        }

    if (m_physicalDevice == VK_NULL_HANDLE)
        {
        LogError ( "No suitable physical device found" );
        return false;
        }

    return true;
    }

bool DeviceManager::IsDeviceSuitable ( VkPhysicalDevice device ) const
    {
    if (!CheckDeviceExtensionSupport ( device ))
        {
        return false;
        }

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (InstanceManager * mgr = dynamic_cast< InstanceManager * >( m_info->Managers.InstanceManager.get () ))
        {
        surface = mgr->GetSurface ();
        }

    if (surface == VK_NULL_HANDLE)
        {
        LogError ( "No surface available" );
        return false;
        }

        // Используем v2 функции
    VkSurfaceCapabilities2KHR surfaceCapabilities2 {};
    surfaceCapabilities2.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;

    VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2 {};
    surfaceInfo2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    surfaceInfo2.surface = surface;

    VkResult result = vkGetPhysicalDeviceSurfaceCapabilities2KHR ( device, &surfaceInfo2, &surfaceCapabilities2 );

    if (result != VK_SUCCESS)
        {
        return false;
        }

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR ( device, surface, &formatCount, nullptr );

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR ( device, surface, &presentModeCount, nullptr );

    if (formatCount == 0 || presentModeCount == 0)
        {
        LogDebug ( "Device doesn't support swapchain formats or present modes" );
        return false;
        }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures ( device, &supportedFeatures );

    if (!supportedFeatures.samplerAnisotropy)
        {
        LogDebug ( "Device doesn't support anisotropic filtering" );
        }

    return true;
    }

bool DeviceManager::CheckDeviceExtensionSupport ( VkPhysicalDevice device ) const
    {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties ( device, nullptr, &extensionCount, nullptr );

    std::vector<VkExtensionProperties> availableExtensions ( extensionCount );
    vkEnumerateDeviceExtensionProperties ( device, nullptr, &extensionCount, availableExtensions.data () );

    std::set<std::string> requiredExtensions ( DeviceExtensions.begin (), DeviceExtensions.end () );

    for (const auto & extension : availableExtensions)
        {
        requiredExtensions.erase ( extension.extensionName );
        }

    if (!requiredExtensions.empty ())
        {
        LogError ( "Missing required device extensions:" );
        for (const auto & ext : requiredExtensions)
            {
            LogError ( "  - ", ext );
            }
        return false;
        }

    LogDebug ( "All required device extensions supported" );
    return true;
    }

bool DeviceManager::FindQueueFamilies ( VkPhysicalDevice device )
    {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, nullptr );

    std::vector<VkQueueFamilyProperties> queueFamilies ( queueFamilyCount );
    vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, queueFamilies.data () );

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (InstanceManager * mgr = dynamic_cast< InstanceManager * > ( m_info->Managers.InstanceManager.get () ))
        {
        surface = mgr->GetSurface ();
        }

    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t presentFamily = UINT32_MAX;

    for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
        if (queueFamilies[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
            graphicsFamily = i;
            }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR ( device, i, surface, &presentSupport );
        if (presentSupport)
            {
            presentFamily = i;
            }

        if (graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX)
            {
            break;
            }
        }

    if (graphicsFamily == UINT32_MAX || presentFamily == UINT32_MAX)
        {
        LogError ( "Failed to find required queue families" );
        return false;
        }

    m_graphicsQueueFamily = graphicsFamily;
    m_presentQueueFamily = presentFamily;

    LogDebug ( "Found queue families - Graphics: ", m_graphicsQueueFamily,
               ", Present: ", m_presentQueueFamily );

    return true;
    }

bool DeviceManager::CreateLogicalDevice ()
    {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { m_graphicsQueueFamily, m_presentQueueFamily };

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

        // Простые фичи устройства - без pNext
    VkPhysicalDeviceFeatures deviceFeatures {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;
    deviceFeatures.wideLines = VK_TRUE;

    VkDeviceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast< uint32_t >( queueCreateInfos.size () );
    createInfo.pQueueCreateInfos = queueCreateInfos.data ();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast< uint32_t >( DeviceExtensions.size () );
    createInfo.ppEnabledExtensionNames = DeviceExtensions.data ();

    VkResult result = vkCreateDevice ( m_physicalDevice, &createInfo, nullptr, &m_device );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create logical device: ", static_cast< int >( result ) );
        return false;
        }

    LogDebug ( "Logical device created successfully" );
    return true;
    }

bool DeviceManager::GetQueues ()
    {
    if (!m_device)
        {
        LogError ( "Device not created" );
        return false;
        }

    vkGetDeviceQueue ( m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue );
    if (!m_graphicsQueue)
        {
        LogError ( "Failed to get graphics queue" );
        return false;
        }

    vkGetDeviceQueue ( m_device, m_presentQueueFamily, 0, &m_presentQueue );
    if (!m_presentQueue)
        {
        LogError ( "Failed to get present queue" );
        return false;
        }

    LogDebug ( "Queues obtained - Graphics: ", ( void * ) m_graphicsQueue,
               ", Present: ", ( void * ) m_presentQueue );
    return true;
    }

uint32_t DeviceManager::FindMemoryType ( uint32_t TypeFilter, VkMemoryPropertyFlags Properties ) const
    {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties ( m_physicalDevice, &memProperties );

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

    //=============================================================================
    // Extended Properties Getters (Vulkan 1.1+)
    //=============================================================================

void DeviceManager::GetPhysicalDeviceProperties2 ( VkPhysicalDeviceProperties2 & props2 ) const
    {
    props2 = {};
    props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    vkGetPhysicalDeviceProperties2 ( m_physicalDevice, &props2 );
    }

void DeviceManager::GetPhysicalDeviceFeatures2 ( VkPhysicalDeviceFeatures2 & features2 ) const
    {
    features2 = {};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    vkGetPhysicalDeviceFeatures2 ( m_physicalDevice, &features2 );
    }

void DeviceManager::GetPhysicalDeviceMemoryProperties2 ( VkPhysicalDeviceMemoryProperties2 & memProps2 ) const
    {
    memProps2 = {};
    memProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    vkGetPhysicalDeviceMemoryProperties2 ( m_physicalDevice, &memProps2 );
    }

void DeviceManager::GetSurfaceCapabilities2 ( VkSurfaceKHR surface, VkSurfaceCapabilities2KHR & caps2 ) const
    {
    VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2 {};
    surfaceInfo2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    surfaceInfo2.surface = surface;

    caps2 = {};
    caps2.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;

    vkGetPhysicalDeviceSurfaceCapabilities2KHR ( m_physicalDevice, &surfaceInfo2, &caps2 );
    }
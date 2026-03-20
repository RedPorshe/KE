#include "KE/Vulkan/Managers/DeviceManager.h"
#include "KE/Vulkan/VKinfo.h"
#include "KE/Vulkan/Managers/InstanceManager.h"
#include <set>

DeviceManager::DeviceManager () : IVKManager()
	{
	   // Добавляем необходимые расширения
	DeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		// Можно добавить другие расширения при необходимости
		// VK_KHR_MAINTENANCE1_EXTENSION_NAME,
		// VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
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
	if ( InstanceManager * mgr = dynamic_cast< InstanceManager * >( m_info->Managers.InstanceManager.get () ) )
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
	vkEnumeratePhysicalDevices ( instance, &deviceCount, devices.data() );

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

	// Если все еще не нашли, берем любую подходящую
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

bool DeviceManager::CreateLogicalDevice ()
	{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	// Уникальные семьи для создания очередей
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
	createInfo.enabledExtensionCount = static_cast< uint32_t >(DeviceExtensions.size () );
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

		// Получаем графическую очередь
	vkGetDeviceQueue ( m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue );
	if (!m_graphicsQueue)
		{
		LogError ( "Failed to get graphics queue" );
		return false;
		}

		// Получаем очередь презентации
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

bool DeviceManager::IsDeviceSuitable ( VkPhysicalDevice device ) const
	{
		// Проверяем поддержку необходимых расширений
	if (!CheckDeviceExtensionSupport ( device ))
		{
		return false;
		}

		// Проверяем поддержку swapchain
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

	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( device, surface, &capabilities );

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR ( device, surface, &formatCount, nullptr );

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR ( device, surface, &presentModeCount, nullptr );

	if (formatCount == 0 || presentModeCount == 0)
		{
		LogDebug ( "Device doesn't support swapchain formats or present modes" );
		return false;
		}

		// Проверяем поддержку анизотропной фильтрации (для текстур)
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures ( device, &supportedFeatures );
	if (!supportedFeatures.samplerAnisotropy)
		{
		LogDebug ( "Device doesn't support anisotropic filtering" );
		// Можем продолжать, но с предупреждением
		}

	return true;
	}
bool DeviceManager::CheckDeviceExtensionSupport ( VkPhysicalDevice device ) const
	{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties ( device, nullptr, &extensionCount, nullptr );

	std::vector<VkExtensionProperties> availableExtensions ( extensionCount );
	vkEnumerateDeviceExtensionProperties ( device, nullptr, &extensionCount, availableExtensions.data () );

	// Проверяем наличие всех необходимых расширений
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
		vkGetPhysicalDeviceSurfaceSupportKHR ( device, i, surface, &presentSupport );
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
	m_graphicsQueueFamily = graphicsFamily;
	m_presentQueueFamily = presentFamily;

	LogDebug ( "Found queue families - Graphics: ", m_graphicsQueueFamily,
			   ", Present: ", m_presentQueueFamily );

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
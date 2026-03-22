#include "KE/Vulkan/VulkanContext.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "KE/Vulkan/VKinfo.h"
#include "KE/Vulkan/Managers/InstanceManager.h"
#include "KE/Vulkan/Managers/DeviceManager.h"
#include "KE/Vulkan/Managers/SwapchainManager.h"
#include "KE/Vulkan/Managers/BufferManager.h"
#include "KE/Vulkan/Managers/CommandManager.h"
#include "KE/Vulkan/Managers/DescriptorManager.h"
#include "KE/Vulkan/Managers/SyncManager.h"
#include "KE/Vulkan/Managers/RenderPassManager.h"
#include "KE/Vulkan/Managers/PipelineManager.h"

VulkanContext::VulkanContext () :m_info ( new VkInfo () ) 
	{
	LOG_DEBUG ( "[VulkanContext] Created" );
	}

VulkanContext::~VulkanContext ()
	{
	Shutdown ();
	}

bool VulkanContext::PreInit ()
	{
	LOG_DEBUG ( "[VulkanContext] PreInit - Checking Vulkan support" );

	  // Проверяем доступность Vulkan
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties ( nullptr, &extensionCount, nullptr );

	if (extensionCount == 0)
		{
		LOG_ERROR ( "No Vulkan extensions found" );
		return false;
		}
	LOG_DEBUG ( "Found ", extensionCount, " Vulkan extensions" );

	m_info->Managers.InstanceManager = MakeShared<InstanceManager> ();
	m_info->Managers.DeviceManager = MakeShared<DeviceManager> ();
	m_info->Managers.SwapchainManager = MakeShared<SwapchainManager> ();
	m_info->Managers.CommandManager = MakeShared<CommandManager> ();
	m_info->Managers.SyncManager = MakeShared<CSyncManager> ();
	m_info->Managers.RenderPassManager = MakeShared<RenderPassManager> ();
	m_info->Managers.DescriptorManager = MakeShared<DescriptorManager> ();
	m_info->Managers.PipelineManager = MakeShared<PipelineManager> ();
	m_info->Managers.BufferManager = MakeShared<BufferManager> ();

	return true;
	}

bool VulkanContext::Init ()
	{
	LOG_DEBUG ( "[VulkanContext] Init" );

	if (!m_window)
		{
		LOG_WARN ( "[VulkanContext] Window not set" );
		}

	m_info->Window = m_window;

	
	m_info->Managers.InstanceManager->SetInfoPtr ( m_info );
	if (!m_info->Managers.InstanceManager->Init ())
		{
		LOG_ERROR ( "Fail init Instance Manager" );
		return false;
		}

	
	m_info->Managers.DeviceManager->SetInfoPtr ( m_info );
	if (!m_info->Managers.DeviceManager->Init ())
		{
		LOG_ERROR ( "Fail to Init Device Manager" );
		return false;
		}

	
	m_info->Managers.SwapchainManager->SetInfoPtr ( m_info );
	if (!m_info->Managers.SwapchainManager->Init ())
		{
		LOG_ERROR ( "Fail to init Swapchain Manager" );
		return false;
		}
	
	m_info->Managers.CommandManager->SetInfoPtr ( m_info );
	if (!m_info->Managers.CommandManager->Init ())
		{
		LOG_ERROR ( "Fail to init Command Manager" );
		return false;
		}

	
	m_info->Managers.SyncManager->SetInfoPtr ( m_info );
	if (!m_info->Managers.SyncManager->Init ())
		{
		LOG_ERROR ( "Fail to init SyncManager" );
		return false;
		}

	
	m_info->Managers.RenderPassManager->SetInfoPtr ( m_info );
	if (!m_info->Managers.RenderPassManager->Init ())
		{
		LOG_ERROR ( "Fail to init RenderPass Manager" );
		return false;
		}

	
	m_info->Managers.DescriptorManager->SetInfoPtr ( m_info );
	if (!m_info->Managers.DescriptorManager->Init ())
		{
		LOG_ERROR ( "Fail to init Descriptor Manager" );
		return false;
		}
	
	m_info->Managers.PipelineManager->SetInfoPtr ( m_info );
	if (PipelineManager * PipelineMgr = dynamic_cast< PipelineManager * >( m_info->Managers.PipelineManager.get () ))
		{
		if (DescriptorManager * descMgr = dynamic_cast< DescriptorManager * >( m_info->Managers.DescriptorManager.get () ))
			{
			PipelineMgr->SetDescriptorManager ( descMgr );
			if (!m_info->Managers.PipelineManager->Init ())
				{
				LOG_ERROR ( "Fail to init Pipeline Manager" );
				return false;
				}
			}
		}



	
	m_info->Managers.BufferManager->SetInfoPtr ( m_info );
	if (!m_info->Managers.BufferManager->Init ())
		{
		LOG_ERROR ( "Fail to init Buffer Manager" );
		return false;
		}


	bIsInitialized = true;
	LOG_INFO ( "[VulkanContext] Initialized successfully" );
	return true;
	}

void VulkanContext::Update ( float Deltatime )
	{
	if (!bIsInitialized) return;

	static int count = 0;
	if (count < 5)
		{
		LOG_DEBUG ( "[VulkanContext] Updating with dt: ", Deltatime, " ms" );
		count++;
		}


	}

void VulkanContext::SetEngineName ( const std::string & inName )
	{
	m_info->EngineName = inName;
	}

void VulkanContext::SetAplicationName ( const std::string & inName )
	{
	m_info->ApplicationName = inName;
	}

void VulkanContext::Shutdown ()
	{

	LOG_DEBUG ( "[VulkanContext] Shutting down" );
	m_info->Shutdown ();

	bIsInitialized = false;
	m_window = nullptr;
	}
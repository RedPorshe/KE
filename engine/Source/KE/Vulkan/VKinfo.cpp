#include "KE/Vulkan/VKinfo.h"

void VKManagers::Shutdown ()
	{
    DescriptorManager.reset ();
    BufferManager.reset ();
    PipelineManager.reset ();
    RenderPassManager.reset ();
    SyncManager.reset ();
    CommandManager.reset ();
    SwapchainManager.reset ();
    DeviceManager.reset ();
    InstanceManager.reset ();
    LOG_DEBUG ( "managers resets" );
	}



void VkInfo::Shutdown ()
    {
    Managers.Shutdown ();
    Window = nullptr;
    LOG_DEBUG ( "Info shutdown complete" );
    }

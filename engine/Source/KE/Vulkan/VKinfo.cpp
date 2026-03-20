#include "KE/Vulkan/VKinfo.h"

void VKManagers::Shutdown ()
	{
    SyncManager.reset ();
    PipelineManager.reset ();
    DescriptorManager.reset ();
    CommandManager.reset ();
    BufferManager.reset ();
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

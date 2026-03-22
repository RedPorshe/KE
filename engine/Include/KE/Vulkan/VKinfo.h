#pragma once

#include "CoreMinimal.h"
class IVKManager;

class WindowSystem;

struct GLFWwindow;

struct KE_API VKManagers
	{
	TSharedPtr<IVKManager> InstanceManager;
	TSharedPtr<IVKManager> DeviceManager;
	TSharedPtr<IVKManager> SwapchainManager;
	TSharedPtr<IVKManager> BufferManager;
	TSharedPtr<IVKManager> CommandManager;
	TSharedPtr<IVKManager> DescriptorManager;
	TSharedPtr<IVKManager> PipelineManager;
	TSharedPtr<IVKManager> RenderPassManager;
	TSharedPtr<IVKManager> SyncManager;	
	void Shutdown ();
	};

struct KE_API VkInfo
	{
	GLFWwindow * Window = nullptr;
	VKManagers Managers;
	std::string ApplicationName {};
	std::string EngineName {};
	TSharedPtr< WindowSystem> WindowSystem;
	VkInfo () = default;
	void Shutdown ();
	};
#pragma once
#include "KE/Vulkan/VKManager.h"
#include <vulkan/vulkan.h>
#include <vector>

struct FEngineInfo;

class KE_API CSyncManager final : public IVKManager
    {
    public:
        CSyncManager ();
        virtual ~CSyncManager ();

        // IVulkanManager
        bool Init () override;
        void Shutdown () override;
      

        // Create sync objects
        VkSemaphore CreateSemaphore ();
        VkFence CreateFence ( bool bCreateSignaled = false );

        // Frame sync objects (for triple buffering)
        bool CreateFrameSyncObjects ( uint32_t FramesInFlight );

        // Getters for frame sync objects
        VkSemaphore GetImageAvailableSemaphore ( uint32_t FrameIndex ) const;
        VkSemaphore GetRenderFinishedSemaphore ( uint32_t FrameIndex ) const;
        VkFence GetInFlightFence ( uint32_t FrameIndex ) const;

        // Get current frame index
        uint32_t GetCurrentFrame () const { return m_CurrentFrame; }
        void SetCurrentFrame ( uint32_t Frame ) { m_CurrentFrame = Frame % m_FramesInFlight; }
        void NextFrame () { m_CurrentFrame = ( m_CurrentFrame + 1 ) % m_FramesInFlight; }

        // Sync operations
        VkResult WaitForFence ( VkFence Fence, uint64_t Timeout = UINT64_MAX );
        VkResult WaitForFences ( const std::vector<VkFence> & Fences, bool bWaitAll, uint64_t Timeout = UINT64_MAX );
        void ResetFence ( VkFence Fence );
        void ResetFences ( const std::vector<VkFence> & Fences );

        // Destroy sync objects
        void DestroySemaphore ( VkSemaphore Semaphore );
        void DestroyFence ( VkFence Fence );

    private:
        struct KE_API FFrameSyncObjects
            {
            VkSemaphore ImageAvailable = VK_NULL_HANDLE;
            VkSemaphore RenderFinished = VK_NULL_HANDLE;
            VkFence InFlight = VK_NULL_HANDLE;

            bool IsValid () const
                {
                return ImageAvailable != VK_NULL_HANDLE &&
                    RenderFinished != VK_NULL_HANDLE &&
                    InFlight != VK_NULL_HANDLE;
                }
            };

    private:
        std::vector<FFrameSyncObjects> m_FrameSyncObjects;
        uint32_t m_FramesInFlight = 0;
        uint32_t m_CurrentFrame = 0;

        // Inherited via IVKManager
        const std::string & GetManagerName () const override;
    };
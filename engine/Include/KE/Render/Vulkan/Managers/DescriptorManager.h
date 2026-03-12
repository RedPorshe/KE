#pragma once
#include "Render/Vulkan/VulkanInterface.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <memory>

struct FEngineInfo;

// Структура для описания набора дескрипторов
struct KE_API FDescriptorSetLayoutInfo
    {
    std::vector<VkDescriptorSetLayoutBinding> Bindings;
    std::string Name;
    };

    // Структура для хранения пула дескрипторов и наборов
struct KE_API FDescriptorPoolInfo
    {
    VkDescriptorPool Pool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> DescriptorSets;
    std::string Name;
    uint32_t MaxSets = 0;
    bool IsValid () const { return Pool != VK_NULL_HANDLE; }
    };

    // Структура для описания ресурса в дескрипторе
struct KE_API FDescriptorResourceInfo
    {
    VkDescriptorType Type;
    union
        {
        VkBuffer Buffer;
        VkImageView ImageView;
        VkSampler Sampler;
        };
    VkDeviceSize Offset = 0;
    VkDeviceSize Range = VK_WHOLE_SIZE;
    VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    };

class KE_API CDescriptorManager final : public IVulkanManager
    {
    public:
        CDescriptorManager ( FEngineInfo & Info );
        virtual ~CDescriptorManager ();

        // IVulkanManager
        bool Initialize () override;
        void Shutdown () override;
        const char * GetManagerName () const override;

        // Создание layout дескрипторов
        VkDescriptorSetLayout CreateDescriptorSetLayout (
            const std::string & LayoutName,
            const std::vector<VkDescriptorSetLayoutBinding> & Bindings );

        VkDescriptorSetLayout CreateDescriptorSetLayout (
            const std::string & LayoutName,
            const FDescriptorSetLayoutInfo & Info );

        // Получение layout по имени
        VkDescriptorSetLayout GetDescriptorSetLayout ( const std::string & LayoutName ) const;
        bool HasDescriptorSetLayout ( const std::string & LayoutName ) const;

        // Создание пула дескрипторов
        VkDescriptorPool CreateDescriptorPool (
            const std::string & PoolName,
            uint32_t MaxSets,
            const std::vector<VkDescriptorPoolSize> & PoolSizes,
            VkDescriptorPoolCreateFlags Flags = 0 );

        // Получение пула по имени
        VkDescriptorPool GetDescriptorPool ( const std::string & PoolName ) const;
        bool HasDescriptorPool ( const std::string & PoolName ) const;

        // Выделение наборов дескрипторов из пула
        std::vector<VkDescriptorSet> AllocateDescriptorSets (
            const std::string & PoolName,
            const std::vector<VkDescriptorSetLayout> & Layouts );

        VkDescriptorSet AllocateDescriptorSet (
            const std::string & PoolName,
            VkDescriptorSetLayout Layout );

        // Освобождение наборов дескрипторов
        void FreeDescriptorSet ( const std::string & PoolName, VkDescriptorSet DescriptorSet );
        void FreeDescriptorSets ( const std::string & PoolName, const std::vector<VkDescriptorSet> & DescriptorSets );

        // Обновление дескрипторов
        void UpdateDescriptorSet (
            VkDescriptorSet DescriptorSet,
            uint32_t Binding,
            VkDescriptorType Type,
            const FDescriptorResourceInfo & ResourceInfo );

        void UpdateDescriptorSets (
            const std::vector<VkWriteDescriptorSet> & Writes );

        // Удобные методы для обновления разных типов дескрипторов
        void UpdateBufferDescriptor (
            VkDescriptorSet DescriptorSet,
            uint32_t Binding,
            VkBuffer Buffer,
            VkDeviceSize Offset = 0,
            VkDeviceSize Range = VK_WHOLE_SIZE,
            VkDescriptorType Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER );

        void UpdateImageDescriptor (
            VkDescriptorSet DescriptorSet,
            uint32_t Binding,
            VkImageView ImageView,
            VkSampler Sampler,
            VkImageLayout Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VkDescriptorType Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER );

        // Создание стандартных layout'ов
        bool CreateDefaultLayouts ();

        // Создание стандартных пулов
        bool CreateDefaultPools ();

        // Получение стандартных layout'ов
        VkDescriptorSetLayout GetGlobalLayout () const { return m_GlobalLayout; }
        VkDescriptorSetLayout GetPerObjectLayout () const { return m_PerObjectLayout; }
        VkDescriptorSetLayout GetTextureLayout () const { return m_TextureLayout; }

        // Получение стандартных пулов
        VkDescriptorPool GetGlobalPool () const { return m_GlobalPool; }
        VkDescriptorPool GetPerFramePool () const { return m_PerFramePool; }

        // Сброс пула (очистка всех наборов)
        bool ResetDescriptorPool ( const std::string & PoolName );
        bool ResetDescriptorPool ( VkDescriptorPool Pool );

    private:
        // Вспомогательные методы
        VkDescriptorSetLayout CreateDescriptorSetLayoutInternal (
            const std::vector<VkDescriptorSetLayoutBinding> & Bindings );

        VkDescriptorPool CreateDescriptorPoolInternal (
            uint32_t MaxSets,
            const std::vector<VkDescriptorPoolSize> & PoolSizes,
            VkDescriptorPoolCreateFlags Flags );

        // Кэши
        std::unordered_map<std::string, VkDescriptorSetLayout> m_LayoutCache;
        std::unordered_map<std::string, VkDescriptorPool> m_PoolCache;
        std::unordered_map<std::string, FDescriptorPoolInfo> m_PoolInfoCache;

        // Стандартные layout'ы
        VkDescriptorSetLayout m_GlobalLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_PerObjectLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_TextureLayout = VK_NULL_HANDLE;

        // Стандартные пулы
        VkDescriptorPool m_GlobalPool = VK_NULL_HANDLE;
        VkDescriptorPool m_PerFramePool = VK_NULL_HANDLE;
    };
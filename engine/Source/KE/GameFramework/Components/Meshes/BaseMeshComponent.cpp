#include "KE/GameFramework/Components/Meshes/BaseMeshComponent.h"
#include "KE/GameFramework/Actors/Actor.h"
#include "KE/GameFramework/Meshes/BaseMesh.h"
#include "KE/Vulkan/Managers/BufferManager.h"
#include "KE/Vulkan/Managers/DeviceManager.h"
#include "KE/Vulkan/VertexStructs/AllVertices.h"
#include "KE/Vulkan/Managers/PipelineManager.h"
#include "KE/Systems/RenderSystem.h"
#include "KE/Systems/ResourceSystem.h"
#include "KE/Systems/Managers/MeshManager.h"
#include "KE/Engine.h"

CBaseMeshComponent::CBaseMeshComponent ( CObject * inOwner, const std::string & inDisplayName )
    : Super ( inOwner, inDisplayName )
    {
    LOG_DEBUG ( "BaseMeshComponent created: ", GetName () );
    MeshData = AddSubObject<CBaseMesh> ( GetName () + "_MeshData" );
    auto MeshManager = CEngine::Get ().GetResourceSystem ()->GetMeshManager ();
    MeshData->SetMeshManager ( MeshManager );
    }

CBaseMeshComponent::~CBaseMeshComponent ()
    {
    DestroyRenderResources ();
    }

void CBaseMeshComponent::InitComponent ()
    {
    Super::InitComponent ();

    auto * renderSystem = CEngine::Get ().GetRenderer ();
    if (renderSystem && renderSystem->IsInitialized ())
        {
        LOG_DEBUG ( "[", GetName (), "] InitComponent - will create resources in OnBeginPlay" );
        }
    }

void CBaseMeshComponent::SetMesh ( const std::string & MeshPath )
    {
    if (MeshPath.empty ())
        {
        LOG_WARN ( "[", GetName (), "] Empty mesh path" );
        return;
        }

        // Получаем MeshManager через ResourceSystem
   const auto * resourceSystem = CEngine::Get ().GetResourceSystem ();
    if (!resourceSystem)
        {
        LOG_ERROR ( "[", GetName (), "] ResourceSystem not available!" );
        return;
        }

    const auto& meshManager = resourceSystem->GetManager<MeshManager> ();
    if (!meshManager)
        {
        LOG_ERROR ( "[", GetName (), "] MeshManager not available!" );
        return;
        }

        // Загружаем меш
   const auto mesh = meshManager->LoadMesh ( MeshPath );
    if (mesh)
        {       
        m_bUseMeshData = true;
        SetMesh ( mesh.get () );
        }
    else
        {
        LOG_ERROR ( "[", GetName (), "] Failed to load mesh: ", MeshPath );
        }
    }

void CBaseMeshComponent::SetMesh ( CBaseMesh * InMeshData )
    {
    if (InMeshData == nullptr)
        {
        LOG_WARN ( "[", GetName (), "] Setting null mesh" );
        return;
        }
    if (InMeshData->IsValid ())
        {
        MeshData =AddSubObject<CBaseMesh> ( InMeshData->GetName () );
        MeshData->SetIndices ( InMeshData->GetIndices () );
        MeshData->SetVertices ( InMeshData->GetVertices () );
       // MeshData = ( InMeshData);
        }
        // Если уже есть ресурсы - уничтожаем
    if (m_bRenderResourcesCreated)
        {
        DestroyRenderResources ();
        }

   // MeshData = std::shared_ptr<CBaseMesh> ( InMeshData );
    m_bUseMeshData = true;

    LOG_DEBUG ( "[", GetName (), "] Mesh set: ", InMeshData->GetName (),
                " (", InMeshData->GetVertices ().size (), " vertices, ",
                InMeshData->GetIndices ().size (), " indices)" );

      // Если компонент уже в игре, создаём ресурсы сразу
   
        CreateRenderResources ();
       
    }

void CBaseMeshComponent::Tick ( float DeltaTime )
    {
    Super::Tick ( DeltaTime );
    }

void CBaseMeshComponent::OnEndPlay ()
    {
    Super::OnEndPlay ();
    DestroyRenderResources ();
    }

void CBaseMeshComponent::OnBeginPlay ()
    {
    Super::OnBeginPlay ();

    // Создаём ресурсы здесь, когда всё готово
    if (!m_bRenderResourcesCreated && ( m_bUseMeshData || !m_bUseMeshData ))
        {
        LOG_DEBUG ( "[", GetName (), "] Creating resources in OnBeginPlay" );
        CreateRenderResources ();
        }
    }

void CBaseMeshComponent::UpdateFromMeshData ()
    {
    if (!MeshData || !MeshData->IsValid ())
        {
        return;
        }

    m_VertexCount = static_cast< uint32_t >( MeshData->GetVertices ().size () );
    m_IndexCount = static_cast< uint32_t >( MeshData->GetIndices ().size () );

    LOG_DEBUG ( "[", GetName (), "] Updated from mesh data: ", m_VertexCount,
                " vertices, ", m_IndexCount, " indices" );
    }

FMeshInfo CBaseMeshComponent::GetMeshInfo ()
    {
    FMeshInfo meshInfo;

    if (!IsReadyForRender ())
        {
        return meshInfo;
        }

    if (bIsTransformDirty)
        {
        this->UpdateTransform ();
        }

    meshInfo.VertexBuffer = m_VertexBuffer;
    meshInfo.VertexCount = m_VertexCount;
    meshInfo.IndexBuffer = m_IndexBuffer;
    meshInfo.IndexCount = m_IndexCount;
    meshInfo.Model = GetTransformMatrix ();
    meshInfo.MaterialId = m_MaterialID;
    meshInfo.PipelineName = m_PipelineName;
   

    return meshInfo;
    }

bool CBaseMeshComponent::IsReadyForRender () const
    {
    return m_bVisible &&
        HasRenderResources () &&
        m_bRenderResourcesCreated;
    }

void CBaseMeshComponent::CreateRenderResources ()
    {
    auto * renderSystem = CEngine::Get ().GetRenderer ();
    if (!renderSystem || !renderSystem->IsInitialized ())
        {
        LOG_ERROR ( "[", GetName (), "] RenderSystem not available!" );
        return;
        }

    BufferManager * bufferManager = renderSystem->GetBufferManager ();
    if (!bufferManager)
        {
        LOG_ERROR ( "[", GetName (), "] BufferManager is null!" );
        return;
        }

        // Проверяем, не созданы ли уже ресурсы
    if (m_bRenderResourcesCreated)
        {
        LOG_DEBUG ( "[", GetName (), "] Render resources already created, skipping..." );
        return;
        }

    std::vector<FMeshVertex> vertices;
    // Если есть MeshData - используем его
    if (m_bUseMeshData && MeshData && MeshData->IsValid ())
        {
        vertices = MeshData->GetVertices ();
        m_IndexCount = static_cast< uint32_t >( MeshData->GetIndices ().size () );

        LOG_DEBUG ( "[", GetName (), "] Using mesh data from: ", MeshData->GetName (),
                    " (", vertices.size (), " vertices, ", m_IndexCount, " indices)" );
        }
    else
        {
            // Иначе генерируем через виртуальные методы (для наследников)
        GenerateVertices ( vertices );

        if (!vertices.empty ())
            {
            std::vector<uint32_t> indices;
            GenerateIndices ( indices );
            m_IndexCount = static_cast< uint32_t >( indices.size () );

            LOG_DEBUG ( "[", GetName (), "] Generated ", vertices.size (),
                        " vertices, ", m_IndexCount, " indices" );
            }
        }

    m_VertexCount = static_cast< uint32_t >( vertices.size () );

    if (m_VertexCount == 0)
        {
        LOG_WARN ( "[", GetName (), "] No vertices to render!" );
        return;
        }

        // Создаём вершинный буфер
    FBuffer vertexBuffer = bufferManager->CreateVertexBuffer ( vertices );
    if (!vertexBuffer.IsValid ())
        {
        LOG_ERROR ( "[", GetName (), "] Failed to create vertex buffer!" );
        return;
        }

    m_VertexBuffer = vertexBuffer.Buffer;
    m_VertexMemory = vertexBuffer.Memory;
    m_MappedVertexMemory = vertexBuffer.MappedData;
    m_VertexBufferSize = vertexBuffer.Size;

    LOG_DEBUG ( "[", GetName (), "] Vertex buffer created: ", m_VertexCount,
                " vertices, size: ", m_VertexBufferSize, " bytes" );

      // Создаём индексный буфер (если есть индексы)
    if (m_IndexCount > 0)
        {
        const auto & indices = ( m_bUseMeshData && MeshData ) ?
            MeshData->GetIndices () :
            std::vector<uint32_t> ();

// Если нет индексов из MeshData, генерируем
        std::vector<uint32_t> finalIndices = indices;
        if (finalIndices.empty () && !m_bUseMeshData)
            {
            GenerateIndices ( finalIndices );
            }

        if (!finalIndices.empty ())
            {
            FBuffer indexBuffer = bufferManager->CreateIndexBuffer ( finalIndices );
            if (indexBuffer.IsValid ())
                {
                m_IndexBuffer = indexBuffer.Buffer;
                m_IndexMemory = indexBuffer.Memory;
                m_MappedIndexMemory = indexBuffer.MappedData;
                m_IndexBufferSize = indexBuffer.Size;

                LOG_DEBUG ( "[", GetName (), "] Index buffer created: ", m_IndexCount,
                            " indices, size: ", m_IndexBufferSize, " bytes" );
                }
            else
                {
                LOG_WARN ( "[", GetName (), "] Failed to create index buffer, drawing without indices" );
                m_IndexCount = 0;
                }
            }
        else
            {
            LOG_DEBUG ( "[", GetName (), "] No indices to create, drawing non-indexed" );
            m_IndexCount = 0;
            }
        }

    m_bRenderResourcesCreated = true;
    UpdateTransform ();

    LOG_DEBUG ( "[", GetName (), "] Render resources created successfully" );
    }

void CBaseMeshComponent::DestroyRenderResources ()
    {
    if (!m_bRenderResourcesCreated)
        {
        return;
        }

    auto * renderSystem = CEngine::Get ().GetRenderer ();
    if (renderSystem && renderSystem->IsInitialized ())
        {
        auto device = CEngine::Get ().GetRenderer ()->GetDeviceManager ()->GetDevice ();
        if (device) vkDeviceWaitIdle ( device );
        BufferManager * bufferManager = renderSystem->GetBufferManager ();
        if (bufferManager)
            {
                // Уничтожаем вершинный буфер
            if (m_VertexBuffer != VK_NULL_HANDLE)
                {
                FBuffer buffer;
                buffer.Buffer = m_VertexBuffer;
                buffer.Memory = m_VertexMemory;
                buffer.MappedData = m_MappedVertexMemory;
                buffer.Size = m_VertexBufferSize;

                bufferManager->DestroyBuffer ( buffer );
                LOG_DEBUG ( "[", GetName (), "] Vertex buffer destroyed" );
                }

                // Уничтожаем индексный буфер
            if (m_IndexBuffer != VK_NULL_HANDLE)
                {
                FBuffer buffer;
                buffer.Buffer = m_IndexBuffer;
                buffer.Memory = m_IndexMemory;
                buffer.MappedData = m_MappedIndexMemory;
                buffer.Size = m_IndexBufferSize;

                bufferManager->DestroyBuffer ( buffer );
                LOG_DEBUG ( "[", GetName (), "] Index buffer destroyed" );
                }
            }
        }

        // Обнуляем все указатели
    m_VertexBuffer = VK_NULL_HANDLE;
    m_IndexBuffer = VK_NULL_HANDLE;
    m_VertexMemory = VK_NULL_HANDLE;
    m_IndexMemory = VK_NULL_HANDLE;
    m_MappedVertexMemory = nullptr;
    m_MappedIndexMemory = nullptr;
    m_VertexBufferSize = 0;
    m_IndexBufferSize = 0;
    m_VertexCount = 0;
    m_IndexCount = 0;
    m_bRenderResourcesCreated = false;

    LOG_DEBUG ( "[", GetName (), "] Render resources destroyed" );
    }

void CBaseMeshComponent::GenerateVertices ( std::vector<FMeshVertex> & OutVertices ) const
    {
    OutVertices.clear ();
    // Базовый класс не генерирует вершины
    // Наследники должны переопределить этот метод для процедурной генерации
    }

void CBaseMeshComponent::GenerateIndices ( std::vector<uint32_t> & OutIndices ) const
    {
    OutIndices.clear ();
    // Базовый класс не генерирует индексы
    // Наследники должны переопределить этот метод для процедурной генерации
    }

FVertexInputDescription CBaseMeshComponent::GetVertexInputDescription () const
    {
    FVertexInputDescription desc;

    VkVertexInputBindingDescription binding = FMeshVertex::GetBindingDescription ();
    desc.Bindings.push_back ( binding );

    auto attributes = FMeshVertex::GetAttributeDescriptions ();
    desc.Attributes = attributes;

    return desc;
    }
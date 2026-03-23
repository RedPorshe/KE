#include "KE/GameFramework/Components/Meshes/BaseMeshComponent.h"
#include "KE/GameFramework/Actors/Actor.h"
#include "KE/Vulkan/Managers/BufferManager.h"
#include "KE/Vulkan/VertexStructs/AllVertices.h"
#include "KE/Vulkan/Managers/PipelineManager.h"
#include "KE/Systems/RenderSystem.h"
#include "KE/Engine.h"

CBaseMeshComponent::CBaseMeshComponent ( CObject * inOwner, const std::string & inDisplayName )
	: Super ( inOwner, inDisplayName )
	{
	LOG_DEBUG ( "BaseMeshComponent created: ", GetName () );
	}

CBaseMeshComponent::~CBaseMeshComponent ()
	{
	DestroyRenderResources ();
	}

void CBaseMeshComponent::InitComponent ()
	{
	Super::InitComponent ();

	CEngine::Get ();
	if (CEngine::Get ().GetRenderer ())
		{
		BufferManager * bufferManager = CEngine::Get ().GetRenderer ()->GetBufferManager ();
		if (bufferManager)
			{
				// Убираем создание ресурсов здесь - переносим в OnBeginPlay
			LOG_DEBUG ( "[", GetName (), "] InitComponent - will create resources in OnBeginPlay" );
			}
		}
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
	if (!m_bRenderResourcesCreated)
		{
		LOG_DEBUG ( "[", GetName (), "] Creating resources in OnBeginPlay" );
		CreateRenderResources ();
		}
	}

FMeshInfo CBaseMeshComponent::GetMeshInfo ()
	{
	FMeshInfo meshInfo;

	if (!IsReadyForRender ())
		{
		return meshInfo;
		}
	if (bIsTransformDirty) this->UpdateTransform ();

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
	BufferManager * BufferManager = CEngine::Get ().GetRenderer ()->GetBufferManager ();
	if (!BufferManager)
		{
		LOG_ERROR ( "[", GetName (), "] BufferManager is null!" );
		return;
		}

		// Важно! Проверяем, не созданы ли уже ресурсы
	if (m_bRenderResourcesCreated)
		{
		LOG_DEBUG ( "[", GetName (), "] Render resources already created, skipping..." );
		return;
		}

	std::vector<FMeshVertex> vertices;
	GenerateVertices ( vertices );
	m_VertexCount = static_cast< uint32_t >( vertices.size () );

	if (m_VertexCount == 0)
		{
		LOG_WARN ( "[", GetName (), "] No vertices generated!" );
		return;
		}

		// Создаём вершинный буфер
	FBuffer vertexBuffer = BufferManager->CreateVertexBuffer ( vertices );
	if (vertexBuffer.IsValid ())
		{
		m_VertexBuffer = vertexBuffer.Buffer;
		m_vertexmemory = vertexBuffer.Memory;
		mapedVertexMemory = vertexBuffer.MappedData;
		LOG_DEBUG ( "[", GetName (), "] Vertex buffer created: ", m_VertexCount, " vertices, handle: ", ( void * ) m_VertexBuffer );
		}
	else
		{
		LOG_ERROR ( "[", GetName (), "] Failed to create vertex buffer!" );
		return;
		}

		// Генерируем индексы
	std::vector<uint32_t> indices;
	GenerateIndices ( indices );
	m_IndexCount = static_cast< uint32_t >( indices.size () );

	// Создаём индексный буфер (если есть индексы)
	if (m_IndexCount > 0)
		{
		FBuffer indexBuffer = BufferManager->CreateIndexBuffer ( indices );
		if (indexBuffer.IsValid ())
			{
			m_IndexBuffer = indexBuffer.Buffer;
			m_indexmemory = indexBuffer.Memory;
			mapedIndexMemory = indexBuffer.MappedData;
			LOG_DEBUG ( "[", GetName (), "] Index buffer created: ", m_IndexCount, " indices, handle: ", ( void * ) m_IndexBuffer );
			}
		else
			{
			LOG_ERROR ( "[", GetName (), "] Failed to create index buffer!" );
			}
		}

	m_bRenderResourcesCreated = true;
	UpdateTransform ();
	LOG_DEBUG ( "[", GetName (), "] Render resources created successfully" );
	}

void CBaseMeshComponent::DestroyRenderResources ()
	{
	
	m_VertexBuffer = VK_NULL_HANDLE;
	m_IndexBuffer = VK_NULL_HANDLE;
	m_VertexCount = 0;
	m_IndexCount = 0;
	m_bRenderResourcesCreated = false;
	}

void CBaseMeshComponent::GenerateVertices ( std::vector<FMeshVertex> & OutVertices ) const
	{
	OutVertices.clear ();
	}

void CBaseMeshComponent::GenerateIndices ( std::vector<uint32_t> & OutIndices ) const
	{
	OutIndices.clear ();
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
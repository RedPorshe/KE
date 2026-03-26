#pragma once
#include "KE/GameFramework/Components/Meshes/BaseMeshComponent.h"

struct FMeshVertex;

class KE_API CStaticMeshComponent : public CBaseMeshComponent
	{
	CHUDDO_DECLARE_CLASS ( CStaticMeshComponent, CBaseMeshComponent );
	public:
		CStaticMeshComponent ( CObject * inOwner = nullptr, const std::string & inDisplayName = "StaticMeshComponent" );
		virtual ~CStaticMeshComponent ();
		 // ========== Жизненный цикл ==========
		virtual void InitComponent () override;
		virtual void Tick ( float DeltaTime ) override;
		virtual void OnBeginPlay () override;
		void CreateFallBackCube ( float Size  = 1.0f  );
		void ResizeCube ( float NewSize );
		void CreateBolt ();
	protected:
	// ========== Переопределённые методы из CBaseMeshComponent ==========
		virtual void GenerateVertices ( std::vector<FMeshVertex> & OutVertices ) const override;
		virtual void GenerateIndices ( std::vector<uint32_t> & OutIndices ) const override;
		virtual const std::string & GetPipelineName () const override { return m_PipelineName; }

		std::vector<FMeshVertex> StaticMesh_vertices;
		std::vector<uint32_t> StaticMesh_indices;
	};
REGISTER_CLASS_FACTORY(CStaticMeshComponent)
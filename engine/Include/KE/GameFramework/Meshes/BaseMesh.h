#pragma once
#include "CoreMinimal.h"
#include "Core/Object.h"
#include "KE/Vulkan/VertexStructs/Allvertices.h"

class MeshManager;


class KE_API CBaseMesh : public CObject
	{
	CHUDDO_DECLARE_CLASS ( CBaseMesh, CObject );
	public:
		CBaseMesh (CObject* inOwner = nullptr, const std::string& inDisplayName = "BaseMesh");
		virtual ~CBaseMesh ();
		void CleanUpResources ();
		CBaseMesh * LoadMesh ( const std::string & MeshPath );
		void DestroyMesh ();

		const std::vector<FMeshVertex> & GetVertices () const { return m_Vertices; }
		void SetVertices ( const std::vector<FMeshVertex> & vertices ) { m_Vertices = vertices; }
		void SetIndices ( const std::vector<uint32_t> & indices ) { m_Indices = indices; }
		const std::vector<uint32_t> & GetIndices () const { return m_Indices; }
		bool IsValid () const { return !m_Vertices.empty (); }
		void SetMeshManager ( class MeshManager * Manager ) { m_MeshManager = Manager; }
	private:
		class MeshManager * m_MeshManager = nullptr;

		bool LoadOBJ ( const std::string & Path );
		bool LoadFBX ( const std::string & Path );
		std::vector<FMeshVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
	};
REGISTER_CLASS_FACTORY ( CBaseMesh );
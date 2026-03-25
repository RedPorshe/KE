#include "KE/ResourceManagerInterface.h"

class CBaseMesh;

class KE_API MeshManager : public IResourceManager
	{	
	public:
		MeshManager ();
		virtual ~MeshManager () override;
		bool PreInit () override;
		bool Init ()override;
		void Shutdown () override;
		void Update ( float DeltaTime ) override ;
		const std::string GetSystemName () const override;
		std::shared_ptr<CBaseMesh> LoadMesh ( const std::string & MeshPath );
		
		void UnloadMesh ( const std::string & MeshPath );
	protected:
		void CleanupUnusedResources ()override;
		std::shared_ptr<CBaseMesh> LoadMeshFromFile ( const std::string & Path );
		std::unordered_map<std::string, TWeakPtr<CBaseMesh>> m_Meshes;
		std::mutex m_Mutex;
	};
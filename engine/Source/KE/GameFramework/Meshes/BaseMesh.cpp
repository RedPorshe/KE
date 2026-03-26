#include "KE/GameFramework/Meshes/BaseMesh.h"
#include "KE/Systems/Managers/MeshManager.h"
#include "KE/Engine.h"

CBaseMesh::CBaseMesh ( CObject * inOwner, const std::string & inDisplayName ) :CObject ( inOwner, inDisplayName )
	{
	LOG_DEBUG ( "Mesh Created" );
	}
CBaseMesh::~CBaseMesh ()
	{
	DestroyMesh ();
	}
void CBaseMesh::CleanUpResources ()
	{
    m_Vertices.clear ();
    m_Indices.clear ();
    m_Vertices.shrink_to_fit ();
    m_Indices.shrink_to_fit ();
    LOG_DEBUG ( "Mesh resources cleaned up: ", GetName () );
	}

void CBaseMesh::DestroyMesh ()
	{
	CleanUpResources ();
	}


CBaseMesh * CBaseMesh::LoadMesh ( const std::string & MeshPath )
    {
   

    LOG_DEBUG ( "Loading mesh from: ", MeshPath );

    // Очищаем старые данные
    CleanUpResources ();

    std::string MeshName = MeshPath;
    size_t lastSlash = MeshName.find_last_of ( "/\\" );
    if (lastSlash != std::string::npos)
        {
        MeshName = MeshName.substr ( lastSlash + 1 );
        }
    size_t lastDot = MeshName.find_last_of ( '.' );
    if (lastDot != std::string::npos)
        {
        MeshName = MeshName.substr ( 0, lastDot );
        }

    // Пытаемся загрузить из файла
    bool bLoaded = false;

    if (MeshPath.ends_with ( ".obj" ))
        {
        if (LoadOBJ ( MeshPath ))
            {
            bLoaded = true;
            LOG_DEBUG ( "Successfully loaded OBJ: ", MeshName );
            }
        else
            {
            LOG_WARN ( "Failed to load OBJ, creating fallback cube: ", MeshName );
            }
        }
    else if (MeshPath.ends_with ( ".fbx" ))
        {
        if (LoadFBX ( MeshPath ))
            {
            bLoaded = true;
            LOG_DEBUG ( "Successfully loaded FBX: ", MeshName );
            }
        else
            {
            LOG_WARN ( "Failed to load FBX, creating fallback cube: ", MeshName );
            }
        }

        // Если загрузка не удалась или формат не распознан - создаём куб
    if (!bLoaded)
        {
        LOG_DEBUG ( "Creating fallback cube mesh for: ", MeshName );

        // Вершины куба (24 вершины, 36 индексов)
        float vertices [] = {
            // Front face
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            // Back face
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            // Top face
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            // Bottom face
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            // Right face
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            // Left face
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f
            };

        uint32_t indices [] = {
            // Front
            0, 1, 2, 2, 3, 0,
            // Back
            4, 6, 5, 4, 7, 6,
            // Top
            8, 9, 10, 10, 11, 8,
            // Bottom
            12, 14, 13, 12, 15, 14,
            // Right
            16, 17, 18, 18, 19, 16,
            // Left
            20, 22, 21, 20, 23, 22
            };

            // Конвертируем в FMeshVertex
        m_Vertices.reserve ( 24 );
        for (int i = 0; i < 24; i++)
            {
            FMeshVertex vertex;
            vertex.Position = FVector ( vertices[ i * 8 + 0 ], vertices[ i * 8 + 1 ], vertices[ i * 8 + 2 ] );
            vertex.Normal = FVector ( vertices[ i * 8 + 3 ], vertices[ i * 8 + 4 ], vertices[ i * 8 + 5 ] );
            vertex.UV = FVector2D ( vertices[ i * 8 + 6 ], vertices[ i * 8 + 7 ] );
            m_Vertices.push_back ( vertex );
            }

        m_Indices.assign ( indices, indices + 36 );

        LOG_DEBUG ( "Fallback cube created: ", m_Vertices.size (), " vertices, ", m_Indices.size (), " indices" );
        }

    LOG_DEBUG ( "Mesh loaded: ", GetName (), " (", m_Vertices.size (), " vertices, ", m_Indices.size (), " indices)" );

    return this;
    }
    bool CBaseMesh::IsValid ()const
        {
        if (this == nullptr) return false;
        return !m_Vertices.empty ();
        }
bool CBaseMesh::LoadOBJ ( const std::string & Path )
	{
    LOG_DEBUG ( "Loading OBJ: ", Path );

    std::ifstream file ( Path );
    if (!file.is_open ())
        {
        LOG_ERROR ( "Failed to open OBJ file: ", Path );
        return false;
        }

    std::vector<FVector> positions;
    std::vector<FVector2D> texCoords;
    std::vector<FVector> normals;
    std::vector<uint32_t> positionIndices;
    std::vector<uint32_t> texCoordIndices;
    std::vector<uint32_t> normalIndices;

    std::string line;
    while (std::getline ( file, line ))
        {
        if (line.empty () || line[ 0 ] == '#') continue;

        std::istringstream iss ( line );
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") // Vertex position
            {
            float x, y, z;
            iss >> x >> y >> z;
            positions.push_back ( FVector ( x, y, z ) );
            }
        else if (prefix == "vt") // Texture coordinate
            {
            float u, v;
            iss >> u >> v;
            texCoords.push_back ( FVector2D ( u, v ) );
            }
        else if (prefix == "vn") // Normal
            {
            float x, y, z;
            iss >> x >> y >> z;
            normals.push_back ( FVector ( x, y, z ) );
            }
        else if (prefix == "f") // Face
            {
            std::string vertexStr;
            while (iss >> vertexStr)
                {
                size_t pos1 = vertexStr.find ( '/' );
                size_t pos2 = vertexStr.find ( '/', pos1 + 1 );

                uint32_t posIdx = std::stoi ( vertexStr.substr ( 0, pos1 ) ) - 1;
                positionIndices.push_back ( posIdx );

                if (pos1 != std::string::npos && pos1 + 1 != pos2)
                    {
                    uint32_t texIdx = std::stoi ( vertexStr.substr ( pos1 + 1, pos2 - pos1 - 1 ) ) - 1;
                    texCoordIndices.push_back ( texIdx );
                    }

                if (pos2 != std::string::npos)
                    {
                    uint32_t normIdx = std::stoi ( vertexStr.substr ( pos2 + 1 ) ) - 1;
                    normalIndices.push_back ( normIdx );
                    }
                }
            }
        }

    file.close ();

    if (positionIndices.empty ())
        {
        LOG_ERROR ( "No faces found in OBJ file" );
        return false;
        }

    m_Vertices.clear ();
    m_Indices.clear ();

    for (size_t i = 0; i < positionIndices.size (); i++)
        {
        FMeshVertex vertex;
        vertex.Position = positions[ positionIndices[ i ] ];

        if (!normals.empty () && i < normalIndices.size ())
            vertex.Normal = normals[ normalIndices[ i ] ];
        else
            vertex.Normal = FVector ( 0, 0, 1 );

        if (!texCoords.empty () && i < texCoordIndices.size ())
            vertex.UV = texCoords[ texCoordIndices[ i ] ];
        else
            vertex.UV = FVector2D ( 0, 0 );

        m_Vertices.push_back ( vertex );
        m_Indices.push_back ( static_cast< uint32_t > ( i ) );
        }

    LOG_DEBUG ( "OBJ loaded: ", m_Vertices.size (), " vertices, ", m_Indices.size (), " indices" );
    return true;
	}

bool CBaseMesh::LoadFBX ( const std::string & Path )
	{
		// Реализация парсера FBX
	LOG_DEBUG ( "Loading FBX: ", Path );
	return false;
	}
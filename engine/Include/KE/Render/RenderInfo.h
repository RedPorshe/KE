#pragma once

#include "CoreMinimal.h"
#include "Utils/Math/MathTypes.h"
#include "Core/Collision.h"
#include "Render/Vulkan/VertexStructs/WireframeVertex.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace CollisionDebugColors
    {
    const FVector Default = FVector ( 0.0f, 1.0f, 0.0f );   // Зеленый
    const FVector Block = FVector ( 1.0f, 0.0f, 0.0f );     // Красный
    const FVector Overlap = FVector ( 0.0f, 0.0f, 1.0f );   // Синий
    const FVector Trigger = FVector ( 1.0f, 1.0f, 0.0f );   // Желтый
    const FVector Terrain = FVector ( 0.5f, 0.5f, 0.5f );   // Серый
    const FVector Character = FVector ( 1.0f, 0.5f, 0.0f ); // Оранжевый
    const FVector Pawn = FVector ( 1.0f, 0.0f, 1.0f );      // Розовый
    };

struct KE_API  FCameraInfo
    {
        // ... содержимое как в вашем файле ...
    FMat4 ViewMatrix { 1.f };
    FMat4 ProjectionMatrix { 1.f };
    FVector Location { FVector::Zero () };
    FVector ViewTarget { FVector::Zero () };
    float NearPlane { 0.1f };
    float FarPlane { 1000.f };
    float FOV { 90.f };

    FCameraInfo () = default;
    FCameraInfo ( const FVector & inLocation, const FVector & inViewTarget,
                  const FMat4 & inViewMatrix = FMat4 ( 1.f ),
                  const FMat4 & inProjectionMatrix = FMat4 ( 1.f ),
                  const float & inNearPlane = 0.1f,
                  const float & inFarPlane = 1000.f,
                  const float & inFOVAngles = 90.f );

    void Clear ();
    void UpdateViewMatrix ();
    void UpdateProjectionMatrix ( float AspectRatio );

    FMat4 GetViewMatrix () const { return ViewMatrix; }
    FMat4 GetProjectionMatrix () const { return ProjectionMatrix; }
    FVector GetLocation () const { return Location; }
    FVector GetViewTarget () const { return ViewTarget; }

    void SetLocation ( const FVector & InLocation ) { Location = InLocation; }
    void SetViewTarget ( const FVector & InViewTarget ) { ViewTarget = InViewTarget; }
    void SetFOV ( float InFOV ) { FOV = InFOV; }
    void SetNearPlane ( float InNearPlane ) { NearPlane = InNearPlane; }
    void SetFarPlane ( float InFarPlane ) { FarPlane = InFarPlane; }
    };

struct KE_API FMeshInfo
    {
    FMat4 Model = FMat4 ( 1.f );
    VkBuffer VertexBuffer = VK_NULL_HANDLE;
    VkBuffer IndexBuffer = VK_NULL_HANDLE;
    uint32_t VertexCount = 0;
    uint32_t IndexCount = 0;
    uint32_t MaterialId = 0;
    std::string PipelineName = "StaticMesh";

    FMeshInfo () = default;
    FMeshInfo ( VkBuffer InVertexBuffer, uint32_t InVertexCount,
                VkBuffer InIndexBuffer = VK_NULL_HANDLE, uint32_t InIndexCount = 0,
                const FMat4 & InModel = FMat4 ( 1.f ),
                const std::string & InPipelineName = "StaticMesh" );

    void Clear ();
    bool IsValid () const;
    };

struct KE_API FTerrainRenderInfo
    {
    VkBuffer VertexBuffer = VK_NULL_HANDLE;
    VkBuffer IndexBuffer = VK_NULL_HANDLE;
    uint32_t VertexCount = 0;
    uint32_t IndexCount = 0;
    FMat4 Model = FMat4 ( 1.f );
    int32 Width = 0;
    int32 Height = 0;
    float CellSize = 100.0f;
    float MinHeight = 0.0f;
    float MaxHeight = 0.0f;

    struct KE_API FTerrainParams
        {
        float TilingFactor = 1.0f;
        float HeightScale = 1.0f;
        float FogDensity = 0.001f;
        float UseTexture = 0.0f;
        float SandHeight = 0.0f;
        float GrassHeight = 30.0f;
        float RockHeight = 60.0f;
        float SnowHeight = 90.0f;
        } Params;

    std::string PipelineName = "TerrainPipeline";

    FTerrainRenderInfo () = default;
    bool IsValid () const;
    void Clear ();
    };

struct KE_API FTerrainDebugInfo
    {
    std::vector<FWireframeVertex> WireframeVertices;
    FVector DebugColor = CollisionDebugColors::Terrain;
    int32 Width = 0;
    int32 Height = 0;
    float CellSize = 0.0f;

    FTerrainDebugInfo () = default;
    FTerrainDebugInfo ( const FTerrainDebugInfo & Other );
    FTerrainDebugInfo & operator=( const FTerrainDebugInfo & Other );
    ~FTerrainDebugInfo () = default;

    bool IsValid () const;
    void Clear ();
    size_t GetLineCount () const;
    };

struct KE_API FCollisionDebugInfo
    {
    ECollisionShape ShapeType = ECollisionShape::NONE;
    FVector WorldLocation = FVector::Zero ();
    FQuat WorldRotation = FQuat::Identity ();
    FVector WorldScale = FVector::One ();

    union ParamsUnion
        {
        ParamsUnion () {}
        ~ParamsUnion () {}

        struct SphereParams { float Radius; } Sphere;
        struct BoxParams { FVector HalfExtents; } Box;
        struct CapsuleParams { float Radius; float HalfHeight; } Capsule;
        struct CylinderParams { float Radius; float Height; } Cylinder;
        struct ConeParams { float Radius; float Height; } Cone;
        } Params;

    FVector DebugColor = FVector ( 0.0f, 1.0f, 0.0f );

    FCollisionDebugInfo ();
    FCollisionDebugInfo ( const FCollisionDebugInfo & Other );
    FCollisionDebugInfo & operator=( const FCollisionDebugInfo & Other );
    ~FCollisionDebugInfo () = default;

    static FCollisionDebugInfo CreateSphere ( const FVector & Location, float Radius,
                                              const FVector & Color = FVector ( 0.0f, 1.0f, 0.0f ) );
    static FCollisionDebugInfo CreateBox ( const FVector & Location, const FQuat & Rotation,
                                           const FVector & HalfExtents, const FVector & Color = FVector ( 0.0f, 1.0f, 0.0f ) );
    static FCollisionDebugInfo CreateCapsule ( const FVector & Location, const FQuat & Rotation,
                                               float Radius, float HalfHeight, const FVector & Color = FVector ( 0.0f, 1.0f, 0.0f ) );
    static FCollisionDebugInfo CreateCylinder ( const FVector & Location, const FQuat & Rotation,
                                                float Radius, float Height, const FVector & Color = FVector ( 0.0f, 1.0f, 0.0f ) );
    static FCollisionDebugInfo CreateCone ( const FVector & Location, const FQuat & Rotation,
                                            float Radius, float Height, const FVector & Color = FVector ( 0.0f, 1.0f, 0.0f ) );

    bool IsValid () const;
    };

struct KE_API FRenderInfo
    {
    bool HasInfo = false;
    FCameraInfo Camera;
    std::vector<FMeshInfo> RenderMeshes;
    std::vector<FTerrainRenderInfo> Terrains;
    std::vector<FCollisionDebugInfo> DebugCollisions;
    std::vector<FTerrainDebugInfo> TerrainWireframes;
    bool bDrawCollisions = false;

    FRenderInfo () = default;
    FRenderInfo ( const FRenderInfo & ) = default;
    FRenderInfo & operator=( const FRenderInfo & ) = default;
    ~FRenderInfo () = default;

    void Clear ();
    bool HasDebugCollisions () const { return !DebugCollisions.empty () || !TerrainWireframes.empty (); }
    bool IsEmpty () const;
    size_t GetMeshCount () const { return RenderMeshes.size (); }
    size_t GetTerrainCount () const { return Terrains.size (); }
    size_t GetDebugCollisionCount () const { return DebugCollisions.size (); }
    size_t GetTerrainWireframeCount () const { return TerrainWireframes.size (); }

    void Reserve ( size_t MeshCount, size_t TerrainCount = 0, size_t DebugCount = 0, size_t TerrainWireframeCount = 0 );
    void AddMesh ( const FMeshInfo & Mesh ) { RenderMeshes.push_back ( Mesh ); }
    void AddTerrain ( const FTerrainRenderInfo & Terrain ) { Terrains.push_back ( Terrain ); }
    void AddDebugCollision ( const FCollisionDebugInfo & Collision ) { DebugCollisions.push_back ( Collision ); }
    void AddTerrainWireframe ( const FTerrainDebugInfo & Wireframe ) { TerrainWireframes.push_back ( Wireframe ); }
    bool IsValid () const;
    };

    // Вспомогательные функции
KE_API FCameraInfo CreatePerspectiveCamera ( const FVector & Position, const FVector & Target,
                                      float FOVDegrees, float AspectRatio,
                                      float Near = 0.1f, float Far = 1000.f );

KE_API FCameraInfo CreateOrthographicCamera ( const FVector & Position, const FVector & Target,
                                       float Left, float Right, float Bottom, float Top,
                                       float Near = 0.1f, float Far = 1000.f );
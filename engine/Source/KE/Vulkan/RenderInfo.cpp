#include "KE/Vulkan/RenderInfo.h"
#include "Utils/Math/CE_MathConstants.h"
#include "Utils/Math/CE_MathHelpers.h"

//=============================================================================
// FCameraInfo Implementation
//=============================================================================

FCameraInfo::FCameraInfo ( const FVector & inLocation, const FVector & inViewTarget,
                           const FMat4 & inViewMatrix, const FMat4 & inProjectionMatrix,
                           const float & inNearPlane, const float & inFarPlane,
                           const float & inFOVAngles )
    : ViewMatrix ( inViewMatrix )
    , ProjectionMatrix ( inProjectionMatrix )
    , Location ( inLocation )
    , ViewTarget ( inViewTarget )
    , NearPlane ( inNearPlane )
    , FarPlane ( inFarPlane )
    , FOV ( inFOVAngles )
    {}

void FCameraInfo::Clear ()
    {
    ViewMatrix = FMat4::IdentityMatrix ();
    ProjectionMatrix = FMat4::IdentityMatrix ();
    Location = FVector::Zero ();
    ViewTarget = FVector::Zero ();
    NearPlane = 0.1f;
    FarPlane = 1000.f;
    FOV = 90.f;
    }

void FCameraInfo::UpdateViewMatrix ()
    {
    ViewMatrix = FMat4::LookAtMatrix ( Location, ViewTarget, FVector::Up () );   
    }

void FCameraInfo::UpdateProjectionMatrix ( float AspectRatio )
    {
    if (AspectRatio <= 0.0f)
        AspectRatio = 16.0f / 9.0f;

    float RadFOV = FOV * CEMath::DEG_TO_RAD;
    float tanHalfFOV = CEMath::Tan ( RadFOV * 0.5f );
    float range = NearPlane - FarPlane;

    ProjectionMatrix = FMat4 (
        1.0f / ( AspectRatio * tanHalfFOV ), 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f / tanHalfFOV, 0.0f, 0.0f,
        0.0f, 0.0f, ( FarPlane + NearPlane ) / range, -1.0f,
        0.0f, 0.0f, ( 2.0f * FarPlane * NearPlane ) / range, 0.0f
    );
    }

    //=============================================================================
    // FMeshInfo Implementation
    //=============================================================================

FMeshInfo::FMeshInfo ( VkBuffer InVertexBuffer, uint32_t InVertexCount,
                       VkBuffer InIndexBuffer, uint32_t InIndexCount,
                       const FMat4 & InModel, const std::string & InPipelineName )
    : Model ( InModel )
    , VertexBuffer ( InVertexBuffer )
    , IndexBuffer ( InIndexBuffer )
    , VertexCount ( InVertexCount )
    , IndexCount ( InIndexCount )
    , MaterialId ( 0 )
    , PipelineName ( InPipelineName )
    {}

void FMeshInfo::Clear ()
    {
    Model = FMat4 ( 1.f );
    VertexBuffer = VK_NULL_HANDLE;
    IndexBuffer = VK_NULL_HANDLE;
    VertexCount = 0;
    IndexCount = 0;
    MaterialId = 0;
    PipelineName.clear ();
    }

bool FMeshInfo::IsValid () const
    {
    return VertexBuffer != VK_NULL_HANDLE && VertexCount > 0;
    }

    //=============================================================================
    // FTerrainRenderInfo Implementation
    //=============================================================================

bool FTerrainRenderInfo::IsValid () const
    {
    return VertexBuffer != VK_NULL_HANDLE && VertexCount > 0 && Width > 0 && Height > 0;
    }

void FTerrainRenderInfo::Clear ()
    {
    VertexBuffer = VK_NULL_HANDLE;
    IndexBuffer = VK_NULL_HANDLE;
    VertexCount = 0;
    IndexCount = 0;
    Model = FMat4 ( 1.f );
    Width = 0;
    Height = 0;
    CellSize = 100.0f;
    MinHeight = 0.0f;
    MaxHeight = 0.0f;
    Params = FTerrainParams ();
    }

    //=============================================================================
    // FTerrainDebugInfo Implementation
    //=============================================================================

FTerrainDebugInfo::FTerrainDebugInfo ( const FTerrainDebugInfo & Other )
    : WireframeVertices ( Other.WireframeVertices )
    , DebugColor ( Other.DebugColor )
    , Width ( Other.Width )
    , Height ( Other.Height )
    , CellSize ( Other.CellSize )
    {}

FTerrainDebugInfo & FTerrainDebugInfo::operator=( const FTerrainDebugInfo & Other )
    {
    if (this != &Other)
        {
        WireframeVertices = Other.WireframeVertices;
        DebugColor = Other.DebugColor;
        Width = Other.Width;
        Height = Other.Height;
        CellSize = Other.CellSize;
        }
    return *this;
    }

bool FTerrainDebugInfo::IsValid () const
    {
    return !WireframeVertices.empty ();
    }

void FTerrainDebugInfo::Clear ()
    {
    WireframeVertices.clear ();
    Width = 0;
    Height = 0;
    CellSize = 0.0f;
    }

size_t FTerrainDebugInfo::GetLineCount () const
    {
    return WireframeVertices.size () / 2;
    }

    //=============================================================================
    // FCollisionDebugInfo Implementation
    //=============================================================================

FCollisionDebugInfo::FCollisionDebugInfo ()
    : ShapeType ( ECollisionShape::NONE )
    {}

FCollisionDebugInfo::FCollisionDebugInfo ( const FCollisionDebugInfo & Other )
    : ShapeType ( Other.ShapeType )
    , WorldLocation ( Other.WorldLocation )
    , WorldRotation ( Other.WorldRotation )
    , DebugColor ( Other.DebugColor )
    {
    switch (ShapeType)
        {
            case ECollisionShape::SPHERE:
                Params.Sphere = Other.Params.Sphere;
                break;
            case ECollisionShape::BOX:
                Params.Box = Other.Params.Box;
                break;
            case ECollisionShape::CAPSULE:
                Params.Capsule = Other.Params.Capsule;
                break;
            case ECollisionShape::CYLINDER:
                Params.Cylinder = Other.Params.Cylinder;
                break;
            case ECollisionShape::CONE:
                Params.Cone = Other.Params.Cone;
                break;
            default:
                break;
        }
    }

FCollisionDebugInfo & FCollisionDebugInfo::operator=( const FCollisionDebugInfo & Other )
    {
    if (this != &Other)
        {
        ShapeType = Other.ShapeType;
        WorldLocation = Other.WorldLocation;
        WorldRotation = Other.WorldRotation;
        DebugColor = Other.DebugColor;

        switch (ShapeType)
            {
                case ECollisionShape::SPHERE:
                    Params.Sphere = Other.Params.Sphere;
                    break;
                case ECollisionShape::BOX:
                    Params.Box = Other.Params.Box;
                    break;
                case ECollisionShape::CAPSULE:
                    Params.Capsule = Other.Params.Capsule;
                    break;
                case ECollisionShape::CYLINDER:
                    Params.Cylinder = Other.Params.Cylinder;
                    break;
                case ECollisionShape::CONE:
                    Params.Cone = Other.Params.Cone;
                    break;
                default:
                    break;
            }
        }
    return *this;
    }

bool FCollisionDebugInfo::IsValid () const
    {
    return ShapeType != ECollisionShape::NONE && ShapeType != ECollisionShape::MAX;
    }

FCollisionDebugInfo FCollisionDebugInfo::CreateSphere ( const FVector & Location, float Radius, const FVector & Color )
    {
    FCollisionDebugInfo Info;
    Info.ShapeType = ECollisionShape::SPHERE;
    Info.WorldLocation = Location;
    Info.WorldRotation = FQuat::Identity ();
    Info.Params.Sphere.Radius = Radius;
    Info.DebugColor = Color;
    return Info;
    }

FCollisionDebugInfo FCollisionDebugInfo::CreateBox ( const FVector & Location, const FQuat & Rotation,
                                                     const FVector & HalfExtents, const FVector & Color )
    {
    FCollisionDebugInfo Info;
    Info.ShapeType = ECollisionShape::BOX;
    Info.WorldLocation = Location;
    Info.WorldRotation = Rotation;
    Info.Params.Box.HalfExtents = HalfExtents;
    Info.DebugColor = Color;
    return Info;
    }

FCollisionDebugInfo FCollisionDebugInfo::CreateCapsule ( const FVector & Location, const FQuat & Rotation,
                                                         float Radius, float HalfHeight, const FVector & Color )
    {
    FCollisionDebugInfo Info;
    Info.ShapeType = ECollisionShape::CAPSULE;
    Info.WorldLocation = Location;
    Info.WorldRotation = Rotation;
    Info.Params.Capsule.Radius = Radius;
    Info.Params.Capsule.HalfHeight = HalfHeight;
    Info.DebugColor = Color;
    return Info;
    }

FCollisionDebugInfo FCollisionDebugInfo::CreateCylinder ( const FVector & Location, const FQuat & Rotation,
                                                          float Radius, float Height, const FVector & Color )
    {
    FCollisionDebugInfo Info;
    Info.ShapeType = ECollisionShape::CYLINDER;
    Info.WorldLocation = Location;
    Info.WorldRotation = Rotation;
    Info.Params.Cylinder.Radius = Radius;
    Info.Params.Cylinder.Height = Height;
    Info.DebugColor = Color;
    return Info;
    }

FCollisionDebugInfo FCollisionDebugInfo::CreateCone ( const FVector & Location, const FQuat & Rotation,
                                                      float Radius, float Height, const FVector & Color )
    {
    FCollisionDebugInfo Info;
    Info.ShapeType = ECollisionShape::CONE;
    Info.WorldLocation = Location;
    Info.WorldRotation = Rotation;
    Info.Params.Cone.Radius = Radius;
    Info.Params.Cone.Height = Height;
    Info.DebugColor = Color;
    return Info;
    }

    //=============================================================================
    // FRenderInfo Implementation
    //=============================================================================

void FRenderInfo::Clear ()
    {
    HasInfo = false;
    Camera.Clear ();
    RenderMeshes.clear ();
    Terrains.clear ();
    DebugCollisions.clear ();
    TerrainWireframes.clear ();
    bDrawCollisions = false;
    }

bool FRenderInfo::IsEmpty () const
    {
    return RenderMeshes.empty () && Terrains.empty () &&
        DebugCollisions.empty () && TerrainWireframes.empty ();
    }

void FRenderInfo::Reserve ( size_t MeshCount, size_t TerrainCount,
                            size_t DebugCount, size_t TerrainWireframeCount )
    {
    RenderMeshes.reserve ( MeshCount );
    Terrains.reserve ( TerrainCount );
    DebugCollisions.reserve ( DebugCount );
    TerrainWireframes.reserve ( TerrainWireframeCount );
    }

bool FRenderInfo::IsValid () const
    {
    return HasInfo && ( !RenderMeshes.empty () || !Terrains.empty () ||
                        ( bDrawCollisions && HasDebugCollisions () ) );
    }

    //=============================================================================
    // Camera Creation Functions
    //=============================================================================

FCameraInfo CreatePerspectiveCamera ( const FVector & Position, const FVector & Target,
                                      float FOVDegrees, float AspectRatio,
                                      float Near, float Far )
    {
    FCameraInfo Camera;
    Camera.SetLocation ( Position );
    Camera.SetViewTarget ( Target );
    Camera.SetFOV ( FOVDegrees );
    Camera.SetNearPlane ( Near );
    Camera.SetFarPlane ( Far );

    Camera.UpdateViewMatrix ();
    Camera.UpdateProjectionMatrix ( AspectRatio );

    return Camera;
    }

FCameraInfo CreateOrthographicCamera ( const FVector & Position, const FVector & Target,
                                       float Left, float Right, float Bottom, float Top,
                                       float Near, float Far )
    {
    FCameraInfo Camera;
    Camera.SetLocation ( Position );
    Camera.SetViewTarget ( Target );
    Camera.SetNearPlane ( Near );
    Camera.SetFarPlane ( Far );

    Camera.UpdateViewMatrix ();

    float Width = Right - Left;
    float Height = Top - Bottom;
    float Depth = Far - Near;

    Camera.ProjectionMatrix = FMat4 (
        2.0f / Width, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / Height, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f / Depth, 0.0f,
        -( Right + Left ) / Width, -( Top + Bottom ) / Height, -Near / Depth, 1.0f
    );

    return Camera;
    }
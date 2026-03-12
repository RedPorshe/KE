#include "Vulkan/Managers/WireframePipeline.h"
#include "RenderInfo.h"
#include "Vulkan/VertexStructs/AllVertices.h"

void CWireframeGenerator::GenerateSphere ( std::vector<FWireframeVertex> & OutVertices, const FVector & Center, float Radius, const FVector & Color, int Segments )

    {
    OutVertices.clear ();
    OutVertices.reserve ( Segments * 6 * 3 ); // 3 круга (XY, XZ, YZ) * 2 вершины на сегмент

    // Круг в плоскости XY
    for (int i = 0; i < Segments; i++)
        {
        float angle1 = ( float ) i / Segments * 2.0f * 3.14159f;
        float angle2 = ( float ) ( i + 1 ) / Segments * 2.0f * 3.14159f;

        FVector p1 ( Center.x + Radius * cos ( angle1 ), Center.y + Radius * sin ( angle1 ), Center.z );
        FVector p2 ( Center.x + Radius * cos ( angle2 ), Center.y + Radius * sin ( angle2 ), Center.z );

        OutVertices.push_back ( { p1, Color } );
        OutVertices.push_back ( { p2, Color } );
        }

        // Круг в плоскости XZ
    for (int i = 0; i < Segments; i++)
        {
        float angle1 = ( float ) i / Segments * 2.0f * 3.14159f;
        float angle2 = ( float ) ( i + 1 ) / Segments * 2.0f * 3.14159f;

        FVector p1 ( Center.x + Radius * cos ( angle1 ), Center.y, Center.z + Radius * sin ( angle1 ) );
        FVector p2 ( Center.x + Radius * cos ( angle2 ), Center.y, Center.z + Radius * sin ( angle2 ) );

        OutVertices.push_back ( { p1, Color } );
        OutVertices.push_back ( { p2, Color } );
        }

        // Круг в плоскости YZ
    for (int i = 0; i < Segments; i++)
        {
        float angle1 = ( float ) i / Segments * 2.0f * 3.14159f;
        float angle2 = ( float ) ( i + 1 ) / Segments * 2.0f * 3.14159f;

        FVector p1 ( Center.x, Center.y + Radius * cos ( angle1 ), Center.z + Radius * sin ( angle1 ) );
        FVector p2 ( Center.x, Center.y + Radius * cos ( angle2 ), Center.z + Radius * sin ( angle2 ) );

        OutVertices.push_back ( { p1, Color } );
        OutVertices.push_back ( { p2, Color } );
        }
    }

void CWireframeGenerator::GenerateBox ( std::vector<FWireframeVertex> & OutVertices, const FVector & Center, const FQuat & Rotation, const FVector & HalfExtents, const FVector & Color )

    {
    OutVertices.clear ();
    OutVertices.reserve ( 24 ); // 12 рёбер * 2 вершины

    // 8 углов бокса в локальных координатах
    std::array<FVector, 8> corners = {
        FVector ( -HalfExtents.x, -HalfExtents.y, -HalfExtents.z ),
        FVector ( HalfExtents.x, -HalfExtents.y, -HalfExtents.z ),
        FVector ( HalfExtents.x,  HalfExtents.y, -HalfExtents.z ),
        FVector ( -HalfExtents.x,  HalfExtents.y, -HalfExtents.z ),
        FVector ( -HalfExtents.x, -HalfExtents.y,  HalfExtents.z ),
        FVector ( HalfExtents.x, -HalfExtents.y,  HalfExtents.z ),
        FVector ( HalfExtents.x,  HalfExtents.y,  HalfExtents.z ),
        FVector ( -HalfExtents.x,  HalfExtents.y,  HalfExtents.z )
        };

        // Применяем поворот и смещение
    for (auto & corner : corners)
        {
        corner = Center + Rotation * corner;
        }

        // 12 рёбер бокса
    const int edges[ 12 ][ 2 ] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Нижняя грань
        {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Верхняя грань
        {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Вертикальные рёбра
        };

    for (int i = 0; i < 12; i++)
        {
        OutVertices.push_back ( { corners[ edges[ i ][ 0 ] ], Color } );
        OutVertices.push_back ( { corners[ edges[ i ][ 1 ] ], Color } );
        }
    }

void CWireframeGenerator::GenerateCapsule ( std::vector<FWireframeVertex> & OutVertices, const FVector & Center, const FQuat & Rotation, float Radius, float HalfHeight, const FVector & Color, int Segments )

    {
    OutVertices.clear ();

    // Центры полусфер
    FVector topCenter = Center + Rotation * FVector ( 0, 0, HalfHeight );
    FVector bottomCenter = Center + Rotation * FVector ( 0, 0, -HalfHeight );

    // Цилиндрическая часть - вертикальные линии
    int vertSegments = 8;
    for (int i = 0; i < vertSegments; i++)
        {
        float angle = ( float ) i / vertSegments * 2.0f * 3.14159f;
        float dx = Radius * cos ( angle );
        float dy = Radius * sin ( angle );

        FVector dir = Rotation * FVector ( dx, dy, 0 );

        FVector p1 = Center + dir + Rotation * FVector ( 0, 0, HalfHeight );
        FVector p2 = Center + dir + Rotation * FVector ( 0, 0, -HalfHeight );

        OutVertices.push_back ( { p1, Color } );
        OutVertices.push_back ( { p2, Color } );
        }

        // Горизонтальные круги на полусферах
    auto generateHalfCircle = [ & ] ( const FVector & sphereCenter, float sign )
        {
        for (int i = 0; i < Segments; i++)
            {
            float angle1 = ( float ) i / Segments * 3.14159f;
            float angle2 = ( float ) ( i + 1 ) / Segments * 3.14159f;

            for (int j = 0; j < 8; j++)
                {
                float ringAngle1 = ( float ) j / 8 * 2.0f * 3.14159f;
                float ringAngle2 = ( float ) ( j + 1 ) / 8 * 2.0f * 3.14159f;

                // Первая точка
                float x1 = Radius * sin ( angle1 ) * cos ( ringAngle1 );
                float y1 = Radius * sin ( angle1 ) * sin ( ringAngle1 );
                float z1 = Radius * cos ( angle1 ) * sign;

                // Вторая точка
                float x2 = Radius * sin ( angle1 ) * cos ( ringAngle2 );
                float y2 = Radius * sin ( angle1 ) * sin ( ringAngle2 );
                float z2 = Radius * cos ( angle1 ) * sign;

                FVector p1 = sphereCenter + Rotation * FVector ( x1, y1, z1 );
                FVector p2 = sphereCenter + Rotation * FVector ( x2, y2, z2 );

                OutVertices.push_back ( { p1, Color } );
                OutVertices.push_back ( { p2, Color } );
                }
            }
        };

    generateHalfCircle ( topCenter, 1.0f );
    generateHalfCircle ( bottomCenter, -1.0f );
    }

void CWireframeGenerator::GenerateCylinder ( std::vector<FWireframeVertex> & OutVertices, const FVector & Center, const FQuat & Rotation, float Radius, float Height, const FVector & Color, int Segments )

    {
    OutVertices.clear ();
    OutVertices.reserve ( Segments * 6 );

    float halfHeight = Height * 0.5f;

    FVector topCenter = Center + Rotation * FVector ( 0, 0, halfHeight );
    FVector bottomCenter = Center + Rotation * FVector ( 0, 0, -halfHeight );

    // Вертикальные линии
    for (int i = 0; i < Segments; i++)
        {
        float angle = ( float ) i / Segments * 2.0f * 3.14159f;
        float dx = Radius * cos ( angle );
        float dy = Radius * sin ( angle );

        FVector dir = Rotation * FVector ( dx, dy, 0 );

        FVector p1 = topCenter + dir;
        FVector p2 = bottomCenter + dir;

        OutVertices.push_back ( { p1, Color } );
        OutVertices.push_back ( { p2, Color } );
        }

        // Верхний и нижний круги
    for (int i = 0; i < Segments; i++)
        {
        float angle1 = ( float ) i / Segments * 2.0f * 3.14159f;
        float angle2 = ( float ) ( i + 1 ) / Segments * 2.0f * 3.14159f;

        float dx1 = Radius * cos ( angle1 );
        float dy1 = Radius * sin ( angle1 );
        float dx2 = Radius * cos ( angle2 );
        float dy2 = Radius * sin ( angle2 );

        FVector dir1 = Rotation * FVector ( dx1, dy1, 0 );
        FVector dir2 = Rotation * FVector ( dx2, dy2, 0 );

        // Верхний круг
        OutVertices.push_back ( { topCenter + dir1, Color } );
        OutVertices.push_back ( { topCenter + dir2, Color } );

        // Нижний круг
        OutVertices.push_back ( { bottomCenter + dir1, Color } );
        OutVertices.push_back ( { bottomCenter + dir2, Color } );
        }
    }

void CWireframeGenerator::GenerateCone ( std::vector<FWireframeVertex> & OutVertices, const FVector & Center, const FQuat & Rotation, float Radius, float Height, const FVector & Color, int Segments )

    {
    OutVertices.clear ();

    float halfHeight = Height * 0.5f;

    FVector tip = Center + Rotation * FVector ( 0, 0, halfHeight );
    FVector baseCenter = Center + Rotation * FVector ( 0, 0, -halfHeight );

    // Линии от вершины к основанию
    for (int i = 0; i < Segments; i++)
        {
        float angle = ( float ) i / Segments * 2.0f * 3.14159f;
        float dx = Radius * cos ( angle );
        float dy = Radius * sin ( angle );

        FVector dir = Rotation * FVector ( dx, dy, 0 );
        FVector basePoint = baseCenter + dir;

        OutVertices.push_back ( { tip, Color } );
        OutVertices.push_back ( { basePoint, Color } );
        }

        // Круг основания
    for (int i = 0; i < Segments; i++)
        {
        float angle1 = ( float ) i / Segments * 2.0f * 3.14159f;
        float angle2 = ( float ) ( i + 1 ) / Segments * 2.0f * 3.14159f;

        float dx1 = Radius * cos ( angle1 );
        float dy1 = Radius * sin ( angle1 );
        float dx2 = Radius * cos ( angle2 );
        float dy2 = Radius * sin ( angle2 );

        FVector dir1 = Rotation * FVector ( dx1, dy1, 0 );
        FVector dir2 = Rotation * FVector ( dx2, dy2, 0 );

        OutVertices.push_back ( { baseCenter + dir1, Color } );
        OutVertices.push_back ( { baseCenter + dir2, Color } );
        }
    }

void CWireframeGenerator::GenerateTerrainWireframe ( std::vector<FWireframeVertex> & OutVertices, const CTerrainComponent * TerrainComp, const FVector & Color, bool bDrawDiagonals )

    {
    OutVertices.clear ();

    if (!TerrainComp) return;

    const FTerrainData & data = TerrainComp->GetTerrainData ();

    if (data.Width < 2 || data.Height < 2 || data.Heights.empty ())
        return;

    // Количество рёбер:
    // - горизонтальные: (Width-1) * Height
    // - вертикальные: Width * (Height-1)
    // - диагонали (опционально): (Width-1) * (Height-1) * 2
    size_t edgeCount = ( data.Width - 1 ) * data.Height + data.Width * ( data.Height - 1 );
    if (bDrawDiagonals)
        edgeCount += ( data.Width - 1 ) * ( data.Height - 1 ) * 2;

    OutVertices.reserve ( edgeCount * 2 ); // 2 вершины на ребро

    // 1. Горизонтальные линии (вдоль оси X)
    for (int32 z = 0; z < data.Height; ++z)
        {
        for (int32 x = 0; x < data.Width - 1; ++x)
            {
            FVector p1 = TerrainComp->GetWorldPositionAt ( x, z );
            FVector p2 = TerrainComp->GetWorldPositionAt ( x + 1, z );

            OutVertices.push_back ( { p1, Color } );
            OutVertices.push_back ( { p2, Color } );
            }
        }

        // 2. Вертикальные линии (вдоль оси Z)
    for (int32 z = 0; z < data.Height - 1; ++z)
        {
        for (int32 x = 0; x < data.Width; ++x)
            {
            FVector p1 = TerrainComp->GetWorldPositionAt ( x, z );
            FVector p2 = TerrainComp->GetWorldPositionAt ( x, z + 1 );

            OutVertices.push_back ( { p1, Color } );
            OutVertices.push_back ( { p2, Color } );
            }
        }

        // 3. Опционально: диагонали (для более плотной сетки)
    if (bDrawDiagonals)
        {
        for (int32 z = 0; z < data.Height - 1; ++z)
            {
            for (int32 x = 0; x < data.Width - 1; ++x)
                {
                FVector p00 = TerrainComp->GetWorldPositionAt ( x, z );
                FVector p11 = TerrainComp->GetWorldPositionAt ( x + 1, z + 1 );

                // Диагональ p00-p11
                OutVertices.push_back ( { p00, Color } );
                OutVertices.push_back ( { p11, Color } );

                // Вторая диагональ p10-p01 (опционально)
                // FVector p10 = TerrainComp->GetWorldPositionAt(x + 1, z);
                // FVector p01 = TerrainComp->GetWorldPositionAt(x, z + 1);
                // OutVertices.push_back({p10, Color});
                // OutVertices.push_back({p01, Color});
                }
            }
        }
    }

void CWireframeGenerator::GenerateTerrainOutline ( std::vector<FWireframeVertex> & OutVertices, const CTerrainComponent * TerrainComp, const FVector & Color )

    {
    OutVertices.clear ();

    if (!TerrainComp) return;

    const FTerrainData & data = TerrainComp->GetTerrainData ();

    if (data.Width < 2 || data.Height < 2)
        return;

    // Контур: 4 стороны прямоугольника, но с учётом рельефа
    OutVertices.reserve ( ( data.Width + data.Height ) * 4 * 2 );

    // Нижняя сторона (z = 0)
    for (int32 x = 0; x < data.Width - 1; ++x)
        {
        FVector p1 = TerrainComp->GetWorldPositionAt ( x, 0 );
        FVector p2 = TerrainComp->GetWorldPositionAt ( x + 1, 0 );
        OutVertices.push_back ( { p1, Color } );
        OutVertices.push_back ( { p2, Color } );
        }

        // Верхняя сторона (z = Height-1)
    for (int32 x = 0; x < data.Width - 1; ++x)
        {
        FVector p1 = TerrainComp->GetWorldPositionAt ( x, data.Height - 1 );
        FVector p2 = TerrainComp->GetWorldPositionAt ( x + 1, data.Height - 1 );
        OutVertices.push_back ( { p1, Color } );
        OutVertices.push_back ( { p2, Color } );
        }

        // Левая сторона (x = 0)
    for (int32 z = 0; z < data.Height - 1; ++z)
        {
        FVector p1 = TerrainComp->GetWorldPositionAt ( 0, z );
        FVector p2 = TerrainComp->GetWorldPositionAt ( 0, z + 1 );
        OutVertices.push_back ( { p1, Color } );
        OutVertices.push_back ( { p2, Color } );
        }

        // Правая сторона (x = Width-1)
    for (int32 z = 0; z < data.Height - 1; ++z)
        {
        FVector p1 = TerrainComp->GetWorldPositionAt ( data.Width - 1, z );
        FVector p2 = TerrainComp->GetWorldPositionAt ( data.Width - 1, z + 1 );
        OutVertices.push_back ( { p1, Color } );
        OutVertices.push_back ( { p2, Color } );
        }
    }

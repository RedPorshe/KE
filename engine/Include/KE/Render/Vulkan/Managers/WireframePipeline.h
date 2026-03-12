#pragma once

#include "Render/Vulkan/Managers/PipelineManager.h"
#include "Components/Collisions/TerrainComponent.h"
#include <vector>
#include <array>

struct FWireframeVertex;


    // Класс для генерации геометрии коллизий
class KE_API CWireframeGenerator
    {
    public:
        // Генерация вершин для сферы (как набор линий)
        static void GenerateSphere ( std::vector<FWireframeVertex> & OutVertices,
                                     const FVector & Center,
                                     float Radius,
                                     const FVector & Color,
                                     int Segments = 16 );

            // Генерация вершин для бокса (12 рёбер)
        static void GenerateBox ( std::vector<FWireframeVertex> & OutVertices,
                                  const FVector & Center,
                                  const FQuat & Rotation,
                                  const FVector & HalfExtents,
                                  const FVector & Color );

            // Генерация вершин для капсулы
        static void GenerateCapsule ( std::vector<FWireframeVertex> & OutVertices,
                                      const FVector & Center,
                                      const FQuat & Rotation,
                                      float Radius,
                                      float HalfHeight,
                                      const FVector & Color,
                                      int Segments = 12 );

            // Генерация вершин для цилиндра
        static void GenerateCylinder ( std::vector<FWireframeVertex> & OutVertices,
                                       const FVector & Center,
                                       const FQuat & Rotation,
                                       float Radius,
                                       float Height,
                                       const FVector & Color,
                                       int Segments = 16 );

            // Генерация вершин для конуса
        static void GenerateCone ( std::vector<FWireframeVertex> & OutVertices,
                                   const FVector & Center,
                                   const FQuat & Rotation,
                                   float Radius,
                                   float Height,
                                   const FVector & Color,
                                   int Segments = 16 );

            // ========== НОВЫЙ МЕТОД: Генерация wireframe для террейна ==========
            /**
             * Генерирует wireframe сетку для террейна на основе данных коллизии
             * @param OutVertices - выходной массив вершин (пары вершин для линий)
             * @param TerrainComp - компонент террейна с данными коллизии
             * @param Color - цвет линий
             * @param bDrawDiagonals - рисовать ли диагонали (для более плотной сетки)
             */
        static void GenerateTerrainWireframe ( std::vector<FWireframeVertex> & OutVertices,
                                               const class CTerrainComponent * TerrainComp,
                                               const FVector & Color = FVector ( 0.7f, 0.7f, 0.7f ),
                                               bool bDrawDiagonals = false );

            // ========== УПРОЩЁННАЯ ВЕРСИЯ: Только контур террейна ==========
            /**
             * Генерирует только контур террейна (внешние границы)
             */
        static void GenerateTerrainOutline ( std::vector<FWireframeVertex> & OutVertices,
                                             const class CTerrainComponent * TerrainComp,
                                             const FVector & Color = FVector ( 0.7f, 0.7f, 0.7f ) );
    };
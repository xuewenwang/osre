/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015-2019 OSRE ( Open Source Render Engine ) by Kim Kulling

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-----------------------------------------------------------------------------------------------*/
#include "osre_testcommon.h"
#include <osre/Scene/GeometryBuilder.h>
#include <osre/RenderBackend/RenderCommon.h>
#include <osre/RenderBackend/Mesh.h>

namespace OSRE {
namespace UnitTest {

using namespace ::OSRE::Scene;
using namespace ::OSRE::RenderBackend;

class MeshBuilderTest : public ::testing::Test {
    // empty
};

TEST_F( MeshBuilderTest, allocEmptyGeometryTest ) {
    MeshBuilder meshBuilder;
    meshBuilder.allocEmptyMesh(VertexType::ColorVertex, 2);
    Mesh *meshArray = meshBuilder.getMesh();
    EXPECT_NE( meshArray, nullptr );

    for ( ui32 i = 0; i < 2; i++ ) {
        Mesh &currentGeo( meshArray[ i ] );
        EXPECT_EQ( currentGeo.m_vertextype, VertexType::ColorVertex );
    }
    Mesh::destroy( &meshArray );
}

TEST_F( MeshBuilderTest, allocTrianglesTest ) {
    MeshBuilder meshBuilder;
    printf("1\n");
    meshBuilder.allocTriangles(VertexType::ColorVertex, BufferAccessType::ReadOnly);
    printf("2\n");
    Mesh *mesh = meshBuilder.getMesh();
    printf("3\n");
    ASSERT_NE( mesh, nullptr );
    EXPECT_EQ( mesh->m_vertextype, VertexType::ColorVertex );
    EXPECT_NE( mesh->m_vb, nullptr );
    EXPECT_NE( mesh->m_ib, nullptr );
    EXPECT_NE( mesh->m_material, nullptr );
    printf("4\n");
    Mesh::destroy( &mesh );
    printf("5\n");
}

TEST_F( MeshBuilderTest, allocLineListTest ) {
    const ui32 numLines = 2;
    glm::vec3 pos[ 3 ], col[3];
    pos[ 0 ].x = 0;
    pos[ 0 ].y = 0;
    pos[ 0 ].z = 0;

    pos[ 1 ].x = 1;
    pos[ 1 ].y = 0;
    pos[ 1 ].z = 0;

    pos[ 2 ].x = 2;
    pos[ 2 ].y = 0;
    pos[ 2 ].z = 0;

    col[ 0 ].x = 0;
    col[ 0 ].y = 0;
    col[ 0 ].z = 0;

    col[ 1 ].x = 0.5f;
    col[ 1 ].y = 0.5f;
    col[ 1 ].z = 0.5f;

    col[ 2 ].x = 0.8f;
    col[ 2 ].y = 0.8f;
    col[ 2 ].z = 0.8f;

    ui32 indices[ 4 ];
    indices[ 0 ]=0;
    indices[ 1 ]=1;
    indices[ 2 ]=1;
    indices[ 3 ]=2;

    Scene::MeshBuilder geoBuilder;
    geoBuilder.allocLineList(VertexType::ColorVertex, BufferAccessType::ReadOnly, numLines, pos, col, indices);
    Mesh *mesh = geoBuilder.getMesh();
    EXPECT_NE( nullptr, mesh );
    Mesh::destroy( &mesh );
}

TEST_F( MeshBuilderTest, allocPointsTest ) {
    const ui32 numPoints = 3;
    glm::vec3 pos[ 3 ], col[ 3 ];
    pos[ 0 ].x = 0;
    pos[ 0 ].y = 0;
    pos[ 0 ].z = 0;

    pos[ 1 ].x = 1;
    pos[ 1 ].y = 0;
    pos[ 1 ].z = 0;

    pos[ 2 ].x = 2;
    pos[ 2 ].y = 0;
    pos[ 2 ].z = 0;

    col[ 0 ].x = 0;
    col[ 0 ].y = 0;
    col[ 0 ].z = 0;

    col[ 1 ].x = 0.5f;
    col[ 1 ].y = 0.5f;
    col[ 1 ].z = 0.5f;

    col[ 2 ].x = 0.8f;
    col[ 2 ].y = 0.8f;
    col[ 2 ].z = 0.8f;
    Scene::MeshBuilder meshBuilder;
    meshBuilder.allocPoints(VertexType::ColorVertex, BufferAccessType::ReadOnly, numPoints, pos, col);
    Mesh *mesh = meshBuilder.getMesh();
    EXPECT_NE( nullptr, mesh );
    Mesh::destroy( &mesh );
}

class GeometryDiagnosticUtilsTest : public ::testing::Test {
    // empty
};

TEST_F( GeometryDiagnosticUtilsTest, dumpVerticesTest_invalidInput ) {
    EXPECT_NO_THROW( MeshDiagnostic::dumpVertices( nullptr, 1 ) );
    
    CPPCore::TArray<RenderVert> vertices;
    EXPECT_NO_THROW(MeshDiagnostic::dumpVertices( vertices ) );
}

} // Namespace UnitTest
} // Namespace OSRE

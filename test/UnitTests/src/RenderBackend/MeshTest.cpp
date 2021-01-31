/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015-2021 OSRE ( Open Source Render Engine ) by Kim Kulling

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
#include <osre/RenderBackend/Mesh.h>

namespace OSRE {
namespace UnitTest {

using namespace ::OSRE::RenderBackend;

class MeshTest : public ::testing::Test {
    // empty
};

TEST_F(MeshTest, createTest) {
    Mesh *meshes = Mesh::create(10);
    EXPECT_NE(nullptr, meshes);
    
    Mesh::destroy(&meshes);
    EXPECT_EQ(nullptr, meshes);
}

TEST_F(MeshTest, createPrimitiveGroupsTest) {
    Mesh* mesh = Mesh::create(1);
    PrimitiveGroup* group = mesh->createPrimitiveGroup(IndexType::UnsignedByte, 10, PrimitiveType::TriangleList, 0);
    EXPECT_EQ(1, mesh->m_numPrimGroups);
    EXPECT_NE(nullptr, group);
}

}
}

/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015-2017 OSRE ( Open Source Render Engine ) by Kim Kulling

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
#include <gtest/gtest.h>
#include <osre/Common/osre_common.h>

namespace OSRE {
namespace UnitTest {

using namespace ::OSRE;

class CommonTest : public ::testing::Test {
    //
};

TEST_F( CommonTest, TypesTest ) {
    EXPECT_EQ( 4U, sizeof( ui32 ) );
    EXPECT_EQ( 4U, sizeof( i32 ) );
    EXPECT_EQ( 4U, sizeof( f32 ) );

    EXPECT_EQ( 2U, sizeof( ui16 ) );
    EXPECT_EQ( 2U, sizeof( i16 ) );

    EXPECT_EQ( 1U, sizeof( uc8 ) );
    EXPECT_EQ( 1U, sizeof( c8 ) );
}

TEST_F( CommonTest, HandleTest ) {
    Handle testHandle;
    EXPECT_EQ( -1, testHandle.m_idx );

    testHandle.m_idx = 1;
    testHandle.init( 1);
    EXPECT_EQ( 1U, testHandle.m_idx );
}

TEST_F( CommonTest, Color4Test ) {
    Color4 col1( 0,0,0,0 ), col2( 1,1,1,1 ), col3( 1, 1, 1, 1 );
    EXPECT_NE( col1, col2 );
    EXPECT_EQ( col2, col3 );
}

TEST_F( CommonTest, TVec2Test ) {
    Vec2f vec2( 1, 2 );
    EXPECT_FLOAT_EQ( 1, vec2.getX() );
    EXPECT_FLOAT_EQ( 2, vec2.getY() );
}

TEST_F( CommonTest, TVec3Test ) {
    Vec3f vec3( 1, 2, 3 );
    EXPECT_FLOAT_EQ( 1, vec3.getX() );
    EXPECT_FLOAT_EQ( 2, vec3.getY() );
    EXPECT_FLOAT_EQ( 3, vec3.getZ() );
}

TEST_F( CommonTest, TVec4Test ) {
    Vec4f vec4( 1, 2, 3, 1 );
    EXPECT_FLOAT_EQ( 1, vec4.getX() );
    EXPECT_FLOAT_EQ( 2, vec4.getY() );
    EXPECT_FLOAT_EQ( 3, vec4.getZ() );
    EXPECT_FLOAT_EQ( 1, vec4.getW() );
}

TEST_F( CommonTest, TQuatTest ) {
    Quatf q;
    EXPECT_FLOAT_EQ( 0, q.m_x );
    EXPECT_FLOAT_EQ( 0, q.m_y );
    EXPECT_FLOAT_EQ( 0, q.m_z );
    EXPECT_FLOAT_EQ( 1, q.m_w );

}

} // Namespace UnitTest
} // Namespace OSRE

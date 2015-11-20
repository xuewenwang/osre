/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015 OSRE ( Open Source Render Engine ) by Kim Kulling

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
#include <osre/Scene/GeometryBuilder.h>
#include <osre/Common/Logger.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <sstream>

namespace OSRE {
namespace Scene {

using namespace ::OSRE::RenderBackend;

static const String Tag = "GeometryBuilder";

const String VsSrc =
    "#version 400 core\n"
    "\n"
    "layout(location = 0) in vec3 position;	      // object space vertex position\n"
    "layout(location = 1) in vec3 normal;	            // object space vertex normal\n"
    "layout(location = 2) in vec3 color0;  // per-vertex colour\n"
    "\n"
    "// output from the vertex shader\n"
    "smooth out vec4 vSmoothColor;		//smooth colour to fragment shader\n"
    "\n"
    "// uniform\n"
    "uniform mat4 MVP;	//combined modelview projection matrix\n"
    "\n"
    "void main()\n"
    "{\n"
    "    //assign the per-vertex color to vSmoothColor varying\n"
    "    vSmoothColor = vec4(color0,1);\n"
    "\n"
    "    //get the clip space position by multiplying the combined MVP matrix with the object space\n"
    "    //vertex position\n"
    "    gl_Position = MVP*vec4(position,1);\n"
    "}\n";

const String FsSrc =
    "#version 400 core\n"
    "\n"
    "layout(location=0) out vec4 vFragColor; //fragment shader output\n"
    "\n"
    "//input form the vertex shader\n"
    "smooth in vec4 vSmoothColor;		//interpolated colour to fragment shader\n"
    "\n"
    "void main()\n"
    "{\n"
    "    //set the interpolated color as the shader output\n"
    "    vFragColor = vSmoothColor;\n"
    "}\n";


static const String TextVsSrc =
    "#version 400 core\n"
    "\n"
    "layout(location = 0) in vec3 position;	      // object space vertex position\n"
	"layout(location = 1) in vec3 normal;	            // object space vertex normal\n"
	"layout(location = 2) in vec2 texcoord0;	        // texture coordinate\n"
	"out vec2 UV;\n"
    "\n"
    "uniform mat4 MVP;	//combined modelview projection matrix\n"
	"\n"
	"void main() {\n"
	"    gl_Position = MVP*vec4( position, 1 );\n"
	"    // UV of the vertex. No special space for this one.\n"
	"    UV = texcoord0;\n"
	"};\n";

static const String TextFsSrc =
    "#version 400 core\n"
    "\n"
    "in vec2 UV;\n"
	"// Output data\n"
	"out vec4 vFragColor;\n"
	"uniform sampler2D tex0;\n"

	"void main() {\n"
    "//    vFragColor = vec4(1,1,1,1 );\n"
    "    vFragColor = texture( tex0, UV );\n"
	"};\n";


GeometryBuilder::GeometryBuilder() {
    // empty
}

GeometryBuilder::~GeometryBuilder() {
    // empty
}

RenderBackend::Geometry *GeometryBuilder::allocEmptyGeometry( RenderBackend::VertexType type ) {
    Geometry *geo = new Geometry;
    geo->m_vertextype = type;
    geo->m_indextype = UnsignedShort;

    return geo;
}

BufferData *allocVertices( VertexType type, ui32 numVerts, glm::vec3 *pos, glm::vec3 *col ) {
    BufferData *data( nullptr );
    ui32 size( 0 );
    switch (type) {
        case ColorVertex: {
                ColorVert *colVerts = new ColorVert[ numVerts ];
                if ( nullptr != pos) {
                    for ( ui32 i = 0; i < numVerts; i++) {
                        colVerts[ i ].position = pos[ i ];
                    }
                }
                if ( nullptr != col) {
                    for ( ui32 j = 0; j < numVerts; j++) {
                        colVerts[ j ].color = col[ j ];
                    }
                }
                size = sizeof( ColorVert ) * numVerts;
                data = BufferData::alloc( VertexBuffer, size, ReadOnly );
                ::memcpy( data->m_pData, colVerts, size );
            }
            break;

        case RenderVertex: {
                RenderVert *renderVerts = new RenderVert[ numVerts ];
                if ( nullptr != pos) {
                    for ( ui32 j = 0; j < numVerts; j++) {
                        renderVerts[ j ].position = pos[ j ];
                    }
                }
                size = sizeof( RenderVert ) * numVerts;
                data = BufferData::alloc( VertexBuffer, size, ReadOnly );
                ::memcpy( data->m_pData, renderVerts, size );
            }
            break;

        default:
            break;
    }

    return data;
}

Geometry *GeometryBuilder::allocTriangles( VertexType type ) {
    Geometry *geo = new Geometry;
    geo->m_vertextype = type;
    geo->m_indextype = UnsignedShort;

    // setup triangle vertices    
    static const ui32 NumVert = 3;
    ColorVert vertices[ NumVert ];

    glm::vec3 col[ NumVert ];
    col[ 0 ] = glm::vec3( 1, 0, 0 );
    col[ 1 ] = glm::vec3( 0, 1, 0 );
    col[ 2 ] = glm::vec3( 0, 0, 1 );

    glm::vec3 pos[ NumVert ];
    pos[ 0 ] = glm::vec3( -1, -1, 0 );
    pos[ 1 ] = glm::vec3( 0, 1, 0 );
    pos[ 2 ] = glm::vec3( 1, -1, 0 );
    geo->m_vb = allocVertices( geo->m_vertextype,  NumVert, pos, col );

    // setup triangle indices
    static const ui32 NumIndices = 3;
    GLushort  indices[ NumIndices ];
    indices[ 0 ] = 0;
    indices[ 1 ] = 1;
    indices[ 2 ] = 2;
    
    ui32 size = sizeof( GLushort ) * NumIndices;
    geo->m_ib = BufferData::alloc( IndexBuffer, size, ReadOnly );
    ::memcpy( geo->m_ib->m_pData, indices, size );

	// setup primitives
    geo->m_numPrimGroups = 1;
    geo->m_pPrimGroups   = new PrimitiveGroup[ geo->m_numPrimGroups ];
    geo->m_pPrimGroups[ 0 ].m_indexType     = UnsignedShort;
    geo->m_pPrimGroups[ 0 ].m_numPrimitives = 3 * geo->m_numPrimGroups;
    geo->m_pPrimGroups[ 0 ].m_primitive     = TriangleList;
    geo->m_pPrimGroups[ 0 ].m_startIndex    = 0;

	// setup material
    geo->m_material = new Material;
    geo->m_material->m_numTextures = 0;
    geo->m_material->m_type = ShaderMaterial;
    geo->m_material->m_pShader = new Shader;
    geo->m_material->m_pShader->m_src[ SH_VertexShaderType ] = VsSrc;
    geo->m_material->m_pShader->m_src[ SH_FragmentShaderType ] = FsSrc;

	// setup shader attributes and variables
    if( nullptr != geo->m_material->m_pShader ) {
        ui32 numAttribs( ColorVert::getNumAttributes() );
        const String *attribs( ColorVert::getAttributes() );
        geo->m_material->m_pShader->m_attributes.add(attribs, numAttribs);
        geo->m_material->m_pShader->m_parameters.add( "MVP" );
    }

    return geo;
}

Geometry *GeometryBuilder::allocQuads( VertexType type ) {
    Geometry *geo = new Geometry;
    geo->m_vertextype = type;
    geo->m_indextype = UnsignedShort;

    // setup triangle vertices    
    static const ui32 NumVert = 4;
    glm::vec3 col[ NumVert ];
    col[ 0 ] = glm::vec3( 1, 0, 0 );
    col[ 1 ] = glm::vec3( 0, 1, 0 );
    col[ 2 ] = glm::vec3( 0, 0, 1 );
    col[ 3 ] = glm::vec3( 1, 0, 0 );

    glm::vec3 pos[ NumVert ];
    pos[ 0 ] = glm::vec3( -1, -1, 0 );
    pos[ 1 ] = glm::vec3( -1, 1, 0 );
    pos[ 2 ] = glm::vec3( 1, -1, 0 );
    pos[ 3 ] = glm::vec3( 1, 1, 0 );

    geo->m_vb = allocVertices( geo->m_vertextype, NumVert, pos, col );

    // setup triangle indices
    static const ui32 NumIndices = 6;
    GLushort  indices[ NumIndices ];
    indices[ 0 ] = 0;
    indices[ 1 ] = 1;
    indices[ 2 ] = 2;

    indices[ 3 ] = 1;
    indices[ 4 ] = 2;
    indices[ 5 ] = 3;

    ui32 size = sizeof( GLushort ) * NumIndices;
    geo->m_ib = BufferData::alloc( IndexBuffer, size, ReadOnly );
    ::memcpy( geo->m_ib->m_pData, indices, size );

    // setup primitives
    geo->m_numPrimGroups = 1;
    geo->m_pPrimGroups = new PrimitiveGroup[ geo->m_numPrimGroups ];
    geo->m_pPrimGroups[ 0 ].m_indexType = UnsignedShort;
    geo->m_pPrimGroups[ 0 ].m_numPrimitives = 6 * geo->m_numPrimGroups;
    geo->m_pPrimGroups[ 0 ].m_primitive = TriangleList;
    geo->m_pPrimGroups[ 0 ].m_startIndex = 0;

    // setup material
    geo->m_material = new Material;
    geo->m_material->m_numTextures = 0;
    geo->m_material->m_type = ShaderMaterial;
    geo->m_material->m_pShader = new Shader;
    geo->m_material->m_pShader->m_src[ SH_VertexShaderType ] = VsSrc;
    geo->m_material->m_pShader->m_src[ SH_FragmentShaderType ] = FsSrc;

    // setup shader attributes and variables
    if (nullptr != geo->m_material->m_pShader) {
        ui32 numAttribs( ColorVert::getNumAttributes() );
        const String *attribs( ColorVert::getAttributes() );
        geo->m_material->m_pShader->m_attributes.add( attribs, numAttribs );
        geo->m_material->m_pShader->m_parameters.add( "MVP" );
    }

    return geo;
}

static void dumpTextBox( ui32 i, glm::vec3 *textPos, ui32 VertexOffset ) {
    std::stringstream stream;
    stream << std::endl;
    stream << "i = " << i << " : " << textPos[ VertexOffset + 0 ].x << ", " << textPos[ VertexOffset + 0 ].y << std::endl;
    stream << "i = " << i << " : " << textPos[ VertexOffset + 1 ].x << ", " << textPos[ VertexOffset + 1 ].y << std::endl;
    stream << "i = " << i << " : " << textPos[ VertexOffset + 2 ].x << ", " << textPos[ VertexOffset + 2 ].y << std::endl;
    stream << "i = " << i << " : " << textPos[ VertexOffset + 3 ].x << ", " << textPos[ VertexOffset + 3 ].y << std::endl;
    osre_info( Tag, stream.str() );

}

RenderBackend::Geometry *GeometryBuilder::allocTextBox(  f32 x, f32 y, f32 textSize, const String &text ) {
	if ( text.empty() ) {
		return nullptr;
	}

    Geometry *geo = new Geometry;
    geo->m_vertextype = ColorVertex;
    geo->m_indextype = UnsignedShort;

    // setup triangle vertices    
    static const ui32 NumQuadVert = 4;
    glm::vec3 col[ NumQuadVert ];
    col[ 0 ] = glm::vec3( 1, 0, 0 );
    col[ 1 ] = glm::vec3( 0, 1, 0 );
    col[ 2 ] = glm::vec3( 0, 0, 1 );
    col[ 3 ] = glm::vec3( 1, 0, 0 );

    glm::vec3 pos[ NumQuadVert ];
    pos[ 0 ] = glm::vec3( 0, 0, 0 );
    pos[ 1 ] = glm::vec3( 0, textSize, 0 );
    pos[ 2 ] = glm::vec3( textSize, 0, 0 );
    pos[ 3 ] = glm::vec3( textSize, textSize, 0 );

    static const ui32 NumQuadIndices = 6;
    GLushort  indices[ NumQuadIndices ];
    indices[ 0 ] = 0;
    indices[ 1 ] = 1;
    indices[ 2 ] = 2;

    indices[ 3 ] = 1;
    indices[ 4 ] = 2;
    indices[ 5 ] = 3;


    const ui32 NumTextVerts = NumQuadVert * text.size();
    glm::vec3 *textPos = new glm::vec3[ NumTextVerts ];
    glm::vec3 *colors = new glm::vec3[ NumTextVerts ];
    GLushort *textIndices = new GLushort[ NumQuadIndices * text.size() ];

    for (ui32 i = 0; i < text.size(); i++) {
        const ui32 VertexOffset( i * NumQuadVert );
        textPos[ VertexOffset + 0 ].x = pos[ 0 ].x + (i*textSize);
        textPos[ VertexOffset + 0 ].y = pos[ 0 ].y;

        textPos[ VertexOffset + 1 ].x = pos[ 1 ].x + (i*textSize);
        textPos[ VertexOffset + 1 ].y = pos[ 1 ].y;

        textPos[ VertexOffset + 2 ].x = pos[ 2 ].x + (i*textSize);
        textPos[ VertexOffset + 2 ].y = pos[ 2 ].y;

        textPos[ VertexOffset + 3 ].x = pos[ 3 ].x + (i*textSize);
        textPos[ VertexOffset + 3 ].y = pos[ 3 ].y;

        dumpTextBox( i, textPos, VertexOffset );

        colors[ VertexOffset + 0 ] = col[ 0 ];
        colors[ VertexOffset + 1 ] = col[ 1 ];
        colors[ VertexOffset + 2 ] = col[ 2 ];
        colors[ VertexOffset + 3 ] = col[ 3 ];
        const ui32 IndexOffset( i * NumQuadIndices );
        textIndices[ 0 + IndexOffset ] = 0 + VertexOffset;
        textIndices[ 1 + IndexOffset ] = 1 + VertexOffset;
        textIndices[ 2 + IndexOffset ] = 2 + VertexOffset;

        textIndices[ 3 + IndexOffset ] = 1 + VertexOffset;
        textIndices[ 4 + IndexOffset ] = 2 + VertexOffset;
        textIndices[ 5 + IndexOffset ] = 3 + VertexOffset;
    }

    geo->m_vb = allocVertices( geo->m_vertextype, text.size() * NumQuadVert, textPos, colors );

    // setup triangle indices
    ui32 size = sizeof( GLushort ) * 6 * text.size();
    geo->m_ib = BufferData::alloc( IndexBuffer, size, ReadOnly );
    ::memcpy( geo->m_ib->m_pData, textIndices, size );


    // setup primitives
    geo->m_numPrimGroups = text.size();
    geo->m_pPrimGroups = new PrimitiveGroup[ geo->m_numPrimGroups ];
    geo->m_pPrimGroups[ 0 ].m_indexType = UnsignedShort;
    geo->m_pPrimGroups[ 0 ].m_numPrimitives = 6 * geo->m_numPrimGroups;
    geo->m_pPrimGroups[ 0 ].m_primitive = TriangleList;
    geo->m_pPrimGroups[ 0 ].m_startIndex = 0;

    // setup material
    geo->m_material = new Material;
    geo->m_material->m_numTextures = 0;
    geo->m_material->m_type = ShaderMaterial;
    geo->m_material->m_pShader = new Shader;
    geo->m_material->m_pShader->m_src[ SH_VertexShaderType ] = VsSrc;
    geo->m_material->m_pShader->m_src[ SH_FragmentShaderType ] = FsSrc;

    // setup shader attributes and variables
    if (nullptr != geo->m_material->m_pShader) {
        ui32 numAttribs( ColorVert::getNumAttributes() );
        const String *attribs( ColorVert::getAttributes() );
        geo->m_material->m_pShader->m_attributes.add( attribs, numAttribs );
        geo->m_material->m_pShader->m_parameters.add( "MVP" );
    }

    return geo;
    // setup material
	/*geo->m_material = new Material;
    geo->m_material->m_numTextures = 1;
    geo->m_material->m_pTextures = new Texture[ 1 ];
#ifdef _WIN32
    geo->m_material->m_pTextures[ 0 ].m_textureName = "../../media/Textures/Fonts/buildin_arial.bmp";
#else
    geo->m_material->m_pTextures[ 0 ].m_textureName = "../media/Textures/Fonts/buildin_arial.bmp";
#endif

    
   	geo->m_material->m_type = ShaderMaterial;
	geo->m_material->m_pShader = new Shader;

	geo->m_material->m_pShader->m_src[ SH_VertexShaderType ]   = TextVsSrc;
	geo->m_material->m_pShader->m_src[ SH_FragmentShaderType ] = TextFsSrc;

	// setup shader attributes and variables
	if ( nullptr != geo->m_material->m_pShader) {
		ui32 numAttribs( RenderVert::getNumAttributes() );
		const String *attribs( ColorVert::getAttributes() );
		geo->m_material->m_pShader->m_attributes.add( attribs, numAttribs );
		geo->m_material->m_pShader->m_parameters.add( "MVP" );
	}

	return geo;*/
}


} // Namespace Scene
} // namespace OSRE

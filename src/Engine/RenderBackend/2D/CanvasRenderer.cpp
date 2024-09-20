/*-----------------------------------------77------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015-2024 OSRE ( Open Source Render Engine ) by Kim Kulling

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
#include "CanvasRenderer.h"
#include "RenderBackend/Mesh.h"
#include "RenderBackend/RenderBackendService.h"
#include "RenderBackend/MaterialBuilder.h"
#include "RenderBackend/Shader.h"
#include "Common/Logger.h"

#include "Debugging/MeshDiagnostic.h"

#include <cppcore/Memory/TPoolAllocator.h>

namespace OSRE {
namespace RenderBackend {

using namespace cppcore;

static constexpr c8 Tag[] = "CanvasRenderer";

/// This struct is used to store all 2d draw commands.
struct DrawCmd {
    PrimitiveType PrimType;    // The primitive type
    size_t NumVertices;        // The number of vertices
    RenderVert *Vertices;      // The vertex buffer
    size_t NumIndices;         // Number of indices
    ui16 *Indices;             // The number of indices

    // The class constructor.
    DrawCmd() :
            PrimType(PrimitiveType::Invalid), NumVertices(0u), Vertices(nullptr), NumIndices(0u), Indices(nullptr) {}

    // The class descructor.
    ~DrawCmd() = default;
};

// will rescale coordinates from absolute coordinates into model space coordinates
inline void mapCoordinates(const Rect2i &resolution, i32 x, i32 y, f32 &xOut, f32 &yOut) {
    xOut = (2.0f * static_cast<f32>(x)  / static_cast<f32>(resolution.width)) - 1.0f;
    yOut = (2.0f * static_cast<f32>(y) / static_cast<f32>(resolution.height)) - 1.0f;
    //yOut = -1.0f * yOut;
}

inline void clip(const Rect2i &resolution, i32 x, i32 y, i32 &x_out, i32 &y_out) {
    x_out = x;
    y_out = y;
    if (x < resolution.getX1()) {
        x_out = resolution.getX1();
    }

    if (x > resolution.getX2()) {
        x_out = resolution.getX2();
    }

    if (y < resolution.getY1()) {
        y_out = resolution.getY1();
    }

    if (y > resolution.getY2()) {
        y_out = resolution.getY2();
    }
}

static TPoolAllocator<DrawCmd> sAllocator;

DrawCmd *alloc() {
    if (sAllocator.capacity() == 0) {
        sAllocator.reserve(1024);
    }

    DrawCmd *dc = sAllocator.alloc();
    return dc;
}

void dealloc(DrawCmd *cmd) {
    if (cmd == nullptr) {
        osre_debug(Tag, "Invalid command to release");
        return;
    }

    delete cmd;
}

CanvasRenderer::CanvasRenderer(i32 numLayers, i32 x, i32 y, i32 w, i32 h) :
        mDirty(true), mPenColor(1, 1, 1, 0), mResolution(), mActiveLayer(0), mNumLayers(numLayers), mMesh(nullptr) {
    setResolution(x, y, w, h);
}

CanvasRenderer::~CanvasRenderer() {
    for (size_t i = 0; i < mDrawCmdArray.size(); ++i) {
        auto &dc = *mDrawCmdArray[i];
        dealloc(&dc);
    }
}

void CanvasRenderer::preRender(RenderBackendService *rbSrv) {
    if (rbSrv == nullptr) {
        osre_assert(rbSrv != nullptr);
        return;
    }

    glm::mat4 m(1);
    rbSrv->setMatrix(MatrixType::Model, m);
    rbSrv->setMatrix(MatrixType::View, m);
    rbSrv->setMatrix(MatrixType::Projection, m);
}

void CanvasRenderer::render(RenderBackendService *rbSrv) {
    if (rbSrv == nullptr) {
        osre_assert(rbSrv != nullptr);
        return;
    }

    if (!isDirty()) {
        return;
    }

    if (mMesh == nullptr) {
        mMesh = new Mesh("2d", VertexType::RenderVertex, IndexType::UnsignedShort);
        Material *mat2D = MaterialBuilder::create2DMaterial();
        if (mat2D == nullptr) {
            osre_debug(Tag, "Invalid material instance detected.");
            return;
        }
        mMesh->setMaterial(mat2D);
    }

    PrimitiveType prim = PrimitiveType::TriangleList;
    size_t numVertices = 0l, numIndices = 0l;
    for (size_t i=0; i<mDrawCmdArray.size(); ++i) {
        const auto &dc = *mDrawCmdArray[i];
        if (dc.Vertices == nullptr) {
            osre_debug(Tag, "Invalid draw command detecetd.");
            continue;
        }
        
        const ui32 lastIndex = mMesh->getLastIndex();
        if (numVertices > 0) {
            for (size_t j = 0; j < dc.NumIndices; ++j) {
                dc.Indices[j] += static_cast<ui16>(numVertices);
            }
        }

        Debugging::MeshDiagnostic::dumpVertices(dc.Vertices, dc.NumVertices);
        mMesh->attachVertices(dc.Vertices, dc.NumVertices * sizeof(RenderVert));
        mMesh->attachIndices(dc.Indices, dc.NumIndices * sizeof(ui16));
        prim = dc.PrimType;
        mMesh->setLastIndex(lastIndex + static_cast<ui16>(dc.NumIndices));
        numVertices += dc.NumVertices;
        numIndices += dc.NumIndices;
    }
    mMesh->addPrimitiveGroup(numIndices, prim, 0);
    //Debugging::MeshDiagnostic::dumpIndices((ui16 *)mMesh->getIndexBuffer()->getData(), numIndices);

    rbSrv->addMesh(mMesh, 0);
    mDrawCmdArray.resize(0);
    setClean();
}

void CanvasRenderer::postRender(RenderBackendService *rbSrv) {
    if (rbSrv == nullptr) {
        osre_assert(rbSrv != nullptr);
        return;
    }
}

void CanvasRenderer::setResolution(const Rect2i& resolution) {
    if (mResolution == resolution) {
        return;
    }
    mResolution = resolution;
    setDirty();
}

void CanvasRenderer::setResolution(i32 x, i32 y, i32 w, i32 h) {
    const Rect2i newResolution(x, y, w, h);
    setResolution(newResolution);
}

bool CanvasRenderer::selectLayer(i32 layer) {
    if (layer < 0 || layer >= mNumLayers) {
        osre_debug(Tag, "Invalid layer selected.");
        return false;
    }

    mActiveLayer = layer;

    return true;
}

i32 CanvasRenderer::getActiveLayer() const {
    return mActiveLayer;
}

void CanvasRenderer::setColor(const Color4 &color) {
    mPenColor = color;
}

const Color4& CanvasRenderer::getColor() const {
    return mPenColor;
}

void CanvasRenderer::drawline(i32 x1, i32 y1, i32 x2, i32 y2) {
    DrawCmd *dc = alloc();

    i32 x_clipped, y_clipped;
    clip(mResolution, x1, y1, x_clipped, y_clipped);
    dc->NumVertices = 2;
    dc->Vertices = new RenderVert[dc->NumVertices];
    dc->Vertices[0].color0 = mPenColor.toVec4();
    dc->Vertices[0].position.x = (f32)x_clipped;
    dc->Vertices[0].position.y = (f32)y_clipped;
    dc->Vertices[0].position.z = static_cast<f32>(-mActiveLayer);

    clip(mResolution, x2, y2, x_clipped, y_clipped);
    dc->Vertices[1].color0 = mPenColor.toVec4();
    dc->Vertices[1].position.x = (f32)x_clipped;
    dc->Vertices[1].position.y = (f32)y_clipped;
    dc->Vertices[1].position.z = static_cast<f32>(-mActiveLayer);

    dc->NumIndices = 2;
    dc->Indices = new ui16[dc->NumIndices];
    dc->Indices[0] = 0;
    dc->Indices[0] = 1;

    mDrawCmdArray.add(dc);

    setDirty();
}

void CanvasRenderer::drawTriangle(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, bool filled) {
    DrawCmd *dc = alloc();

    i32 x_clipped, y_clipped;
    dc->NumVertices = 3;
    dc->Vertices = new RenderVert[dc->NumVertices];
    clip(mResolution, x1, y1, x_clipped, y_clipped);
    dc->Vertices[0].color0 = mPenColor.toVec4();
    dc->Vertices[0].position.x = (f32)x_clipped;
    dc->Vertices[0].position.y = (f32)y_clipped;
    dc->Vertices[0].position.z = static_cast<f32>(-mActiveLayer);

    clip(mResolution, x2, y2, x_clipped, y_clipped);
    dc->Vertices[1].color0 = mPenColor.toVec4();
    dc->Vertices[1].position.x = (f32)x_clipped;
    dc->Vertices[1].position.y = (f32)y_clipped;
    dc->Vertices[1].position.z = static_cast<f32>(-mActiveLayer);

    clip(mResolution, x3, y3, x_clipped, y_clipped);
    dc->Vertices[2].color0 = mPenColor.toVec4();
    dc->Vertices[2].position.x = (f32)x_clipped;
    dc->Vertices[2].position.y = (f32)y_clipped;
    dc->Vertices[2].position.z = static_cast<f32>(-mActiveLayer);
    
    if (filled) {
        dc->NumIndices = 3;
        dc->Indices = new ui16[dc->NumIndices];
        dc->Indices[0] = 0;
        dc->Indices[1] = 1;
        dc->Indices[2] = 2;
    } else {
        dc->NumIndices = 6;
        dc->Indices = new ui16[dc->NumIndices];

        dc->Indices[0] = 0;
        dc->Indices[1] = 1;

        dc->Indices[1] = 1;
        dc->Indices[2] = 2;

        dc->Indices[2] = 1;
        dc->Indices[0] = 2;
    }

    mDrawCmdArray.add(dc);

    setDirty();
}

static void createRectVertices(DrawCmd *drawCmd, const Color4 &penColor, const Rect2i &resolution, i32 x, i32 y, i32 w, i32 h, i32 layer) {
    i32 x_clipped, y_clipped;
    f32 x_model, y_model;

    drawCmd->PrimType = PrimitiveType::TriangleList;
    drawCmd->NumVertices = 6;
    drawCmd->Vertices = new RenderVert[drawCmd->NumVertices];
    
    clip(resolution, x, y, x_clipped, y_clipped);
    mapCoordinates(resolution, x_clipped, y_clipped, x_model, y_model);
    drawCmd->Vertices[0].color0 = penColor.toVec4();
    drawCmd->Vertices[0].position.x = x_model;
    drawCmd->Vertices[0].position.y = y_model;
    drawCmd->Vertices[0].position.z = static_cast<f32>(-layer);

    clip(resolution, x+w, y, x_clipped, y_clipped);
    mapCoordinates(resolution, x_clipped, y_clipped, x_model, y_model);
    drawCmd->Vertices[1].color0 = penColor.toVec4();
    drawCmd->Vertices[1].position.x = x_model;
    drawCmd->Vertices[1].position.y = y_model;
    drawCmd->Vertices[1].position.z = static_cast<f32>(-layer);

    clip(resolution, x+w, y+h, x_clipped, y_clipped);
    mapCoordinates(resolution, x_clipped, y_clipped, x_model, y_model);
    drawCmd->Vertices[2].color0 = penColor.toVec4();
    drawCmd->Vertices[2].position.x = x_model;
    drawCmd->Vertices[2].position.y = y_model;
    drawCmd->Vertices[2].position.z = static_cast<f32>(-layer);

    clip(resolution, x+w, y+h, x_clipped, y_clipped);
    mapCoordinates(resolution, x_clipped, y_clipped, x_model, y_model);
    drawCmd->Vertices[3].color0 = penColor.toVec4();
    drawCmd->Vertices[3].position.x = x_model;
    drawCmd->Vertices[3].position.y = y_model;
    drawCmd->Vertices[3].position.z = static_cast<f32>(-layer);

    clip(resolution, x, y+h, x_clipped, y_clipped);
    mapCoordinates(resolution, x_clipped, y_clipped, x_model, y_model);
    drawCmd->Vertices[4].color0 = penColor.toVec4();
    drawCmd->Vertices[4].position.x = x_model;
    drawCmd->Vertices[4].position.y = y_model;
    drawCmd->Vertices[4].position.z = static_cast<f32>(-layer);

    clip(resolution, x, y, x_clipped, y_clipped);
    mapCoordinates(resolution, x_clipped, y_clipped, x_model, y_model);
    drawCmd->Vertices[5].color0 = penColor.toVec4();
    drawCmd->Vertices[5].position.x = x_model;
    drawCmd->Vertices[5].position.y = y_model;
    drawCmd->Vertices[5].position.z = static_cast<f32>(-layer);

    drawCmd->NumIndices = 6;
    drawCmd->Indices = new ui16[drawCmd->NumIndices];
    drawCmd->Indices[0] = 0;
    drawCmd->Indices[1] = 1;
    drawCmd->Indices[2] = 2;

    drawCmd->Indices[3] = 3;
    drawCmd->Indices[4] = 4;
    drawCmd->Indices[5] = 5;
}

void CanvasRenderer::drawRect(i32 x, i32 y, i32 w, i32 h, bool filled) {
    DrawCmd *drawCmd = nullptr;
    if (filled) {
        drawCmd = alloc();
        createRectVertices(drawCmd, mPenColor, mResolution, x, y, w, h, mActiveLayer);
        mDrawCmdArray.add(drawCmd);
    } else {
        const ui32 thickness = 2;
        drawCmd = alloc();
        createRectVertices(drawCmd, mPenColor, mResolution, x, y, w, thickness, mActiveLayer);
        mDrawCmdArray.add(drawCmd);

        drawCmd = alloc();
        createRectVertices(drawCmd, mPenColor, mResolution, x, y + h, w, thickness, mActiveLayer);
        mDrawCmdArray.add(drawCmd);

        drawCmd = alloc();
        createRectVertices(drawCmd, mPenColor, mResolution, x, y, thickness, h, mActiveLayer);
        mDrawCmdArray.add(drawCmd);
        
        drawCmd = alloc();
        createRectVertices(drawCmd, mPenColor, mResolution, x+w, y, thickness, h, mActiveLayer);
        mDrawCmdArray.add(drawCmd);
    }

    setDirty();
}

} // namespace RenderBackend
} // namespace OSRE

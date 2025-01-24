/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015-2025 OSRE ( Open Source Render Engine ) by Kim Kulling

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
#include "RenderBackend/RenderCommon.h"
#include "App/AssetRegistry.h"
#include "Common/Ids.h"
#include "Common/Logger.h"
#include "IO/Uri.h"
#include "RenderBackend/Mesh.h"
#include "RenderBackend/Shader.h"
#include "Common/glm_common.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace OSRE {
namespace RenderBackend {

using namespace ::cppcore;
using namespace ::OSRE::Common;
using namespace ::glm;

VertComponent VertexLayout::ErrorComp;

// The log tag for messages
static constexpr c8 Tag[] = "RenderCommon";

/// @brief  The corresponding names for vertex components in a vertex layout
static const String
        VertCompName[static_cast<ui32>(VertexAttribute::Count)] = {
            "position",  ///< Position
            "normal",    ///< Normal
            "texcoord0", ///< TexCoord0
            "texcoord1", ///< TexCoord1
            "texcoord2", ///< TexCoord2
            "texcoord3", ///< TexCoord3
            "tangent",   ///< Tangent
            "binormal",  ///< Binormal
            "weights",   ///< Weights
            "indices",   ///< Indices
            "color0",    ///< Color0
            "color1",    ///< Color1
            "instance0", ///< Instance0
            "instance1", ///< Instance1
            "instance2", ///< Instance2
            "instance3"  ///< Instance3
        };

static const String ErrorCmpName = "Error";

// List of attributes for color vertices
static constexpr ui32 NumColorVertAttributes = 3;
static const String ColorVertAttributes[NumColorVertAttributes] = {
    "position", 
    "normal", 
    "color0"
};

ColorVert::ColorVert() : position(), normal(), color0(1, 1, 1) {
    // empty
}

size_t ColorVert::getNumAttributes() {
    return NumColorVertAttributes;
}

const String *ColorVert::getAttributes() {
    return ColorVertAttributes;
}

// List of attributes for render vertices
static constexpr ui32 NumRenderVertAttributes = 4;

static const String RenderVertAttributes[NumRenderVertAttributes] = {
    "position", 
    "normal", 
    "color0", 
    "texcoord0"
};

RenderVert::RenderVert() :
        position(),
        normal(),
        color0(1, 1, 1),
        tex0() {
    // empty
}

bool RenderVert::operator == (const RenderVert &rhs) const {
    return (position == rhs.position && normal == rhs.normal &&
            color0 == rhs.color0 && tex0 == rhs.tex0);
}

bool RenderVert::operator != (const RenderVert &rhs) const {
    return !(*this == rhs);
}

size_t RenderVert::getNumAttributes() {
    return NumRenderVertAttributes;
}

const String *RenderVert::getAttributes() {
    return RenderVertAttributes;
}

const String &getVertCompName(VertexAttribute attrib) {
    if (attrib > VertexAttribute::Instance3  || attrib == VertexAttribute::Invalid) {
        return ErrorCmpName;
    }
    return VertCompName[static_cast<size_t>(attrib)];
}

constexpr size_t NumUIVertAttributes = 2;

static const String UIVertAttributes[NumUIVertAttributes] = {
    "position", 
    "color0"
};

size_t UIVert::getNumAttributes() {
    return NumUIVertAttributes;
}

const String *UIVert::getAttributes() {
    return UIVertAttributes;
}

VertComponent::VertComponent() :
        attrib(VertexAttribute::Invalid),
        format(VertexFormat::Invalid) {
    // empty
}

VertComponent::VertComponent(VertexAttribute attrib_, VertexFormat format_) :
        attrib(attrib_),
        format(format_) {
    // empty
}

VertexLayout::VertexLayout() :
        attributes(nullptr),
        components(),
        offsets(),
        currentOffset(0),
        size(0) {
    // empty
}

VertexLayout::~VertexLayout() {
    delete[] attributes;
}

void VertexLayout::clear() {
    if (!components.isEmpty()) {
        for (size_t i = 0; i < components.size(); ++i) {
            delete components[i];
        }
        components.clear();
    }

    offsets.clear();
    currentOffset = 0;
}

size_t VertexLayout::sizeInBytes() {
    if (0 == size) {
        for (size_t i = 0; i < components.size(); ++i) {
            const size_t compSizeInBytes = getVertexFormatSize(components[i]->format);
            size += compSizeInBytes;
        }
    }

    return size;
}

size_t VertexLayout::numComponents() const {
    return components.size();
}

VertexLayout &VertexLayout::add(VertComponent *comp) {
    if (nullptr == comp) {
        return *this;
    }

    components.add(comp);
    const size_t offset(getVertexFormatSize(comp->format));
    offsets.add(currentOffset);
    currentOffset += offset;

    return *this;
}

VertComponent &VertexLayout::getAt(size_t idx) const {
    if (idx >= components.size()) {
        return ErrorComp;
    }

    return *components[idx];
}

const String *VertexLayout::getAttributes() {
    if (components.isEmpty()) {
        return nullptr;
    }

    if (nullptr == attributes) {
        const size_t numAttributes = components.size();
        attributes = new String[numAttributes];
        for (size_t i = 0; i < components.size(); ++i) {
            attributes[i] = VertCompName[static_cast<int>(components[i]->attrib)];
        }
    }

    return attributes;
}

BufferData::BufferDataAllocator BufferData::sBufferDataAllocator(256);

BufferData::BufferData() :
        type(BufferType::EmptyBuffer),
        buffer(),
        cap(0),
        access(BufferAccessType::ReadOnly) {
    // empty
}

BufferData *BufferData::alloc(BufferType type, size_t sizeInBytes, BufferAccessType access) {
    BufferData *buffer = sBufferDataAllocator.alloc();
    buffer->cap = sizeInBytes;
    buffer->access = access;
    buffer->type = type;
    buffer->buffer.resize(sizeInBytes);

    return buffer;
}

void BufferData::copyFrom(void *data, size_t size) {
    if (nullptr == data) {
        return;
    }
    if (size > cap) {
        osre_error(Tag, "Out of buffer error.");
        return;
    }

    ::memcpy(&buffer[0], data, size);
}

void BufferData::attach(const void *data, size_t size) {
    if (size == 0l) {
        return;
    }
    osre_assert(data != nullptr);
    
    const size_t oldSize = buffer.size();
    buffer.resize(oldSize + size);
    ::memcpy(&buffer[oldSize], data, size);
}

BufferType BufferData::getBufferType() const {
    return type;
}

BufferAccessType BufferData::getBufferAccessType() const {
    return access;
}

PrimitiveGroup::PrimitiveGroup() :
        primitive(PrimitiveType::LineList), startIndex(0), numIndices(0), indexType(IndexType::UnsignedShort) {
    // empty
}

void PrimitiveGroup::init(IndexType indexType_, size_t numPrimitives_, PrimitiveType primType_, size_t startIdx_) {
    indexType = indexType_;
    numIndices = numPrimitives_;
    primitive = primType_;
    startIndex = startIdx_;
}

Texture::Texture() :
        textureName(""),
        loc(),
        targetType(TextureTargetType::Texture2D),
        pixelFormat(PixelFormatType::R8G8B8),
        size(0),
        data(nullptr),
        width(0),
        height(0),
        channels(0),
        texHandle() {
    // empty
}

Texture::~Texture() {
    clear();
}

void Texture::clear() {
    delete[] data;
    data = nullptr;
}

size_t TextureLoader::load(const IO::Uri &uri, Texture *tex) {
    if (nullptr == tex) {
        return 0l;
    }

    const String &filename = uri.getAbsPath();
    if (filename.find("$default") != String::npos) {
        tex = TextureLoader::getDefaultTexture();
    }

    String root = App::AssetRegistry::getPath("media");
    String path = App::AssetRegistry::resolvePathFromUri(uri);

    i32 width = 0, height = 0, channels = 0;
    
    tex->data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (nullptr == tex->data) {
        osre_debug(Tag, "Cannot load texture " + filename);
        return 0;
    }
    tex->width = width;
    tex->height = height;
    tex->channels = channels;

    // swap the texture data
    for (i32 j = 0; j * 2 < height; ++j) {
        i32 index1 = j * width * channels;
        i32 index2 = (height - 1 - j) * width * channels;
        for (i32 i = width * channels; i > 0; --i) {
            uc8 temp = tex->data[index1];
            tex->data[index1] = tex->data[index2];
            tex->data[index2] = temp;
            ++index1;
            ++index2;
        }
    }

    const size_t size = width * height * channels;

    return size;
}

static Texture *DefaultTexture = nullptr;

RenderBackend::Texture *TextureLoader::getDefaultTexture() {
    if (DefaultTexture != nullptr) {
        return DefaultTexture;
    }
    const ui32 Size = 256u;
    const ui32 FullColorChannel = 255u;
    Texture *texture = new Texture;
    texture->textureName = "default";
    texture->targetType = TextureTargetType::Texture2D;
    texture->width = Size;
    texture->height = Size;
    texture->channels = 4;
    const size_t data_size = Size * Size;
    texture->data = new unsigned char[data_size * texture->channels];
    unsigned char rgba_fg[4] = { FullColorChannel, FullColorChannel, 0, FullColorChannel }; // yellow
    unsigned char rgba_bg[4] = { FullColorChannel, 0, 0, FullColorChannel }; // red
    for (auto it = texture->data; it < texture->data + data_size; it += 20) {
        memset(it, 0, 20);
        if (((it - texture->data) + 40) % (20 * 400) == 0) {
            it += 40;
        } else if (((it - texture->data) + 20) % (20 * 400) != 0) {
            it += 20;
        }
    }

    return DefaultTexture;
}

bool TextureLoader::unload(Texture *tex) {
    if (nullptr == tex) {
        return false;
    }

    stbi_image_free(tex->data);
    tex->data = nullptr;
    tex->width = 0;
    tex->height = 0;
    tex->channels = 0;

    return true;
}

TextureResource::TextureResource(const String &name, const IO::Uri &uri) :
        TResource(name, uri),
        m_targetType(TextureTargetType::Texture2D),
        m_stage(TextureStageType::TextureStage0) {
    // empty
}

void TextureResource::setTargetType(TextureTargetType targetType) {
    m_targetType = targetType;
}

TextureTargetType TextureResource::getTargetType() const {
    return m_targetType;
}

void TextureResource::setTextureStage(TextureStageType stage) {
    m_stage = stage;
}

TextureStageType TextureResource::setTextureStage() const {
    return m_stage;
}

ResourceState TextureResource::onLoad(const IO::Uri &uri, TextureLoader &loader) {
    if (getState() == ResourceState::Loaded) {
        return getState();
    }

    Texture *tex = create();
    if (nullptr == tex) {
        return ResourceState::Error;
    }

    tex->textureName = getName();
    if (tex->textureName.find("$default") != String::npos) {
        tex = TextureLoader::getDefaultTexture();
        setState(ResourceState::Loaded);
        return getState();
    }

    getStats().m_memory = loader.load(uri, tex);
    tex->targetType = m_targetType;
    if (0 == getStats().m_memory) {
        setState(ResourceState::Error);
        osre_debug(Tag, "Cannot load texture " + uri.getAbsPath());
        return getState();
    }

    setState(ResourceState::Loaded);

    return getState();
}

ResourceState TextureResource::onUnload(TextureLoader &loader) {
    if (getState() == ResourceState::Unloaded) {
        return getState();
    }

    loader.unload(get());
    getStats().m_memory = 0;
    setState(ResourceState::Unloaded);

    return getState();
}

TransformState::TransformState() :
        translate(1.0f),
        scale(1.0f),
        rotation(1.0f) {
    // empty
}

TransformState::~TransformState() {
    // empty
}

void TransformState::setTranslation(f32 x, f32 y, f32 z) {
    translate.x = x;
    translate.y = y;
    translate.z = z;
}

void TransformState::setScale(f32 sx, f32 sy, f32 sz) {
    scale.x = sx;
    scale.y = sy;
    scale.z = sz;
}

bool TransformState::operator==(const TransformState &rhs) const {
    return (translate == rhs.translate && scale == rhs.scale &&
            rotation == rhs.rotation);
}

bool TransformState::operator!=(const TransformState &rhs) const {
    return !(*this == rhs);
}

void TransformState::toMatrix(mat4 &m) const {
    m *= glm::translate(m, translate);
    m *= glm::scale(m, scale);
    m *= rotation;
}

Viewport::Viewport() :
        x(-1), y(-1), w(-1), h(-1) {
    // empty
}

Viewport::Viewport(i32 x_, i32 y_, i32 w_, i32 h_) :
        x(x_), y(y_), w(w_), h(h_) {
    // empty
}

Viewport::Viewport(const Viewport &rhs) :
        x(rhs.x), y(rhs.y), w(rhs.w), h(rhs.h) {
    // empty
}
        
bool Viewport::operator==(const Viewport &rhs) const {
    return (x == rhs.x && y == rhs.y && w == rhs.w && h == rhs.h);
}

bool Viewport::operator!=(const Viewport &rhs) const {
    return !(*this == rhs);
}

Light::Light() :
        position(0.0f, 0.0f, 0.0f, 1.0f),
        specular(1.0f, 1.0f, 1.0f),
        diffuse(1.0f, 1.0f, 1.0f),
        ambient(1.0f, 1.0f, 1.0f),
        direction(0.0f, 0.0f, 1.0f, 1.0f),
        specularExp(1.0f),
        radius(1.0f),
        type(LightType::Invalid) {
    // empty
}

MeshEntry *RenderBatchData::getMeshEntryByName(const c8 *name) {
    if (nullptr == name) {
        return nullptr;
    }

    for (auto &i : meshArray) {
        for (ui32 j = 0; j < i->mMeshArray.size(); ++j) {
            if (i->mMeshArray[j]->getName() == name) {
                return i;
            }
        }
    }

    return nullptr;
}

UniformVar *RenderBatchData::getVarByName(const c8 *name) {
    if (nullptr == name) {
        return nullptr;
    }

    for (auto &uniform : uniforms) {
        if (uniform->name == name) {
            return uniform;
        }
    }

    return nullptr;
}

RenderBatchData *PassData::getBatchById(const c8 *id_) const {
    if (nullptr == id_) {
        return nullptr;
    }

    for (ui32 i = 0; i < meshBatches.size(); ++i) {
        if (0 == strncmp(meshBatches[i]->id, id, strlen(id))) {
            return meshBatches[i];
        }
    }

    return nullptr;
}

static constexpr size_t MaxSubmitCmds = 500;

Frame::Frame() :
        newPasses(),
        submitCmds(),
        submitCmdAllocator(),
        uniforBuffers(nullptr),
        pipeline(nullptr) {
    submitCmdAllocator.reserve(MaxSubmitCmds);
}

Frame::~Frame() {
    delete[] uniforBuffers;
}

void Frame::init(TArray<PassData*> &newPasses_) {
    if (newPasses.isEmpty()) {
        return;
    }

    for (auto newPasse : newPasses_) {
        newPasses.add(newPasse);
    }
    uniforBuffers = new UniformBuffer[newPasses.size()];
}

FrameSubmitCmd *Frame::enqueue(const char *passId, const char *batchId) {
    if (passId == nullptr || batchId == nullptr) {
        return nullptr;
    }
    
    FrameSubmitCmd *cmd = submitCmdAllocator.alloc();
    if (nullptr != cmd) {
        cmd->passId = passId;
        cmd->batchId = batchId;
        submitCmds.add(cmd);
    }

    return cmd;
}

UniformDataBlob::UniformDataBlob() :
        data(nullptr),
        size(0) {
    // empty
}

UniformDataBlob::~UniformDataBlob() {
    clear();
}

void *UniformDataBlob::getData() const {
    return data;
}

void UniformDataBlob::clear() {
    ::free(data);
    data = nullptr;
    size = 0;
}

UniformDataBlob *UniformDataBlob::create(ParameterType type, size_t arraySize) {
    UniformDataBlob *blob = new UniformDataBlob;
    switch (type) {
        case ParameterType::PT_Int:
            blob->size = sizeof(i32);
            break;
        case ParameterType::PT_Float:
            blob->size = sizeof(f32);
            break;
        case ParameterType::PT_Float2:
            blob->size = sizeof(f32) * 2;
            break;
        case ParameterType::PT_Float3:
            blob->size = sizeof(f32) * 3;
            break;
        case ParameterType::PT_Mat4:
            blob->size = sizeof(f32) * 16;
            break;
        case ParameterType::PT_Mat4Array:
            blob->size = sizeof(f32) * 16 * arraySize;
            break;
        default:
            blob->size = 0;
            break;
    }
    blob->data = ::malloc(blob->size);
    ::memset(blob->data, 0, blob->size);

    return blob;
}

UniformVar::UniformVar() :
        name(""),
        type(ParameterType::PT_None),
        numItems(1),
        next(nullptr) {
    // empty
}

ui32 UniformVar::getParamDataSize(ParameterType type, ui32 arraySize) {
    ui32 size = 0u;
    switch (type) {
        case ParameterType::PT_Int:
            size = sizeof(i32);
            break;
        case ParameterType::PT_Float:
            size = sizeof(f32);
            break;
        case ParameterType::PT_Float2:
            size = sizeof(f32) * 2;
            break;
        case ParameterType::PT_Float3:
            size = sizeof(f32) * 3;
            break;
        case ParameterType::PT_Mat4:
            size = sizeof(f32) * 16;
            break;
        case ParameterType::PT_Mat4Array:
            size = sizeof(f32) * 16 * arraySize;
            break;

        default:
            break;
    }

    return size;
}

UniformVar *UniformVar::create(const String &name, ParameterType type, ui32 arraySize) {
    if (name.empty()) {
        osre_debug(Tag, "Empty name for parameter.");
        return nullptr;
    }

    UniformVar *param = new UniformVar;
    param->name = name;
    param->type = type;
    param->numItems = arraySize;
    param->data.size = UniformVar::getParamDataSize(type, arraySize);
    param->data.data = new uc8[param->data.size];
    param->next = nullptr;
    ::memset(param->data.data, 0, param->data.size);

    return param;
}

void UniformVar::destroy(UniformVar *param) {
    if (nullptr != param) {
        delete param;
    }
}

size_t UniformVar::size() const {
    // len of name | name | buffer
    return name.size() + 1 + data.size;
}

static const c8 *GlslVersionStringArray[(size_t)GLSLVersion::Count] = {
    "1.10",
    "1.20",
    "1.30",
    "1.40",
    "1.50",
    "3.30",
    "4.00",
    "4.10",
    "4.20",
    "4.30",
    "4.40",
    "4.50",
    "4.60"
};

GLSLVersion getGlslVersionFromeString(const c8 *versionString) {
    GLSLVersion version = GLSLVersion::Invalid;
    if (versionString == nullptr) {
        return version;
    }

    std::string str(versionString);
    for (size_t i = 0; i < (size_t)GLSLVersion::Count; ++i) {
        const String v = GlslVersionStringArray[i];
        std::string::size_type pos = str.find(v);
        if (pos != std::string::npos) {
            version = static_cast<GLSLVersion>(i);
            break;
        }
    }

    return version;
}

} // Namespace RenderBackend
} // Namespace OSRE

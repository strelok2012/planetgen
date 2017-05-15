/****************************************************************************
 Copyright (c) 2014-2017 Chukong Technologies Inc.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "TriangleDraw.h"

NS_CC_BEGIN

static Vec2 v2fzero(0.0f, 0.0f);

static inline Tex2F __t(const Vec2 &v) {
    return *(Tex2F*) & v;
}

TriangleDraw::TriangleDraw()
: _vao(0)
, _vbo(0)
, _bufferCapacity(0)
, _bufferCount(0)
, _buffer(nullptr)
, _dirty(false) {
    _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
}

TriangleDraw::~TriangleDraw() {
    free(_buffer);
    _buffer = nullptr;

    glDeleteBuffers(1, &_vbo);
    _vbo = 0;

    if (Configuration::getInstance()->supportsShareableVAO()) {
        glDeleteVertexArrays(1, &_vao);
        GL::bindVAO(0);
        _vao = 0;
    }
}

TriangleDraw* TriangleDraw::create() {
    TriangleDraw* ret = new (std::nothrow) TriangleDraw();
    if (ret && ret->init()) {
        ret->autorelease();
    } else {
        CC_SAFE_DELETE(ret);
    }

    return ret;
}

void TriangleDraw::ensureCapacity(int count) {
    CCASSERT(count >= 0, "capacity must be >= 0");

    if (_bufferCount + count > _bufferCapacity) {
        _bufferCapacity += MAX(_bufferCapacity, count);
        _buffer = (V2F_C4B*) realloc(_buffer, _bufferCapacity * sizeof (V2F_C4B));
    }
}

bool TriangleDraw::init() {
    _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;

    setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_COLOR));

    ensureCapacity(512);

    if (Configuration::getInstance()->supportsShareableVAO()) {
        glGenVertexArrays(1, &_vao);
        GL::bindVAO(_vao);
    }

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof (V2F_C4B) * _bufferCapacity, _buffer, GL_STREAM_DRAW);

    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof (V2F_C4B), 0);

    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (V2F_C4B), (GLvoid *) offsetof(V2F_C4B, colors));

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (Configuration::getInstance()->supportsShareableVAO()) {
        GL::bindVAO(0);
    }

    CHECK_GL_ERROR_DEBUG();

    _dirty = true;

#if CC_ENABLE_CACHE_TEXTURE_DATA
    // Need to listen the event only when not use batchnode, because it will use VBO
    auto listener = EventListenerCustom::create(EVENT_COME_TO_FOREGROUND, [this](EventCustom * event) {
        /** listen the event that coming to foreground on Android */
        this->init();
    });

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
#endif

    return true;
}

void TriangleDraw::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) {
    _customCommand.init(_globalZOrder, transform, flags);
    _customCommand.func = CC_CALLBACK_0(TriangleDraw::onDraw, this, transform, flags);
    renderer->addCommand(&_customCommand);
}

void TriangleDraw::onDraw(const Mat4 &transform, uint32_t flags) {
    getGLProgramState()->apply(transform);
    auto glProgram = this->getGLProgram();
    glProgram->setUniformLocationWith1f(glProgram->getUniformLocation("u_alpha"), _displayedOpacity / 255.0);
    GL::blendFunc(_blendFunc.src, _blendFunc.dst);

    if (_dirty) {
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof (V2F_C4B) * _bufferCapacity, _buffer, GL_STREAM_DRAW);
        _dirty = false;
    }
    if (Configuration::getInstance()->supportsShareableVAO()) {
        GL::bindVAO(_vao);
    } 
    glDrawArrays(GL_TRIANGLES, 0, _bufferCount);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _bufferCount);
    CHECK_GL_ERROR_DEBUG();
}

void TriangleDraw::drawTriangle(std::vector<Vec2Color> &triangle) {
    unsigned int vertex_count = 3;
    ensureCapacity(vertex_count);

    STA_Triangle *triangles = (STA_Triangle *) (_buffer + _bufferCount);
    STA_Triangle *cursor = triangles;

    STA_Triangle tmp = {
        {triangle[0].vector, Color4B(triangle[0].color)},
        {triangle[1].vector, Color4B(triangle[1].color)},
        {triangle[2].vector, Color4B(triangle[2].color)},
    };

    *cursor++ = tmp;

    /* V3F_C4B a = {Vec3(triangle[0].vector.x, triangle[0].vector.y, 0), Color4B(triangle[0].color)};
     V3F_C4B b = {Vec3(triangle[1].vector.x, triangle[1].vector.y, 0), Color4B(triangle[1].color)};
     V3F_C4B c = {Vec3(triangle[2].vector.x, triangle[2].vector.y, 0), Color4B(triangle[2].color)};

     V3F_C4B *lines = (V3F_C4B *) (_buffer + _bufferCount);
     lines[0] = a;
     lines[1] = b;
     lines[2] = c;*/

    _bufferCount += vertex_count;
    _dirty = true;

}

void TriangleDraw::clear() {
    _bufferCount = 0;
    _dirty = true;
}

const BlendFunc& TriangleDraw::getBlendFunc() const {
    return _blendFunc;
}

void TriangleDraw::setBlendFunc(const BlendFunc &blendFunc) {
    _blendFunc = blendFunc;
}

NS_CC_END

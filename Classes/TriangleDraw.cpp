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

bool TriangleDraw::init() {
    _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;

    setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_COLOR));

    if (Configuration::getInstance()->supportsShareableVAO()) {
        glGenVertexArrays(1, &_vao);
        GL::bindVAO(_vao);
    }

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof (V2F_C4B) * _buffer2.size(), _buffer2.data(), GL_STREAM_DRAW);

    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof (V2F_C4B), 0);

    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (V2F_C4B), (GLvoid *) offsetof(V2F_C4B, colors));


    if (Configuration::getInstance()->supportsShareableVAO()) {
        glGenVertexArrays(1, &_vaoVertex);
        GL::bindVAO(_vaoVertex);
    }

    glGenBuffers(1, &_vboVertex);
    glBindBuffer(GL_ARRAY_BUFFER, _vboVertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof (V2F_C4B) * _vertexBuffer.size(), _vertexBuffer.data(), GL_STREAM_DRAW);

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
        glBufferData(GL_ARRAY_BUFFER, sizeof (V2F_C4B) * _buffer2.size(), _buffer2.data(), GL_STREAM_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, _vboVertex);
        glBufferData(GL_ARRAY_BUFFER, sizeof (V2F_C4B) * _vertexBuffer.size(), _vertexBuffer.data(), GL_STREAM_DRAW);
        _dirty = false;
    }
    if (Configuration::getInstance()->supportsShareableVAO()) {
        GL::bindVAO(_vao);
    } else {
        GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        // vertex
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof (V2F_C4B), 0);

        // color
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (V2F_C4B), (GLvoid *) offsetof(V2F_C4B, colors));
    }

    glDrawArrays(GL_TRIANGLES, 0, _buffer2.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _buffer2.size());


    if (_dirtyPoints) {
        glBindBuffer(GL_ARRAY_BUFFER, _vboVertex);
        glBufferData(GL_ARRAY_BUFFER, sizeof (V2F_C4B) * _vertexBuffer.size(), _vertexBuffer.data(), GL_STREAM_DRAW);
        _dirtyPoints = false;
    }
    if (Configuration::getInstance()->supportsShareableVAO()) {
        GL::bindVAO(_vaoVertex);
    } else {
        GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

        glBindBuffer(GL_POINTS, _vboVertex);
        // vertex
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof (V2F_C4B), 0);

        // color
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (V2F_C4B), (GLvoid *) offsetof(V2F_C4B, colors));
    }
    
    glPointSize(3.0f);
    glDrawArrays(GL_POINTS, 0, _vertexBuffer.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _vertexBuffer.size());
    CHECK_GL_ERROR_DEBUG();



    CHECK_GL_ERROR_DEBUG();
}

void TriangleDraw::drawAllTriangles(std::vector<std::vector<Vec2Color>> &triangles) {
    for (auto& triangle : triangles) {
        _buffer2.emplace_back(V2F_C4B{triangle[0].vector, Color4B(triangle[0].color)});
        _buffer2.emplace_back(V2F_C4B{triangle[1].vector, Color4B(triangle[1].color)});
        _buffer2.emplace_back(V2F_C4B{triangle[2].vector, Color4B(triangle[2].color)});

        _vertexBuffer.emplace_back(V2F_C4B{triangle[0].vector, Color4B(255, 0, 0, 255)});
        _vertexBuffer.emplace_back(V2F_C4B{triangle[1].vector, Color4B(255, 255, 255, 255)});
        _vertexBuffer.emplace_back(V2F_C4B{triangle[2].vector, Color4B(255, 255, 255, 255)});
    }

    _dirty = true;
    _dirtyPoints = true;

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

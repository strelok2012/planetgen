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

#include "DrawNode3D.h"

NS_CC_BEGIN


DrawNode3D::DrawNode3D()
: _vao(0)
, _vbo(0)
, _dirty(false)
, _dirtyPoints(false){
    _blendFunc = BlendFunc::DISABLE;
}

DrawNode3D::~DrawNode3D() {
    glDeleteBuffers(1, &_vbo);
    _vbo = 0;

    if (Configuration::getInstance()->supportsShareableVAO()) {
        glDeleteVertexArrays(1, &_vao);
        GL::bindVAO(0);
        _vao = 0;
    }
}

DrawNode3D* DrawNode3D::create() {
    DrawNode3D* ret = new (std::nothrow) DrawNode3D();
    if (ret && ret->init()) {
        ret->autorelease();
    } else {
        CC_SAFE_DELETE(ret);
    }

    return ret;
}

bool DrawNode3D::init() {
    _blendFunc = BlendFunc::ADDITIVE;

    setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_COLOR));

    if (Configuration::getInstance()->supportsShareableVAO()) {
        glGenVertexArrays(1, &_vao);
        GL::bindVAO(_vao);
    }

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof (V3F_C4B) * _buffer.size(), _buffer.data(), GL_STREAM_DRAW);

    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof (V3F_C4B), (GLvoid *) offsetof(V3F_C4B, vertices));

    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (V3F_C4B), (GLvoid *) offsetof(V3F_C4B, colors));

    if (Configuration::getInstance()->supportsShareableVAO()) {
        glGenVertexArrays(1, &_vaoVertex);
        GL::bindVAO(_vaoVertex);
    }


    glGenBuffers(1, &_vboVertex);
    glBindBuffer(GL_ARRAY_BUFFER, _vboVertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof (V3F_C4B) * _vertexBuffer.size(), _vertexBuffer.data(), GL_STREAM_DRAW);

    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof (V3F_C4B), (GLvoid *) offsetof(V3F_C4B, vertices));

    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (V3F_C4B), (GLvoid *) offsetof(V3F_C4B, colors));


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

void DrawNode3D::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) {
    _customCommand.init(_globalZOrder, transform, flags);
    _customCommand.func = CC_CALLBACK_0(DrawNode3D::onDraw, this, transform, flags);
    renderer->addCommand(&_customCommand);
}

void DrawNode3D::onDraw(const Mat4 &transform, uint32_t flags) {
    auto glProgram = getGLProgram();
    glProgram->use();
    glProgram->setUniformsForBuiltins(transform);
    //glEnable(GL_DEPTH_TEST);
    //RenderState::StateBlock::_defaultState->setDepthTest(true);
    GL::blendFunc(_blendFunc.src, _blendFunc.dst);

    if (_dirty) {
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof (V3F_C4B) * _buffer.size(), _buffer.data(), GL_STREAM_DRAW);
        _dirty = false;
    }
    if (Configuration::getInstance()->supportsShareableVAO()) {
        GL::bindVAO(_vao);
    } else {
        GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        // vertex
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof (V3F_C4B), (GLvoid *) offsetof(V3F_C4B, vertices));

        // color
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (V3F_C4B), (GLvoid *) offsetof(V3F_C4B, colors));
    }

    glDrawArrays(GL_LINES, 0, _buffer.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _buffer.size());
    CHECK_GL_ERROR_DEBUG();


    if (_dirtyPoints) {
        glBindBuffer(GL_ARRAY_BUFFER, _vboVertex);
        glBufferData(GL_ARRAY_BUFFER, sizeof (V3F_C4B) * _vertexBuffer.size(), _vertexBuffer.data(), GL_STREAM_DRAW);
        _dirtyPoints = false;
    }
    if (Configuration::getInstance()->supportsShareableVAO()) {
        GL::bindVAO(_vaoVertex);
    } else {
        GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POSITION | GL::VERTEX_ATTRIB_FLAG_COLOR);

        glBindBuffer(GL_POINTS, _vboVertex);
        // vertex
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof (V3F_C4B), (GLvoid *) offsetof(V3F_C4B, vertices));

        // color
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (V3F_C4B), (GLvoid *) offsetof(V3F_C4B, colors));
    }
glMatrixMode(GL_MODELVIEW);
    glPointSize(30.0f);
    glDrawArrays(GL_POINTS, 0, _vertexBuffer.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _vertexBuffer.size());
    CHECK_GL_ERROR_DEBUG();
}

void DrawNode3D::drawLine(const Vec3 &from, const Vec3 &to, const Color4F &color) {

    Color4B col = Color4B(color);
    V3F_C4B a = {Vec3(from.x, from.y, from.z), Color4B(255, 255, 255, 255)};
    V3F_C4B b = {Vec3(to.x, to.y, to.z), Color4B(0, 0, 255, 255)};

   /* _buffer.emplace_back(a);
    _buffer.emplace_back(b);
    _dirty = true;*/
    
    _vertexBuffer.emplace_back(a);
    //_vertexBuffer.emplace_back(b);
    _dirtyPoints = true;

}


void DrawNode3D::drawCube(Vec3* vertices, const Color4F &color) {
    // front face
    drawLine(vertices[0], vertices[1], color);
    drawLine(vertices[1], vertices[2], color);
    drawLine(vertices[2], vertices[3], color);
    drawLine(vertices[3], vertices[0], color);

    // back face
    drawLine(vertices[4], vertices[5], color);
    drawLine(vertices[5], vertices[6], color);
    drawLine(vertices[6], vertices[7], color);
    drawLine(vertices[7], vertices[4], color);

    // edge
    drawLine(vertices[0], vertices[7], color);
    drawLine(vertices[1], vertices[6], color);
    drawLine(vertices[2], vertices[5], color);
    drawLine(vertices[3], vertices[4], color);
}

void DrawNode3D::clear() {
    _dirty = true;
}

const BlendFunc& DrawNode3D::getBlendFunc() const {
    return _blendFunc;
}

void DrawNode3D::setBlendFunc(const BlendFunc &blendFunc) {
    _blendFunc = blendFunc;
}

NS_CC_END

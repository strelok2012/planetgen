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

#ifndef __TRIANGLE_DRAW_H__
#define __TRIANGLE_DRAW_H__

#include "cocos2d.h"

struct Vec2Color {
    cocos2d::Vec2 vector;
    cocos2d::Color4F color;
};

NS_CC_BEGIN



/**
 * Copy DrawNode for 3D geometry drawing.
 */
class TriangleDraw : public cocos2d::Node {
public:
    /** creates and initialize a TriangleDraw node */
    static cocos2d::TriangleDraw* create();
    void drawAllTriangles(std::vector<std::vector<Vec2Color>> &triangles);

    /** Clear the geometry in the node's buffer. */
    void clear();

    /**
     * @js NA
     * @lua NA
     */
    const BlendFunc& getBlendFunc() const;

    /**
     * @code
     * When this function bound into js or lua,the parameter will be changed
     * In js: var setBlendFunc(var src, var dst)
     * @endcode
     * @lua NA
     */
    void setBlendFunc(const BlendFunc &blendFunc);

    void onDraw(const cocos2d::Mat4& transform, uint32_t flags);

    // Overrides
    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;


CC_CONSTRUCTOR_ACCESS:
    TriangleDraw();
    virtual ~TriangleDraw();
    virtual bool init() override;

protected:

    struct V2F_C4B {
        cocos2d::Vec2 vertices;
        Color4B colors;
    };

    struct STA_Triangle {
        V2F_C4B a;
        V2F_C4B b;
        V2F_C4B c;
    };

    GLuint _vao;
    GLuint _vaoVertex;
    GLuint _vbo;
    GLuint _vboVertex;

    int _bufferCapacity;
    GLsizei _bufferCount;
    V2F_C4B* _buffer;

    std::vector<V2F_C4B> _buffer2;
    std::vector<V2F_C4B> _vertexBuffer;

    BlendFunc _blendFunc;
    cocos2d::CustomCommand _customCommand;

    cocos2d::CustomCommand _customCommandVertex;

    bool _dirty;
    bool _dirtyPoints;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(TriangleDraw);
};

NS_CC_END

#endif // __DRAW_NODE_3D_H__

#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "TriangleDraw.h"

class HelloWorld : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();

    cocos2d::DrawNode* draw2d;
    cocos2d::TriangleDraw* triangleDraw;

    std::random_device rd;

    std::vector<std::vector<Vec2Color>> allTriangles;
    std::vector<std::vector<Vec2Color>> allPolygons;

    int midpointMinIters = 1;
    int midpointMaxIters = 4;

    int midpointMinLength = 99999;
    int midpointMaxLength = 0;

    virtual bool init();

    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    void midpoint(std::vector<Vec2Color> &points, Vec2Color start, Vec2Color end, cocos2d::Vec2 normal, unsigned iters, float r = 0.5f);

    void drawTriangles(std::vector<Vec2Color> &points, Vec2Color verticle);
    void drawPolygons(std::vector<Vec2Color> &points, Vec2Color verticle);


    void drawAllPolygons();
    std::vector<Vec2Color> midpointDisplacement(Vec2Color &start, Vec2Color &end, float r, std::vector<cocos2d::Vec2> border);

    bool pointInTriangle(cocos2d::Vec2 pt, std::vector<cocos2d::Vec2> triangle);
    cocos2d::Vec2 getClosestPoint(cocos2d::Vec2 a, cocos2d::Vec2 b, cocos2d::Vec2 p);

    cocos2d::Vec2 getIntersectPoint(const cocos2d::Vec2& A, const cocos2d::Vec2& B, const cocos2d::Vec2& C, const cocos2d::Vec2& D);
    static bool pointsSort(cocos2d::Vec2 i, cocos2d::Vec2 j);

    cocos2d::Color4F colorAvg(cocos2d::Color4F cFirst, cocos2d::Color4F cSecond, cocos2d::Color4F cThird);
    cocos2d::Color4F colorAvg(cocos2d::Color4F cFirst, cocos2d::Color4F cSecond);
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);
};

#endif // __HELLOWORLD_SCENE_H__

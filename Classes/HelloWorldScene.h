#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class HelloWorld : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();

    cocos2d::DrawNode* draw2d;

    virtual bool init();

    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    void midpoint(std::vector<cocos2d::Vec2> &points, cocos2d::Vec2 start, cocos2d::Vec2 end, cocos2d::Vec2 normal, unsigned iters, float r = 0.5f);
    std::vector<cocos2d::Vec2> midpointDisplacement(cocos2d::Vec2 &start, cocos2d::Vec2 &end, float r, std::vector<cocos2d::Vec2> border);

    bool pointInTriangle(cocos2d::Vec2 pt, std::vector<cocos2d::Vec2> triangle);
    cocos2d::Vec2 getClosestPoint(cocos2d::Vec2 a, cocos2d::Vec2 b, cocos2d::Vec2 p); 

    cocos2d::Vec2 getIntersectPoint(const cocos2d::Vec2& A, const cocos2d::Vec2& B, const cocos2d::Vec2& C, const cocos2d::Vec2& D);
    static bool pointsSort(cocos2d::Vec2 i, cocos2d::Vec2 j);
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);
};

#endif // __HELLOWORLD_SCENE_H__

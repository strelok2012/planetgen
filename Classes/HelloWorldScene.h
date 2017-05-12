#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class HelloWorld : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    void midpoint(std::vector<cocos2d::Vec2> &points, unsigned iStart, unsigned iEnd, float r);
    std::vector<cocos2d::Vec2> midpointDisplacement(cocos2d::Vec2 &start, cocos2d::Vec2 &end, float r, float precision = 1.0f);
    static bool pointsSort(cocos2d::Vec2 i, cocos2d::Vec2 j);
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);
};

#endif // __HELLOWORLD_SCENE_H__

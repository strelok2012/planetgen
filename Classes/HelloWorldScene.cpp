#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "Icosphere.h"
#include "DrawNode3D.h"
#include "math/Vec3.h"
#include <vector>
#include <algorithm>

USING_NS_CC;

Scene* HelloWorld::createScene() {
    return HelloWorld::create();
}

// on "init" you need to initialize your instance

bool HelloWorld::init() {
    //////////////////////////////
    // 1. super init first
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);

    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    auto draw3d = DrawNode3D::create();
    draw3d->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
    this->addChild(draw3d, 2);


    CCLOG("Bunga");
    IndexedMesh icoV = Icosphere::make_icosphere(0);
    auto verticles = icoV.first;
    auto triangles = icoV.second;
    CCLOG("verticles %d", (int) verticles.size());
    CCLOG("triangles %d", (int) triangles.size());
    std::vector<cocos2d::Vec3> triangleCenters;
    int scale = 300;
    int index1 = 0;
    for (auto const& value : triangles) {

        int i1 = (int) value.vertex[0];
        int i2 = (int) value.vertex[1];
        int i3 = (int) value.vertex[2];

        //draw3d->drawLine(verticles[i1] * scale, verticles[i2] * scale, Color4F(1, 0, 0, 0.2f));
        //draw3d->drawLine(verticles[i2] * scale, verticles[i3] * scale, Color4F(1, 0, 0, 0.2f));
        //draw3d->drawLine(verticles[i3] * scale, verticles[i1] * scale, Color4F(1, 0, 0, 0.2f));

        CCLOG("Index:%d Triangle %d %d %d", index1, i1, i2, i3);
        auto centerX = (verticles[i1].x + verticles[i2].x + verticles[i3].x) / 3;
        auto centerY = (verticles[i1].y + verticles[i2].y + verticles[i3].y) / 3;
        auto centerZ = (verticles[i1].z + verticles[i2].z + verticles[i3].z) / 3;
        auto centerVec = Vec3(centerX, centerY, centerZ);
        triangleCenters.push_back(centerVec);
        index1++;
    }
    index1 = 0;
    std::vector<std::vector<int>> verticlesTriangles;
    for (unsigned i = 0; i < verticles.size(); i++) {
        verticlesTriangles.push_back({});
        for (auto const& triangle : triangles) {
            auto vertex = triangle.vertex;
            if (vertex[0] == i || vertex[1] == i || vertex[2] == i) {
                verticlesTriangles[i].push_back(index1);
            }
            index1++;
        }
        index1 = 0;
    }
    index1 = 0;
    for (auto const& verticleTriangle : verticlesTriangles) {
        cocos2d::Vec3 lastCenter = cocos2d::Vec3(0, 0, 0);
        std::vector<cocos2d::Vec3> currentCenters;
        for (unsigned i = 0; i < verticleTriangle.size(); i++) {
            currentCenters.push_back(triangleCenters[verticleTriangle[i]]);
        }

        CCLOG("Count %d", currentCenters.size());

        auto firstCenter = currentCenters[0];

        cocos2d::Vec3 minCenter = cocos2d::Vec3(9999, 9999, 9999);
        float dist = 999999.0f;
        for (unsigned i = 0; i < currentCenters.size(); i++) {
            if (firstCenter != currentCenters[i]) {
                auto distX = (firstCenter.x - currentCenters[i].x)*(firstCenter.x - currentCenters[i].x);
                auto distY = (firstCenter.y - currentCenters[i].y)*(firstCenter.y - currentCenters[i].y);
                auto distZ = (firstCenter.z - currentCenters[i].z)*(firstCenter.z - currentCenters[i].z);
                if (sqrtf(distX + distY + distZ) < dist) {
                    dist = sqrtf(distX + distY + distZ);
                    minCenter = currentCenters[i];
                }
            }

        }

        draw3d->drawLine(firstCenter * scale, minCenter * scale, Color4F(1, 0, 0, 0.2f));

        
        break;




        index1++;
    }
    auto rotation = RotateBy::create(30, Vec3(360, 0, 0));
    draw3d->runAction(RepeatForever::create(rotation));
    return true;
}

void HelloWorld::menuCloseCallback(Ref* pSender) {
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}

#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "Icosphere.h"
#include "DrawNode3D.h"
#include "3d/CCObjLoader.h"
#include <random>
#include <cmath>

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


    auto draw3d = DrawNode3D::create();
    draw3d->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));

    auto draw2d = DrawNode::create(0.1f);
    /*draw2d->setPosition(Vec2(origin.x,
            origin.y));*/
    //this->addChild(draw2d, 2);



    CCLOG("Bunga");
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::random_device rd; //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0, 1);

    std::string fullPath = FileUtils::getInstance()->fullPathForFilename("icosphere-3.obj");
    auto ret = tinyobj::LoadObj(shapes, materials, fullPath.c_str());
    int scale = 200;
    int uvSize = 512;
    auto uvColor = Color4F(1, 1, 1, 1);
    auto uvRadius = 3.0f;

    auto renderTexture = RenderTexture::create(uvSize, uvSize);
    renderTexture->beginWithClear(0, 0, 0, 0.5f); // black
    draw2d->retain();

    if (ret.empty()) {
        for (auto& shape : shapes) {
            auto mesh = shape.mesh;
            auto vertexNum = mesh.positions.size() / 3;
            std::vector<std::vector <std::pair < std::pair<cocos2d::Vec3, Color4F>, cocos2d::Vec2>>> allFaces;
            std::vector<std::pair < std::pair<cocos2d::Vec3, Color4F>, cocos2d::Vec2>> tmpFaces;
            for (unsigned int k = 0; k < vertexNum; ++k) {
                auto randColor = Color4F((float) dis(gen), (float) dis(gen), (float) dis(gen), 1);
                tmpFaces.push_back({
                    {cocos2d::Vec3(mesh.positions[k * 3], mesh.positions[k * 3 + 1], mesh.positions[k * 3 + 2]), randColor}, cocos2d::Vec2(mesh.texcoords[k * 2], mesh.texcoords[k * 2 + 1])
                });
                if (tmpFaces.size() == 3) {
                    allFaces.push_back(tmpFaces);
                    tmpFaces.clear();
                }
            }
            for (auto& face : allFaces) {
                //draw3d->drawLine(face[0].first.first * scale, face[1].first.first * scale, Color4F(0, 1, 0, 1));
                //draw3d->drawLine(face[1].first.first * scale, face[2].first.first * scale, Color4F(0, 1, 0, 1));
                //draw3d->drawLine(face[2].first.first * scale, face[0].first.first * scale, Color4F(0, 1, 0, 1));
                std::vector<cocos2d::Vec2> triangle = {face[0].second*uvSize, face[1].second*uvSize, face[2].second * uvSize};
                //draw2d->drawPoly(triangle.data(),triangle.size(),true,uvColor);

                //draw2d->drawDot(face[0].second*uvSize, uvRadius, face[0].first.second);
                //draw2d->drawDot(face[1].second*uvSize, uvRadius, face[0].first.second);
                //draw2d->drawDot(face[2].second*uvSize, uvRadius, face[0].first.second);
            }

        }
    } else {
        CCLOG("Error");
    }

    cocos2d::Vec2 left = cocos2d::Vec2(200, 500);
    cocos2d::Vec2 right = cocos2d::Vec2(600, 500);



    std::vector<cocos2d::Vec2> poly;
    poly.push_back(left);
    poly.push_back(right);
    //draw2d->drawPoly(poly.data(), poly.size(), false, uvColor);

    std::vector<cocos2d::Vec2> points;
    for (unsigned i = 0; i < 200; i++) {
        points.push_back(cocos2d::Vec2(200 + i, 0));
    }

    midpoint(points, 0, points.size(), 25, 7.5f);
    
    for (auto& point : points) {
        point.y+=300;
    }

    draw2d->drawPoly(points.data(), points.size(), false, uvColor);
    for (auto& point : points) {
        CCLOG("Point height %f", point.y);
    }

    CCLOG("MID %d", points.size());

    draw2d->visit();
    renderTexture->end();
    renderTexture->retain();
    renderTexture->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
    this->addChild(renderTexture, 2);

    auto rotation = RotateBy::create(30, Vec3(0, 360, 0));
    draw3d->runAction(RepeatForever::create(rotation));

    auto sprite = Sprite3D::create("icosphere-3.obj"); //c3b file, created with the FBX-converter
    renderTexture->setScale(-1.0f);
    sprite->setScale(100.f); //sets the object scale in float
    sprite->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
    auto textSprite = renderTexture->getSprite();
    sprite->setTexture(textSprite->getTexture());
    //sprite->setTexture("balltex.png");
    renderTexture->saveToFile("kungalai.png", Image::Format::PNG, true, [sprite] (RenderTexture* texture, const std::string & kunga) {
        CCLOG(kunga.c_str());
        sprite->setTexture("/home/strelok/.config/MyGame/kungalai.png");
    });
    //this->addChild(sprite, 3); //adds sprite to scene, z-index: 1
    //sprite->runAction(RepeatForever::create(rotation));

    return true;
}

void HelloWorld::midpoint(std::vector<cocos2d::Vec2> &points, unsigned iLeft, unsigned iRight, unsigned length, float r) {
    std::random_device rd; //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(-r * length, r * length);
    if (iRight - iLeft < 2) {
        return;
    }
    auto hl = points[iLeft].y; //высота левой точки
    auto hr = points[iRight].y; //высота правой
    auto h = (hl + hr) / 2 + dis(gen); //считаем высоту
    auto index = floor(iLeft + (iRight - iLeft) / 2); //ищем середину

    CCLOG("Mid point %d", (int)index);
    points[index].y = h;

    midpoint(points, iLeft, index, length / 2, r);
    midpoint(points, index, iRight, length / 2, r);
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

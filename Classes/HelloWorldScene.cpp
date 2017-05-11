#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "Icosphere.h"
#include "DrawNode3D.h"
#include "3d/CCObjLoader.h"
#include <random>

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

    auto draw2d = DrawNode::create();
    /*draw2d->setPosition(Vec2(origin.x,
            origin.y));*/
    //this->addChild(draw2d, 2);



    CCLOG("Bunga");
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::random_device rd; //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0, 1);

    std::string fullPath = FileUtils::getInstance()->fullPathForFilename("icosphere-5.obj");
    auto ret = tinyobj::LoadObj(shapes, materials, fullPath.c_str());
    int scale = 200;
    int uvSize = 1024;
    auto uvColor = Color4F(1, 1, 1, 1);
    auto uvRadius = 10.0f;

    auto renderTexture = RenderTexture::create(uvSize, uvSize);
    renderTexture->beginWithClear(0, 1, 0, 0.5f); // black
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
                draw3d->drawLine(face[0].first.first * scale, face[1].first.first * scale, Color4F(0, 1, 0, 1));
                draw3d->drawLine(face[1].first.first * scale, face[2].first.first * scale, Color4F(0, 1, 0, 1));
                draw3d->drawLine(face[2].first.first * scale, face[0].first.first * scale, Color4F(0, 1, 0, 1));

                draw2d->drawLine(face[0].second*uvSize, face[1].second*uvSize, uvColor);
                draw2d->drawLine(face[1].second*uvSize, face[2].second*uvSize, uvColor);
                draw2d->drawLine(face[0].second*uvSize, face[2].second*uvSize, uvColor);

                auto firstDot = face[0].second*uvSize;
                if (firstDot.x > uvSize || firstDot.y > uvSize) {
                    CCLOG("DOT POS %f %f", firstDot.x, firstDot.y);
                }

                draw2d->drawDot(face[0].second*uvSize, uvRadius, face[0].first.second);
                draw2d->drawDot(face[1].second*uvSize, uvRadius, face[0].first.second);
                draw2d->drawDot(face[2].second*uvSize, uvRadius, face[0].first.second);
            }

        }
    } else {
        CCLOG("Error");
    }

    draw2d->visit();
    renderTexture->end();
    renderTexture->retain();
    renderTexture->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
    //this->addChild(renderTexture, 2);

    auto rotation = RotateBy::create(30, Vec3(0, 360, 0));
    draw3d->runAction(RepeatForever::create(rotation));

    auto sprite = Sprite3D::create("icosphere-5.obj"); //c3b file, created with the FBX-converter
    sprite->setScale(100.f); //sets the object scale in float
    sprite->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
    sprite->setTexture(renderTexture->getSprite()->getTexture());
    //sprite->setTexture("balltex.png");
    renderTexture->saveToFile("kungalai.png", Image::Format::PNG, true, [sprite] (RenderTexture* texture, const std::string & kunga) {
        CCLOG(kunga.c_str());
        sprite->setTexture("/home/strelok/.config/MyGame/kungalai.png");
    });
    this->addChild(sprite, 3); //adds sprite to scene, z-index: 1
    sprite->runAction(RepeatForever::create(rotation));

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

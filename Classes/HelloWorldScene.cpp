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

    auto currentModel = "icosphere-3.obj";

    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(currentModel);
    auto ret = tinyobj::LoadObj(shapes, materials, fullPath.c_str());
    int scale = 200;
    int uvSize = 2048;
    auto uvColor = Color4F(1, 1, 1, 1);
    auto uvRadius = 3.0f;
    auto uvRoughness = 0.5f;
    auto uvPrecision = 3.0f;

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
                std::vector<cocos2d::Vec2> firstEdge;
                std::vector<cocos2d::Vec2> secondEdge;
                std::vector<cocos2d::Vec2> thirdEdge;

                cocos2d::Vec2 first = face[0].second*uvSize;
                cocos2d::Vec2 second = face[1].second*uvSize;
                cocos2d::Vec2 third = face[2].second*uvSize;


                std::vector<cocos2d::Vec2> uvTriangle = {face[0].second*uvSize, face[1].second*uvSize, face[2].second * uvSize};
                //draw2d->drawPoly(uvTriangle.data(), uvTriangle.size(), true, uvColor);


                cocos2d::Vec2 uvTriangleCenter = (face[0].second * uvSize + face[1].second * uvSize + face[2].second * uvSize) / 3;

                std::uniform_real_distribution<> angleDis(0, 2 * M_PI);

                auto lengthVector = second - first;
                auto circleRadius = sqrt(3) * lengthVector.length() / 6;
                std::uniform_real_distribution<> radiusDis(0, circleRadius);

                auto angle = angleDis(gen);
                auto radius = radiusDis(gen);
                CCLOG("%f %f %f", uvTriangleCenter.x, radius, angle);
                uvTriangleCenter.x = uvTriangleCenter.x + radius * sin(angle);
                uvTriangleCenter.y = uvTriangleCenter.y + radius * cos(angle);

                auto firstCenter = (first + second) / 2;
                auto secondCenter = (second + third) / 2;
                auto thirdCenter = (first + third) / 2;

                auto firstInnerLine = midpointDisplacement(firstCenter, uvTriangleCenter, uvRoughness, uvPrecision);
                auto secondInnerLine = midpointDisplacement(secondCenter, uvTriangleCenter, uvRoughness, uvPrecision);
                auto thirdInnerLine = midpointDisplacement(thirdCenter, uvTriangleCenter, uvRoughness, uvPrecision);

                draw2d->drawPoly(firstInnerLine.data(), firstInnerLine.size(), false, Color4F(0, 1, 0, 1));
                draw2d->drawPoly(secondInnerLine.data(), secondInnerLine.size(), false, Color4F(0, 1, 0, 1));
                draw2d->drawPoly(thirdInnerLine.data(), thirdInnerLine.size(), false, Color4F(0, 1, 0, 1));

                //draw2d->drawDot(uvTriangleCenter, 2.0f, Color4F(1, 1, 1, 1));
            }

        }
    } else {
        CCLOG("Error");
    }

    cocos2d::Vec2 start = cocos2d::Vec2(200, 200);
    cocos2d::Vec2 end = cocos2d::Vec2(200, 600);

    /*std::vector<cocos2d::Vec2> firstEdge;
    std::vector<cocos2d::Vec2> pts;
    pts = midpointDisplacement(start, end, 0.2f);
    draw2d->drawPoly(pts.data(), pts.size(), false, Color4F(1, 1, 1, 1));*/


    //std::vector<cocos2d::Vec2> original;
    // original.push_back(left);
    // original.push_back(right);
    //draw2d->drawPoly(original.data(), original.size(), false, Color4F(1, 0, 0, 1));


    /*
        midpoint(points, 0, points.size(), 25, 7.5f);
    
        for (auto& point : points) {
            point.y+=300;
        }

        draw2d->drawPoly(points.data(), points.size(), false, uvColor);
        for (auto& point : points) {
            CCLOG("Point height %f", point.y);
        }

        CCLOG("MID %d", points.size());*/

    draw2d->visit();
    renderTexture->end();
    renderTexture->retain();
    renderTexture->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
    //this->addChild(renderTexture, 2);

    auto rotation = RotateBy::create(30, Vec3(0, 360, 0));
    draw3d->runAction(RepeatForever::create(rotation));

    auto sprite = Sprite3D::create(currentModel); //c3b file, created with the FBX-converter
    renderTexture->setScale(-1.0f);
    sprite->setScale(200.f); //sets the object scale in float
    sprite->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
    auto textSprite = renderTexture->getSprite();
    sprite->setTexture(textSprite->getTexture());
    //sprite->setTexture("balltex.png");
    renderTexture->saveToFile("kungalai.png", Image::Format::PNG, true, [sprite] (RenderTexture* texture, const std::string & kunga) {
        CCLOG(kunga.c_str());
        sprite->setTexture("/home/strelok/.config/MyGame/kungalai.png");
    });
    this->addChild(sprite, 3); //adds sprite to scene, z-index: 1
    sprite->runAction(RepeatForever::create(rotation));

    return true;
}

bool HelloWorld::pointsSort(cocos2d::Vec2 i, cocos2d::Vec2 j) {
    return (i < j);
}

std::vector<cocos2d::Vec2> HelloWorld::midpointDisplacement(cocos2d::Vec2 &start, cocos2d::Vec2 &end, float r, float precision) {
    auto lengthVector = end - start;
    int length = (int) (lengthVector.length() / precision);

    if (length == 0) {
        return {start};
    }

    std::vector<cocos2d::Vec2> points(length);
    points[0] = start;
    points[points.size() - 1] = end;
    midpoint(points, 0, points.size() - 1, r);
    return points;
}

void HelloWorld::midpoint(std::vector<cocos2d::Vec2> &points, unsigned iStart, unsigned iEnd, float r) {
    if (iEnd - iStart < 2) {
        return;
    }
    auto start = points[iStart];
    auto end = points[iEnd];

    auto lengthVector = end - start;
    auto length = lengthVector.length();
    std::random_device rd; //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(-r*length, r * length);


    auto mid = (start + end) / 2;
    auto dx = lengthVector.x;
    auto dy = lengthVector.y;

    int idx = floor(iStart + iEnd) / 2;

    auto factor = dis(gen);
    auto normal1 = cocos2d::Vec2(dy, -dx);
    normal1.normalize();
    auto normal2 = cocos2d::Vec2(-dy, dx);
    normal2.normalize();
    cocos2d::Vec2 newPoint;
    newPoint = mid + factor*normal1;
    /*if (factor > 0) {
        newPoint = mid + factor*normal1;
    } else {
        newPoint = mid - factor*normal2;
    }*/

    points[idx] = newPoint;
    midpoint(points, iStart, idx, r);
    midpoint(points, idx, iEnd, r);
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

#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "Icosphere.h"
#include "DrawNode3D.h"
#include "3d/CCObjLoader.h"
#include <random>
#include <cmath>
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


    auto draw3d = DrawNode3D::create();
    draw3d->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));

    draw2d = DrawNode::create(0.1f);
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
    auto uvColor = Color4F(1, 1, 1, 0.4f);
    auto uvRadius = 3.0f;
    auto uvRoughness = 0.55f;
    auto uvPrecision = 3.5f;

    auto renderTexture = RenderTexture::create(uvSize, uvSize);
    renderTexture->beginWithClear(0, 0, 0, 0.5f); // black
    draw2d->retain();

    if (ret.empty()) {
        for (auto& shape : shapes) {
            auto mesh = shape.mesh;
            auto vertexNum = mesh.positions.size() / 3;
            std::vector<std::vector <std::pair < std::pair<cocos2d::Vec3, Color4F>, cocos2d::Vec2>>> allFaces;
            std::vector<std::pair < std::pair<cocos2d::Vec3, Color4F>, cocos2d::Vec2>> tmpFaces;
            std::vector<std::pair < cocos2d::Vec3, Color4F>> usedVerticles;
            for (unsigned int k = 0; k < vertexNum; ++k) {
                bool inUsed = false;
                auto randColor = Color4F((float) dis(gen), (float) dis(gen), (float) dis(gen), (float) dis(gen));
                for (auto& usedV : usedVerticles) {
                    if (usedV.first == cocos2d::Vec3(mesh.positions[k * 3], mesh.positions[k * 3 + 1], mesh.positions[k * 3 + 2])) {
                        randColor = usedV.second;
                        inUsed = true;
                    }
                }
                tmpFaces.push_back({
                    {cocos2d::Vec3(mesh.positions[k * 3], mesh.positions[k * 3 + 1], mesh.positions[k * 3 + 2]), randColor}, cocos2d::Vec2(mesh.texcoords[k * 2], mesh.texcoords[k * 2 + 1])
                });

                if (!inUsed) {
                    usedVerticles.push_back({cocos2d::Vec3(mesh.positions[k * 3], mesh.positions[k * 3 + 1], mesh.positions[k * 3 + 2]), randColor});
                }

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
                auto aLength = lengthVector.length();

                auto angle = angleDis(gen);

                //CCLOG("%f %f %f", uvTriangleCenter.x, radius, angle);

                auto checkVector = cocos2d::Vec2(uvTriangleCenter.x + aLength * sin(angle), uvTriangleCenter.y + aLength * cos(angle));

                auto checkFirst = getIntersectPoint(uvTriangle[0], uvTriangle[1], uvTriangleCenter, checkVector);
                auto checkSecond = getIntersectPoint(uvTriangle[1], uvTriangle[2], uvTriangleCenter, checkVector);
                auto checkThird = getIntersectPoint(uvTriangle[0], uvTriangle[2], uvTriangleCenter, checkVector);

                auto moveRadius = 0.f;
                auto moveVec = cocos2d::Vec2::ZERO;
                if (checkFirst != cocos2d::Vec2::ZERO) {
                    moveVec = checkFirst - uvTriangleCenter;
                    moveRadius = moveVec.length();
                } else if (checkSecond != cocos2d::Vec2::ZERO) {
                    moveVec = checkSecond - uvTriangleCenter;
                    moveRadius = moveVec.length();
                } else if (checkThird != cocos2d::Vec2::ZERO) {
                    moveVec = checkThird - uvTriangleCenter;
                    moveRadius = moveVec.length();
                }

                std::uniform_real_distribution<> radiusDis(0, 0.9f * moveRadius);
                auto radius = radiusDis(gen);

                uvTriangleCenter.x = uvTriangleCenter.x + radius * sin(angle);
                uvTriangleCenter.y = uvTriangleCenter.y + radius * cos(angle);

                auto firstCenter = (first + second) / 2;
                auto secondCenter = (second + third) / 2;
                auto thirdCenter = (first + third) / 2;

                auto firstInnerLine = midpointDisplacement(firstCenter, uvTriangleCenter, uvRoughness, uvTriangle);
                auto secondInnerLine = midpointDisplacement(secondCenter, uvTriangleCenter, uvRoughness, uvTriangle);
                auto thirdInnerLine = midpointDisplacement(thirdCenter, uvTriangleCenter, uvRoughness, uvTriangle);

                //draw2d->drawPoly(firstInnerLine.data(), firstInnerLine.size(), false, Color4F(1, 0, 0, 1));
                //draw2d->drawPoly(secondInnerLine.data(), secondInnerLine.size(), false, Color4F(0, 1, 0, 1));
                //draw2d->drawPoly(thirdInnerLine.data(), thirdInnerLine.size(), false, Color4F(0, 0, 1, 1));

                std::vector<cocos2d::Vec2> triangleFan;

                for (unsigned i = 0; i < firstInnerLine.size() - 1; i++) {
                    std::vector<cocos2d::Vec2> triangle = {first, firstInnerLine[i], firstInnerLine[i + 1]};
                    draw2d->drawPolygon(triangle.data(), triangle.size(), face[0].first.second, 0, Color4F(1, 1, 0, 0));
                }

                for (unsigned i = 0; i < thirdInnerLine.size() - 1; i++) {
                    std::vector<cocos2d::Vec2> triangle = {first, thirdInnerLine[i], thirdInnerLine[i + 1]};
                    draw2d->drawPolygon(triangle.data(), triangle.size(), face[0].first.second, 0, Color4F(1, 1, 0, 0));
                }

                for (unsigned i = 0; i < secondInnerLine.size() - 1; i++) {
                    std::vector<cocos2d::Vec2> triangle = {second, secondInnerLine[i], secondInnerLine[i + 1]};
                    draw2d->drawPolygon(triangle.data(), triangle.size(), face[1].first.second, 0, Color4F(1, 1, 0, 0));
                }

                for (unsigned i = 0; i < firstInnerLine.size() - 1; i++) {
                    std::vector<cocos2d::Vec2> triangle = {second, firstInnerLine[i], firstInnerLine[i + 1]};
                    draw2d->drawPolygon(triangle.data(), triangle.size(), face[1].first.second, 0, Color4F(1, 1, 0, 0));
                }

                for (unsigned i = 0; i < thirdInnerLine.size() - 1; i++) {
                    std::vector<cocos2d::Vec2> triangle = {third, thirdInnerLine[i], thirdInnerLine[i + 1]};
                    draw2d->drawPolygon(triangle.data(), triangle.size(), face[2].first.second, 0, Color4F(1, 1, 0, 0));
                }

                for (unsigned i = 0; i < secondInnerLine.size() - 1; i++) {
                    std::vector<cocos2d::Vec2> triangle = {third, secondInnerLine[i], secondInnerLine[i + 1]};
                    draw2d->drawPolygon(triangle.data(), triangle.size(), face[2].first.second, 0, Color4F(1, 1, 0, 0));
                }








                /*for (unsigned i = 0; i < secondInnerLine.size() - 1; i++) {
                    std::vector<cocos2d::Vec2> triangle = {second, secondInnerLine[i], secondInnerLine[i + 1]};
                    draw2d->drawPolygon(triangle.data(), triangle.size(), face[1].first.second, 0, Color4F(1, 1, 0, 0));
                }

                for (unsigned i = 0; i < thirdInnerLine.size() - 1; i++) {
                    std::vector<cocos2d::Vec2> triangle = {second, thirdInnerLine[i], thirdInnerLine[i + 1]};
                    draw2d->drawPolygon(triangle.data(), triangle.size(), face[1].first.second, 0, Color4F(1, 1, 0, 0));
                }
                
                
                
                
                
                

                for (unsigned i = 0; i < thirdInnerLine.size() - 1; i++) {
                    std::vector<cocos2d::Vec2> triangle = {third, thirdInnerLine[i], thirdInnerLine[i + 1]};
                    draw2d->drawPolygon(triangle.data(), triangle.size(), face[2].first.second, 0, Color4F(1, 1, 0, 0));
                }*/

                //draw2d->drawDot(uvTriangleCenter, 2.0f, Color4F(1, 1, 1, 1));
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

cocos2d::Vec2 HelloWorld::getIntersectPoint(const cocos2d::Vec2& A, const cocos2d::Vec2& B, const cocos2d::Vec2& C, const cocos2d::Vec2& D) {
    float S, T;

    if (cocos2d::Vec2::isSegmentIntersect(A, B, C, D) && cocos2d::Vec2::isLineIntersect(A, B, C, D, &S, &T)) {
        // Vec2 of intersection
        cocos2d::Vec2 P;
        P.x = A.x + S * (B.x - A.x);
        P.y = A.y + S * (B.y - A.y);
        return P;
    }

    return cocos2d::Vec2::ZERO;
}

bool HelloWorld::pointsSort(cocos2d::Vec2 i, cocos2d::Vec2 j) {
    return (i < j);
}

std::vector<cocos2d::Vec2> HelloWorld::midpointDisplacement(cocos2d::Vec2 &start, cocos2d::Vec2 &end, float r, std::vector<cocos2d::Vec2> border) {
    auto lengthVector = end - start;
    int length = (int) lengthVector.length();

    if (length == 0) {
        return {start};
    }

    auto dx = lengthVector.x;
    auto dy = lengthVector.y;

    cocos2d::Vec2 normal = cocos2d::Vec2(-dy, dx);
    normal.normalize();

    std::vector<cocos2d::Vec2> points;
    points.push_back(start);
    midpoint(points, start, end, normal, 4, r);
    points.push_back(end);

    //setting verticles on enge if it out of triangle
    for (auto& point : points) {
        if (!pointInTriangle(point, border) && !point.equals(start)&& !point.equals(end)) {
            auto first = getClosestPoint(border[0], border[1], point);
            auto second = getClosestPoint(border[1], border[2], point);
            auto third = getClosestPoint(border[0], border[2], point);

            auto firstV = first - point;
            auto secondV = second - point;
            auto thirdV = third - point;

            if (firstV.length() < secondV.length() && firstV.length() < thirdV.length()) {
                point.x = first.x;
                point.y = first.y;
            } else if (secondV.length() < firstV.length() && secondV.length() < thirdV.length()) {
                point.x = second.x;
                point.y = second.y;
            } else if (thirdV.length() < firstV.length() && thirdV.length() < secondV.length()) {
                point.x = third.x;
                point.y = third.y;
            }
        }
    }
    return points;
}

void HelloWorld::midpoint(std::vector<cocos2d::Vec2> &points, cocos2d::Vec2 start, cocos2d::Vec2 end, cocos2d::Vec2 normal, unsigned iters, float r) {
    if (iters == 0) {
        return;
    }

    auto lengthVector = end - start;
    auto length = lengthVector.length();
    std::random_device rd; //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(-r*length, r * length);


    auto mid = (start + end) / 2;

    auto factor = dis(gen);
    cocos2d::Vec2 newPoint;
    newPoint = mid + factor*normal;

    midpoint(points, start, newPoint, normal, iters - 1, r);
    points.push_back(newPoint);
    midpoint(points, newPoint, end, normal, iters - 1, r);
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

float sign(cocos2d::Vec2 p1, cocos2d::Vec2 p2, cocos2d::Vec2 p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool HelloWorld::pointInTriangle(cocos2d::Vec2 pt, std::vector<cocos2d::Vec2> triangle) {
    bool b1, b2, b3;

    b1 = sign(pt, triangle[0], triangle[1]) <= 0.0f;
    b2 = sign(pt, triangle[1], triangle[2]) <= 0.0f;
    b3 = sign(pt, triangle[2], triangle[0]) <= 0.0f;

    return ((b1 == b2) && (b2 == b3));
}

cocos2d::Vec2 HelloWorld::getClosestPoint(cocos2d::Vec2 a, cocos2d::Vec2 b, cocos2d::Vec2 p) {
    auto v = a - b;
    v.normalize();
    return b + v * cocos2d::Vec2::dot(v, p - b);
}
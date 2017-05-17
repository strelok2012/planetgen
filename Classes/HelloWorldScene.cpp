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

struct midpointStruct {
    Vec2Color firstCenter;
    Vec2Color secondCenter;
    Vec2Color thirdCenter;
    Vec2Color centerColored;
    Vec2Color first;
    Vec2Color second;
    Vec2Color third;
    std::vector<cocos2d::Vec2> border;
};


// on "init" you need to initialize your instance

bool HelloWorld::init() {

    std::mt19937 gen(rd());
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




    draw2d = DrawNode::create(3.0f);
    /*draw2d->setPosition(Vec2(origin.x,
            origin.y));*/
    //this->addChild(draw2d, 2);

    triangleDraw = TriangleDraw::create();
    triangleDraw->setPosition(Vec2(origin.x,
            origin.y));
    // this->addChild(triangleDraw, 2);

    CCLOG("Bunga");
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    //Will be used to obtain a seed for the random number engine
    //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0.4f, 1);

    auto currentModel = "icosphere-3.obj";

    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(currentModel);
    auto ret = tinyobj::LoadObj(shapes, materials, fullPath.c_str());
    int scale = 200;
    int uvSize = 2048;
    auto uvColor = Color4F(1, 1, 1, 0.4f);
    auto uvRadius = 3.0f;
    auto uvRoughness = 0.3f;
    auto uvPrecision = 3.5f;

    auto sprite = Sprite3D::create(currentModel); //c3b file, created with the FBX-converter

    auto renderTexture = RenderTexture::create(uvSize, uvSize);
    auto renderTexture2 = RenderTexture::create(uvSize, uvSize);

    std::vector<midpointStruct> midpoints;

    std::vector<icoVertex> icoGraph;


    if (ret.empty()) {
        for (auto& shape : shapes) {
            auto mesh = shape.mesh;
            auto vertexNum = mesh.positions.size() / 3;
            CCLOG("VERTEX NUM %d", (int) vertexNum);
            std::vector<std::vector <std::pair < std::pair<cocos2d::Vec3, Color4F>, cocos2d::Vec2>>> allFaces;
            std::vector<std::pair < std::pair<cocos2d::Vec3, Color4F>, cocos2d::Vec2>> tmpFaces;
            std::vector<std::pair < std::pair<cocos2d::Vec3, Color4F>, cocos2d::Vec2>> usedVerticles;

            for (unsigned int k = 0; k < vertexNum; ++k) {
                bool inUsed = false;
                auto randColor = Color4F((float) dis(gen), (float) dis(gen), (float) dis(gen), (float) 1);
                for (auto& usedV : usedVerticles) {
                    if (usedV.first.first == cocos2d::Vec3(mesh.positions[k * 3], mesh.positions[k * 3 + 1], mesh.positions[k * 3 + 2])) {
                        randColor = usedV.first.second;
                        inUsed = true;
                    }
                }
                tmpFaces.push_back({
                    {cocos2d::Vec3(mesh.positions[k * 3], mesh.positions[k * 3 + 1], mesh.positions[k * 3 + 2]), randColor}, cocos2d::Vec2(mesh.texcoords[k * 2], mesh.texcoords[k * 2 + 1])
                });

                if (!inUsed) {
                    usedVerticles.push_back({
                        {cocos2d::Vec3(mesh.positions[k * 3], mesh.positions[k * 3 + 1], mesh.positions[k * 3 + 2]), randColor}, cocos2d::Vec2(mesh.texcoords[k * 2], mesh.texcoords[k * 2 + 1])
                    });
                }

                if (tmpFaces.size() == 3) {
                    allFaces.push_back(tmpFaces);
                    tmpFaces.clear();
                }
            }

            CCLOG("USED LENGTH %d", (int) usedVerticles.size());


            for (auto& uVert : usedVerticles) {
                icoVertex vert;
                vert.coords = uVert.first.first;
                vert.uvCoords = uVert.second;
                icoGraph.push_back(vert);
            }

            for (auto& face : allFaces) {
                unsigned first = findByCoords(icoGraph, face[0].first.first);
                unsigned second = findByCoords(icoGraph, face[1].first.first);
                unsigned third = findByCoords(icoGraph, face[2].first.first);

                if (std::find(icoGraph[first].neighbours.begin(), icoGraph[first].neighbours.end(), second) == icoGraph[first].neighbours.end()) {
                    icoGraph[first].neighbours.push_back(second);
                }

                if (std::find(icoGraph[first].neighbours.begin(), icoGraph[first].neighbours.end(), third) == icoGraph[first].neighbours.end()) {
                    icoGraph[first].neighbours.push_back(third);
                }


                if (std::find(icoGraph[second].neighbours.begin(), icoGraph[second].neighbours.end(), first) == icoGraph[second].neighbours.end()) {
                    icoGraph[second].neighbours.push_back(first);
                }

                if (std::find(icoGraph[second].neighbours.begin(), icoGraph[second].neighbours.end(), third) == icoGraph[second].neighbours.end()) {
                    icoGraph[second].neighbours.push_back(third);
                }

                if (std::find(icoGraph[third].neighbours.begin(), icoGraph[third].neighbours.end(), first) == icoGraph[third].neighbours.end()) {
                    icoGraph[third].neighbours.push_back(first);
                }

                if (std::find(icoGraph[third].neighbours.begin(), icoGraph[third].neighbours.end(), second) == icoGraph[third].neighbours.end()) {
                    icoGraph[third].neighbours.push_back(second);
                }

            }

            /*CCLOG("ICOGRAPG %d", (int) icoGraph.size());
            auto trianglesCount = icoGraph.size() / 3;
            for (unsigned int k = 0; k < trianglesCount; ++k) {
                icoGraph[k * 3].neighbours.push_back(icoGraph[k * 3 + 1]);
                icoGraph[k * 3].neighbours.push_back(icoGraph[k * 3 + 2]);

                icoGraph[k * 3 + 1].neighbours.push_back(icoGraph[k * 3]);
                icoGraph[k * 3 + 1].neighbours.push_back(icoGraph[k * 3 + 2]);

                icoGraph[k * 3 + 2].neighbours.push_back(icoGraph[k * 3]);
                icoGraph[k * 3 + 2].neighbours.push_back(icoGraph[k * 3 + 1]);
            }*/



            for (auto& vert : icoGraph) {
                CCLOG("NEIGHBOUR COUNT %d", (int) vert.neighbours.size());
                for (auto& neighbour : vert.neighbours) {
                    //draw2d->drawLine(vert.uvCoords*uvSize, icoGraph[neighbour].uvCoords*uvSize, uvColor);
                    draw3d->drawLine(vert.coords*scale,icoGraph[neighbour].coords*scale,Color4F(1,0,0,1));
                }
            }

            /* for (auto& face : allFaces) {
                 cocos2d::Vec2 first = face[0].second*uvSize;
                 cocos2d::Vec2 second = face[1].second*uvSize;
                 cocos2d::Vec2 third = face[2].second*uvSize;


                 std::vector<cocos2d::Vec2> uvTriangle = {face[0].second*uvSize, face[1].second*uvSize, face[2].second * uvSize};
                 draw2d->drawPoly(uvTriangle.data(), uvTriangle.size(), true, uvColor);


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

                 std::uniform_real_distribution<> radiusDis(0, 0.7f * moveRadius);
                 auto radius = radiusDis(gen);

                 uvTriangleCenter.x = uvTriangleCenter.x + radius * sin(angle);
                 uvTriangleCenter.y = uvTriangleCenter.y + radius * cos(angle);

                 midpoints.push_back({
                     {(first + second) / 2, colorAvg(face[0].first.second, face[1].first.second)},
                     {(second + third) / 2, colorAvg(face[1].first.second, face[2].first.second)},
                     {(first + third) / 2, colorAvg(face[0].first.second, face[2].first.second)},
                     {uvTriangleCenter, colorAvg(face[0].first.second, face[1].first.second, face[2].first.second)},
                     {first, face[0].first.second},
                     {second, face[1].first.second},
                     {third, face[2].first.second},
                     uvTriangle
                 });
             }*/

            for (auto& midpoint : midpoints) {
                auto firstV = midpoint.centerColored.vector - midpoint.firstCenter.vector;
                auto secondV = midpoint.centerColored.vector - midpoint.secondCenter.vector;
                auto thirdV = midpoint.centerColored.vector - midpoint.thirdCenter.vector;

                int firstLength = (int) firstV.length();
                int secondLength = (int) secondV.length();
                int thirdLength = (int) thirdV.length();

                if (firstLength > midpointMaxLength) {
                    midpointMaxLength = firstLength;
                }

                if (secondLength > midpointMaxLength) {
                    midpointMaxLength = secondLength;
                }

                if (thirdLength > midpointMaxLength) {
                    midpointMaxLength = thirdLength;
                }

                if (firstLength < midpointMinLength) {
                    midpointMinLength = firstLength;
                }

                if (secondLength < midpointMinLength) {
                    midpointMinLength = secondLength;
                }

                if (thirdLength < midpointMinLength) {
                    midpointMinLength = thirdLength;
                }




            }

            CCLOG("MAX LENGTH %d", midpointMaxLength);
            CCLOG("MIN LENGTH %d", midpointMinLength);

            for (auto& midpoint : midpoints) {
                auto firstInnerLine = midpointDisplacement(midpoint.firstCenter, midpoint.centerColored, uvRoughness, midpoint.border);
                auto secondInnerLine = midpointDisplacement(midpoint.secondCenter, midpoint.centerColored, uvRoughness, midpoint.border);
                auto thirdInnerLine = midpointDisplacement(midpoint.thirdCenter, midpoint.centerColored, uvRoughness, midpoint.border);

                drawTriangles(firstInnerLine, midpoint.first);
                drawTriangles(thirdInnerLine, midpoint.first);

                drawPolygons(firstInnerLine, midpoint.first);
                drawPolygons(thirdInnerLine, midpoint.first);



                drawTriangles(secondInnerLine, midpoint.second);
                drawTriangles(firstInnerLine, midpoint.second);

                drawPolygons(secondInnerLine, midpoint.second);
                drawPolygons(firstInnerLine, midpoint.second);

                drawTriangles(thirdInnerLine, midpoint.third);
                drawTriangles(secondInnerLine, midpoint.third);

                drawPolygons(thirdInnerLine, midpoint.third);
                drawPolygons(secondInnerLine, midpoint.third);
            }
            /*renderTexture->beginWithClear(0, 0, 0, 0.5f); // black
            triangleDraw->retain();
            triangleDraw->drawAllTriangles(allTriangles);
            triangleDraw->visit();
            renderTexture->end();
            renderTexture->retain();
             */

            renderTexture2->beginWithClear(0, 0, 0, 0.5f); // black
            draw2d->retain();
            //drawAllPolygons();
            draw2d->visit();
            renderTexture2->end();
            renderTexture2->retain();
        }
    } else {
        CCLOG("Error");
    }


    /*renderTexture->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));*/
    //this->addChild(renderTexture, 2);

    auto rotation = RotateBy::create(30, Vec3(0, 360, 0));
    draw3d->runAction(RepeatForever::create(rotation));
    this->addChild(draw3d,3);


    renderTexture->setScale(-1.0f);
    sprite->setScale(200.f); //sets the object scale in float
    sprite->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
    auto textSprite = renderTexture->getSprite();
    sprite->setTexture(textSprite->getTexture());
    //sprite->setTexture("balltex.png");
    /*renderTexture->saveToFile("kungalai.png", Image::Format::PNG, true, [sprite] (RenderTexture* texture, const std::string & kunga) {
        sprite->setTexture("/home/strelok/.config/MyGame/kungalai.png");
    });*/

    renderTexture2->saveToFile("kungalai_2.png", Image::Format::PNG, true, [sprite] (RenderTexture* texture, const std::string & kunga) {
        sprite->setTexture("/home/strelok/.config/MyGame/kungalai_2.png");
    });
    //this->addChild(sprite, 3); //adds sprite to scene, z-index: 1
    //sprite->runAction(RepeatForever::create(rotation));

    return true;
}

unsigned HelloWorld::findByCoords(std::vector<icoVertex> graph, cocos2d::Vec3 coords) {
    for (unsigned i = 0; i < graph.size(); i++) {
        if (graph[i].coords == coords) {
            return i;
        }
    }
    return 0;
}

cocos2d::Vec2 HelloWorld::getIntersectPoint(const cocos2d::Vec2& A, const cocos2d::Vec2& B, const cocos2d::Vec2& C, const cocos2d::Vec2 & D) {
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

std::vector<Vec2Color> HelloWorld::midpointDisplacement(Vec2Color &start, Vec2Color &end, float r, std::vector<cocos2d::Vec2> border) {
    auto lengthVector = end.vector - start.vector;
    int length = (int) lengthVector.length();

    int iters = (int) ceil(((float) length / midpointMaxLength) * midpointMaxIters);

    if (length == 0) {
        return {Vec2Color{start.vector, start.color}};
    }

    auto dx = lengthVector.x;
    auto dy = lengthVector.y;

    cocos2d::Vec2 normal = cocos2d::Vec2(-dy, dx);
    normal.normalize();

    std::vector<Vec2Color> points;
    points.push_back(start);
    midpoint(points, start, end, normal, iters, r);
    points.push_back(end);

    //setting verticles on enge if it out of triangle
    for (auto& point : points) {
        if (!pointInTriangle(point.vector, border) && !point.vector.equals(start.vector)&& !point.vector.equals(end.vector)) {
            auto first = getClosestPoint(border[0], border[1], point.vector);
            auto second = getClosestPoint(border[1], border[2], point.vector);
            auto third = getClosestPoint(border[0], border[2], point.vector);

            auto firstV = first - point.vector;
            auto secondV = second - point.vector;
            auto thirdV = third - point.vector;

            if (firstV.length() < secondV.length() && firstV.length() < thirdV.length()) {
                point.vector.x = first.x;
                point.vector.y = first.y;
            } else if (secondV.length() < firstV.length() && secondV.length() < thirdV.length()) {
                point.vector.x = second.x;
                point.vector.y = second.y;
            } else if (thirdV.length() < firstV.length() && thirdV.length() < secondV.length()) {
                point.vector.x = third.x;
                point.vector.y = third.y;
            }
        }
    }
    return points;
}

void HelloWorld::midpoint(std::vector<Vec2Color> &points, Vec2Color start, Vec2Color end, cocos2d::Vec2 normal, unsigned iters, float r) {
    if (iters == 0) {
        return;
    }

    auto lengthVector = end.vector - start.vector;
    auto length = lengthVector.length();

    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(-r*length, r * length);


    auto mid = (start.vector + end.vector) / 2;

    auto factor = dis(gen);
    Vec2Color newPoint;
    newPoint = {mid + factor*normal, colorAvg(start.color, end.color)};

    midpoint(points, start, newPoint, normal, iters - 1, r);
    points.push_back(newPoint);
    midpoint(points, newPoint, end, normal, iters - 1, r);
}

void HelloWorld::menuCloseCallback(Ref * pSender) {
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

Color4F HelloWorld::colorAvg(Color4F cFirst, Color4F cSecond, Color4F cThird) {
    auto avgR = (cFirst.r + cSecond.r + cThird.r) / 3;
    auto avgG = (cFirst.g + cSecond.g + cThird.g) / 3;
    auto avgB = (cFirst.b + cSecond.b + cThird.b) / 3;
    auto avgA = (cFirst.a + cSecond.a + cThird.a) / 3;

    return Color4F(avgR, avgG, avgB, avgA);
}

Color4F HelloWorld::colorAvg(Color4F cFirst, Color4F cSecond) {
    auto avgR = (cFirst.r + cSecond.r) / 2;
    auto avgG = (cFirst.g + cSecond.g) / 2;
    auto avgB = (cFirst.b + cSecond.b) / 2;
    auto avgA = (cFirst.a + cSecond.a) / 2;

    return Color4F(avgR, avgG, avgB, avgA);
}

void HelloWorld::drawTriangles(std::vector<Vec2Color> &points, Vec2Color verticle) {
    for (unsigned i = 0; i < points.size() - 1; i++) {
        allTriangles.push_back({
            verticle, points[i], points[i + 1]
        });
    }
}

void HelloWorld::drawPolygons(std::vector<Vec2Color> &points, Vec2Color verticle) {
    for (unsigned i = 0; i < points.size() - 1; i++) {
        std::vector<Vec2Color> triangle;
        triangle.push_back(verticle);
        triangle.push_back(points[i]);
        triangle.push_back(points[i + 1]);
        allPolygons.push_back(triangle);
    }
}

void HelloWorld::drawAllPolygons() {
    for (auto& poly : allPolygons) {
        std::vector<cocos2d::Vec2> triangle = {poly[0].vector, poly[1].vector, poly[2].vector};
        draw2d->drawPolygon(triangle.data(), triangle.size(), poly[1].color, 0, Color4F(1, 1, 0, 0));
    }
}
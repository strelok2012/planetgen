#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "Icosphere.h"
#include "DrawNode3D.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));
    
    // add the label as a child to this layer
    this->addChild(label, 1);
    
    auto draw3d = DrawNode3D::create();
    draw3d->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height/2));
    this->addChild(draw3d, 2);
    
    

    CCLOG("Bunga");
    auto icoV = Icosphere::make_icosphere(5);
    auto verticles = icoV.first;
    auto triangles = icoV.second;
    CCLOG("verticles %d",(int)verticles.size());
    CCLOG("triangles %d",(int)triangles.size());
    for(auto const& value: triangles) {
        int scale = 300;
        int i1 = (int)value.vertex[0];
        int i2 = (int)value.vertex[1];
        int i3 = (int)value.vertex[2];
        draw3d->drawLine(verticles[i1]*scale,verticles[i2]*scale,Color4F(1,0,0,1));
        draw3d->drawLine(verticles[i2]*scale,verticles[i3]*scale,Color4F(1,0,0,1));
        draw3d->drawLine(verticles[i3]*scale,verticles[i1]*scale,Color4F(1,0,0,1));
    }
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
    
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/
    
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
    
    
}

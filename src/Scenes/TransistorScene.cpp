//
//  TransistorScene.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/28/15.
//
//

#include "TransistorScene.h"
#include "cinder/Log.h"
#include "SystemEvents.h"
#include "Actor.h"
#include "Components.h"
#include "CameraManager.h"
#include "ActorManager.h"
#include "Light.h"
#include "LightComponent.h"
#include "Controller.h"
#include "Scene.h"
#include "AppSceneBase.h"
#include "Events.h"
#include "TunnelComponent.h"
#include "CameraComponent.h"
#include "GUIManager.h"
#include "GBuffer.h"
#include "OculusRiftComponent.h"
#include "DebugManager.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"
#include "RoomComponent.h"

using namespace ci;
using namespace ci::app;

TransistorSceneRef TransistorScene::create( const std::string& name )
{
    return TransistorSceneRef( new TransistorScene(name) );
}

TransistorScene::TransistorScene( const std::string& name ):AppSceneBase(name), mSpeed(0), mFlash(0), mFlashDuration(.1), mSpikeSize(0.),mPause(false)
{
    //initialize stuff
    CI_LOG_V("Transistor Scene constructed");

}

TransistorScene::~TransistorScene()
{
}

std::vector<ec::ActorUId> TransistorScene::shutdown(){
    ec::Controller::get()->eventManager()->removeListener(fastdelegate::MakeDelegate(this, &TransistorScene::shutDown), ec::ShutDownEvent::TYPE);
    return ec::Scene::shutdown();
}

void TransistorScene::initialize(const ci::JsonTree &init)
{
    //init super
    AppSceneBase::initialize(init);
    
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &TransistorScene::shutDown), ec::ShutDownEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(this, &TransistorScene::handlePresentScene), FinishRenderEvent::TYPE);
    
    CI_LOG_V("Tunnel scene initialized");
    
}

void TransistorScene::shutDown(ec::EventDataRef)
{
    CI_LOG_V( "Transistor Scene handle shutdown");
}

void TransistorScene::postInit()
{
    ///like setup
    
    
}

void TransistorScene::moveFlash(){
    
    static const vec3 corners[8] = { vec3(-12,-12,-12),
        vec3(12,-12,-12),
        vec3(12,12,-12),
        vec3(-12,12,-12),
        vec3(-12,-12,12),
        vec3(12,-12,12),
        vec3(12,12,12),
        vec3(-12,12,12)
    };
    
    int r = randInt(8);
    auto flashing_light = std::dynamic_pointer_cast<PointLight>(ec::ActorManager::get()->retreiveUnique(ec::getHash("flashing")).lock()->getComponent<LightComponent>().lock()->getLight() );
    flashing_light->setPosition(corners[r]);
    
    if( auto scene = ec::Controller::get()->scene().lock()){
        auto scene_trans=  std::dynamic_pointer_cast<TransistorScene>(scene);
        scene_trans->mFlashDuration = randFloat(.1, .3);
        
        auto uFn = std::bind(  &TransistorScene::flash, scene_trans.get() );
        auto fFn = std::bind(  &TransistorScene::moveFlash, scene_trans.get() );

        timeline().apply(&mFlash, 1.f - mFlash, mFlashDuration).updateFn(uFn).finishFn(fFn);
    }
    

}

void TransistorScene::flash()
{
    if(auto flashing = ec::ActorManager::get()->retreiveUnique(ec::getHash("flashing")).lock()){
        if(auto flashing_light =  std::dynamic_pointer_cast<PointLight>( flashing->getComponent<LightComponent>().lock()->getLight() )){
                flashing_light->setIntensity( mFlash );
        }
    }
}

void TransistorScene::update()
{
    //do stuff
    CI_LOG_V("Transistor Scene scene updating");
    
    if(!mPause){
        mSpeed += .0005;
        if(mSpeed > 1.)mSpeed = 1.;
    }
    
    auto room = ec::ActorManager::get()->retreiveUnique(ec::getHash("room")).lock();
    auto bit = ec::ActorManager::get()->retreiveUnique(ec::getHash("bit")).lock();
    auto spot = ec::ActorManager::get()->retreiveUnique(ec::getHash("spot_light")).lock();
    auto main_cam = ec::ActorManager::get()->retreiveUnique(ec::getHash("main_camera")).lock();

    auto spot_light = std::dynamic_pointer_cast<SpotLight>( spot->getComponent<LightComponent>().lock()->getLight() );
    auto bit_light = ec::ActorManager::get()->retreiveUnique(ec::getHash("bit_light")).lock();

    spot_light->setPosition(vec3( 11.*cos( getElapsedSeconds()*mSpeed ), 11.*sin( getElapsedSeconds()*.4*mSpeed ), 11.*sin(getElapsedSeconds()*mSpeed+20.) ));
    spot_light->pointAt(vec3(0));
    
    auto room_component = room->getComponent<RoomComponent>().lock();
    
    static bool applied = false;
    if(!applied && mSpeed > .5){
        auto uFn = std::bind(  &TransistorScene::flash, this );
        auto fFn = std::bind(  &TransistorScene::moveFlash, this );
        timeline().apply(&mFlash, 1.f, mFlashDuration).updateFn(uFn).finishFn(fFn);
        applied = true;
    }
    if(applied){
        room_component->setNoiseScale( mSpeed );
    }
    
    //auto cam_tracking = main_cam->getComponent<ec::TransformComponent>().lock();
    auto bit_trans = bit->getComponent<ec::TransformComponent>().lock();
    bit_trans->setScale(vec3(5.*mSpeed));
    
    //cam_tracking->setTranslation(vec3( 10.*cos( getElapsedSeconds()*.2*mSpeed ), 10.*sin( getElapsedSeconds()*.1*mSpeed ), 10.*sin(getElapsedSeconds()*.2*mSpeed) ));
    //cam_tracking->setRotation( quat( inverse(glm::lookAt(cam_tracking->getTranslation(), vec3(0), vec3(0,1,0))) ) );
    
    CI_LOG_V("update components event triggered");
    mSceneManager->triggerEvent( UpdateEvent::create() );
    
    //update the superclass
    AppSceneBase::update();
}

void TransistorScene::preDraw()
{
    CI_LOG_V("cull visible event triggered");
    mSceneManager->triggerEvent( CullEvent::create() );
}

void TransistorScene::draw()
{
    ///DRAW SHADOWS
    CI_LOG_V("firing draw command");
    manager()->triggerEvent(DrawEvent::create());
}

void TransistorScene::initGUI(const ec::GUIManagerRef &gui_manager)
{
    AppSceneBase::initGUI(gui_manager);
    auto params = gui_manager->findGUI(getId());
    params->addParam("pause", &mPause);
    params->addParam("speed", &mSpeed).max(1.).min(.0).step(.01);
    
}

void TransistorScene::handlePresentScene(ec::EventDataRef event)
{
    auto e = std::dynamic_pointer_cast<FinishRenderEvent>(event);
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
    if( ec::Controller::isRiftEnabled() ){
        
        auto rift = ec::ActorManager::get()->retreiveUnique(ec::getHash("rift")).lock();
        auto rift_component = rift->getComponent<OculusRiftComponent>().lock();
        auto oculus = rift_component->getRift();
        
        hmd::ScopedBind bind{ *oculus };
        gl::clear();
        
        auto tex = e->getFinalTexture();
        {
            gl::ScopedMatrices pushMatrix;
            gl::setMatricesWindow(oculus->getFboSize());
            gl::ScopedViewport view( vec2(0), oculus->getFboSize());
            
            if( ec::Controller::get()->debugEnabled() ){
                gl::draw( mDebug->getDebugTexture(), Rectf( vec2(0), oculus->getFboSize() ) );
            }else{
                gl::draw( tex, Rectf( vec2(0), oculus->getFboSize() ) );
            }
        }
    }else{
        auto tex = e->getFinalTexture();
        {
            
            gl::ScopedMatrices pushMatrix;
            gl::setMatricesWindow(getWindowSize());
            gl::ScopedViewport view( vec2(0), getWindowSize());
            
            if( ec::Controller::get()->debugEnabled() ){
                gl::draw(mDebug->getDebugTexture());
            }
            else{
                gl::draw( tex );
            }
            
        }
    }
    
}

void TransistorScene::postDraw()
{
    //do stuff post draw
    
}

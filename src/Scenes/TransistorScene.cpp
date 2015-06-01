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

using namespace ci;
using namespace ci::app;

TransistorSceneRef TransistorScene::create( const std::string& name )
{
    return TransistorSceneRef( new TransistorScene(name) );
}

TransistorScene::TransistorScene( const std::string& name ):AppSceneBase(name)
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

void TransistorScene::update()
{
    //do stuff
    CI_LOG_V("Transistor Scene scene updating");
    
    static float speed = 0.;
    speed += .0001;
    if(speed > 1.)speed = 1.;
    
    auto bit = ec::ActorManager::get()->retreiveUnique(ec::getHash("bit")).lock();
    auto spot = ec::ActorManager::get()->retreiveUnique(ec::getHash("spot_light")).lock();
    auto flashing = ec::ActorManager::get()->retreiveUnique(ec::getHash("flashing")).lock();
    auto main_cam = ec::ActorManager::get()->retreiveUnique(ec::getHash("main_camera")).lock();

    auto spot_light = std::dynamic_pointer_cast<SpotLight>( spot->getComponent<LightComponent>().lock()->getLight() );
    auto flashing_light =  std::dynamic_pointer_cast<PointLight>( flashing->getComponent<LightComponent>().lock()->getLight() );
    
    spot_light->setPosition(vec3( 11.*cos( getElapsedSeconds()*speed ), 11.*sin( getElapsedSeconds()*.4*speed ), 11.*sin(getElapsedSeconds()*speed+20.) ));
    spot_light->pointAt(vec3(0));
    
    Perlin p;
    if( speed > .5 )flashing_light->setIntensity( constrain(p.noise(getElapsedSeconds()*10.*speed),0.01f,1.f) );
    else flashing_light->setIntensity( .01 );
    
    static const vec3 corners[8] = { vec3(-12,-12,-12),
                                     vec3(12,-12,-12),
                                     vec3(12,12,-12),
                                     vec3(-12,12,-12),
                                     vec3(-12,-12,12),
                                     vec3(12,-12,12),
                                     vec3(12,12,12),
                                     vec3(-12,12,12)
                                    };
    
    static int frames = 5;
    
    if( getElapsedFrames() % frames == 0 && speed > .5 ){
        int r = randInt(8);
        flashing_light->setPosition(corners[r]);
        frames = 5 + randInt(5);
    }
    
    auto cam_tracking = main_cam->getComponent<ec::TransformComponent>().lock();
    auto bit_trans = bit->getComponent<ec::TransformComponent>().lock();
    bit_trans->setScale(vec3(5.*speed));
   
    cam_tracking->setTranslation(vec3( 10.*cos( getElapsedSeconds()*speed ), 10.*sin( getElapsedSeconds()*.1*speed ), 10.*sin(getElapsedSeconds()*speed) ));
    cam_tracking->setRotation( quat( inverse(glm::lookAt(cam_tracking->getTranslation(), vec3(0), vec3(0,1,0))) ) );
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

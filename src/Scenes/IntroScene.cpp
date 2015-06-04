//
//  IntroScene.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/28/15.
//
//

#include "IntroScene.h"
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
#include "LandscapeComponent.h"
#include "cinder/Perlin.h"

using namespace ci;
using namespace ci::app;

IntroSceneRef IntroScene::create( const std::string& name )
{
    return IntroSceneRef( new IntroScene(name) );
}

IntroScene::IntroScene( const std::string& name ):AppSceneBase(name),mScale(0.),mEnableSinkHole(false)
{
    //initialize stuff
    CI_LOG_V("Intro scene constructed");
}

std::vector<ec::ActorUId> IntroScene::shutdown(){
    ec::Controller::get()->eventManager()->removeListener(fastdelegate::MakeDelegate(this, &IntroScene::shutDown), ec::ShutDownEvent::TYPE);
    return ec::Scene::shutdown();
}

IntroScene::~IntroScene()
{
}

void IntroScene::initialize(const ci::JsonTree &init)
{
    //init super
    AppSceneBase::initialize(init);
    
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &IntroScene::shutDown), ec::ShutDownEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(this, &IntroScene::handlePresentScene), FinishRenderEvent::TYPE);
    
    CI_LOG_V("Intro scene initialized");
    
}

void IntroScene::shutDown(ec::EventDataRef)
{
    CI_LOG_V( "Intro handle shutdown");
}

void IntroScene::postInit()
{
    ///like setup
    
    
}

void IntroScene::update()
{
    //do stuff
    CI_LOG_V("Intro scene updating");
    
    if( mEnableSinkHole )
    {
        mScale += .1;
        
        if(mScale > 200.){
            ec::Controller::get()->eventManager()->queueEvent(ec::RequestSceneChangeEvent::create());
            return;
        }
    
        auto main_cam = ec::ActorManager::get()->retreiveUnique( ec::getHash("main_camera") ).lock();
        auto cam_transform = main_cam->getComponent<ec::TransformComponent>().lock();
        auto landscape =  ec::ActorManager::get()->retreiveUnique( ec::getHash("landscape") ).lock();
        auto l_component = landscape->getComponent<LandscapeComponent>().lock();
        l_component->setSinkHoleScale( mScale );
        cam_transform->setTranslation(  vec3(0,-l_component->getSinkHoleScale()*10. + 40.f,0 ) );
        
    }
    CI_LOG_V("update components event triggered");
    mSceneManager->triggerEvent( UpdateEvent::create() );
    
    //update the superclass
    AppSceneBase::update();
}

void IntroScene::preDraw()
{
    CI_LOG_V("cull visible event triggered");
    mSceneManager->triggerEvent( CullEvent::create() );
}

void IntroScene::draw()
{
    ///DRAW SHADOWS
    CI_LOG_V("firing draw command");
    manager()->triggerEvent(DrawEvent::create());
}

void IntroScene::initGUI(const ec::GUIManagerRef &gui_manager)
{
    AppSceneBase::initGUI(gui_manager);
    auto params = gui_manager->findGUI(getId());
    params->addParam("enable sinkhole", &mEnableSinkHole);
    params->addParam("progress", &mScale);
    
}

void IntroScene::handlePresentScene(ec::EventDataRef event)
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

void IntroScene::postDraw()
{
    //do stuff post draw
    
}

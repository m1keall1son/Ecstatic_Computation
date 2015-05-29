//
//  TunnelScene.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/28/15.
//
//

#include "TunnelScene.h"
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

using namespace ci;
using namespace ci::app;

TunnelSceneRef TunnelScene::create( const std::string& name )
{
    return TunnelSceneRef( new TunnelScene(name) );
}

TunnelScene::TunnelScene( const std::string& name ):AppSceneBase(name), mTunnelSpeed(0.),mTunnelAccel(.00001),mTunnelSamplePt(0),mScrubTunnel(false)
{
    //initialize stuff
    CI_LOG_V("Tunnel scene constructed");
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &TunnelScene::shutDown), ec::ShutDownEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(this, &TunnelScene::handlePresentScene), FinishRenderEvent::TYPE);
    
}

void TunnelScene::shutDown(ec::EventDataRef)
{
    CI_LOG_V( "Tunnel handle shutdown");
}

void TunnelScene::postInit()
{
    ///like setup
    
    
}

void TunnelScene::update()
{
    //do stuff
    CI_LOG_V("Tunnel scene updating");
    
    auto tunnel = ec::ActorManager::get()->retreiveUnique(ec::getHash("tunnel")).lock();
    auto main_camera_actor = ec::ActorManager::get()->retreiveUnique(ec::getHash("main_camera")).lock();
    auto follow_light = ec::ActorManager::get()->retreiveUnique(ec::getHash("follow_light")).lock();
    auto head_lamp = ec::ActorManager::get()->retreiveUnique(ec::getHash("head_lamp")).lock();

    auto tunnel_transform = tunnel->getComponent<ec::TransformComponent>().lock();
    auto tunnel_component = tunnel->getComponent<TunnelComponent>().lock();
    
    auto camera_transform = main_camera_actor->getComponent<ec::TransformComponent>().lock();
    
    if(!mScrubTunnel){
        mTunnelSpeed += mTunnelAccel;
        mTunnelSamplePt += mTunnelSpeed*.005;
    }
    
    static float light_sample = 0.;
    light_sample+=.001;
    
    auto sample_pt = ci::constrain( mTunnelSamplePt , 0.f, 1.f);
    auto new_pos = tunnel_transform->getTranslation() + tunnel_component->getSpline().getPosition(sample_pt);
    camera_transform->setTranslation( new_pos );
    
    auto lamp = std::dynamic_pointer_cast<SpotLight>(head_lamp->getComponent<LightComponent>().lock()->getLight());
    lamp->setPosition( new_pos + vec3( 0,2,0 ) );
    lamp->pointAt( main_camera_actor->getComponent<CameraComponent>().lock()->getCamera().getCenterOfInterestPoint() );
    
    tunnel_component->getNoiseScale() = .5 + sample_pt*2.;
    
    auto light = std::dynamic_pointer_cast<PointLight>(follow_light->getComponent<LightComponent>().lock()->getLight());
    if( light_sample > .99){
        light_sample = 0.;
    }
    auto light_pos = tunnel_transform->getTranslation() + tunnel_component->getSpline().getPosition(light_sample);
    light->setPosition( light_pos + vec3( 6.*cos(light_pos.z*.04), 6.*sin(light_pos.z*.0153), 0.) );
    
    
    CI_LOG_V("update components event triggered");
    mSceneManager->triggerEvent( UpdateEvent::create() );
    
    //update the superclass
    AppSceneBase::update();
}

void TunnelScene::preDraw()
{
    CI_LOG_V("cull visible event triggered");
    mSceneManager->triggerEvent( CullEvent::create() );
}

void TunnelScene::draw()
{
    ///DRAW SHADOWS
    CI_LOG_V("firing draw command");
    manager()->triggerEvent(DrawEvent::create());
}

void TunnelScene::initGUI(const ec::GUIManagerRef &gui_manager)
{
    AppSceneBase::initGUI(gui_manager);
    auto params = gui_manager->findGUI(getId());
    params->addParam("scrub tunnel", &mScrubTunnel);
    params->addParam("tunnel position", &mTunnelSamplePt).max(1.).min(0.).step(.001);
    
}

void TunnelScene::handlePresentScene(ec::EventDataRef event)
{
    auto e = std::dynamic_pointer_cast<FinishRenderEvent>(event);
    
    auto tex = e->getFinalTexture();
    {
        gl::ScopedMatrices pushMatrix;
        gl::setMatricesWindow(getWindowSize());
        gl::ScopedViewport view( vec2(0), getWindowSize() );
        gl::draw( tex );
    }
    
}

void TunnelScene::postDraw()
{
    //do stuff post draw
    
}

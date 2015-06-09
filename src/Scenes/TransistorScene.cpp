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
#include "RoomParticlesComponent.h"
#include "KinectComponent.h"
#include "OSCComponent.h"

using namespace ci;
using namespace ci::app;

TransistorSceneRef TransistorScene::create( const std::string& name )
{
    return TransistorSceneRef( new TransistorScene(name) );
}

TransistorScene::TransistorScene( const std::string& name ):AppSceneBase(name), mSpeed(.2),mBitScale(0.), mFlash(0), mFlashDuration(.1), mSpikeSize(0.),mPause(false),mRotateStuff(false),mDisintegrate(false),mSpotMove(0.),mDec(10.),mCamLerp(0),mApplied(false),mFinish(false),mInitDisitegrate(false),mAllowAdvance(true)
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
    mSceneManager->addListener(fastdelegate::MakeDelegate(this, &TransistorScene::handleAdvance), AdvanceEvent::TYPE);

    CI_LOG_V("Tunnel scene initialized");
    
}

void TransistorScene::shutDown(ec::EventDataRef)
{
    CI_LOG_V( "Transistor Scene handle shutdown");
}

void TransistorScene::postInit()
{
    ///like setup
    auto osc = ec::ActorManager::get()->retreiveUnique(ec::getHash("OSC")).lock();
    auto osc_component = osc->getComponent<OSCComponent>().lock();
    if( osc_component ){
        osc_component->sendFloat(ec::getHash("touchosc"),"/1/ready", 1.);
    }
}

void TransistorScene::handleAdvance(ec::EventDataRef){
    
    if( mAllowAdvance ){
        
        if( mDisintegrate ){
            
            auto osc = ec::ActorManager::get()->retreiveUnique(ec::getHash("OSC")).lock();
            auto osc_component = osc->getComponent<OSCComponent>().lock();
            if( osc_component ){
                osc_component->sendFloat(ec::getHash("touchosc"),"/1/ready", 0.);
                auto osc_id = ec::getHash("audio");
                osc_component->sendFloat(osc_id, "/noteoff", 0.f);
            }
            mAllowAdvance = false;
  
            ec::Controller::get()->reset( true );
            
        }else{
            mDisintegrate = true;
            auto osc = ec::ActorManager::get()->retreiveUnique(ec::getHash("OSC")).lock();
            auto osc_component = osc->getComponent<OSCComponent>().lock();
            if( osc_component ){
                osc_component->sendFloat(ec::getHash("touchosc"),"/1/ready", 0.);
            }
            mAllowAdvance = false;
        }
        
    }
    
}

void TransistorScene::moveFlash(){
    
    static const vec3 corners[8] = { vec3(-10,-10,-10),
        vec3(10,-10,-10),
        vec3(10,10,-10),
        vec3(-10,10,-10),
        vec3(-10,-10,10),
        vec3(10,-10,10),
        vec3(10,10,10),
        vec3(-10,10,10)
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

void TransistorScene::moveCamera()
{
    if(auto camera = ec::ActorManager::get()->retreiveUnique(ec::getHash("main_camera")).lock()){
        auto cam_tracking = camera->getComponent<ec::TransformComponent>().lock();
        
        auto circle = vec3( mDec*cos( getElapsedSeconds()*.2*mSpeed ), mDec*sin( getElapsedSeconds()*.1*mSpeed ), mDec*sin(getElapsedSeconds()*.2*mSpeed) );
        
            cam_tracking->setTranslation( lerp( mCamPos, circle, mCamLerp.value() ) );
        
            cam_tracking->setRotation( quat( inverse(glm::lookAt(cam_tracking->getTranslation(), vec3(0), vec3(0,1,0))) ) );
            auto bit_light = ec::ActorManager::get()->retreiveUnique(ec::getHash("bit_light")).lock();
        auto light = bit_light->getComponent<LightComponent>().lock()->getLight();
        light->setIntensity( .01 + (10.f - mDec) );
    }
}

void TransistorScene::update()
{
    if(auto camera = ec::ActorManager::get()->retreiveUnique(ec::getHash("main_camera")).lock()){
        auto cam_tracking = camera->getComponent<ec::TransformComponent>().lock();
        mCamPos = cam_tracking->getTranslation();
    }
    //do stuff
    CI_LOG_V("Transistor Scene scene updating");
    
    if(mDisintegrate){
        mSpeed += .0005;
        if(mSpeed > 1.)mSpeed = 1.;
    }
    
    if( mDisintegrate && !mInitDisitegrate ){
        
        auto kinect = ec::ActorManager::get()->retreiveUnique(ec::getHash("kinect")).lock();
        auto k_particles = kinect->getComponent<RoomParticlesComponent>().lock();
        auto k_mesh = kinect->getComponent<KinectComponent>().lock();
        
        mSceneManager->removeListener(fastdelegate::MakeDelegate(k_mesh.get(), &KinectComponent::draw), DrawToMainBufferEvent::TYPE);
        mSceneManager->removeListener(fastdelegate::MakeDelegate(k_mesh.get(), &KinectComponent::drawRift), DrawToRiftBufferEvent::TYPE);
        mSceneManager->removeListener(fastdelegate::MakeDelegate(k_mesh.get(), &KinectComponent::drawShadow), DrawShadowEvent::TYPE);

        mSceneManager->addListener(fastdelegate::MakeDelegate(k_particles.get(), &RoomParticlesComponent::drawTF), UpdateEvent::TYPE);
        mSceneManager->addListener(fastdelegate::MakeDelegate(k_particles.get(), &RoomParticlesComponent::drawShadows), DrawShadowEvent::TYPE);
        mSceneManager->addListener(fastdelegate::MakeDelegate(k_particles.get(), &RoomParticlesComponent::drawRift), DrawToRiftBufferEvent::TYPE);
        mSceneManager->addListener(fastdelegate::MakeDelegate(k_particles.get(), &RoomParticlesComponent::draw), DrawToMainBufferEvent::TYPE);

        auto osc = ec::ActorManager::get()->retreiveUnique(ec::getHash("OSC")).lock();
        if(osc){
            if(auto osc_component = osc->getComponent<OSCComponent>().lock()){
                
                auto osc_id = ec::getHash("audio");
                
                osc_component->sendFloat(osc_id, "/noteon", 1.f);
            }
        }
        
        mInitDisitegrate = true;
    }
    
    auto room = ec::ActorManager::get()->retreiveUnique(ec::getHash("room")).lock();
    auto bit = ec::ActorManager::get()->retreiveUnique(ec::getHash("bit")).lock();
    auto spot = ec::ActorManager::get()->retreiveUnique(ec::getHash("spot_light")).lock();
    auto main_cam = ec::ActorManager::get()->retreiveUnique(ec::getHash("main_camera")).lock();

    auto spot_light = std::dynamic_pointer_cast<SpotLight>( spot->getComponent<LightComponent>().lock()->getLight() );
    auto bit_light = ec::ActorManager::get()->retreiveUnique(ec::getHash("bit_light")).lock();

    if(mDisintegrate){
        
        vec3 pos = lerp(spot_light->getPosition(), vec3( 12.*cos( getElapsedSeconds()*.2*mSpeed ), 12.*sin( getElapsedSeconds()*.2*mSpeed ), 12.*sin(getElapsedSeconds()*.2*mSpeed+20.) ), mSpotMove.value());
        spot_light->setPosition( pos );
        spot_light->pointAt(vec3(0));
    }
    
    auto room_component = room->getComponent<RoomComponent>().lock();
    
    if(!mApplied && mDisintegrate){
        
        if( mFinish ){
           ec::Controller::get()->reset( true );
        }else{
        
      //  auto endScene =[&]{ ec::Controller::get()->reset( true ); };
            auto endScene =[&]{
                auto osc = ec::ActorManager::get()->retreiveUnique(ec::getHash("OSC")).lock();
                auto osc_component = osc->getComponent<OSCComponent>().lock();
                if( osc_component ){
                    osc_component->sendFloat(ec::getHash("touchosc"),"/1/ready", 1.);
                }
                std::dynamic_pointer_cast<TransistorScene>( ec::Controller::get()->scene().lock() )->allowAdvance();
            };
            
            auto camFun = [&]{
                timeline().apply(&mDec, 0.f, 25.f).updateFn( std::bind( &TransistorScene::moveCamera, this ) ).finishFn(endScene);
                timeline().apply(&mCamLerp, 1.f, 5.f);
            };
            
            auto uFn = std::bind(  &TransistorScene::flash, this );
            auto fFn = std::bind(  &TransistorScene::moveFlash, this );
            timeline().apply(&mFlash, 1.f, mFlashDuration).updateFn(uFn).finishFn(fFn);
            //after this engage scene end
            timeline().apply(&mBitScale, 5.f, 60.f ).finishFn(camFun);
            timeline().apply(&mSpotMove, 1.f, 5.f );
        }
        mApplied = true;
    }
    auto bit_trans = bit->getComponent<ec::TransformComponent>().lock();

    if(mApplied){
        room_component->setNoiseScale( mSpeed );
        bit_trans->setScale( vec3(mBitScale) );
    }
    
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

void TransistorScene::initGUI( ec::GUIManager* gui_manager)
{
//    auto params = gui_manager->findGUI(getId());
    auto gui = gui_manager->getMainGui();
    gui->addSeparator();
    gui->addText("Scene: "+getName());
    gui->addParam("pause", &mPause);
    gui->addParam("rotate stuff", &mRotateStuff);
    gui->addParam("disitegrate", &mDisintegrate);
    gui->addParam("speed", &mSpeed).max(1.).min(.0).step(.01);
    gui->addParam("finish", &mFinish);
    
    AppSceneBase::initGUI(gui_manager);

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

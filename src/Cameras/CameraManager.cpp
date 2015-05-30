//
//  CameraManager.cpp
//  System_test
//
//  Created by Mike Allison on 5/21/15.
//
//

#include "CameraManager.h"
#include "ActorManager.h"
#include "Actor.h"
#include "Controller.h"
#include "SystemEvents.h"
#include "EventManager.h"
#include "Scene.h"
#include "Events.h"

using namespace ci;
using namespace ci::app;

const ci::CameraPersp& CameraManager::getActiveCamera()
{
    return getCamera(mCurrentCamera);
}

ci::CameraPersp& CameraManager::getCamera( const CameraComponent::CameraType& cam_type )
{
    
    auto it = mCameras.find(cam_type);
    if( it != mCameras.end() ){
        auto cam_id = it->second;
        if( auto actor_strong = ec::ActorManager::get()->retreiveUnique(cam_id).lock()){
            return actor_strong->getComponent<CameraComponent>().lock()->getCamera();
        }
    }
    CI_LOG_E("Didn't find requested camera returning default");
    return mDefaultCamera;
}

CameraManager::CameraManager():mShuttingDown(false),mId( ec::getHash("camera_manager") ),mCurrentCamera(CameraComponent::CameraType::MAIN_CAMERA)
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &CameraManager::handleShutDown), ec::ShutDownEvent::TYPE);
    
    mUI.connect(ci::app::getWindow());
    ///init default camera
    mDefaultCamera.setPerspective(60, ci::app::getWindowAspectRatio(), .1, 10000.);
    mDefaultCamera.lookAt(ci::vec3(0,0,1),ci::vec3(0));
    mUI.setCurrentCam(mDefaultCamera);
    
    auto window = ci::app::getWindow();
    //window->getSignalKeyDown().connect( std::bind( &CameraManager::keyDown, this , std::placeholders::_1 ) );
    window->getSignalKeyUp().connect( std::bind( &CameraManager::keyUp, this , std::placeholders::_1 ) );
    
}

void CameraManager::handleShutDown(ec::EventDataRef)
{
    CI_LOG_V( "camera_manager handle shutdown");
    mShuttingDown = true;
}

CameraManager::~CameraManager()
{
    //TODO: how to shutdown properly
    if(!mShuttingDown){
        ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &CameraManager::handleShutDown), ec::ShutDownEvent::TYPE);
    }
}

void CameraManager::updateCamera(ec::EventDataRef)
{
    if( mCurrentCamera == CameraComponent::CameraType::DEBUG_CAMERA && !ec::Controller::isRiftEnabled() ){
        auto cur_cam_id = mCameras.find(mCurrentCamera)->second;
        if( auto cur_actor = ec::ActorManager::get()->retreiveUnique(cur_cam_id).lock()){
            auto transform = cur_actor->getComponent<ec::TransformComponent>().lock();
            auto cur_settings = mUI.getCamera();
            transform->setTranslation(cur_settings.getEyePoint());
            transform->setRotation(cur_settings.getOrientation());
            cur_actor->getComponent<CameraComponent>().lock()->getCamera().setCenterOfInterestPoint(cur_settings.getCenterOfInterestPoint());
        }
    }
    
}

void CameraManager::handleSwitchCamera(ec::EventDataRef event)
{
    
    auto e = std::dynamic_pointer_cast<SwitchCameraEvent>(event);
    auto cam_id = mCameras.find(e->getType())->second;
    if( auto actor = ec::ActorManager::get()->retreiveUnique(cam_id).lock()){
        
        auto cur_cam_id = mCameras.find(mCurrentCamera)->second;
        if( auto cur_actor = ec::ActorManager::get()->retreiveUnique(cur_cam_id).lock()){
            if( mCurrentCamera==CameraComponent::CameraType::DEBUG_CAMERA  && !ec::Controller::isRiftEnabled()){
                auto transform = cur_actor->getComponent<ec::TransformComponent>().lock();
                auto cur_settings = mUI.getCamera();
                transform->setTranslation(cur_settings.getEyePoint());
                transform->setRotation(cur_settings.getOrientation());
                cur_actor->getComponent<CameraComponent>().lock()->getCamera().setCenterOfInterestPoint(cur_settings.getCenterOfInterestPoint());
            }
        }
        auto & cam_comp = actor->getComponent<CameraComponent>().lock()->getCamera();
        if( e->getType() == CameraComponent::CameraType::DEBUG_CAMERA && !ec::Controller::isRiftEnabled() ){
            mUI.setCurrentCam( cam_comp );
        }
        mCurrentCamera = e->getType();
    }
    
}

void CameraManager::handleCameraRegistration( ec::EventDataRef event )
{
    auto e = std::dynamic_pointer_cast<ComponentRegistrationEvent>( event );
    
    if( e->getType() == ComponentRegistrationEvent::RegistrationType::CAMERA ){
        
        auto cam_component = std::dynamic_pointer_cast<CameraComponent>(e->getComponentBase());
        
        CI_LOG_V("Registering camera");
        auto cam_type =  cam_component->getCamType();
        mCameras.insert( std::make_pair( cam_type , e->getActorUId() ) );
        if( cam_type == CameraComponent::CameraType::DEBUG_CAMERA && !ec::Controller::isRiftEnabled() ){
            auto & cam = cam_component->getCamera();
            mUI.setCurrentCam(cam);
        }
    }
}

void CameraManager::keyUp(ci::app::KeyEvent & event)
{
    if(ec::Controller::isRiftEnabled() && mCurrentCamera == CameraComponent::CameraType::DEBUG_CAMERA){
        
        auto cur_cam_id = mCameras.find(mCurrentCamera)->second;
        auto cam = ec::ActorManager::get()->retreiveUnique(cur_cam_id).lock()->getComponent<CameraComponent>().lock()->getCamera();
        
        if(event.getCode() == KeyEvent::KEY_UP){
            cam.setEyePoint( cam.getEyePoint() + vec3( 0.,0,-.1 ) );
        }
        else if(event.getCode() == KeyEvent::KEY_DOWN){
            cam.setEyePoint( cam.getEyePoint() + vec3( 0.,.0,.1 ) );
        }
        else if(event.getCode() == KeyEvent::KEY_LEFT){
            cam.setEyePoint( cam.getEyePoint() + vec3( -.1,.0,.0 ) );
        }
        else if(event.getCode() == KeyEvent::KEY_RIGHT){
            cam.setEyePoint( cam.getEyePoint() + vec3( .1,.0,.0 ) );
        }
    }
}

void CameraManager::keyDown(ci::app::KeyEvent & event)
{
    
}


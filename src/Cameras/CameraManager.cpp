//
//  CameraManager.cpp
//  System_test
//
//  Created by Mike Allison on 5/21/15.
//
//

#include "CameraManager.h"
#include "CameraComponent.h"
#include "ActorManager.h"
#include "Actor.h"
#include "Controller.h"
#include "SystemEvents.h"
#include "EventManager.h"
#include "Scene.h"
#include "Events.h"

CameraManager::CameraType CameraManager::parseCameraType( const ec::ActorTypeQualifier &qualifier )
{
    if( qualifier == "main" )return CameraType::MAIN_CAMERA;
    else if( qualifier == "debug" )return CameraType::DEBUG_CAMERA;
    else return CameraType::MAIN_CAMERA;
}

const ci::CameraPersp& CameraManager::getActiveCamera()
{
    return mUI.getCamera();
}

const ci::CameraPersp& CameraManager::getCamera( const CameraType& cam_type )
{
    
    auto it = mCameras.find(cam_type);
    if( it != mCameras.end() ){
        auto cam_id = it->second;
        if( auto actor_strong = ec::ActorManager::get()->retreiveUnique(cam_id).lock()){
            return actor_strong->getComponent<CameraComponent>().lock()->getCamera();
        }
    }
    CI_LOG_E("Didn't find requested camera returning default");
    return mUI.getCamera();
}

CameraManager::CameraManager():mShuttingDown(false)
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &CameraManager::handleCameraRegistration), ec::ReturnActorCreatedEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &CameraManager::handleShutDown), ec::ShutDownEvent::TYPE);
    
    mUI.connect(ci::app::getWindow());
    ///init default camera
    mDefaultCamera.setPerspective(60, ci::app::getWindowAspectRatio(), .1, 10000.);
    mDefaultCamera.lookAt(ci::vec3(0,0,1),ci::vec3(0));
    mUI.setCurrentCam(mDefaultCamera);
    
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
        ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &CameraManager::handleCameraRegistration), ec::ReturnActorCreatedEvent::TYPE);
        ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &CameraManager::handleShutDown), ec::ShutDownEvent::TYPE);
    }
}

void CameraManager::handleSwitchCamera(ec::EventDataRef event)
{
    
    auto e = std::dynamic_pointer_cast<SwitchCameraEvent>(event);
    auto cam_id = mCameras.find(e->getType())->second;
    if( auto actor = ec::ActorManager::get()->retreiveUnique(cam_id).lock()){
        
        auto cur_cam_id = mCameras.find(mCurrentCamera)->second;
        if( auto cur_actor = ec::ActorManager::get()->retreiveUnique(cur_cam_id).lock()){

            auto transform = cur_actor->getComponent<ec::TransformComponent>().lock();
            auto cur_settings = mUI.getCamera();
            transform->setTranslation(cur_settings.getEyePoint());
            transform->setRotation(cur_settings.getOrientation());

        }
        
        mUI.setCurrentCam( actor ->getComponent<CameraComponent>().lock()->getCamera() );
        mCurrentCamera = e->getType();
    }
    
}

void CameraManager::handleCameraRegistration( ec::EventDataRef event )
{
    auto e = std::dynamic_pointer_cast<ec::ReturnActorCreatedEvent>( event );
    auto strong = e->getActorWeakRef().lock();
    if( strong ){
        if( strong->getType() == "camera" ){
            CI_LOG_V("Registering camera");
            auto cam_type =  parseCameraType( strong->getTypeQualifier() );
            mCameras.insert( std::make_pair( cam_type , strong->getUId() ) );
            if( cam_type == CameraType::MAIN_CAMERA )mUI.setCurrentCam(strong->getComponent<CameraComponent>().lock()->getCamera());
        }
    }
}
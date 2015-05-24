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

CameraManager::CameraType CameraManager::parseCameraType( const ec::ActorTypeQualifier &qualifier )
{
    if( qualifier == "main" )return CameraType::MAIN_CAMERA;
    else if( qualifier == "debug" )return CameraType::DEBUG_CAMERA;
    else return CameraType::MAIN_CAMERA;
}

const ci::CameraPersp& CameraManager::getCamera( const CameraType& cam_type )
{
    auto actor_id = mCameras.find(MAIN_CAMERA)->second;
    auto actor = ec::ActorManager::get()->retreiveUnique(actor_id).lock();
    if( actor ){
        auto camera_component_weak = actor->getComponent<CameraComponent>();
        return camera_component_weak.lock()->getCamera();
    }
    
    CI_LOG_E("Camera not found, retreiving default");
    return mDefaultCamera;
}

CameraManager::CameraManager():mShuttingDown(false)
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &CameraManager::handleCameraRegistration), ec::ReturnActorCreatedEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &CameraManager::handleShutDown), ec::ShutDownEvent::TYPE);
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

void CameraManager::handleCameraRegistration( ec::EventDataRef event )
{
    auto e = std::dynamic_pointer_cast<ec::ReturnActorCreatedEvent>( event );
    auto strong = e->getActorWeakRef().lock();
    if( strong ){
        if( strong->getType() == "camera" ){
            CI_LOG_V("Registering camera");
            mCameras.insert( std::make_pair( parseCameraType( strong->getTypeQualifier() ), strong->getUId() ) );
        }
    }
}
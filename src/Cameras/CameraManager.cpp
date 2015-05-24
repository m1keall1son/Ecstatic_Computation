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

CameraManager::CameraManager()
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &CameraManager::handleCameraRegistration), ec::ReturnActorCreatedEvent::TYPE);
}

CameraManager::~CameraManager()
{
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &CameraManager::handleCameraRegistration), ec::ReturnActorCreatedEvent::TYPE);
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
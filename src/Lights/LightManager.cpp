//
//  LightManager.cpp
//  System_test
//
//  Created by Mike Allison on 5/21/15.
//
//

#include "LightManager.h"
#include "CameraComponent.h"
#include "ActorManager.h"
#include "Actor.h"
#include "Controller.h"
#include "SystemEvents.h"
#include "EventManager.h"
#include "FrustumCullComponent.h"
#include "LightComponent.h"
#include "Scene.h"
#include "CameraManager.h"
#include "AppSceneBase.h"
#include "Events.h"

using namespace ci;
using namespace ci::app;

ci::Light::Type LightManager::parseLightType( const std::string &type )
{
    if( type == "directional" )return ci::Light::Type::Directional;
    else if( type == "point" )return ci::Light::Type::Point;
    else if( type == "spot" )return ci::Light::Type::Spot;
    else if( type == "wedge" )return ci::Light::Type::Wedge;
    else if( type == "capsule" )return ci::Light::Type::Capsule;
    else return ci::Light::Type::Directional;
}

std::string LightManager::parseLightTypeToString( const ci::Light::Type &type )
{
    if( type == ci::Light::Type::Directional )return "directional";
    else if( type == ci::Light::Type::Point )return "point";
    else if( type == ci::Light::Type::Spot )return "spot";
    else if( type == ci::Light::Type::Wedge )return "wedge";
    else if( type == ci::Light::Type::Capsule )return "capsule";
    else return "directional";
}

LightManager::LightManager():mShuttingDown(false),mId(ec::getHash("light_manager"))
{
    
    mLightUboLocation = 0;
    
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &LightManager::handleShutDown), ec::ShutDownEvent::TYPE);
    Lights lights;
    lights.numLights = 0;
    lights.upDirection = vec4(0,1,0,0);
    
    mLightUbo = ci::gl::Ubo::create( sizeof(Lights), nullptr , GL_DYNAMIC_DRAW);
    mLightUbo->bindBufferBase(mLightUboLocation);
}

LightManager::~LightManager()
{
    if(!mShuttingDown){
        ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &LightManager::handleLightRegistration), ec::ReturnActorCreatedEvent::TYPE);
        ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &LightManager::handleShutDown), ec::ShutDownEvent::TYPE);
    }
}

void LightManager::handleShutDown(ec::EventDataRef)
{
    CI_LOG_V( "light_manager handle shutdown");
    mShuttingDown = true;
}

void LightManager::initShadowMap(const ci::JsonTree &init)
{
    try {
        
        auto size = init["size"].getValue<int>();
        mShadowMap = ShadowMap::create(size);
        
    } catch (const ci::JsonTree::ExcChildNotFound &e) {
        CI_LOG_W("shadow map size not found, creating default size 1024");
        mShadowMap = ShadowMap::create(1024);
    }
}

void LightManager::handleLightRegistration( ec::EventDataRef event )
{
    auto e = std::dynamic_pointer_cast<ComponentRegistrationEvent>( event );
    if( e->getType() == ComponentRegistrationEvent::RegistrationType::LIGHT ){
        CI_LOG_V("Registering light");
        mLights.push_back( e->getActorUId() );
    }
}

void LightManager::update() {
    
    bool updateAll = false;
    ///TODO: could do an event registration here instead of running through all scene lights every frame
    
    std::vector< ci::LightRef > activeLights;
    
    for( auto & light : mLights ){
        
        auto light_strong = ec::ActorManager::get()->retreiveUnique(light).lock();
        if(light_strong){
            auto light_component = light_strong->getComponent<LightComponent>().lock();
            if(light_component){
                if( light_strong->isActive() && light_component->getLight()->isVisible() ){
                    if(activeLights.size() < 24)
                        activeLights.push_back(light_component->getLight() );
                    //if( light_component->needsUpdate() ){
                        updateAll = true;
                   // }
                }
            }
        }
    }
    
    if( updateAll ){
        
        auto viewMat = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() )->cameras()->getActiveCamera().getViewMatrix();
        
        Lights lights;
        lights.numLights = activeLights.size();
        lights.upDirection = viewMat * vec4( 0, 1, 0, 0 );
        
        for( size_t i = 0; i < activeLights.size(); ++i ) {
            Light::Data light = activeLights[i]->getData( getElapsedSeconds(), viewMat );
            lights.data[i] = light;
        }
        
        mLightUbo->bufferSubData( 0, sizeof( Lights ), &lights );
        
    }
    
}
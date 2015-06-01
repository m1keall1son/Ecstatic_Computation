//
//  DebugManager.cpp
//  System_test
//
//  Created by Mike Allison on 5/21/15.
//
//

#include "DebugManager.h"
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
#include "DebugComponent.h"
#include "RenderManager.h"
#include "OculusRiftComponent.h"

using namespace ci;
using namespace ci::app;


DebugManager::DebugManager():mShuttingDown(false),mId(ec::getHash("debug_manager"))
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &DebugManager::handleShutDown), ec::ShutDownEvent::TYPE);
    
}

DebugManager::~DebugManager()
{
    if(!mShuttingDown){
        ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &DebugManager::handleShutDown), ec::ShutDownEvent::TYPE);
    }
}

void DebugManager::handleShutDown(ec::EventDataRef)
{
    CI_LOG_V( "light_manager handle shutdown");
    mShuttingDown = true;
}

void DebugManager::handleDebugComponentRegistration( ec::EventDataRef event )
{
    auto e = std::dynamic_pointer_cast<ComponentRegistrationEvent>( event );
    if( e->getType() == ComponentRegistrationEvent::RegistrationType::DEBUG_COMPONENT ){
        
        switch (e->getRegistration()) {
            case ComponentRegistrationEvent::REGISTER:
            {
                CI_LOG_V("Registering debug component");
                mDebugComponents.insert( std::make_pair( e->getActorUId(), std::dynamic_pointer_cast<DebugComponent>(e->getComponentBase() ))  );
            }
                break;
            case ComponentRegistrationEvent::UNREGISTER:
            {
                CI_LOG_V("Unregistering debug component");
                mDebugComponents.erase( e->getActorUId() );
            }
                break;
                
            default:
                break;
        }
            }
}

void DebugManager::initDebug(ec::EventDataRef event )
{
    auto e = std::dynamic_pointer_cast<ShareGeometryDepthTextureEvent>(event);
    
    vec2 size;
    
    size = getWindowSize();
    
    mDebugTexture = gl::Texture::create( size.x, size.y, gl::Texture::Format().internalFormat(GL_RGBA).minFilter(GL_LINEAR).magFilter(GL_LINEAR) );
    
    auto fmt = gl::Fbo::Format().attachment(GL_DEPTH_ATTACHMENT, e->getDepthTexture()).attachment(GL_COLOR_ATTACHMENT0, mDebugTexture);
    
    mDebugFbo = gl::Fbo::create( size.x, size.y, fmt);
    
    gl::ScopedFramebuffer fbo(mDebugFbo);
    gl::clear(GL_COLOR_BUFFER_BIT);
    gl::clearColor(ColorA(0,0,0,0));
    
}

void DebugManager::handleRiftDebugDraw(ec::EventDataRef event )
{
    auto e = std::dynamic_pointer_cast<DrawToRiftBufferEvent>(event);
    switch (e->getStyle()) {
        case DrawToRiftBufferEvent::TWICE:
        {
            if( ec::Controller::get()->debugEnabled() )handleDebugDraw(nullptr);
        }
            break;
        default:
            break;
    }
}


void DebugManager::handleDebugDraw(ec::EventDataRef)
{
    for(auto & component : mDebugComponents){
        component.second->draw();
    }
}

void DebugManager::handleDeferredDebugDraw(ec::EventDataRef)
{

    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    
    gl::ScopedFramebuffer fbo( mDebugFbo );
    gl::clear(GL_COLOR_BUFFER_BIT);
    gl::clearColor( ColorA(0,0,0,0) );
    gl::ScopedViewport view( vec2(), mDebugFbo->getSize() );

    {
        gl::disableDepthRead();
        gl::disableDepthWrite();
        gl::ScopedMatrices pushMatrix;
        gl::setMatricesWindow( mDebugFbo->getSize() );
        //grab backbuffer
        auto final_tex = RenderManager::getWindowFbo( 1 - RenderManager::getPingPong() )->getColorTexture();
        gl::draw(final_tex);
    }
    
    {
        gl::enableDepthRead();
        gl::enableDepthWrite();
        gl::ScopedMatrices pushMatrix;
        gl::setMatrices(scene->cameras()->getActiveCamera());
        
        for(auto & component : mDebugComponents){
            component.second->draw();
        }
    }
    
}

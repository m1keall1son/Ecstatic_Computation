//
//  LightPass.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#include "LightPass.h"
#include "DebugComponent.h"
#include "Actor.h"
#include "Events.h"
#include "Scene.h"
#include "AppSceneBase.h"
#include "Controller.h"
#include "TransformComponent.h"
#include "FrustumCullComponent.h"
#include "Controller.h"
#include "EventManager.h"
#include "ShadowPass.h"
#include "ShadowMap.h"
#include "GBufferPass.h"
#include "GBuffer.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType LightPass::TYPE = 0x100;

LightPassRef LightPass::create( ec::Actor* context )
{
    return LightPassRef( new LightPass(context) );
}

const PassPriority LightPass::getPriority() const
{
    return mPriority;
}

void LightPass::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void LightPass::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void LightPass::registerHandlers()
{
    //TODO this should be in initilialize with ryan's code
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
   
}
void LightPass::unregisterHandlers()
{
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );

}

bool LightPass::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    return true;
}


bool LightPass::postInit()
{
    
    ec::Controller::get()->scene().lock()->manager()->queueEvent( ComponentRegistrationEvent::create(ComponentRegistrationEvent::RegistrationType::PASS, mContext->getUId(), shared_from_this() ) );
    
    mFbo = gl::Fbo::create(getWindowWidth(), getWindowHeight(), gl::Fbo::Format().disableDepth() );
    
    
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

LightPass::LightPass( ec::Actor* context ): PassBase( context ), mId( ec::getHash( context->getName() + "_light_pass" ) ),mShuttingDown(false)
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &LightPass::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &LightPass::handleSceneChange), ec::SceneChangeEvent::TYPE);
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

LightPass::~LightPass()
{
    if(!mShuttingDown)unregisterHandlers();
}

const ec::ComponentNameType LightPass::getName() const
{
    return "light_pass";
}

const ec::ComponentUId LightPass::getId() const
{
    return mId;
}

const ec::ComponentType LightPass::getType() const
{
    return TYPE;
}

ci::JsonTree LightPass::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    
    return save;
    
}

void LightPass::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
}

void LightPass::process()
{
    gl::enableDepthRead();
    gl::disableDepthWrite();
    
    bool shadows = mContext->hasComponent(ShadowPass::TYPE);
    
    if( shadows )
        mContext->getComponent<ShadowPass>().lock()->getShadowTexture()->bind( 3 );
    
    
    auto gbuffer = mContext->getComponent<GBufferPass>().lock()->getGBuffer();
    
    gl::ScopedFramebuffer fbo( mFbo );
    
    gl::clear( GL_COLOR_BUFFER_BIT );
    
    ///TODO: texture bind locations!
    
    gl::ScopedTextureBind depthTex( gbuffer->getDepthTexture(), 10 );
    gl::ScopedTextureBind dataTex( gbuffer->getTexture("uData"), 11 );
    gl::ScopedTextureBind colorTex( gbuffer->getTexture("uAlbedo"), 12 );

    gl::ScopedMatrices pushMatrix;
    gl::setMatricesWindow(mFbo->getSize());
    gl::ScopedViewport view( vec2(0), mFbo->getSize() );
    
    mScreenSpace->draw();
    
    if(shadows)
        mContext->getComponent<ShadowPass>().lock()->getShadowTexture()->unbind();

}


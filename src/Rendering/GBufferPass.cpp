//
//  GBufferPass.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#include "GBufferPass.h"
#include "GBuffer.h"
#include "Controller.h"
#include "EventManager.h"
#include "Scene.h"
#include "Events.h"
#include "AppSceneBase.h"


using namespace ci;
using namespace ci::app;

ec::ComponentType GBufferPass::TYPE = 0x101;

GBufferPassRef GBufferPass::create( ec::Actor* context )
{
    return GBufferPassRef( new GBufferPass(context) );
}

const PassPriority GBufferPass::getPriority() const
{
    return mPriority;
}

void GBufferPass::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void GBufferPass::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void GBufferPass::registerHandlers()
{
    //TODO this should be in initilialize with ryan's code
    
}
void GBufferPass::unregisterHandlers()
{
    
}

bool GBufferPass::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    return true;
}


bool GBufferPass::postInit()
{
    
    ec::Controller::get()->scene().lock()->manager()->queueEvent( ComponentRegistrationEvent::create(ComponentRegistrationEvent::RegistrationType::PASS, mContext->getUId(), shared_from_this() ) );
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

GBufferPass::GBufferPass( ec::Actor* context ): PassBase( context ), mId( ec::getHash( context->getName() + "_gbuffer_pass" ) ),mShuttingDown(false)
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &GBufferPass::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &GBufferPass::handleSceneChange), ec::SceneChangeEvent::TYPE);
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

GBufferPass::~GBufferPass()
{
    if(!mShuttingDown)unregisterHandlers();
}

const ec::ComponentNameType GBufferPass::getName() const
{
    return "gbuffer_pass";
}

const ec::ComponentUId GBufferPass::getId() const
{
    return mId;
}

const ec::ComponentType GBufferPass::getType() const
{
    return TYPE;
}

ci::JsonTree GBufferPass::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    
    return save;
    
}

void GBufferPass::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
}

void GBufferPass::process()
{
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    
    gl::ScopedFramebuffer gbuffer( mGBuffer->getFbo() );
    gl::clear();
    
    gl::ScopedMatrices pushMatrix;
    gl::setMatrices(scene->cameras()->getActiveCamera());
    gl::ScopedViewport view( vec2(0), mGBuffer->getFbo()->getSize() );

    CI_LOG_V("draw geometry event triggered");
    scene->manager()->triggerEvent( DrawGeometryEvent::create() );
    
    
//    if( ec::Controller::get()->debugEnabled() ){
//        mSceneManager->triggerEvent( DrawDebugEvent::create() );
//        gl::drawCoordinateFrame();
//    }
    
    
    
}


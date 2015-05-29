//
//  RenderManager.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#include "RenderManager.h"
#include "Actor.h"
#include "Scene.h"
#include "AppSceneBase.h"
#include "Controller.h"
#include "Events.h"
#include "PassBase.h"

using namespace ci;
using namespace ci::app;

static ci::gl::FboRef sWindowFbo[2] = { nullptr, nullptr };
static int sPingPong = 0;

ci::gl::FboRef RenderManager::getWindowFbo( int pingPong )
{
    CI_ASSERT(pingPong == 0 || pingPong == 1);
    if( !sWindowFbo[0] || !sWindowFbo[1] ){
        sWindowFbo[0] = gl::Fbo::create(getWindowWidth(), getWindowHeight(), gl::Fbo::Format().disableDepth());
        sWindowFbo[1] = gl::Fbo::create(getWindowWidth(), getWindowHeight(), gl::Fbo::Format().disableDepth());
        
        {
            gl::ScopedFramebuffer fbo( sWindowFbo[0] );
            gl::clear();
        }
        
        {
            gl::ScopedFramebuffer fbo( sWindowFbo[1] );
            gl::clear();
        }
        
    }
    return sWindowFbo[pingPong];
}

int& RenderManager::getPingPong(){ return sPingPong; }

ec::ComponentType RenderManager::TYPE = ec::getHash("render_manager");

RenderManagerRef RenderManager::create( ec::Actor* context )
{
    return RenderManagerRef( new RenderManager(context) );
}

void RenderManager::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void RenderManager::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void RenderManager::registerHandlers()
{
    auto scene = ec::Controller::get()->scene().lock();
    scene->manager()->addListener(fastdelegate::MakeDelegate( this , &RenderManager::handleDraw), DrawEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate( this, &RenderManager::handlePassRegistration), ComponentRegistrationEvent::TYPE);

}
void RenderManager::unregisterHandlers()
{
    auto scene = ec::Controller::get()->scene().lock();
    scene->manager()->removeListener(fastdelegate::MakeDelegate( this , &RenderManager::handleDraw), DrawEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate( this, &RenderManager::handlePassRegistration), ComponentRegistrationEvent::TYPE);
}

bool RenderManager::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    return true;
}

static bool sortPasses( const PassBaseRef& pass1, const PassBaseRef& pass2 )
{
    return pass1->getPriority() < pass2->getPriority();
}

void RenderManager::handleDraw(ec::EventDataRef)
{
    if( !mSorted ){
        mPasses.sort( sortPasses );
        mSorted = true;
    }
    
    auto it = mPasses.begin();
    auto end = mPasses.end();
    while( it != end ){
        (*it)->process();
        ++it;
    }
    
}

bool RenderManager::postInit()
{
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

RenderManager::RenderManager( ec::Actor* context ): ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "_render_manager" ) ),mShuttingDown(false)
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &RenderManager::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &RenderManager::handleSceneChange), ec::SceneChangeEvent::TYPE);
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

RenderManager::~RenderManager()
{
    if(!mShuttingDown)unregisterHandlers();
}

const ec::ComponentNameType RenderManager::getName() const
{
    return "render_manager";
}

const ec::ComponentUId RenderManager::getId() const
{
    return mId;
}

const ec::ComponentType RenderManager::getType() const
{
    return TYPE;
}

ci::JsonTree RenderManager::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    
    return save;
    
}

void RenderManager::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
}

void RenderManager::handlePassRegistration(ec::EventDataRef event)
{
    auto e = std::dynamic_pointer_cast<ComponentRegistrationEvent>(event);
    if( e->getType() == ComponentRegistrationEvent::RegistrationType::PASS ){
        CI_LOG_V("Registering Pass: " + e->getComponentBase()->getName());
        mPasses.push_back( std::dynamic_pointer_cast<PassBase>(e->getComponentBase()) );
        mSorted = false;
    }
}


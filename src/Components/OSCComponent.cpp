//
//  OSCComponent.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#include "OSCComponent.h"
#include "Actor.h"
#include "Scene.h"
#include "Controller.h"
#include "Events.h"
#include "AppSceneBase.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType OSCComponent::TYPE = ec::getHash("osc_component");

OSCComponentRef OSCComponent::create( ec::Actor* context )
{
    return OSCComponentRef( new OSCComponent(context) );
}

void OSCComponent::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void OSCComponent::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void OSCComponent::registerHandlers()
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &OSCComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &OSCComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &OSCComponent::update), UpdateEvent::TYPE);

}
void OSCComponent::unregisterHandlers()
{
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &OSCComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &OSCComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &OSCComponent::update), UpdateEvent::TYPE);

}

bool OSCComponent::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    
    try {
        mListenPort = tree["listen_port"].getValue<int>();
    } catch (ci::JsonTree::ExcChildNotFound e) {
        CI_LOG_W("listening port not found, defaulting to 12000");
        mListenPort = 12000;
    }
    
    return true;
}

void OSCComponent::cleanup()
{
    unregisterHandlers();
}

bool OSCComponent::postInit()
{
    if(!mInitialized){
        mListener.setup(mListenPort);
        mInitialized = true;
    }
    
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

OSCComponent::OSCComponent( ec::Actor* context ): ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "osc_component" ) ),mShuttingDown(false)
{
    
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

OSCComponent::~OSCComponent()
{
}

const ec::ComponentNameType OSCComponent::getName() const
{
    return "osc_component";
}

const ec::ComponentUId OSCComponent::getId() const
{
    return mId;
}

const ec::ComponentType OSCComponent::getType() const
{
    return TYPE;
}

ci::JsonTree OSCComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    save.addChild( ci::JsonTree( "listen_port", mListenPort ) );
    return save;
    
}

void OSCComponent::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText( mContext->getName() +" : "+ getName());
    auto updateFn = [&]{ mListener.setup(mListenPort); };
    gui->addParam("port", &mListenPort).updateFn(updateFn);
    
}

void OSCComponent::update(ec::EventDataRef)
{
    while( mListener.hasWaitingMessages() ){
        
        auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
        
        osc::Message message;
        mListener.getNextMessage( &message );
        
        if( message.getAddress() == "/micVol" ){
            auto vol = message.getArgAsFloat(0);
            scene->manager()->triggerEvent( MicVolumeEvent::create( vol ) );
        }
        
    }
}


//
//  ComponentTemplate.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#include "ComponentTemplate.h"
#include "Actor.h"
#include "Scene.h"
#include "Controller.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType ComponentTemplate::TYPE = ec::getHash("component_template");

ComponentTemplateRef ComponentTemplate::create( ec::Actor* context )
{
    return ComponentTemplateRef( new ComponentTemplate(context) );
}

void ComponentTemplate::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void ComponentTemplate::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void ComponentTemplate::registerHandlers()
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &ComponentTemplate::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &ComponentTemplate::handleSceneChange), ec::SceneChangeEvent::TYPE);
}
void ComponentTemplate::unregisterHandlers()
{
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &ComponentTemplate::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &ComponentTemplate::handleSceneChange), ec::SceneChangeEvent::TYPE);
}

bool ComponentTemplate::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    return true;
}

void ComponentTemplate::cleanup()
{
    unregisterHandlers();
}

bool ComponentTemplate::postInit()
{
    if(!mInitialized){
        
        mInitialized = true;
    }
    
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

ComponentTemplate::ComponentTemplate( ec::Actor* context ): ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "_template" ) ),mShuttingDown(false)
{
  
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

ComponentTemplate::~ComponentTemplate()
{
}

const ec::ComponentNameType ComponentTemplate::getName() const
{
    return "template";
}

const ec::ComponentUId ComponentTemplate::getId() const
{
    return mId;
}

const ec::ComponentType ComponentTemplate::getType() const
{
    return TYPE;
}

ci::JsonTree ComponentTemplate::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    
    return save;
    
}

void ComponentTemplate::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
}


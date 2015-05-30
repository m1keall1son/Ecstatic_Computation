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
    
}
void ComponentTemplate::unregisterHandlers()
{
    
}

bool ComponentTemplate::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    return true;
}


bool ComponentTemplate::postInit()
{
    
    
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

ComponentTemplate::ComponentTemplate( ec::Actor* context ): ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "_template" ) ),mShuttingDown(false)
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &ComponentTemplate::handleShutDown), ec::ShutDownEvent::TYPE);
    registerHandlers();
    
    auto window = ci::app::getWindow();
    window->getSignalMouseUp().connect( std::bind( &ComponentTemplate::mouseUp, this , std::placeholders::_1 ) );
    window->getSignalMouseDown().connect( std::bind( &ComponentTemplate::mouseDown, this , std::placeholders::_1 ) );
    window->getSignalMouseDrag().connect( std::bind( &ComponentTemplate::mouseDrag, this , std::placeholders::_1 ) );
    window->getSignalMouseMove().connect( std::bind( &ComponentTemplate::mouseMove, this , std::placeholders::_1 ) );
    window->getSignalKeyDown().connect( std::bind( &ComponentTemplate::keyDown, this , std::placeholders::_1 ) );
    window->getSignalKeyUp().connect( std::bind( &ComponentTemplate::keyUp, this , std::placeholders::_1 ) );
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

ComponentTemplate::~ComponentTemplate()
{
    if(!mShuttingDown)unregisterHandlers();
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


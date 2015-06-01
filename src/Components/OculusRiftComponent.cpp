//
//  OculusRiftComponent.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#include "OculusRiftComponent.h"
#include "Actor.h"
#include "Scene.h"
#include "Controller.h"
#include "AppSceneBase.h"
#include "Events.h"
#include "CameraManager.h"

using namespace ci;
using namespace ci::app;

static int sRiftUboLocation = 1;
static ci::vec2 sRiftFboSize = ci::vec2(0);

const ci::vec2& OculusRiftComponent::getRiftFboSize()
{
    return sRiftFboSize;
}

int OculusRiftComponent::getRiftUboLocation()
{
    return sRiftUboLocation;
}

void OculusRiftComponent::initializeRift()
{
    hmd::RiftManager::initialize();
}

ec::ComponentType OculusRiftComponent::TYPE = ec::getHash("oculus_rift_component");

OculusRiftComponentRef OculusRiftComponent::create( ec::Actor* context )
{
    return OculusRiftComponentRef( new OculusRiftComponent(context) );
}

void OculusRiftComponent::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    mRift = nullptr;
}
void OculusRiftComponent::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void OculusRiftComponent::cleanup()
{
    unregisterHandlers();
}

void OculusRiftComponent::registerHandlers()
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &OculusRiftComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &OculusRiftComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &OculusRiftComponent::handleUpdate), UpdateEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &OculusRiftComponent::handleSwitchCamera), SwitchCameraEvent::TYPE);

}
void OculusRiftComponent::unregisterHandlers()
{
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &OculusRiftComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &OculusRiftComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &OculusRiftComponent::handleUpdate), UpdateEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &OculusRiftComponent::handleSwitchCamera), SwitchCameraEvent::TYPE);
}

void OculusRiftComponent::handleUpdate(ec::EventDataRef)
{
    
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    auto & host = scene->cameras()->getActiveCamera();
    mRift->setHostCamera( host );
    
    RiftData data;
    
    for( auto eye : mRift->getEyes() ) {
        mRift->enableEye( eye );
        auto idx = 2 * static_cast<size_t>( eye );
        data.matrices[ idx ] = mRift->getViewMatrix();
        data.matrices[ idx + 1 ] = mRift->getProjectionMatrix();
    }
    
    auto & cam = mRift->getHostCamera();
    
    data.farClip = cam.getFarClip();
    data.nearClip = cam.getNearClip();
    data.reserved0 = 0;
    data.reserved1 = 0;
    
    mRiftUbo->bufferSubData( 0, sizeof( RiftData ), &data );
    
}

bool OculusRiftComponent::initialize( const ci::JsonTree &tree )
{
    if(!mInitialized){
        CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    }
    return true;
}


bool OculusRiftComponent::postInit()
{
    if(!mInitialized){
        auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
        auto & host = scene->cameras()->getCamera(CameraComponent::CameraType::MAIN_CAMERA);
        host.setAspectRatio(mRift->getFboSize().x/mRift->getFboSize().y);
        mRift->setHostCamera( host );
        
        mRiftUbo = ci::gl::Ubo::create( sizeof(RiftData), nullptr , GL_DYNAMIC_DRAW);
        mRiftUbo->bindBufferBase(sRiftUboLocation);
        
        CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
        mInitialized = true;
    }
    ///this could reflect errors...
    return true;
}

void OculusRiftComponent::handleSwitchCamera(ec::EventDataRef event)
{
    auto e = std::dynamic_pointer_cast<SwitchCameraEvent>(event);
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    auto & new_host = scene->cameras()->getCamera(e->getType());
    new_host.setAspectRatio(mRift->getFboSize().x/mRift->getFboSize().y);
    mRift->setHostCamera(new_host);
    
}

OculusRiftComponent::OculusRiftComponent( ec::Actor* context ): ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "_oculus_rift_component" ) ),mShuttingDown(false)
{
    
    registerHandlers();
    
    auto window = ci::app::getWindow();
    window->getSignalKeyDown().connect( std::bind( &OculusRiftComponent::keyDown, this , std::placeholders::_1 ) );
    window->getSignalKeyUp().connect( std::bind( &OculusRiftComponent::keyUp, this , std::placeholders::_1 ) );
    
    mRift = hmd::OculusRiftRef( new hmd::OculusRift );
    
    if( mRift->attachToWindow( window ) ) {
        if( mRift->isDesktopExtended() )
            app::setFullScreen();
        else
            app::setWindowSize( mRift->getNativeWindowResolution() );
        
        mRift->setScreenPercentage( 1.25f );
    }
    
    sRiftFboSize = getWindowSize();
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
    //for instanced stereo rendering
    gl::enable( GL_CLIP_DISTANCE0, true );
}

OculusRiftComponent::~OculusRiftComponent()
{
}

const ec::ComponentNameType OculusRiftComponent::getName() const
{
    return "oculus_rift_component";
}

const ec::ComponentUId OculusRiftComponent::getId() const
{
    return mId;
}

const ec::ComponentType OculusRiftComponent::getType() const
{
    return TYPE;
}

void OculusRiftComponent::keyUp(ci::app::KeyEvent & event)
{
    
}

void OculusRiftComponent::keyDown(ci::app::KeyEvent & event)
{
    
}

ci::JsonTree OculusRiftComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    
    return save;
    
}

void OculusRiftComponent::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
}


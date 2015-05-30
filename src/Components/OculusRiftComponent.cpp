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

using namespace ci;
using namespace ci::app;

static int sRiftUboLocation = 1;

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
    
}
void OculusRiftComponent::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void OculusRiftComponent::registerHandlers()
{
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &OculusRiftComponent::handleUpdate), UpdateEvent::TYPE);
    
}
void OculusRiftComponent::unregisterHandlers()
{
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &OculusRiftComponent::handleUpdate), UpdateEvent::TYPE);
    
}

void OculusRiftComponent::handleUpdate(ec::EventDataRef)
{
    
    mat4 worldToEyeClipMatrices[4];
    
    {
        hmd::ScopedBind bind{ *mRift };
        
        for( auto eye : mRift->getEyes() ) {
            gl::ScopedMatrices push;
            mRift->enableEye( eye );
            auto idx = 2 * static_cast<size_t>( eye );
            worldToEyeClipMatrices[ idx ] = mRift->getViewMatrix();
            worldToEyeClipMatrices[idx + 1] = mRift->getProjectionMatrix();
        }
    }
    
    RiftData data;
    for(int i=0;i<4;i++)
        data.matrices[i] = worldToEyeClipMatrices[i];
    
    mRiftUbo->bufferSubData( 0, sizeof( RiftData ), &data );
    
}

bool OculusRiftComponent::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    return true;
}


bool OculusRiftComponent::postInit()
{
    mRift = hmd::OculusRiftRef( new hmd::OculusRift );
    
    if( mRift->attachToWindow( getWindow() ) ) {
        if( mRift->isDesktopExtended() )
            app::setFullScreen();
        else
            app::setWindowSize( mRift->getNativeWindowResolution() );
        
        auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
        
        auto host = scene->cameras()->getCamera(CameraComponent::CameraType::MAIN_CAMERA);
        mRift->setHostCamera( host );
        mRift->setScreenPercentage( 1.25f );
    }
    
    mRiftUbo = ci::gl::Ubo::create( sizeof(RiftData), nullptr , GL_DYNAMIC_DRAW);
    mRiftUbo->bindBufferBase(sRiftUboLocation);
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

OculusRiftComponent::OculusRiftComponent( ec::Actor* context ): ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "_oculus_rift_component" ) ),mShuttingDown(false)
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &OculusRiftComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &OculusRiftComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

OculusRiftComponent::~OculusRiftComponent()
{
    if(!mShuttingDown)unregisterHandlers();
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


//
//  CameraComponent.cpp
//  System_test
//
//  Created by Mike Allison on 5/21/15.
//
//

#include "CameraComponent.h"
#include "TransformComponent.h"
#include "Actor.h"
#include "ComponentBase.h"
#include "AppSceneBase.h"
#include "Controller.h"
#include "Scene.h"
#include "Events.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType CameraComponent::TYPE = 0x012;

CameraComponentRef CameraComponent::create( ec::Actor * context )
{
    return CameraComponentRef( new CameraComponent(context) );
}

CameraComponent::CameraComponent( ec::Actor* context ) : ec::ComponentBase( context ), mId(ec::getHash( context->getName() + "camera_component" )),mShuttingDown(false), mNear(.1), mFar(1000.), mFov(60)
{
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &CameraComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &CameraComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);

    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
}

CameraComponent::~CameraComponent()
{
    if(!mShuttingDown)unregisterHandlers();
}

void CameraComponent::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;

}
void CameraComponent::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void CameraComponent::registerHandlers()
{
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &CameraComponent::update), UpdateEvent::TYPE);
}
void CameraComponent::unregisterHandlers()
{
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &CameraComponent::update), UpdateEvent::TYPE);
}

bool CameraComponent::initialize( const ci::JsonTree &tree )
{
    //init base class
    
    try {
        
        mFov = tree["fov"].getValue<float>();
        mNear = tree["near"].getValue<float>();
        mFar = tree["far"].getValue<float>();
        auto aspect = tree["aspect"].getValue();
        
        if(aspect == "window")
            mCamera.setPerspective( mFov, getWindowAspectRatio(), mNear, mFar);
        else
            mCamera.setPerspective(mFov, std::stof(aspect), mNear, mFar);
        
    } catch ( JsonTree::ExcChildNotFound ex	) {
        CI_LOG_W(std::string(ex.what())+" setting default camera");
        mCamera.setPerspective(60, getWindowAspectRatio(), .1, 1000);
    }
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialized");
    
    return true;
}

ci::JsonTree CameraComponent::serialize()
{
    auto save = ci::JsonTree();
    
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    save.addChild( ci::JsonTree( "fov", mFov ) );
    save.addChild( ci::JsonTree( "near", mNear ) );
    save.addChild( ci::JsonTree( "far", mFar ) );
    save.addChild( ci::JsonTree( "aspect", mCamera.getAspectRatio() ) );
    
    return save;
    
}

const ec::ComponentNameType CameraComponent::getName() const
{
    return "camera_component";
}

const ec::ComponentUId CameraComponent::getId() const
{
    return mId;
}

const ec::ComponentType CameraComponent::getType() const
{
    return TYPE;
}

void CameraComponent::update(ec::EventDataRef event)
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" update");
    //TODO: transform must have target (pointer to another actor location), lookat, view direction,
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    auto t = transform->getTranslation();
    mCamera.setEyePoint( t );
    mCamera.setOrientation(transform->getRotation());
    // mCamera.setViewDirection( glm::eulerAngles( getRotation() ) );
}

void CameraComponent::updateCameraParams()
{
    mCamera.setFarClip(mFar);
    mCamera.setFov(mFov);
    mCamera.setNearClip(mNear);
}

void CameraComponent::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
    auto updateFn = std::bind(&CameraComponent::updateCameraParams, this);
    gui->addParam("FOV", &mFov).updateFn(updateFn);
    gui->addParam("Far", &mFar).updateFn(updateFn);
    gui->addParam("Near", &mNear).updateFn(updateFn);

}


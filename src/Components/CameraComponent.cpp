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

CameraComponent::CameraComponent( ec::Actor* context ) : ec::ComponentBase( context ), mId(ec::getHash( context->getName() + "camera_component" ))
{
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &CameraComponent::update), UpdateEvent::TYPE);

    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
}

CameraComponent::~CameraComponent()
{
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &CameraComponent::update), UpdateEvent::TYPE);
}

bool CameraComponent::initialize( const ci::JsonTree &tree )
{
    //init base class
    
    try {
        
        auto fov = tree["fov"].getValue<float>();
        auto near = tree["near"].getValue<float>();
        auto far = tree["far"].getValue<float>();
        auto aspect = tree["aspect"].getValue();
        
        if(aspect == "window")
            mCamera.setPerspective( fov, getWindowAspectRatio(), near, far);
        else
            mCamera.setPerspective(fov, std::stof(aspect), near, far);
        
    } catch ( JsonTree::ExcChildNotFound ex	) {
        CI_LOG_W(std::string(ex.what())+" setting default camera");
        mCamera.setPerspective(60, getWindowAspectRatio(), .1, 1000);
    }
    
    mUI.setCurrentCam(mCamera);
    mUI.connect(ci::app::getWindow());
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialized");
    
    return true;
}

ci::JsonTree CameraComponent::serialize()
{
    auto save = ci::JsonTree();
    
    save.addChild( ci::JsonTree( "name", getName() ) );
    save.addChild( ci::JsonTree( "id", getId() ) );
    save.addChild( ci::JsonTree( "type", "camera_component" ) );
    save.addChild( ci::JsonTree( "fov", mCamera.getFov() ) );
    save.addChild( ci::JsonTree( "near", mCamera.getNearClip() ) );
    save.addChild( ci::JsonTree( "far", mCamera.getFarClip() ) );
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
    mCamera.lookAt( t, vec3(0) );
    // mCamera.setViewDirection( glm::eulerAngles( getRotation() ) );
}


//
//  FrustumCullComponent.cpp
//  System_test
//
//  Created by Mike Allison on 5/22/15.
//
//
#include "FrustumCullComponent.h"
#include "CameraManager.h"
#include "Actor.h"
#include "AppSceneBase.h"
#include "Controller.h"
#include "TransformComponent.h"
#include "DebugComponent.h"
#include "Events.h"

ec::ComponentType FrustumCullComponent::TYPE = 0x013;

FrustumCullComponentRef FrustumCullComponent::create(ec::Actor *context)
{
    return FrustumCullComponentRef( new FrustumCullComponent( context ) );
}

FrustumCullComponent::FrustumCullComponent( ec::Actor * context ): ec::ComponentBase(context), mId( ec::getHash( context->getName()+"_frustum_cull_component" ) ),mShuttingDown(false)
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    registerHandlers();
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &FrustumCullComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &FrustumCullComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
}

FrustumCullComponent::~FrustumCullComponent()
{
    if(!mShuttingDown)unregisterHandlers();
}

void FrustumCullComponent::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
}
void FrustumCullComponent::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void FrustumCullComponent::registerHandlers()
{
    auto scene = ec::Controller::get()->scene().lock();
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &FrustumCullComponent::cull), CullEvent::TYPE);
}
void FrustumCullComponent::unregisterHandlers()
{
    auto scene = ec::Controller::get()->scene().lock();
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &FrustumCullComponent::cull), CullEvent::TYPE);
}

void FrustumCullComponent::cull( ec::EventDataRef )
{
 
    CI_LOG_V( mContext->getName() + " : "+getName()+" cull");
    
    if( mContext->isActive() ){
        
        auto scene = std::dynamic_pointer_cast<AppSceneBase>(ec::Controller::get()->scene().lock());
        if(scene){
            ci::Frustumf visibleWorld( scene->cameras()->getCamera(CameraManager::CameraType::MAIN_CAMERA) );
            ci::AxisAlignedBox3f worldBoundingBox;
            auto transform = mContext->getComponent<ec::TransformComponent>().lock();
            auto boundingbox = mContext->getComponent<DebugComponent>().lock()->getAxisAlignedBoundingBox();
            worldBoundingBox = boundingbox.transformed( transform->getModelMatrix() );
            mIsVisible = !visibleWorld.intersects( worldBoundingBox );
        }
        else
            CI_LOG_E("no scene");
    }
}

ci::JsonTree FrustumCullComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "name", getName() ) );
    save.addChild( ci::JsonTree( "id", getId() ) );
    save.addChild( ci::JsonTree( "type", "frustum_cull_component" ) );
    save.addChild( ci::JsonTree( "current_visibility", mIsVisible ) );
    return save;
}

const ec::ComponentNameType FrustumCullComponent::getName() const
{
    return "frustum_cull_component";
}
const ec::ComponentUId FrustumCullComponent::getId() const
{
    return mId;
}
const ec::ComponentType FrustumCullComponent::getType() const
{
    return TYPE;
}

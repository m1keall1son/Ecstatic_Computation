//
//  ShadowPass.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#include "ShadowPass.h"
#include "ShadowMap.h"
#include "Controller.h"
#include "EventManager.h"
#include "Scene.h"
#include "AppSceneBase.h"
#include "Events.h"
#include "ActorManager.h"
#include "Light.h"
#include "LightComponent.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType ShadowPass::TYPE = 0x103;

ShadowPassRef ShadowPass::create( ec::Actor* context )
{
    return ShadowPassRef( new ShadowPass(context) );
}

const PassPriority ShadowPass::getPriority() const
{
    return mPriority;
}

void ShadowPass::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void ShadowPass::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void ShadowPass::registerHandlers()
{
    
}
void ShadowPass::unregisterHandlers()
{
    
}

bool ShadowPass::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    return true;
}


bool ShadowPass::postInit()
{
    
    ec::Controller::get()->scene().lock()->manager()->queueEvent( ComponentRegistrationEvent::create(ComponentRegistrationEvent::RegistrationType::PASS, mContext->getUId(), shared_from_this() ) );
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

ShadowPass::ShadowPass( ec::Actor* context ): PassBase( context ), mId( ec::getHash( context->getName() + "_shadow_pass" ) ),mShuttingDown(false)
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &ShadowPass::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &ShadowPass::handleSceneChange), ec::SceneChangeEvent::TYPE);
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

ShadowPass::~ShadowPass()
{
    if(!mShuttingDown)unregisterHandlers();
}

const ec::ComponentNameType ShadowPass::getName() const
{
    return "shadow_pass";
}

const ec::ComponentUId ShadowPass::getId() const
{
    return mId;
}

const ec::ComponentType ShadowPass::getType() const
{
    return TYPE;
}

ci::JsonTree ShadowPass::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    
    return save;
    
}

void ShadowPass::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
}

ci::gl::Texture2dRef ShadowPass::getShadowTexture(){ return mShadowMap->getTexture(); }

void ShadowPass::process()
{
    gl::ScopedFramebuffer shadow_buffer( mShadowMap->getFbo() );
    
    gl::clear();
    
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    
    for( auto & light_id : scene->lights()->getLights() ){
        
        auto light_actor = ec::ActorManager::get()->retreiveUnique(light_id).lock();
        
        if(light_actor){
            
            auto light_component = light_actor->getComponent<LightComponent>().lock();
            auto light = light_component->getLight();
            if( light->hasShadows() )
            {
                if( light->getType() == Light::Type::Spot ){
                    
                    auto spot_light = std::dynamic_pointer_cast<SpotLight>(light);
                    gl::ScopedMatrices pushMatrix;
                    gl::setViewMatrix( spot_light->getViewMatrix() );
                    gl::setProjectionMatrix( spot_light->getProjectionMatrix() );
                    gl::setModelMatrix(mat4());
                    auto shadow_view_mapping = spot_light->getMapping();
                    gl::ScopedViewport shadow_view( vec2( shadow_view_mapping.x, shadow_view_mapping.y ), vec2( shadow_view_mapping.x + shadow_view_mapping.z, shadow_view_mapping.y + shadow_view_mapping.w ) );
                    
                    CI_LOG_V("draw shadow event triggered");
                    scene->manager()->triggerEvent( DrawShadowEvent::create() );
                    
                }
                
            }
        }
        
    }

}



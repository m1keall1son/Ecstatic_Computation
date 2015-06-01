//
//  ForwardPass.cpp
//  Oculus_Component_test
//
//  Created by Mike Allison on 5/30/15.
//
//

#include "ForwardPass.h"
#include "ShadowMap.h"
#include "Controller.h"
#include "EventManager.h"
#include "Scene.h"
#include "AppSceneBase.h"
#include "Events.h"
#include "ActorManager.h"
#include "Light.h"
#include "LightComponent.h"
#include "ShadowPass.h"
#include "OculusRiftComponent.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType ForwardPass::TYPE =  ec::getHash("forward_pass");

ForwardPassRef ForwardPass::create( ec::Actor* context )
{
    return ForwardPassRef( new ForwardPass(context) );
}

const PassPriority ForwardPass::getPriority() const
{
    return mPriority;
}

void ForwardPass::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void ForwardPass::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void ForwardPass::registerHandlers()
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &ForwardPass::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &ForwardPass::handleSceneChange), ec::SceneChangeEvent::TYPE);

}
void ForwardPass::unregisterHandlers()
{
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &ForwardPass::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &ForwardPass::handleSceneChange), ec::SceneChangeEvent::TYPE);

}

bool ForwardPass::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    
    try {
        auto val = tree["clear_color"].getChildren();
        auto end = val.end();
        ci::vec4 final;
        int i = 0;
        for( auto it = val.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        mClearColor = final;
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find noise freq, setting default vec2(.01, .01)");
        mClearColor =ColorA(0,0,0,1);
    }
    
    return true;
}


bool ForwardPass::postInit()
{
    if(!mInitialized){
        
        ec::Controller::get()->scene().lock()->manager()->queueEvent( ComponentRegistrationEvent::create(ComponentRegistrationEvent::REGISTER, ComponentRegistrationEvent::RegistrationType::PASS, mContext->getUId(), shared_from_this() ) );
        
        CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
        
        mInitialized = true;
    }
    
    ///this could reflect errors...
    return true;
}

ForwardPass::ForwardPass( ec::Actor* context ): PassBase( context ), mId( ec::getHash( context->getName() + "_forward_pass" ) ),mShuttingDown(false),mPriority(1)
{
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

ForwardPass::~ForwardPass()
{
}

void ForwardPass::cleanup()
{
    unregisterHandlers();
}

const ec::ComponentNameType ForwardPass::getName() const
{
    return "forward_pass";
}

const ec::ComponentUId ForwardPass::getId() const
{
    return mId;
}

const ec::ComponentType ForwardPass::getType() const
{
    return TYPE;
}

ci::JsonTree ForwardPass::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    
    return save;
    
}

void ForwardPass::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
    gui->addParam("clear", &mClearColor);
}

void ForwardPass::process()
{
    
    CI_LOG_V(mContext->getName() + " : " + getName() + " process " );
    
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    bool shadows = mContext->hasComponent(ShadowPass::TYPE);
    if( shadows )
        mContext->getComponent<ShadowPass>().lock()->getShadowTexture()->bind( 3 );
    
    if( !ec::Controller::isRiftEnabled() ){
        
        gl::clear( mClearColor );
        
        gl::ScopedMatrices pushMatrix;
        gl::setMatrices( scene->cameras()->getActiveCamera() );
        gl::ScopedViewport view( vec2(0), getWindowSize() );
        
        scene->manager()->triggerEvent(DrawToMainBufferEvent::create());
        
        if( ec::Controller::get()->debugEnabled() )scene->manager()->triggerEvent(DrawDebugEvent::create());
        
    }else{
        
        auto rift = ec::ActorManager::get()->retreiveUnique(ec::getHash("rift")).lock();
        auto rift_component = rift->getComponent<OculusRiftComponent>().lock();
        auto oculus = rift_component->getRift();
        
        hmd::ScopedBind bind{ *oculus };
        
        gl::clear();

        for(auto & eye : oculus->getEyes()){
            gl::ScopedMatrices push;
            oculus->enableEye( eye );
            scene->manager()->triggerEvent(DrawToRiftBufferEvent::create( DrawToRiftBufferEvent::Style::TWICE, (int)eye ));
        }
        
        {
            gl::ScopedViewport view( vec2(0), oculus->getFboSize() );
            scene->manager()->triggerEvent(DrawToRiftBufferEvent::create( DrawToRiftBufferEvent::Style::STEREO ));
        }
    }

    if(shadows)
        mContext->getComponent<ShadowPass>().lock()->getShadowTexture()->unbind();
    
    
}



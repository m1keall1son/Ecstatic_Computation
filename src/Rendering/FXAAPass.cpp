//
//  FXAAPass.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#include "FXAAPass.h"
#include "Actor.h"
#include "Scene.h"
#include "Controller.h"
#include "Events.h"
#include "RenderManager.h"
#include "AppSceneBase.h"
#include "OculusRiftComponent.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType FXAAPass::TYPE = ec::getHash("FXAA_pass");

FXAAPassRef FXAAPass::create( ec::Actor* context )
{
    return FXAAPassRef( new FXAAPass(context) );
}

void FXAAPass::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void FXAAPass::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void FXAAPass::registerHandlers()
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &FXAAPass::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &FXAAPass::handleSceneChange), ec::SceneChangeEvent::TYPE);
}
void FXAAPass::unregisterHandlers()
{
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &FXAAPass::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &FXAAPass::handleSceneChange), ec::SceneChangeEvent::TYPE);
}

bool FXAAPass::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    return true;
}


bool FXAAPass::postInit()
{
    if(!mInitialized){
        CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
        
        try {
            mFXAARender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/pass.vert")).fragment(loadAsset("shaders/fxaa.frag")).preprocess(true) );
        } catch (gl::GlslProgCompileExc e) {
            CI_LOG_E(e.what());
        }
        
        mFXAARender->uniform( "uSampler", 4 );
        mFXAARender->uniform( "uPixel", vec2(1.) / vec2(getWindowSize()) );
        
        mScreenQuad = gl::Batch::create(geom::Plane().size(getWindowSize()).origin(vec3(getWindowCenter(),0.)).normal(vec3(0,0,1)), mFXAARender);
        
        ec::Controller::get()->scene().lock()->manager()->queueEvent( ComponentRegistrationEvent::create( ComponentRegistrationEvent::REGISTER, ComponentRegistrationEvent::RegistrationType::PASS, mContext->getUId(), shared_from_this() ) );
        
        mInitialized = true;
    }
    ///this could reflect errors...
    return true;
}

FXAAPass::FXAAPass( ec::Actor* context ): PassBase( context ), mId( ec::getHash( context->getName() + "_FXAA_pass" ) ),mShuttingDown(false), mPriority(3)
{
   
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

FXAAPass::~FXAAPass()
{
}

void FXAAPass::cleanup()
{
    unregisterHandlers();
}

const ec::ComponentNameType FXAAPass::getName() const
{
    return "FXAA_pass";
}

const ec::ComponentUId FXAAPass::getId() const
{
    return mId;
}

const ec::ComponentType FXAAPass::getType() const
{
    return TYPE;
}

ci::JsonTree FXAAPass::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    
    return save;
    
}

void FXAAPass::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
}


const PassPriority FXAAPass::getPriority() const
{
    return mPriority;
}

void FXAAPass::process()
{
    CI_LOG_V(mContext->getName() + " : " + getName() + " process " );
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    
    auto & pingpong = RenderManager::getPingPong();
    
    auto window_fbo = RenderManager::getWindowFbo( pingpong );
    auto back_buffer = RenderManager::getWindowFbo( 1 - pingpong )->getColorTexture();
    
    {
        gl::ScopedFramebuffer fbo( window_fbo );
        gl::clear();
        gl::setMatricesWindow(window_fbo->getSize());
        gl::ScopedViewport view( vec2(0), window_fbo->getSize() );
        gl::ScopedTextureBind back( back_buffer, 4 );
        mFXAARender->uniform( "uPixel", vec2(1.) / vec2(getWindowSize()) );
        mScreenQuad->draw();
        
    }
    
    pingpong = 1 - pingpong;

    //TODO: somehow imply if this is the last step, render manager will probably have to check
    scene->manager()->triggerEvent(FinishRenderEvent::create(window_fbo->getColorTexture()));

    
}


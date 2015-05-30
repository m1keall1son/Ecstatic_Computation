//
//  LightPass.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#include "LightPass.h"
#include "DebugComponent.h"
#include "Actor.h"
#include "Events.h"
#include "Scene.h"
#include "AppSceneBase.h"
#include "Controller.h"
#include "TransformComponent.h"
#include "FrustumCullComponent.h"
#include "Controller.h"
#include "EventManager.h"
#include "ShadowPass.h"
#include "ShadowMap.h"
#include "GBufferPass.h"
#include "GBuffer.h"
#include "OculusRiftComponent.h"
#include "RenderManager.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType LightPass::TYPE = ec::getHash("light_pass");

LightPassRef LightPass::create( ec::Actor* context )
{
    return LightPassRef( new LightPass(context) );
}

const PassPriority LightPass::getPriority() const
{
    return mPriority;
}

void LightPass::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void LightPass::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void LightPass::registerHandlers()
{
    //TODO this should be in initilialize with ryan's code
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &LightPass::handleGlslProgReload), ReloadGlslProgEvent::TYPE);
    
}
void LightPass::unregisterHandlers()
{
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &LightPass::handleGlslProgReload), ReloadGlslProgEvent::TYPE);
}

bool LightPass::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    return true;
}


void LightPass::handleGlslProgReload(ec::EventDataRef)
{
    
    try {
        mSSLightingRender = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("shaders/ss_lighting.vert")).fragment(loadAsset("shaders/ss_lighting.frag")).preprocess(true));
    } catch (const gl::GlslProgCompileExc &e ) {
        CI_LOG_E(e.what());
    }
    
    try {
        mSSLightingRiftRender = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("shaders/ss_lighting_rift.vert")).fragment(loadAsset("shaders/ss_lighting_rift.frag")).preprocess(true));
    } catch (const gl::GlslProgCompileExc &e ) {
        CI_LOG_E(e.what());
    }
    
    if(mScreenSpace){
        if(!ec::Controller::isRiftEnabled()){
            mScreenSpace->replaceGlslProg(mSSLightingRender);
        }else{
            mScreenSpace->replaceGlslProg(mSSLightingRiftRender);
        }
    }

}

bool LightPass::postInit()
{

    handleGlslProgReload(ec::EventDataRef());
    
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    
    mSSLightingRender->uniformBlock("uLights", scene->lights()->getLightUboLocation());
    mSSLightingRender->uniform("uShadowMap", 3);
    mSSLightingRender->uniform("uGBufferDepthTexture", 10);
    mSSLightingRender->uniform("uData", 11);
    mSSLightingRender->uniform("uAlbedo", 12);
    
    mSSLightingRiftRender->uniformBlock("uLights", scene->lights()->getLightUboLocation());
    mSSLightingRiftRender->uniformBlock("uRift", OculusRiftComponent::getRiftUboLocation() );
    mSSLightingRiftRender->uniform("uShadowMap", 3);
    mSSLightingRiftRender->uniform("uGBufferDepthTexture", 10);
    mSSLightingRiftRender->uniform("uData", 11);
    mSSLightingRiftRender->uniform("uAlbedo", 12);
    
    if(ec::Controller::isRiftEnabled())
        mScreenSpace = gl::Batch::create( geom::Plane().size(getWindowSize()).origin(vec3(getWindowCenter(),0.)).normal(vec3(0,0,1)) , mSSLightingRiftRender);
    else
        mScreenSpace = gl::Batch::create( geom::Plane().size(getWindowSize()).origin(vec3(getWindowCenter(),0.)).normal(vec3(0,0,1)) , mSSLightingRender);


    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ec::Controller::get()->scene().lock()->manager()->queueEvent( ComponentRegistrationEvent::create(ComponentRegistrationEvent::RegistrationType::PASS, mContext->getUId(), shared_from_this() ) );
    
    ///this could reflect errors...
    return true;
}

LightPass::LightPass( ec::Actor* context ): PassBase( context ), mId( ec::getHash( context->getName() + "_light_pass" ) ),mShuttingDown(false),mPriority(2)
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &LightPass::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &LightPass::handleSceneChange), ec::SceneChangeEvent::TYPE);
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

LightPass::~LightPass()
{
    if(!mShuttingDown)unregisterHandlers();
}

const ec::ComponentNameType LightPass::getName() const
{
    return "light_pass";
}

const ec::ComponentUId LightPass::getId() const
{
    return mId;
}

const ec::ComponentType LightPass::getType() const
{
    return TYPE;
}

ci::JsonTree LightPass::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    
    return save;
    
}

void LightPass::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
}

void LightPass::process()
{
    gl::enableDepthRead();
    gl::disableDepthWrite();
    
    CI_LOG_V(mContext->getName() + " : " + getName() + " process " );
    
    auto gbuffer = mContext->getComponent<GBufferPass>().lock()->getGBuffer();
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    
    auto & pingpong = RenderManager::getPingPong();
    auto window_fbo = RenderManager::getWindowFbo( pingpong );
    
    {
        gl::ScopedFramebuffer fbo( window_fbo );
        
        gl::clear( GL_COLOR_BUFFER_BIT );
        
        ///TODO: texture bind locations!
        
        auto depth = gbuffer->getDepthTexture();
        auto data = gbuffer->getTexture("uData");
        auto albedo = gbuffer->getTexture("uAlbedo");
        
        gl::ScopedTextureBind depthTex( depth, 10 );
        gl::ScopedTextureBind dataTex( data, 11 );
        gl::ScopedTextureBind colorTex( albedo, 12 );
        
        gl::ScopedMatrices pushMatrix;
        gl::setMatricesWindow(window_fbo->getSize());
        gl::ScopedViewport view( vec2(0), window_fbo->getSize() );
        
        auto glsl = mScreenSpace->getGlslProg();
        
        if(!ec::Controller::isRiftEnabled()){
            
            auto cam = scene->cameras()->getActiveCamera();
            
            float nearClip					= cam.getNearClip();
            float farClip					= cam.getFarClip();
            vec2 projectionParams			= vec2( farClip / ( farClip - nearClip ),
                                                   ( -farClip * nearClip ) / ( farClip - nearClip ) );
            
            glsl->uniform("uProjectionParams", projectionParams );
            glsl->uniform("uProjMatrixInverse", inverse( cam.getProjectionMatrix() ) );
            
        }
        
        bool shadows = mContext->hasComponent(ShadowPass::TYPE);
        
        if( shadows )
            mContext->getComponent<ShadowPass>().lock()->getShadowTexture()->bind( 3 );
        
        mScreenSpace->draw();
        
        if(shadows)
            mContext->getComponent<ShadowPass>().lock()->getShadowTexture()->unbind();
        
    }
    
    pingpong = 1 - pingpong;

}


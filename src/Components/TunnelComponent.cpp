//
//  CubeTunnelComponent.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/28/15.
//
//

#include "TunnelComponent.h"
#include "DebugComponent.h"
#include "Actor.h"
#include "Events.h"
#include "Scene.h"
#include "AppSceneBase.h"
#include "Controller.h"
#include "TransformComponent.h"
#include "FrustumCullComponent.h"
#include "cinder/Perlin.h"
#include "OculusRiftComponent.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType TunnelComponent::TYPE = ec::getHash("tunnel_component");

TunnelComponentRef TunnelComponent::create( ec::Actor* context )
{
    return TunnelComponentRef( new TunnelComponent(context) );
}

void TunnelComponent::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void TunnelComponent::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void TunnelComponent::registerHandlers()
{
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &TunnelComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &TunnelComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);

    //TODO this should be in initilialize with ryan's code
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &TunnelComponent::handleGlslProgReload), ReloadGlslProgEvent::TYPE);
    //scene->manager()->addListener(fastdelegate::MakeDelegate(this, &TunnelComponent::drawShadow), DrawShadowEvent::TYPE);
    //scene->manager()->addListener(fastdelegate::MakeDelegate(this, &TunnelComponent::update), UpdateEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &TunnelComponent::draw), DrawToMainBufferEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &TunnelComponent::drawRift), DrawToRiftBufferEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &TunnelComponent::drawGeometry), DrawGeometryEvent::TYPE);
}

void TunnelComponent::unregisterHandlers()
{
    ec::Controller::get()->eventManager()->removeListener(fastdelegate::MakeDelegate(this, &TunnelComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->removeListener(fastdelegate::MakeDelegate(this, &TunnelComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);

    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &TunnelComponent::handleGlslProgReload), ReloadGlslProgEvent::TYPE);
    //scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &TunnelComponent::drawShadow), DrawShadowEvent::TYPE);
    //scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &TunnelComponent::update), UpdateEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &TunnelComponent::drawRift), DrawToRiftBufferEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &TunnelComponent::draw), DrawToMainBufferEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &TunnelComponent::drawGeometry), DrawGeometryEvent::TYPE);
}

void TunnelComponent::update(ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" update");
    //
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    //    transform->setRotation( glm::toQuat( ci::rotate( (float)getElapsedSeconds(), vec3(1.) ) ) );
}

bool TunnelComponent::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    
    try{
        mFaceRadius = tree["face_diameter"].getValue<float>();
    }catch( ci::JsonTree::ExcChildNotFound e ){
        CI_LOG_W("didn't find face diameter, setting default 10.f");
        mFaceRadius = 10.;
    }
    
    try{
        mLength = tree["length"].getValue<float>();
    }catch( ci::JsonTree::ExcChildNotFound e ){
        CI_LOG_W("didn't find face diameter, setting default 5000.f");
        mLength = 5000.;
    }
    
    try{
        mStep = tree["step"].getValue<float>();
    }catch( ci::JsonTree::ExcChildNotFound e ){
        CI_LOG_W("didn't find face diameter, setting default 100.f");
        mStep = 100.f;
    }
    
    
    try {
        auto val = tree["amplitude"].getChildren();
        auto end = val.end();
        ci::vec2 final;
        int i = 0;
        for( auto it = val.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        mAmplitude = final;
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find amplitude, setting default vec2(15.)");
        mAmplitude = vec2(15.);
    }
    
    try {
        auto val = tree["frequency"].getChildren();
        auto end = val.end();
        ci::vec2 final;
        int i = 0;
        for( auto it = val.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        mFrequency = final;
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find frequency, setting default vec2(.01, .05)");
        mFrequency = vec2(.01,.05);
    }
    
    try {
        auto val = tree["noise_frequency"].getChildren();
        auto end = val.end();
        ci::vec2 final;
        int i = 0;
        for( auto it = val.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        mNoiseFrequency = final;
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find noise freq, setting default vec2(.01, .01)");
        mNoiseFrequency = vec2(.01,.01);
    }
    
    try {
        auto val = tree["noise_amplitude"].getChildren();
        auto end = val.end();
        ci::vec2 final;
        int i = 0;
        for( auto it = val.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        mNoiseAmplitude = final;
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find noise amplitude, setting default vec2(.1, .1)");
        mNoiseAmplitude = vec2(.1,.1);
    }
    
    
    return true;
}

void TunnelComponent::drawShadow( ec::EventDataRef event )
{
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" drawShadow");
    
    auto visible = mContext->getComponent<FrustumCullComponent>().lock()->isVisible();
    if(!visible) return;
    
    //gl::ScopedFaceCulling pushFace(true,GL_BACK);
    
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    auto glsl = mTunnelShadow->getGlslProg();
    glsl->uniform("uNoiseScale", mNoiseScale);
    mTunnelShadow->draw();
    //shadow draw?
    
}

void TunnelComponent::drawRift( ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw");
    
    auto e = std::dynamic_pointer_cast<DrawToRiftBufferEvent>(event);
    
    switch (e->getStyle()) {
        case DrawToRiftBufferEvent::TWICE:
        {
            mTunnel->replaceGlslProg(mTunnelBasicRender);
            mTunnel->getGlslProg()->uniform( "uEye", e->getEye() );
            draw( nullptr );
        }
            break;
        case DrawToRiftBufferEvent::STEREO:
        {
//            mTunnel->replaceGlslProg(mTunnelRiftInstancedRender);
//            
//            auto visible = mContext->getComponent<FrustumCullComponent>().lock()->isVisible();
//            if(!visible) return;
//            
//            gl::ScopedModelMatrix model;
//            auto transform = mContext->getComponent<ec::TransformComponent>().lock();
//            gl::multModelMatrix( transform->getModelMatrix() );
//            
//            auto glsl = mTunnel->getGlslProg();
//            glsl->uniform("uNoiseScale", mNoiseScale);
//            
//            mTunnel->drawInstanced(2);
            
        }
            break;
        default:
            break;
    }
    
}

void TunnelComponent::draw( ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw");
    
    auto visible = mContext->getComponent<FrustumCullComponent>().lock()->isVisible();
    if(!visible) return;
    
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    
    auto glsl = mTunnel->getGlslProg();
    glsl->uniform("uNoiseScale", mNoiseScale);
    mTunnel->draw();
}

void TunnelComponent::drawGeometry( ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw");
    
    auto visible = mContext->getComponent<FrustumCullComponent>().lock()->isVisible();
    if(!visible) return;
    
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    auto glsl = mTunnel->getGlslProg();
    glsl->uniform("uNoiseScale", mNoiseScale);
    mTunnel->draw();
    //Draw
    
}

void TunnelComponent::handleGlslProgReload(ec::EventDataRef)
{
    try {
        mTunnelBasicRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/tunnel_basic.vert")).fragment(loadAsset("shaders/tunnel_basic.frag")).geometry(loadAsset("shaders/tunnel_basic.geom")).preprocess(true) );
        
    } catch (const ci::gl::GlslProgCompileExc e) {
        CI_LOG_E(std::string("tunnel render error: ") + e.what());
    }
    
    try {
        mTunnelBasicStereoRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/tunnel_basic_stereo.vert")).fragment(loadAsset("shaders/tunnel_basic_stereo.frag")).geometry(loadAsset("shaders/tunnel_basic_stereo.geom")).preprocess(true) );
        
    } catch (const ci::gl::GlslProgCompileExc e) {
        CI_LOG_E(std::string("tunnel render error: ") + e.what());
    }
    try {
        mTunnelGeometryRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/tunnel_geometry.vert")).fragment(loadAsset("shaders/tunnel_geometry.frag")).geometry(loadAsset("shaders/tunnel_geometry.geom")).preprocess(true) );
        
    } catch (const ci::gl::GlslProgCompileExc e) {
        CI_LOG_E(std::string("tunnel geometry error: ") + e.what());
    }
    
    try {
        mTunnelShadowRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/tunnel_shadow.vert")).fragment(loadAsset("shaders/tunnel_shadow.frag")).preprocess(true) );
        
    } catch (const ci::gl::GlslProgCompileExc e) {
        CI_LOG_E(std::string("tunnel shadow render error: ") + e.what());
    }
    
    try{
        mTunnelRiftInstancedGeometryRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/tunnel_geometry_rift.vert")).fragment(loadAsset("shaders/tunnel_geometry_rift.frag")).geometry(loadAsset("shaders/tunnel_geometry_rift.geom")).preprocess(true) );
        
    } catch (const ci::gl::GlslProgCompileExc e) {
        CI_LOG_E(std::string("tunnel instanced geom render error: ") + e.what());
    }
    
    try{
        mTunnelRiftInstancedRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/tunnel_rift.vert")).fragment(loadAsset("shaders/tunnel_rift.frag")).geometry(loadAsset("shaders/tunnel_rift.geom")).preprocess(true) );
        
    } catch (const ci::gl::GlslProgCompileExc e) {
        CI_LOG_E(std::string("tunnel instanced rift render error: ") + e.what());
    }


    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    mTunnelBasicRender->uniformBlock("uLights", scene->lights()->getLightUboLocation() );
    mTunnelBasicRender->uniform("uShadowMap", 3);
    
    ///replace
    
    mTunnelRiftInstancedGeometryRender->uniformBlock("uRift", OculusRiftComponent::getRiftUboLocation());
    mTunnelRiftInstancedRender->uniformBlock("uRift", OculusRiftComponent::getRiftUboLocation());

    if(mTunnel){
        if( ec::Controller::isRiftEnabled() )
            mTunnel->replaceGlslProg(mTunnelBasicStereoRender);
        else
            mTunnel->replaceGlslProg(mTunnelBasicRender);
    }

}


bool TunnelComponent::postInit()
{
    if(!mInitialized){
        handleGlslProgReload(ec::EventDataRef());
        
        auto & aab_debug = mContext->getComponent<DebugComponent>().lock()->getAxisAlignedBoundingBox();
        
        mFaceRadius = 18.;
        
        Shape2d face;
        for( int i = 0; i < 50; i++ ){
            float t = lmap((float)i, 0.f, 49.f, 0.f, 6.28318530718f);
            if(i==0)face.moveTo(vec2( cos(t)*mFaceRadius, sin(t)*mFaceRadius ));
            else face.lineTo(vec2( cos(t)*mFaceRadius, sin(t)*mFaceRadius ));
        }
        face.close();
        
        Shape2d square;
        square.moveTo(vec2(-5.,-5.));
        square.lineTo(vec2(-5.,5.));
        square.lineTo(vec2(5.,5.));
        square.lineTo(vec2(5.,-5.));
        square.close();
        
        Perlin p;
        std::vector<vec3> path;
        for( int i = 0; i < mLength; i+=mStep ){
            
            float scale = 1.+i/(mLength/2);
            vec3 tunnelpath = vec3(mAmplitude.x*scale*sin(-i * (mFrequency.x + p.noise((i+100.)*mNoiseFrequency.x)*mNoiseAmplitude.x )), mAmplitude.y*scale*cos(-i * (mFrequency.y + p.noise(i*mNoiseFrequency.y)*mNoiseAmplitude.y )), -i); // See distance field.
            path.push_back(tunnelpath);
            
        }
        
        mSpline = BSpline3f( path, 3, false, true );
        
        auto geom = ci::geom::ExtrudeSpline( face, mSpline, 1000 ).backCap(false).frontCap(true) >> geom::Bounds( &aab_debug ) >> geom::Invert(geom::NORMAL);
        
        if( ec::Controller::isRiftEnabled() )
            mTunnel = ci::gl::Batch::create( geom , mTunnelBasicStereoRender );
        else
            mTunnel = ci::gl::Batch::create( geom , mTunnelBasicRender );
        
        
        mTunnelShadow = ci::gl::Batch::create( geom , mTunnelShadowRender );
        
        CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
        mInitialized = true;
    }
    ///this could reflect errors...
    return true;
}

TunnelComponent::TunnelComponent( ec::Actor* context ):ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "_bit_component" ) ),mShuttingDown(false)
{
    
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

TunnelComponent::~TunnelComponent()
{
}

void TunnelComponent::cleanup()
{
    unregisterHandlers();
}

const ec::ComponentNameType TunnelComponent::getName() const
{
    return "tunnel_component";
}

const ec::ComponentUId TunnelComponent::getId() const
{
    return mId;
}

const ec::ComponentType TunnelComponent::getType() const
{
    return TYPE;
}

ci::JsonTree TunnelComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    save.addChild( ci::JsonTree( "face_diameter", mFaceRadius ) );
    save.addChild( ci::JsonTree( "length", mLength ) );
    save.addChild( ci::JsonTree( "step", mStep ) );
    
    auto freq = ci::JsonTree::makeArray( "frequency" );
    for( int i = 0; i<2; i++ ){
        freq.addChild( JsonTree( "", mFrequency[i] ) );
    }
    save.addChild(freq);
    
    auto amp = ci::JsonTree::makeArray( "amplitude" );
    for( int i = 0; i<2; i++ ){
        amp.addChild( JsonTree( "", mAmplitude[i] ) );
    }
    save.addChild(amp);
    
    auto n_freq = ci::JsonTree::makeArray( "noise_frequency" );
    for( int i = 0; i<2; i++ ){
        n_freq.addChild( JsonTree( "", mNoiseFrequency[i] ) );
    }
    save.addChild(n_freq);
    
    auto n_amp = ci::JsonTree::makeArray( "noise_amplitude" );
    for( int i = 0; i<2; i++ ){
        n_amp.addChild( JsonTree( "", mNoiseAmplitude[i] ) );
    }
    save.addChild(n_amp);
    
    return save;
    
}

void TunnelComponent::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
}


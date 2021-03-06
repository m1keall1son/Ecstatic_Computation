//
//  KinectComponent.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/23/15.
//
//

#include "KinectComponent.h"

#include "CameraManager.h"
#include "Actor.h"
#include "AppSceneBase.h"
#include "Controller.h"
#include "TransformComponent.h"
#include "KinectComponent.h"
#include "Scene.h"
#include "FrustumCullComponent.h"
#include "Events.h"
#include "LightComponent.h"
#include "Light.h"
#include "cinder/gl/GlslProg.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType KinectComponent::TYPE = ec::getHash("kinect_component");
static KinectRef sKinect = nullptr;

KinectComponentRef KinectComponent::create(ec::Actor *context)
{
    return KinectComponentRef( new KinectComponent( context ) );
}

void KinectComponent::initializeKinect(){
    sKinect = Kinect::create();
}

KinectComponent::KinectComponent( ec::Actor * context ): ec::ComponentBase(context), mId( ec::getHash( context->getName()+"_kinect_component" ) ),mShuttingDown(false)
{
    ///TODO: need to grab out all the geometry from context and create an aa_bounding_box

   
   registerListeners();
    
    mKinect = sKinect;
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

KinectComponent::~KinectComponent()
{
}

void KinectComponent::cleanup()
{
    unregisterListeners();
}

void KinectComponent::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shut down");
    mShuttingDown = true;    
}
void KinectComponent::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerListeners();
}

void KinectComponent::registerListeners()
{
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &KinectComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &KinectComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    
    auto scene = ec::Controller::get()->scene().lock();
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &KinectComponent::handleReloadGlslProg), ReloadGlslProgEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate( this , &KinectComponent::update), UpdateEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate( this , &KinectComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate( this , &KinectComponent::draw), DrawToMainBufferEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate( this , &KinectComponent::drawRift), DrawToRiftBufferEvent::TYPE);
}
void KinectComponent::unregisterListeners()
{
    ec::Controller::get()->eventManager()->removeListener(fastdelegate::MakeDelegate(this, &KinectComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->removeListener(fastdelegate::MakeDelegate(this, &KinectComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    
    auto scene = ec::Controller::get()->scene().lock();
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &KinectComponent::handleReloadGlslProg), ReloadGlslProgEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate( this , &KinectComponent::update), UpdateEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate( this , &KinectComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate( this , &KinectComponent::draw), DrawToMainBufferEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate( this , &KinectComponent::drawRift), DrawToRiftBufferEvent::TYPE);
}

void KinectComponent::handleReloadGlslProg(ec::EventDataRef)
{
    try {
        mKinectShadowRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/kinect_shadow.vert")).fragment(loadAsset("shaders/kinect_shadow.frag")).geometry(loadAsset("shaders/kinect_shadow.geom")).preprocess(true) );
    } catch (const gl::GlslProgCompileExc &e) {
        CI_LOG_E(e.what());
    }catch (const gl::GlslProgLinkExc &e) {
        CI_LOG_E(e.what());
    }
    
    try {
        mKinectRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/kinect_lighting.vert")).fragment(loadAsset("shaders/kinect_lighting.frag")).geometry(loadAsset("shaders/kinect_lighting.geom")).preprocess(true) );
        
    } catch (const gl::GlslProgCompileExc &e) {
        CI_LOG_E(e.what());
    }catch (const gl::GlslProgLinkExc &e) {
        CI_LOG_E(e.what());
    }
    
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    mKinectRender->uniformBlock("uLights", scene->lights()->getLightUboLocation() );
    
    mKinectRender->uniform( "uDepthTexture", 4 );
    mKinectRender->uniform( "uColorTexture", 5 );

    ///TODO: give shadow map a permanent location
    mKinectRender->uniform( "uShadowMap", 3 );
    
    mKinectRender->uniform( "uRef_pix_size", mKinect->getZeroPlanePixelSize() );
    mKinectRender->uniform( "uRef_distance", mKinect->getZeroPlaneDistance() );
    mKinectRender->uniform( "uConst_shift", mKinect->getRegistrationConstShift() );
    mKinectRender->uniform( "uDcmos_emitter_dist", mKinect->getDcmosEmitterDist() );
    
    mKinectShadowRender->uniform( "uDepthTexture", 4 );
    mKinectShadowRender->uniform( "uThresholds", mThresholds );
    mKinectShadowRender->uniform( "uRef_pix_size", mKinect->getZeroPlanePixelSize() );
    mKinectShadowRender->uniform( "uRef_distance", mKinect->getZeroPlaneDistance() );
    mKinectShadowRender->uniform( "uConst_shift", mKinect->getRegistrationConstShift() );
    mKinectShadowRender->uniform( "uDcmos_emitter_dist", mKinect->getDcmosEmitterDist() );
    
    if(mKinectMesh){
        mKinectMesh->replaceGlslProg(mKinectRender);
    }
}

bool KinectComponent::postInit()
{
    if(!mInitialized){
        mKinectColorTexture	= gl::Texture::create( mKinect->getWidth(), mKinect->getHeight(), gl::Texture::Format().internalFormat(GL_RGB).minFilter(GL_LINEAR).magFilter(GL_LINEAR) );
        
        mKinectDepthTexture	= gl::Texture::create( mKinect->getWidth(), mKinect->getHeight(), gl::Texture::Format().internalFormat(GL_R16UI).dataType(GL_UNSIGNED_SHORT).minFilter(GL_NEAREST).magFilter(GL_NEAREST) );
        
        handleReloadGlslProg(ec::EventDataRef());
        
        mKinectMesh = gl::Batch::create( geom::Plane().size(vec2(640,480)).origin(vec3(getWindowCenter(),0)).subdivisions(vec2(640,480)/3.f), mKinectRender );

        mKinectMeshShadow = gl::Batch::create( geom::Plane().size(vec2(640,480)).origin(vec3(getWindowCenter(),0)).subdivisions(vec2(640,480)/3.f), mKinectShadowRender );

        CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
        mInitialized = true;
    }
    return true;
}

void KinectComponent::update(ec::EventDataRef)
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" update");

    if(mKinect->checkNewDepthFrame())
        mKinectDepthTexture->update(  Channel16u( mKinect->getDepthImage() ) );
    
    if(mKinect->checkNewVideoFrame()){
        mKinectColorTexture->update( Surface8u( mKinect->getVideoImage() ) );
    }
    
}

void KinectComponent::drawShadow(ec::EventDataRef)
{
   
    CI_LOG_V( mContext->getName() + " : "+getName()+" drawShadow");
        
    gl::ScopedTextureBind kinect( mKinectDepthTexture, 4 );
    
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    mKinectShadowRender->uniform( "uThresholds", mThresholds );
    mKinectShadowRender->uniform( "uTriangleCutoff", mTriangleCutoff );
    mKinectMeshShadow->draw();
}

void KinectComponent::draw(ec::EventDataRef)
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw");
    
    gl::ScopedTextureBind kinect( mKinectDepthTexture, 4 );
    gl::ScopedTextureBind kinect_color( mKinectColorTexture, 5 );

    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    mKinectRender->uniform( "uThresholds", mThresholds );
    mKinectRender->uniform( "uTriangleCutoff", mTriangleCutoff );
    mKinectMesh->draw();
}

void KinectComponent::drawRift( ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw rift");
    
    auto e = std::dynamic_pointer_cast<DrawToRiftBufferEvent>(event);
    
    switch (e->getStyle()) {
        case DrawToRiftBufferEvent::TWICE:
        {
            draw( nullptr );
        }
            break;
        case DrawToRiftBufferEvent::STEREO:
        {
        }
            break;
        default:
            break;
    }
    
}

bool KinectComponent::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    
    try {
        auto far = tree["far_threshold"].getValue<float>();
        auto near = tree["near_threshold"].getValue<float>();
        auto triangle = tree["triangle_cutoff"].getValue<float>();
        
        mThresholds.x = near;
        mThresholds.y = far;
        mTriangleCutoff = triangle;
        
    } catch ( ci::JsonTree::ExcChildNotFound ex	) {
        CI_LOG_W("no not cutoffs found ");
    }
    
    return true;
}


ci::JsonTree KinectComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    save.addChild( ci::JsonTree( "near_threshold", mThresholds.x ) );
    save.addChild( ci::JsonTree( "far_threshold", mThresholds.y ) );
    save.addChild( ci::JsonTree( "triangle_cutoff", mTriangleCutoff ) );
//    save.addChild( ci::JsonTree( "name", getName() ) );
//    save.addChild( ci::JsonTree( "id", getId() ) );
//    save.addChild( ci::JsonTree( "type", "debug_component" ) );
//    
//    auto min = ci::JsonTree::makeArray( "aa_bounding_box_min" );
//    for( int i = 0; i<3; i++ ){
//        min.addChild( ci::JsonTree( "", mObjectBoundingBox.getMin()[i] ) );
//    }
//    save.addChild(min);
//    
//    auto max = ci::JsonTree::makeArray( "aa_bounding_box_max" );
//    for( int i = 0; i<3; i++ ){
//        max.addChild( ci::JsonTree( "", mObjectBoundingBox.getMax()[i] ) );
//    }
//    save.addChild(max);
    
    return save;
}

const ec::ComponentNameType KinectComponent::getName() const
{
    return "kinect_component";
}
const ec::ComponentUId KinectComponent::getId() const
{
    return mId;
}
const ec::ComponentType KinectComponent::getType() const
{
    return TYPE;
}

void KinectComponent::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText( mContext->getName() +" : "+ getName());
    gui->addParam("triangle cutoff", &mTriangleCutoff).min(0.).max(10.);
    gui->addParam("near thresh", &mThresholds.x);
    gui->addParam("far threshold", &mThresholds.y);
}

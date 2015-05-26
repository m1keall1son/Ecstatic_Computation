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

ec::ComponentType KinectComponent::TYPE = 0x018;

KinectComponentRef KinectComponent::create(ec::Actor *context)
{
    return KinectComponentRef( new KinectComponent( context ) );
}

KinectComponent::KinectComponent( ec::Actor * context ): ec::ComponentBase(context), mId( ec::getHash( context->getName()+"_kinect_component" ) ),mShuttingDown(false)
{
    ///TODO: need to grab out all the geometry from context and create an aa_bounding_box

    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &KinectComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &KinectComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    
    registerListeners();
    
    auto params = Kinect::FreenectParams();
    params.mDepthRegister = false;
    mKinect = ci::Kinect::create( Kinect::Device( params ) );
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

KinectComponent::~KinectComponent()
{
    if(!mShuttingDown)unregisterListeners();
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
    auto scene = ec::Controller::get()->scene().lock();
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &KinectComponent::handleReloadGlslProg), ReloadGlslProgEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate( this , &KinectComponent::update), UpdateEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate( this , &KinectComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate( this , &KinectComponent::draw), DrawToMainBufferEvent::TYPE);
}
void KinectComponent::unregisterListeners()
{
    auto scene = ec::Controller::get()->scene().lock();
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &KinectComponent::handleReloadGlslProg), ReloadGlslProgEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate( this , &KinectComponent::update), UpdateEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate( this , &KinectComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate( this , &KinectComponent::draw), DrawToMainBufferEvent::TYPE);
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
    mKinectRender->uniform( "uThresholds", mThresholds );

    ///TODO: give shadow map a permanent location
    mKinectRender->uniform( "uShadowMap", 3 );
    
    mKinectRender->uniform( "uRef_pix_size", mKinect->getZeroPlanePixelSize() );
    mKinectRender->uniform( "uRef_distance", mKinect->getZeroPlaneDistance() );
    mKinectRender->uniform( "uConst_shift", mKinect->getRegistrationConstShift() );
    mKinectRender->uniform( "uDcmos_emitter_dist", mKinect->getDcmosEmitterDist() );
    mKinectRender->uniform( "uTriangleCutoff", .2f);
    
    mKinectShadowRender->uniform( "uDepthTexture", 4 );
    mKinectShadowRender->uniform( "uThresholds", mThresholds );
    mKinectShadowRender->uniform( "uRef_pix_size", mKinect->getZeroPlanePixelSize() );
    mKinectShadowRender->uniform( "uRef_distance", mKinect->getZeroPlaneDistance() );
    mKinectShadowRender->uniform( "uConst_shift", mKinect->getRegistrationConstShift() );
    mKinectShadowRender->uniform( "uDcmos_emitter_dist", mKinect->getDcmosEmitterDist() );
    mKinectShadowRender->uniform( "uTriangleCutoff", .2f);
    
    if(mKinectMesh)
        mKinectMesh->replaceGlslProg(mKinectRender);
}

bool KinectComponent::postInit()
{
    
    mKinectColorTexture	= gl::Texture::create( mKinect->getWidth(), mKinect->getHeight(), gl::Texture::Format().internalFormat(GL_RGB).minFilter(GL_LINEAR).magFilter(GL_LINEAR) );
    
    mKinectDepthTexture	= gl::Texture::create( mKinect->getWidth(), mKinect->getHeight(), gl::Texture::Format().internalFormat(GL_R16UI).dataType(GL_UNSIGNED_SHORT).minFilter(GL_NEAREST).magFilter(GL_NEAREST) );
    
    handleReloadGlslProg(ec::EventDataRef());
    
    mKinectMesh = gl::Batch::create( geom::Plane().size(vec2(640,480)).origin(vec3(getWindowCenter(),0)).subdivisions(vec2(640,480)), mKinectRender );
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
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
    
    mKinectMesh->replaceGlslProg(mKinectShadowRender);
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    mKinectMesh->draw();
}

void KinectComponent::draw(ec::EventDataRef)
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw");
    
    gl::ScopedTextureBind kinect( mKinectDepthTexture, 4 );
    gl::ScopedTextureBind kinect_color( mKinectColorTexture, 5 );

    mKinectMesh->replaceGlslProg(mKinectRender);

    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    mKinectMesh->draw();    
}

bool KinectComponent::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    
//    try {
//        auto min = tree["aa_bounding_box_min"].getChildren();
//        auto end = min.end();
//        ci::vec3 aab_min;
//        int i = 0;
//        for( auto it = min.begin(); it != end; ++it ) {
//            aab_min[i++] = (*it).getValue<float>();
//        }
//        
//        auto max = tree["aa_bounding_box_max"].getChildren();
//        auto end2 = max.end();
//        ci::vec3 aab_max;
//        i = 0;
//        for( auto it = max.begin(); it != end2; ++it ) {
//            aab_max[i++] = (*it).getValue<float>();
//        }
//        
//        mObjectBoundingBox = ci::AxisAlignedBox3f( aab_min, aab_max );
//        
//    } catch ( ci::JsonTree::ExcChildNotFound ex	) {
//        CI_LOG_W("no aa_bounding_box found");
//    }
    
    
    return true;
}


ci::JsonTree KinectComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
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
//
//  LandscapeComponent.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#include "LandscapeComponent.h"
#include "Actor.h"
#include "Scene.h"
#include "Controller.h"
#include "AppSceneBase.h"
#include "Events.h"
#include "FrustumCullComponent.h"
#include "DebugComponent.h"
#include "cinder/ObjLoader.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType LandscapeComponent::TYPE = ec::getHash("landscape_component");

LandscapeComponentRef LandscapeComponent::create( ec::Actor* context )
{
    return LandscapeComponentRef( new LandscapeComponent(context) );
}

void LandscapeComponent::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void LandscapeComponent::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void LandscapeComponent::registerHandlers()
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &LandscapeComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &LandscapeComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &LandscapeComponent::handleReloadGlslProg), ReloadGlslProgEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &LandscapeComponent::draw), DrawToMainBufferEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &LandscapeComponent::drawRift), DrawToRiftBufferEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &LandscapeComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &LandscapeComponent::handleMicVolumeEvent), MicVolumeEvent::TYPE);

}
void LandscapeComponent::unregisterHandlers()
{
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &LandscapeComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &LandscapeComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &LandscapeComponent::handleReloadGlslProg), ReloadGlslProgEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &LandscapeComponent::draw), DrawToMainBufferEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &LandscapeComponent::drawRift), DrawToRiftBufferEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &LandscapeComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &LandscapeComponent::handleMicVolumeEvent), MicVolumeEvent::TYPE);

}

void LandscapeComponent::handleMicVolumeEvent(ec::EventDataRef event)
{
    auto e = std::dynamic_pointer_cast<MicVolumeEvent>(event);
    mColorScale = constrain(e->getVolume(), 0.f, 1.f);
}

bool LandscapeComponent::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    return true;
}

void LandscapeComponent::cleanup()
{
    unregisterHandlers();
}

void LandscapeComponent::handleReloadGlslProg(ec::EventDataRef)
{
    try {
        mLandscapeRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/landscape.vert")).geometry( loadAsset("shaders/landscape.geom") ).fragment(loadAsset("shaders/landscape.frag")).preprocess(true) );
    } catch (gl::GlslProgCompileExc e) {
        CI_LOG_E( "landscape render " + std::string(e.what()) );
    }
    try {
        mLandscapeShadowRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/landscape_shadow.vert")).fragment(loadAsset("shaders/landscape_shadow.frag")).preprocess(true) );
    } catch (gl::GlslProgCompileExc e) {
        CI_LOG_E( "landscape shadow render " + std::string(e.what()) );
    }
    
    try {
        mObjectRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/landscape_object.vert")).fragment(loadAsset("shaders/landscape_object.frag")).preprocess(true) );
    } catch (gl::GlslProgCompileExc e) {
        CI_LOG_E( "landscape object render " + std::string(e.what()) );
    }
    
    mLandscapeRender->uniformBlock("uLights", 0 );
    mLandscapeRender->uniform("uMap", 5);
    mLandscapeRender->uniform("uShadowMap", 3);
    
    mLandscapeShadowRender->uniform("uMap", 5);
    
    mObjectRender->uniformBlock("uLights", 0);
    mObjectRender->uniform("uShadowMap", 3);
    
    
    if(mHead)
        mHead->replaceGlslProg(mObjectRender);
    
    if(mTeeth)
        mTeeth->replaceGlslProg(mObjectRender);
    
    if( mFoot )
        mFoot->replaceGlslProg(mObjectRender);
    
    if(mLandscapeShadow)
        mLandscapeShadow->replaceGlslProg(mLandscapeShadowRender);
    
    if(mLandscape)
        mLandscape->replaceGlslProg(mLandscapeRender);
}

bool LandscapeComponent::postInit()
{
    if(!mInitialized){
        
        handleReloadGlslProg(ec::EventDataRef());
        
        mMap = gl::Texture2d::create( loadImage( loadAsset("map.png") ) );

        
        auto & aab_debug = mContext->getComponent<DebugComponent>().lock()->getAxisAlignedBoundingBox();
        auto geom = geom::Plane().size(vec2(5000)).origin(vec3(0,-2,0)).normal(vec3(0,1,0)).subdivisions(vec2(100,100)) >> geom::Bounds( &aab_debug );
        
        mLandscape = gl::Batch::create( geom , mLandscapeRender);
        mLandscapeShadow = gl::Batch::create( geom, mLandscapeShadowRender);
        
        auto foot = ObjLoader( loadAsset("foot.obj") ) >> geom::Transform(  scale(vec3(200)) * toMat4(quat( .27, -.58, -.72, -.28)) * translate( vec3( 4.36, 3.68, -2.36 ) ) );

        mFoot = gl::Batch::create( foot, mObjectRender);
        mFootShadow = gl::Batch::create( foot, gl::getStockShader(gl::ShaderDef()));
        
        auto teeth = ObjLoader( loadAsset("teeth.obj") ) >> geom::Transform(  scale(vec3(1000.)) * toMat4(quat( .37, -.49, .12, -.79)) * translate( vec3( -.5,-.75, -1.82 ) ) );
        mTeeth = gl::Batch::create( teeth, mObjectRender);
        mTeethShadow = gl::Batch::create( teeth, gl::getStockShader(gl::ShaderDef()));

        
        auto head = ObjLoader( loadAsset("head.obj") ) >> geom::Transform(  scale(vec3(300.)) * toMat4(quat( .54, .49, -.3, -.62 )) * translate( vec3( -.35,-2.6, -3.95 ) ) );
        mHead = gl::Batch::create( head, mObjectRender);
        mHeadShadow = gl::Batch::create( head, gl::getStockShader(gl::ShaderDef()));

        mInitialized = true;
    }
    
    timeline().apply(&mFogDensity, 1.f, .0016f, 5.f, EaseOutCubic());
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

LandscapeComponent::LandscapeComponent( ec::Actor* context ): ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "_landscape_component" ) ),mShuttingDown(false),mColorScale(0.),mSinkHoleScale(0.),mFogDensity(.0016)
{
    
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

LandscapeComponent::~LandscapeComponent()
{
}

const ec::ComponentNameType LandscapeComponent::getName() const
{
    return "landscape_component";
}

const ec::ComponentUId LandscapeComponent::getId() const
{
    return mId;
}

const ec::ComponentType LandscapeComponent::getType() const
{
    return TYPE;
}

ci::JsonTree LandscapeComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    
    return save;
    
}

void LandscapeComponent::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
    gui->addParam("sink hole", &mSinkHoleScale);
    gui->addSeparator();
    
    auto uFun = [&]{ mHeadTransform.mUpdated = 1; mFootTransform.mUpdated = 1; mTeethTransform.mUpdated = 1; };
    
    gui->addParam("head translation", &mHeadTransform.mTranslation).updateFn(uFun);
    gui->addParam("head scale", &mHeadTransform.mScale).updateFn(uFun);
    gui->addParam("head rotation", &mHeadTransform.mRotation).updateFn(uFun);
    gui->addSeparator();
    gui->addParam("foot translation", &mFootTransform.mTranslation).updateFn(uFun);
    gui->addParam("foot scale", &mFootTransform.mScale).updateFn(uFun);
    gui->addParam("foot rotation", &mFootTransform.mRotation).updateFn(uFun);
    gui->addSeparator();
    gui->addParam("teeth translation", &mTeethTransform.mTranslation).updateFn(uFun);
    gui->addParam("teeth scale", &mTeethTransform.mScale).updateFn(uFun);
    gui->addParam("teeth rotation", &mTeethTransform.mRotation).updateFn(uFun);
}

void LandscapeComponent::drawRift( ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw");
    
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

void LandscapeComponent::draw(ec::EventDataRef)
{

    {
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    
    gl::ScopedTextureBind map( mMap, 5 );
    mLandscapeRender->uniform("uColorScale", mColorScale);
    mLandscapeRender->uniform("uSinkHoleScale", mSinkHoleScale);
    if( mSinkHoleScale > 150. )mFogDensity = mFogDensity + .00001;
    mLandscapeRender->uniform("uFogDensity", mFogDensity);
    mObjectRender->uniform("uFogDensity", mFogDensity);
    mLandscape->draw();
        
    {
        gl::ScopedModelMatrix model;
        gl::multModelMatrix( mHeadTransform.getModelMatrix() );
        mHead->draw();
    }
    {
        gl::ScopedModelMatrix model;
        gl::multModelMatrix( mTeethTransform.getModelMatrix() );
        mTeeth->draw();
    }
    {
        gl::ScopedModelMatrix model;
        gl::multModelMatrix( mFootTransform.getModelMatrix() );
        mFoot->draw();
    }
    }
}
void LandscapeComponent::drawShadow(ec::EventDataRef)
{
//    auto visible = mContext->getComponent<FrustumCullComponent>().lock()->isVisible();
//    if(!visible) return;
    {
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    
    gl::ScopedTextureBind map( mMap, 5 );
    mLandscapeShadowRender->uniform("uSinkHoleScale", mSinkHoleScale);
    mLandscapeShadow->draw();
    
    {
        gl::ScopedModelMatrix model;
        gl::multModelMatrix( mHeadTransform.getModelMatrix() );
        mHeadShadow->draw();
    }
    {
        gl::ScopedModelMatrix model;
        gl::multModelMatrix( mTeethTransform.getModelMatrix() );
        mTeethShadow->draw();
    }
    {
        gl::ScopedModelMatrix model;
        gl::multModelMatrix( mFootTransform.getModelMatrix() );
        mFootShadow->draw();
    }
    }
}


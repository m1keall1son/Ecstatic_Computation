//
//  RoomComponent.cpp
//  System_test
//
//  Created by Mike Allison on 5/23/15.
//
//

#include "RoomComponent.h"
#include "DebugComponent.h"
#include "ComponentBase.h"
#include "TransformComponent.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "FrustumCullComponent.h"
#include "RoomComponent.h"
#include "cinder/Log.h"
#include "Actor.h"
#include "AppSceneBase.h"
#include "Scene.h"
#include "Controller.h"
#include "LightManager.h"
#include "ShadowMap.h"
#include "Events.h"
#include "SystemEvents.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType RoomComponent::TYPE = ec::getHash("room_component");

RoomComponentRef RoomComponent::create( ec::Actor* context )
{
    return RoomComponentRef( new RoomComponent(context) );
}


void RoomComponent::update(ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" update");
    mTime = getElapsedSeconds();

}

void RoomComponent::drawShadow(ec::EventDataRef)
{
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" drawShadow");
    
    auto visible = mContext->getComponent<FrustumCullComponent>().lock()->isVisible();
    if(!visible) return;
    
    gl::ScopedFaceCulling pushFace(true,GL_FRONT);
    
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    
    mRoomRender->uniform("uTime", mTime);
    mRoomShadowRender->uniform("uNoiseScale", mNoiseScale);
    mRoomShadowRender->uniform("uNoiseMax", mNoiseMax);
    
    mRoomShadow->draw();
}

void RoomComponent::draw(ec::EventDataRef event )
{
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw");

    auto visible = mContext->getComponent<FrustumCullComponent>().lock()->isVisible();
    if(!visible) return;
    
    gl::ScopedFaceCulling pushFace(true,GL_FRONT);
    
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    
    mRoomRender->uniform("uTime", mTime);
    mRoomRender->uniform("uNoiseScale", mNoiseScale);
    mRoomRender->uniform("uNoiseMax", mNoiseMax);
    mRoom->draw();
    
}

void RoomComponent::drawRift( ec::EventDataRef event )
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


bool RoomComponent::initialize(const ci::JsonTree &tree)
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    
    try {
        
        auto size = tree["size"].getValue<float>();
        mRoomSize = size;
        
    } catch (ci::JsonTree::ExcChildNotFound e) {
        CI_LOG_W("found no size, defaulting to 25");
        mRoomSize = 25;
    }
    
    try {
        
        auto size = tree["noise_max"].getValue<float>();
        mNoiseMax = size;
        
    } catch (ci::JsonTree::ExcChildNotFound e) {
        CI_LOG_W("found no size, defaulting to 10");
        mNoiseMax = 10.;
    }
    
    try {
        
        auto size = tree["noise_scale"].getValue<float>();
        mNoiseScale = size;
        
    } catch (ci::JsonTree::ExcChildNotFound e) {
        CI_LOG_W("found no size, defaulting to 0");
        mNoiseScale = 0;
    }
    
    return true;
}

bool RoomComponent::postInit()
{
    if(!mInitialized){
        
        try {
              mRoomRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/room_basic.vert")).geometry(loadAsset("shaders/room_basic.geom")).fragment(loadAsset("shaders/room_basic.frag")).preprocess(true) );
        } catch (ci::gl::GlslProgCompileExc e) {
            CI_LOG_E(e.what());
        }
    
        
        try {
            mRoomShadowRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/room_shadow.vert")).fragment(loadAsset("shaders/room_shadow.frag")).preprocess(true) );
        } catch (ci::gl::GlslProgCompileExc e) {
            CI_LOG_E(e.what());
        }
        
        
        auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
        
        mRoomRender->uniformBlock("uLights", scene->lights()->getLightUboLocation() );
        mRoomRender->uniform("uShadowMap", 3);
        
        auto & aab_debug = mContext->getComponent<DebugComponent>().lock()->getAxisAlignedBoundingBox();
        
        auto flipNormals = []( const vec3& normal ) { return -normal; };
        auto geom = ci::geom::Cube().size( vec3( mRoomSize )).subdivisions(20) >> geom::AttribFn<vec3, vec3>( geom::NORMAL, geom::NORMAL, flipNormals ) >> geom::Bounds( &aab_debug );
        
        mRoom = ci::gl::Batch::create( geom, mRoomRender );
        
        mRoomShadow = gl::Batch::create( ci::geom::Cube().size( vec3( mRoomSize )) >> geom::AttribFn<vec3, vec3>( geom::NORMAL, geom::NORMAL, flipNormals ), mRoomShadowRender );
        
        CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
        mInitialized = true;
    }
    ///this could reflect errors...
    return true;
}



RoomComponent::RoomComponent( ec::Actor* context ):ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "_room_component" ) ),mShuttingDown(false)
{
    registerListeners();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
        //TODO this should be in initilialize with ryan's code
}

RoomComponent::~RoomComponent()
{
}

void RoomComponent::cleanup()
{
    unregisterListeners();
}

void RoomComponent::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
}

void RoomComponent::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerListeners();
}

void RoomComponent::registerListeners()
{
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &RoomComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &RoomComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);

    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &RoomComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &RoomComponent::draw), DrawToMainBufferEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &RoomComponent::update), UpdateEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &RoomComponent::drawRift), DrawToRiftBufferEvent::TYPE);
}
void RoomComponent::unregisterListeners()
{
    ec::Controller::get()->eventManager()->removeListener(fastdelegate::MakeDelegate(this, &RoomComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->removeListener(fastdelegate::MakeDelegate(this, &RoomComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);

    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &RoomComponent::update), UpdateEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &RoomComponent::draw), DrawToMainBufferEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &RoomComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &RoomComponent::drawRift), DrawToRiftBufferEvent::TYPE);
}

const ec::ComponentNameType RoomComponent::getName() const
{
    return "room_component";
}

const ec::ComponentUId RoomComponent::getId() const
{
    return mId;
}

const ec::ComponentType RoomComponent::getType() const
{
    return TYPE;
}

ci::JsonTree RoomComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    save.addChild( ci::JsonTree( "size", mRoomSize ) );
    save.addChild( ci::JsonTree( "noise_scale", mNoiseScale) );
    save.addChild( ci::JsonTree( "noise_max", mNoiseMax) );
    return save;
    
}

void RoomComponent::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
    gui->addParam("noise scale", &mNoiseScale).min(0.).max(1.).step(.01);
    gui->addParam("noise max", &mNoiseMax);

}


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

ec::ComponentType RoomComponent::TYPE = 0x011;

RoomComponentRef RoomComponent::create( ec::Actor* context )
{
    return RoomComponentRef( new RoomComponent(context) );
}


void RoomComponent::update(ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" update");

}

void RoomComponent::drawShadow(ec::EventDataRef)
{
    gl::ScopedFaceCulling pushFace(true,GL_FRONT);

    CI_LOG_V( mContext->getName() + " : "+getName()+" drawShadow");
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    mRoomShadow->draw();
}

void RoomComponent::draw(ec::EventDataRef event )
{
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw");

    gl::ScopedFaceCulling pushFace(true,GL_FRONT);
    
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    mRoom->draw();
    
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
    
    return true;
}

bool RoomComponent::postInit()
{
    
    auto glsl = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/lighting.vert")).fragment(loadAsset("shaders/lighting.frag")).preprocess(true) );
    
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    
    glsl->uniformBlock("uLights", scene->lights()->getLightUboLocation() );

    glsl->uniform("uShadowMap", 3);
    
    auto flipNormals = []( const vec3& normal ) { return -normal; };
    mRoom = ci::gl::Batch::create( ci::geom::Cube().size( vec3( mRoomSize )) >> geom::AttribFn<vec3, vec3>( geom::NORMAL, geom::NORMAL, flipNormals ), glsl );
    
    ///get bounding box;
    auto aab_debug = mContext->getComponent<DebugComponent>().lock()->getAxisAlignedBoundingBox();
    auto trimesh = TriMesh( geom::Cube().size( vec3( mRoomSize ) ));
    aab_debug = trimesh.calcBoundingBox();
    
    mRoomShadow = gl::Batch::create( trimesh, gl::getStockShader(gl::ShaderDef()) );

    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}



RoomComponent::RoomComponent( ec::Actor* context ):ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "_room_component" ) ),mShuttingDown(false)
{
    registerListeners();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &RoomComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &RoomComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    //TODO this should be in initilialize with ryan's code
}

RoomComponent::~RoomComponent()
{
    if(!mShuttingDown)unregisterListeners();
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
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &RoomComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &RoomComponent::draw), DrawToMainBufferEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &RoomComponent::update), UpdateEvent::TYPE);
}
void RoomComponent::unregisterListeners()
{
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &RoomComponent::update), UpdateEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &RoomComponent::draw), DrawToMainBufferEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &RoomComponent::drawShadow), DrawShadowEvent::TYPE);
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
    save.addChild( ci::JsonTree( "name", getName() ) );
    save.addChild( ci::JsonTree( "id", getId() ) );
    save.addChild( ci::JsonTree( "type", "room_component" ) );
    save.addChild( ci::JsonTree( "size", mRoomSize ) );

    return save;
    
}



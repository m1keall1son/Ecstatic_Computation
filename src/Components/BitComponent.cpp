//
//  BitComponent.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/24/15.
//
//

#include "BitComponent.h"
#include "DebugComponent.h"
#include "Actor.h"
#include "Events.h"
#include "Scene.h"
#include "AppSceneBase.h"
#include "Controller.h"
#include "TransformComponent.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType BitComponent::TYPE = 0x019;

BitComponentRef BitComponent::create( ec::Actor* context )
{
    return BitComponentRef( new BitComponent(context) );
}

void BitComponent::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void BitComponent::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void BitComponent::registerHandlers()
{
    //TODO this should be in initilialize with ryan's code
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &BitComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &BitComponent::update), UpdateEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &BitComponent::draw), DrawToMainBufferEvent::TYPE);
}
void BitComponent::unregisterHandlers()
{
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &BitComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &BitComponent::update), UpdateEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &BitComponent::draw), DrawToMainBufferEvent::TYPE);
}

void BitComponent::update(ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" update");
    
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    transform->setRotation( glm::toQuat( ci::rotate( (float)getElapsedSeconds(), vec3(1.) ) ) );
}

bool BitComponent::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    return true;
}

void BitComponent::drawShadow( ec::EventDataRef event )
{
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" drawShadow");
    
    gl::ScopedFaceCulling pushFace(true,GL_BACK);
    
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
   // mTeapotShadow->draw();
   
}

void BitComponent::draw( ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw");
    
    gl::ScopedFaceCulling pushFace(true,GL_BACK);
    
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    //mTeapot->draw();
    
}


bool BitComponent::postInit()
{
    
    auto glsl = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/lighting.vert")).fragment(loadAsset("shaders/lighting.frag")).preprocess(true) );
    
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    
    glsl->uniformBlock("uLights", scene->lights()->getLightUboLocation() );
    glsl->uniform("uShadowMap", 3);
    
    auto aab_debug = mContext->getComponent<DebugComponent>().lock()->getAxisAlignedBoundingBox();
    
    auto trimesh = TriMesh( ci::geom::Sphere().subdivisions(32) );
    
    aab_debug = trimesh.calcBoundingBox();
    
    mBit = ci::gl::Batch::create( trimesh , glsl );
    
    mBitShadow = ci::gl::Batch::create( ci::geom::Teapot(), gl::getStockShader( gl::ShaderDef() ) );
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

BitComponent::BitComponent( ec::Actor* context ):ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "_bit_component" ) ),mShuttingDown(false)
{
    
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &BitComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &BitComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

BitComponent::~BitComponent()
{
    if(!mShuttingDown)unregisterHandlers();
}

const ec::ComponentNameType BitComponent::getName() const
{
    return "bit_component";
}

const ec::ComponentUId BitComponent::getId() const
{
    return mId;
}

const ec::ComponentType BitComponent::getType() const
{
    return TYPE;
}

ci::JsonTree BitComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "name", getName() ) );
    save.addChild( ci::JsonTree( "id", getId() ) );
    save.addChild( ci::JsonTree( "type", "bit_component" ) );
    
    return save;
    
}



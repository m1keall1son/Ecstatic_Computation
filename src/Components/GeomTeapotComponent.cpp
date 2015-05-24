//
//  GeomTeapotComponentComponent.cpp
//  System_test
//
//  Created by Mike Allison on 5/23/15.
//
//

#include "GeomTeapotComponent.h"
#include "DebugComponent.h"
#include "Actor.h"
#include "Events.h"
#include "Scene.h"
#include "AppSceneBase.h"
#include "Controller.h"
#include "TransformComponent.h"

using namespace ci;
using namespace ci::app;

GeomTeapotComponentRef GeomTeapotComponent::create( ec::Actor* context )
{
    return GeomTeapotComponentRef( new GeomTeapotComponent(context) );
}


void GeomTeapotComponent::update(ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" update");

    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    transform->setRotation( glm::toQuat( ci::rotate( (float)getElapsedSeconds(), vec3(1.) ) ) );
}

bool GeomTeapotComponent::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    return true;
}

void GeomTeapotComponent::drawShadow( ec::EventDataRef event )
{
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" drawShadow");
    
    gl::ScopedFaceCulling pushFace(true,GL_BACK);

    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    mTeapotShadow->draw();
    
}

void GeomTeapotComponent::draw( ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw");

    gl::ScopedFaceCulling pushFace(true,GL_BACK);
    
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    mTeapot->draw();
    
}

ec::ComponentType GeomTeapotComponent::TYPE = 0x011;

bool GeomTeapotComponent::postInit()
{
    
    auto glsl = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/lighting.vert")).fragment(loadAsset("shaders/lighting.frag")).preprocess(true) );
    
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    
    glsl->uniformBlock("uLights", scene->lights()->getLightUboLocation() );
    glsl->uniform("uShadowMap", 3);

    auto aab_debug = mContext->getComponent<DebugComponent>().lock()->getAxisAlignedBoundingBox();
    
    auto trimesh = TriMesh( ci::geom::Teapot() );
    
    aab_debug = trimesh.calcBoundingBox();
    
    mTeapot = ci::gl::Batch::create( trimesh , glsl );
    
    mTeapotShadow = ci::gl::Batch::create( ci::geom::Teapot(), gl::getStockShader( gl::ShaderDef() ) );
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

GeomTeapotComponent::GeomTeapotComponent( ec::Actor* context ):ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "geom_teapot_component" ) )
{
    //TODO this should be in initilialize with ryan's code
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &GeomTeapotComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &GeomTeapotComponent::update), UpdateEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &GeomTeapotComponent::draw), DrawToMainBufferEvent::TYPE);
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");

}

GeomTeapotComponent::~GeomTeapotComponent()
{
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &GeomTeapotComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &GeomTeapotComponent::update), UpdateEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &GeomTeapotComponent::draw), DrawToMainBufferEvent::TYPE);
}

const ec::ComponentNameType GeomTeapotComponent::getName() const
{
    return "geom_teapot_component";
}

const ec::ComponentUId GeomTeapotComponent::getId() const
{
    return mId;
}

const ec::ComponentType GeomTeapotComponent::getType() const
{
    return TYPE;
}

ci::JsonTree GeomTeapotComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "name", getName() ) );
    save.addChild( ci::JsonTree( "id", getId() ) );
    save.addChild( ci::JsonTree( "type", "geom_teapot_component" ) );
    
    return save;
    
}



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
#include "FrustumCullComponent.h"

using namespace ci;
using namespace ci::app;

ec::ComponentType BitComponent::TYPE = ec::getHash("bit_component");

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
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &BitComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &BitComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &BitComponent::handleGlslProgReload), ReloadGlslProgEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &BitComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &BitComponent::update), UpdateEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &BitComponent::draw), DrawToMainBufferEvent::TYPE);
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &BitComponent::drawRift), DrawToRiftBufferEvent::TYPE);

}
void BitComponent::unregisterHandlers()
{
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    ec::Controller::get()->eventManager()->removeListener(fastdelegate::MakeDelegate(this, &BitComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->removeListener(fastdelegate::MakeDelegate(this, &BitComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &BitComponent::handleGlslProgReload), ReloadGlslProgEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &BitComponent::drawShadow), DrawShadowEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &BitComponent::update), UpdateEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &BitComponent::draw), DrawToMainBufferEvent::TYPE);
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &BitComponent::drawRift), DrawToRiftBufferEvent::TYPE);

}

void BitComponent::update(ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" update");
//    
//    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
//    transform->setRotation( glm::toQuat( ci::rotate( (float)getElapsedSeconds(), vec3(1.) ) ) );
}

bool BitComponent::initialize( const ci::JsonTree &tree )
{
    if(!mInitialized){
        CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
        mInitialized = true;
    }
    return true;
}

void BitComponent::drawShadow( ec::EventDataRef event )
{
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" drawShadow");
    
    auto visible = mContext->getComponent<FrustumCullComponent>().lock()->isVisible();
    if(!visible) return;
    
    gl::ScopedFaceCulling pushFace(true,GL_BACK);
    
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    mBitShadow->draw();
   
}

void BitComponent::draw( ec::EventDataRef event )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw");
    
    auto visible = mContext->getComponent<FrustumCullComponent>().lock()->isVisible();
    if(!visible) return;
    
    gl::ScopedFaceCulling pushFace(true,GL_BACK);
    
    gl::ScopedModelMatrix model;
    auto transform = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( transform->getModelMatrix() );
    mBit->draw();
    
}

void BitComponent::drawRift( ec::EventDataRef event )
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

void BitComponent::handleGlslProgReload(ec::EventDataRef)
{
    try {
        mBitRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/bit.vert")).geometry(loadAsset("shaders/bit.geom")).fragment(loadAsset("shaders/bit.frag")).preprocess(true) );
       
    } catch (const ci::gl::GlslProgCompileExc e) {
        CI_LOG_E(std::string("Bit render error: ") + e.what());
    }
    
    try{
        mBitShadowRender = gl::GlslProg::create( gl::GlslProg::Format().vertex(loadAsset("shaders/bit_shadow.vert")).fragment(loadAsset("shaders/bit_shadow.frag")).preprocess(true) );
    } catch (const ci::gl::GlslProgCompileExc e) {
        CI_LOG_E(std::string("Bit shadow render error: ") + e.what());
    }
    
    if(mBit)
        mBit->replaceGlslProg(mBitRender);
    
    if(mBitShadow)
        mBitShadow->replaceGlslProg(mBitShadowRender);
    
}


bool BitComponent::postInit()
{
    
    handleGlslProgReload(ec::EventDataRef());
    
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    
    mBitRender->uniformBlock("uLights", scene->lights()->getLightUboLocation() );
    mBitRender->uniform("uShadowMap", 3);
    
    auto & aab_debug = mContext->getComponent<DebugComponent>().lock()->getAxisAlignedBoundingBox();
    auto geom = ci::geom::Icosphere().subdivisions(3) >> geom::Bounds( &aab_debug );
    
    mBit = ci::gl::Batch::create( geom , mBitRender );
    
    mBitShadow = ci::gl::Batch::create( geom, mBitShadowRender );
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

BitComponent::BitComponent( ec::Actor* context ):ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "_bit_component" ) ),mShuttingDown(false)
{
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
}

void BitComponent::cleanup()
{
    unregisterHandlers();
}

BitComponent::~BitComponent()
{
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
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    
    return save;
    
}

void BitComponent::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText( mContext->getName() +" : "+ getName());
}

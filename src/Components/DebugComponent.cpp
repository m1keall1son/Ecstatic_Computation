//
//  DebugComponent.cpp
//  System_test
//
//  Created by Mike Allison on 5/23/15.
//
//

#include "DebugComponent.h"

#include "CameraManager.h"
#include "Actor.h"
#include "AppSceneBase.h"
#include "Controller.h"
#include "TransformComponent.h"
#include "DebugComponent.h"
#include "Scene.h"
#include "FrustumCullComponent.h"
#include "Events.h"
#include "LightComponent.h"
#include "Light.h"

ec::ComponentType DebugComponent::TYPE = 0x016;

DebugComponentRef DebugComponent::create(ec::Actor *context)
{
    return DebugComponentRef( new DebugComponent( context ) );
}

DebugComponent::DebugComponent( ec::Actor * context ): ec::ComponentBase(context), mId( ec::getHash( context->getName()+"_debug_component" ) )
{
    ///TODO: need to grab out all the geometry from context and create an aa_bounding_box
    auto scene = ec::Controller::get()->scene().lock();
    scene->manager()->addListener(fastdelegate::MakeDelegate( this , &DebugComponent::draw), DrawDebugEvent::TYPE);
}

DebugComponent::~DebugComponent()
{
    auto scene = ec::Controller::get()->scene().lock();
    scene->manager()->removeListener(fastdelegate::MakeDelegate( this , &DebugComponent::draw), DrawDebugEvent::TYPE);
}

bool DebugComponent::initialize( const ci::JsonTree &tree )
{
    
    try {
        auto min = tree["aa_bounding_box_min"].getChildren();
        auto end = min.end();
        ci::vec3 aab_min;
        int i = 0;
        for( auto it = min.begin(); it != end; ++it ) {
            aab_min[i++] = (*it).getValue<float>();
        }

        auto max = tree["aa_bounding_box_max"].getChildren();
        auto end2 = max.end();
        ci::vec3 aab_max;
        i = 0;
        for( auto it = max.begin(); it != end2; ++it ) {
            aab_max[i++] = (*it).getValue<float>();
        }
    
        mObjectBoundingBox = ci::AxisAlignedBox3f( aab_min, aab_max );
        
    } catch ( ci::JsonTree::ExcChildNotFound ex	) {
        CI_LOG_W("no aa_bounding_box found");
    }
    
    return true;
}


ci::JsonTree DebugComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "name", getName() ) );
    save.addChild( ci::JsonTree( "id", getId() ) );
    save.addChild( ci::JsonTree( "type", "debug_component" ) );
    
    auto min = ci::JsonTree::makeArray( "aa_bounding_box_min" );
    for( int i = 0; i<3; i++ ){
        min.addChild( ci::JsonTree( "", mObjectBoundingBox.getMin()[i] ) );
    }
    save.addChild(min);
    
    auto max = ci::JsonTree::makeArray( "aa_bounding_box_max" );
    for( int i = 0; i<3; i++ ){
        max.addChild( ci::JsonTree( "", mObjectBoundingBox.getMax()[i] ) );
    }
    save.addChild(max);
    
    return save;
}

const ec::ComponentNameType DebugComponent::getName() const
{
    return "debug_component";
}
const ec::ComponentUId DebugComponent::getId() const
{
    return mId;
}
const ec::ComponentType DebugComponent::getType() const
{
    return TYPE;
}

void DebugComponent::draw( ec::EventDataRef )
{
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" drawDebug");
    
    ci::gl::ScopedModelMatrix pushModel;
    ci::gl::ScopedColor pushColor;
    if( mContext->hasComponent(FrustumCullComponent::TYPE) ){
    
        auto visible = mContext->getComponent<FrustumCullComponent>().lock();
        if( visible->isVisible() ){
            ci::gl::color(1., 1., 0.);
        }else{
            ci::gl::color(1., 0., 0.);
        }
    }
    
    if( mContext->hasComponent(ec::TransformComponent::TYPE) )
    {
        auto transform = mContext->getComponent<ec::TransformComponent>().lock();
        ci::gl::multModelMatrix( transform->getModelMatrix() );
        ci::gl::drawStrokedCube(mObjectBoundingBox);

    }else if( mContext->hasComponent(LightComponent::TYPE) ){
        auto light = mContext->getComponent<LightComponent>().lock()->getLight();
        
        if( light->getType() ==  ci::Light::Type::Spot ){
            auto spot = std::dynamic_pointer_cast<ci::SpotLight>(light);
            ci::gl::multModelMatrix( ci::translate(spot->getPosition()) );
            ci::gl::drawStrokedCube(mObjectBoundingBox);

        }else if( light->getType() == ci::Light::Type::Point ){
            auto point = std::dynamic_pointer_cast<ci::PointLight>(light);
            ci::gl::multModelMatrix( ci::translate(point->getPosition()) );
            ci::gl::drawStrokedCube(mObjectBoundingBox);
        }
        
     
    }
    

    
}

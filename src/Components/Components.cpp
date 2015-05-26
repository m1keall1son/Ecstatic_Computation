//
//  Components.cpp
//  System_test
//
//  Created by Mike Allison on 5/18/15.
//
//

#include "Components.h"
#include "ComponentBase.h"
#include "TransformComponent.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "DebugComponent.h"
#include "FrustumCullComponent.h"
#include "BitComponent.h"
#include "GeomTeapotComponent.h"
#include "KinectComponent.h"
#include "RoomComponent.h"
#include "cinder/Log.h"
#include "Actor.h"
#include "AppSceneBase.h"
#include "Scene.h"
#include "Controller.h"
#include "LightManager.h"
#include "ShadowMap.h"

using namespace ci;
using namespace ci::app;

ComponentFactoryRef ComponentFactory::create()
{
    return ComponentFactoryRef( new ComponentFactory );
}

ec::ComponentBaseRef ComponentFactory::createComponent( ec::Actor* context, const ci::JsonTree& init )
{
    std::string type = "";
    
    try{
        
        type = init.getValueForKey("type");
        CI_LOG_V( "searching factory for type: "+type );
        
    } catch (const ci::JsonTree::ExcChildNotFound &e) {
        CI_LOG_E( e.what() );
    }
    
    if (type == "transform_component")
    {
        CI_LOG_V("parsed transform component");
        auto transform = ec::TransformComponent::create(context);
        transform->initialize(init);
        return transform;
        
    }
    else if (type == "light_component")
    {
        CI_LOG_V("parsed light_component");
        auto c = LightComponent::create(context);
        c->initialize(init);
        return c;
        
    }
    else if (type == "bit_component")
    {
        CI_LOG_V("parsed bit_component");
        auto bit = BitComponent::create(context);
        bit->initialize(init);
        return bit;
        
    }
    else if (type == "kinect_component")
    {
        CI_LOG_V("parsed kinect_component");
        auto kinect = KinectComponent::create(context);
        kinect->initialize(init);
        return kinect;
        
    }
    else if (type == "geom_teapot_component")
    {
        CI_LOG_V("parsed geom_teapot");
        auto teapot = GeomTeapotComponent::create(context);
        teapot->initialize(init);
        return teapot;
        
    }
    else if (type == "frustum_cull_component")
    {
        CI_LOG_V("parsed frustum_cull_component");
        auto cull = FrustumCullComponent::create(context);
        cull->initialize(init);
        return cull;
        
    }
    else if (type == "room_component")
    {
        CI_LOG_V("parsed room_component");
        auto room = RoomComponent::create(context);
        room->initialize(init);
        return room;
        
    }
    else if (type == "camera_component")
    {
        CI_LOG_V("parsed camera_component");
        auto camera = CameraComponent::create(context);
        camera->initialize(init);
        return camera;
        
    }
    else if (type == "debug_component")
    {
        CI_LOG_V("parsed debug_component");
        auto debug = DebugComponent::create(context);
        debug->initialize(init);
        return debug;
        
    }
    else{
        ///TODO exceptions and exception handling
        throw std::runtime_error( "Unknown component type" );
        return nullptr;
    }
    
}


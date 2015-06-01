//
//  Scenes.cpp
//  System_test
//
//  Created by Mike Allison on 5/18/15.
//
//

#include "cinder/Log.h"
#include "Scenes.h"
#include "SystemEvents.h"
#include "Actor.h"
#include "Components.h"
#include "CameraManager.h"
#include "ActorManager.h"
#include "Light.h"
#include "LightComponent.h"
#include "Controller.h"
#include "Scene.h"
#include "AppSceneBase.h"
#include "Events.h"
#include "TunnelComponent.h"
#include "CameraComponent.h"
#include "GUIManager.h"
#include "TunnelScene.h"
#include "TransistorScene.h"

using namespace ci;
using namespace ci::app;

SceneFactoryRef SceneFactory::create()
{
    return SceneFactoryRef( new SceneFactory );
}

ec::SceneRef SceneFactory::createScene( const ci::JsonTree& init )
{
    std::string name = "";
    
    try{
        
        name = init.getValueForKey("name");
        
    } catch (const ci::JsonTree::ExcChildNotFound &e) {
        CI_LOG_E( e.what() );
    }
    
    if (name == "tunnel")
    {
        CI_LOG_V("parsed tunnel scene");
        return TunnelScene::create(name);
    }
    if (name == "transistor")
    {
        CI_LOG_V("parsed transistor scene");
        return TransistorScene::create(name);
    }
    else{
        CI_LOG_E("No scene found for name: "+name);
        return nullptr;
    }

}


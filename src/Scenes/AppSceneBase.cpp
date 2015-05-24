//
//  AppSceneBase.cpp
//  System_test
//
//  Created by Mike Allison on 5/22/15.
//
//

#include "AppSceneBase.h"
#include "Controller.h"
#include "EventManager.h"
#include "Events.h"
#include "cinder/Log.h"


AppSceneBase::~AppSceneBase()
{
    
}

AppSceneBase::AppSceneBase( const std::string& name ):ec::Scene(name)
{
    mLights = LightManagerRef( new LightManager );
    mCameras = CameraManagerRef( new CameraManager );
}

void AppSceneBase::initialize(const ci::JsonTree &init)
{
    try {
        auto shadow_map = init["shadow_map"];
        mLights->initShadowMap(shadow_map);
        CI_LOG_V("initialized shadow map");

    } catch (const ci::JsonTree::ExcChildNotFound &e) {
        
    }
}

void AppSceneBase::update()
{
    mLights->update();
    ec::Scene::update();
}


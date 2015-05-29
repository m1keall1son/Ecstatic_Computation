//
//  Scenes.h
//  System_test
//
//  Created by Mike Allison on 5/18/15.
//
//

#pragma once
#include "Common.h"
#include "SceneFactory.h"
#include "Scene.h"
#include "Components.h"
#include "AppCommon.h"
#include "AppSceneBase.h"

class SceneFactory : public ec::SceneFactory {
    
public:
    
    static SceneFactoryRef create();
    ec::SceneRef createScene( const ci::JsonTree& init ) override;
    
};


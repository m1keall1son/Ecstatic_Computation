//
//  Components.h
//  System_test
//
//  Created by Mike Allison on 5/18/15.
//
//

#pragma once
#include "cinder/gl/Batch.h"
#include "cinder/gl/Shader.h"
#include "ComponentFactory.h"
#include "ComponentBase.h"
#include "AppCommon.h"


class ComponentFactory : public ec::ComponentFactory {
    
public:
    
    static ComponentFactoryRef create();
    ec::ComponentBaseRef createComponent( ec::Actor* context, const ci::JsonTree& init )override;
};


//
//  PostProcessingPassBase.h
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#pragma once
#include "Common.h"
#include "AppCommon.h"
#include "cinder/Json.h"
#include "Actor.h"

class PassBase : public ec::ComponentBase {
    
public:
    
    virtual const PassPriority getPriority() const = 0;
    virtual void process() = 0;
    
    virtual ~PassBase(){}
    
protected:
    
    PassBase( ec::Actor * context );
    
};
//
//  DebugComponent.h
//  System_test
//
//  Created by Mike Allison on 5/23/15.
//
//

#pragma once

#include "Common.h"
#include "AppCommon.h"
#include "ComponentBase.h"

class DebugComponent : public ec::ComponentBase {
    
public:
    
    static ec::ComponentType TYPE;
    
    static DebugComponentRef create( ec::Actor * context );
    
    virtual bool initialize( const ci::JsonTree &tree )override;
    virtual ci::JsonTree serialize()override;
    virtual bool postInit()override{ return true; }

    virtual const ec::ComponentNameType   getName() const override;
    virtual const ec::ComponentUId        getId() const override;
    virtual const ec::ComponentType       getType() const override;

    inline ci::AxisAlignedBox3f& getAxisAlignedBoundingBox(){ return mObjectBoundingBox; }
    
    void draw( ec::EventDataRef );

    virtual ~DebugComponent();

protected:

    DebugComponent( ec::Actor * context );

    ec::ComponentUId mId;
    ci::AxisAlignedBox3f mObjectBoundingBox;
    
};
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
    
    static ci::gl::Texture2dRef getDebugRender();
    
    static ec::ComponentType TYPE;
    
    static DebugComponentRef create( ec::Actor * context );
    
    virtual bool initialize( const ci::JsonTree &tree )override;
    virtual ci::JsonTree serialize()override;
    virtual bool postInit()override;

    virtual const ec::ComponentNameType   getName() const override;
    virtual const ec::ComponentUId        getId() const override;
    virtual const ec::ComponentType       getType() const override;
    virtual void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    virtual void                          cleanup()override;

    inline ci::AxisAlignedBox3f& getAxisAlignedBoundingBox(){ return mObjectBoundingBox; }
    
    void draw();

    virtual ~DebugComponent();

protected:

    DebugComponent( ec::Actor * context );

    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );

    void registerHandlers();
    void unregisterHandlers();

    ec::ComponentUId mId;
    ci::AxisAlignedBox3f mObjectBoundingBox;
    bool mShuttingDown;

};
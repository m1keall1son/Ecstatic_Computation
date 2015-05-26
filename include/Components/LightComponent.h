//
//  LightComponent.h
//  System_test
//
//  Created by Mike Allison on 5/21/15.
//
//

#pragma once

#include "AppCommon.h"
#include "Common.h"
#include "TransformComponent.h"
#include "ComponentBase.h"
#include "Light.h"

class LightComponent : public ec::ComponentBase {
    
public:
    
    static LightComponentRef create( ec::Actor* context );
    
    static ec::ComponentType TYPE;

    bool                            initialize( const ci::JsonTree &tree )override;
    bool                            postInit()override;
    ci::JsonTree                    serialize()override;
    
    const ec::ComponentNameType     getName() const override;
    const ec::ComponentUId          getId() const override;
    const ec::ComponentType         getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;

    void update( ec::EventDataRef );
    
    inline ci::LightRef getLight(){ return mLight; }
    inline bool needsUpdate(){ return mNeedsUpdate; }
    inline void markUpdate( bool doit = true){ mNeedsUpdate = doit; }
    
    virtual ~LightComponent();
    
protected:
    
    LightComponent( ec::Actor* context );
    
    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );
    
    void registerHandlers();
    void unregisterHandlers();
    
    bool         mShuttingDown;
    ci::LightRef mLight;
    bool mNeedsUpdate;
    ec::ComponentUId mId;
    
    friend class LightManager;
    
};
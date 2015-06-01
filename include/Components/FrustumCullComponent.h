//
//  FrustumCullComponent.h
//  System_test
//
//  Created by Mike Allison on 5/22/15.
//
//

#pragma once

#include "Common.h"
#include "AppCommon.h"
#include "ComponentBase.h"

class FrustumCullComponent : public ec::ComponentBase {
    
public:
    
    static ec::ComponentType              TYPE;
    
    static FrustumCullComponentRef        create( ec::Actor * context );
    
    virtual bool                          initialize( const ci::JsonTree &tree )override{ return false; }
    virtual ci::JsonTree                  serialize()override;

    virtual const ec::ComponentNameType   getName() const override;
    virtual const ec::ComponentUId        getId() const override;
    virtual const ec::ComponentType       getType() const override;
    virtual void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    virtual void                          cleanup()override{}

    virtual bool                          postInit()override{ mInitialized = true; return true; }
    virtual void                          cull( ec::EventDataRef );

    inline  bool                          isVisible(){ return mIsVisible; }

    virtual ~FrustumCullComponent();

protected:

    FrustumCullComponent( ec::Actor * context );

    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );

    void registerHandlers();
    void unregisterHandlers();

    ec::ComponentUId mId;
    bool mEnableCull;
    bool mIsVisible;
    bool mShuttingDown;
};
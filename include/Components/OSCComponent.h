//
//  OSCComponent.h
//  System_test
//
//  Created by Mike Allison on 5/23/15.
//
//

#pragma once
#include "Common.h"
#include "ComponentFactory.h"
#include "ComponentBase.h"
#include "AppCommon.h"
#include "OscListener.h"
#include "OscSender.h"

class OSCComponent : public ec::ComponentBase {
    
public:
    
    static ec::ComponentType TYPE;
    
    static OSCComponentRef create( ec::Actor* context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    void                          cleanup()override;
    
    bool postInit()override;
    
    void update( ec::EventDataRef );
    
    ~OSCComponent();
    
private:
    
    OSCComponent( ec::Actor* context );
    
    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );
    
    void registerHandlers();
    void unregisterHandlers();
    
    ec::ComponentUId mId;
    bool             mShuttingDown;
    ci::osc::Listener    mListener;
    int             mListenPort;
    
};
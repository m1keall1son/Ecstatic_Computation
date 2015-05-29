//
//  ComponentTemplate.h
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

class ComponentTemplate : public ec::ComponentBase {
    
public:
    
    static ec::ComponentType TYPE;
    
    static ComponentTemplateRef create( ec::Actor* context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    
    bool postInit()override;

    
    ~ComponentTemplate();
    
private:
    
    ComponentTemplate( ec::Actor* context );
    
    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );
    
    void registerHandlers();
    void unregisterHandlers();
    
    ec::ComponentUId mId;
    bool             mShuttingDown;
    
};
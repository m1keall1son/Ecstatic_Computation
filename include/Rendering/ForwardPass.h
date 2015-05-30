//
//  ForwardPass.h
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#pragma once
#include "Common.h"
#include "AppCommon.h"
#include "PassBase.h"

class ForwardPass : public PassBase {
    
public:
    
    ///ComponentBase
    
    static ec::ComponentType TYPE;
    
    static ForwardPassRef create( ec::Actor* context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    
    bool postInit()override;
    
    ///PostProcessingBase
    
    const PassPriority getPriority() const override;
    void process()override;
        
    ~ForwardPass();
    
private:
    
    ForwardPass(  ec::Actor* context  );
    
    void registerHandlers();
    void unregisterHandlers();
    
    void handleSceneChange( ec::EventDataRef );
    void handleShutDown( ec::EventDataRef );
    
    ShadowMapRef        mShadowMap;
    PassPriority        mPriority;
    bool                mShuttingDown;
    ec::ComponentUId    mId;
};

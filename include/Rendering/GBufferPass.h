//
//  GBufferPass.h
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#pragma once 
#include "Common.h"
#include "AppCommon.h"
#include "PassBase.h"

class GBufferPass : public PassBase {
    
public:
    
    ///ComponentBase
    
    static ec::ComponentType TYPE;
        
    static GBufferPassRef create( ec::Actor* context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    void                          cleanup()override;
    bool postInit()override;
    
    ///PostProcessingBase
    
    const PassPriority getPriority() const override;
    void process()override;
    
    inline GBufferRef getGBuffer(){ return mGBuffer; }
    
    ~GBufferPass();
    
private:
    
    GBufferPass(  ec::Actor* context  );
    
    void registerHandlers();
    void unregisterHandlers();
    
    void handleSceneChange( ec::EventDataRef );
    void handleShutDown( ec::EventDataRef );
    
    GBufferRef mGBuffer;
    PassPriority mPriority;
    bool mShuttingDown;
    ec::ComponentUId mId;
};

//
//  LightPass.h
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#pragma once
#include "Common.h"
#include "AppCommon.h"
#include "PassBase.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"

class LightPass : public PassBase {
    
public:
    
    ///ComponentBase
    
    static ec::ComponentType TYPE;
    
    static LightPassRef create( ec::Actor* context );
    
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
    
    ~LightPass();
    
private:
    
    LightPass(  ec::Actor* context  );
    
    void registerHandlers();
    void unregisterHandlers();
    
    void handleSceneChange( ec::EventDataRef );
    void handleShutDown( ec::EventDataRef );
    void handleGlslProgReload( ec::EventDataRef );

    ci::gl::BatchRef mScreenSpace;
    ci::gl::GlslProgRef mSSLightingRender, mSSLightingRiftRender;
    PassPriority mPriority;
    bool mShuttingDown;
    ec::ComponentUId mId;
    
};

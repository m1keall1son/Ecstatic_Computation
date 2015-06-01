//
//  BitComponent.h
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/24/15.
//
//

#pragma once
#include "cinder/gl/Batch.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/GlslProg.h"
#include "ComponentFactory.h"
#include "ComponentBase.h"
#include "AppCommon.h"

class BitComponent : public ec::ComponentBase {
    
public:
    
    static ec::ComponentType TYPE;
    
    static BitComponentRef create( ec::Actor* context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    void                          cleanup()override;
    
    bool postInit()override;
    
    
    void draw( ec::EventDataRef event );
    void update( ec::EventDataRef );
    void drawShadow( ec::EventDataRef );
    
    ~BitComponent();
    
private:
    
    BitComponent( ec::Actor* context );
    
    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );
    
    void handleGlslProgReload( ec::EventDataRef );
    
    void registerHandlers();
    void unregisterHandlers();
    
    ci::gl::BatchRef mBit, mBitShadow;
    ci::gl::GlslProgRef mBitShadowRender, mBitRender;
    ec::ComponentUId mId;
    bool             mShuttingDown;
    
};
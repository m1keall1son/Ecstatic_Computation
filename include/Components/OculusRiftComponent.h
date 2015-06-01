//
//  OculusRiftComponent.h
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
#include "cinder/gl/Ubo.h"

#include "CinderOculus.h"

class OculusRiftComponent : public ec::ComponentBase {
    
public:
    
    static  const ci::vec2&  getRiftFboSize();
    
    struct RiftData {
        ci::mat4 matrices[4];
        float nearClip;
        float farClip;
        float reserved0;
        float reserved1;
    };
    
    static void                   initializeRift();
    static int                    getRiftUboLocation();
    
    static ec::ComponentType TYPE;
    
    static OculusRiftComponentRef create( ec::Actor* context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    void                          cleanup()override;
    
    bool postInit()override;
    
    inline hmd::OculusRiftRef getRift(){ return mRift; }
    
    void keyUp( ci::app::KeyEvent & );
    void keyDown( ci::app::KeyEvent & );
    
    ~OculusRiftComponent();
    
private:
    
    OculusRiftComponent( ec::Actor* context );
    
    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );
    void handleSwitchCamera( ec::EventDataRef );
    
    void handleUpdate( ec::EventDataRef );
    
    void registerHandlers();
    void unregisterHandlers();
    
    ec::ComponentUId mId;
    bool             mShuttingDown;
    
    hmd::OculusRiftRef mRift;
    ci::gl::UboRef     mRiftUbo;
    
};
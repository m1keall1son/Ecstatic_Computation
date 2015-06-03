//
//  KinectComponent.h
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/23/15.
//
//

#pragma once
#include "AppCommon.h"
#include "Common.h"
#include "ComponentBase.h"
#include "CinderFreenect.h"

class KinectComponent : public ec::ComponentBase {
    
public:
    
    static ec::ComponentType TYPE;
    
    static void initializeKinect();
    static KinectComponentRef create( ec::Actor* context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    void                          cleanup()override;
    bool postInit()override;
    
    void drawShadow( ec::EventDataRef );
    
    void update( ec::EventDataRef );
    void draw( ec::EventDataRef );
    void drawRift( ec::EventDataRef event );
    
    ci::gl::Texture2dRef getKinectTexture();
    
    ~KinectComponent();
    
private:
    
    KinectComponent( ec::Actor * context );
    
    void handleReloadGlslProg( ec::EventDataRef );
    
    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );
    
    void registerListeners();
    void unregisterListeners();
    
    bool mShuttingDown;
    ec::ComponentUId mId;
    ci::KinectRef mKinect;
    
    ci::vec2 mThresholds;
    
    ci::gl::Texture2dRef mKinectDepthTexture;
    ci::gl::Texture2dRef mKinectColorTexture;
    ci::gl::GlslProgRef mKinectRender, mKinectShadowRender, mKinectStereoRender;
    ci::gl::BatchRef mKinectMesh, mKinectMeshShadow;
    
    float mTriangleCutoff;
    
};
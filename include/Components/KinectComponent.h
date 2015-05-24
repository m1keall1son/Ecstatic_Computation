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
    
    static KinectComponentRef create( ec::Actor* context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    
    bool postInit()override;
    
    void drawShadow( ec::EventDataRef );
    
    void update( ec::EventDataRef );
    void draw( ec::EventDataRef );
    
    ci::gl::Texture2dRef getKinectTexture();
    
    ~KinectComponent();
    
private:
    
    KinectComponent( ec::Actor * context );
    
    void reloadGlsl( ec::EventDataRef );
    
    ec::ComponentUId mId;
    ci::KinectRef mKinect;
    
    ci::gl::Texture2dRef mKinectDepthTexture;
    ci::gl::GlslProgRef mKinectRender, mKinectShadowRender;
    ci::gl::BatchRef mKinectMesh;
    
};
//
//  CubeTunnelComponent.h
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/28/15.
//
//

#pragma once
#include "AppCommon.h"
#include "Common.h"
#include "ComponentBase.h"

class TunnelComponent : public ec::ComponentBase {
  
public:
    
    static ec::ComponentType TYPE;
    
    static TunnelComponentRef create( ec::Actor* context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    
    bool postInit()override;
    
    inline const ci::BSpline3f& getSpline(){return mSpline;}
    inline const float& getFaceRadius(){return mFaceRadius;}
    inline float& getNoiseScale(){ return mNoiseScale; }
    inline const float& getTunnelLength(){ return mLength; }

    void draw( ec::EventDataRef event );
    void update( ec::EventDataRef );
    void drawShadow( ec::EventDataRef );
    
    ~TunnelComponent();
    
private:
    
    TunnelComponent( ec::Actor* context );
    
    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );
    
    void handleGlslProgReload( ec::EventDataRef );
    
    void registerHandlers();
    void unregisterHandlers();
    
    ci::gl::BatchRef    mTunnel, mTunnelShadow;
    ci::gl::GlslProgRef mTunnelBasicRender, mTunnelShadowRender;
    ci::BSpline3f       mSpline;
    
    ec::ComponentUId    mId;
    bool                mShuttingDown;
    
    float               mNoiseScale;
    
    ci::vec2            mAmplitude;
    ci::vec2            mFrequency;
    ci::vec2            mNoiseAmplitude;
    ci::vec2            mNoiseFrequency;
    float               mFaceRadius;
    float               mLength;
    float               mStep;

};
//
//  LandscapeComponent.h
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
#include "TransformComponent.h"
#include "cinder/Timeline.h"

class LandscapeComponent : public ec::ComponentBase {
    
public:
    
    static ec::ComponentType TYPE;
    
    static LandscapeComponentRef create( ec::Actor* context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    void                          cleanup()override;
    
    bool postInit()override;
    
    void draw( ec::EventDataRef );
    void drawRift( ec::EventDataRef );
    void drawShadow( ec::EventDataRef );

    inline void setSinkHoleScale( float scale ){ mSinkHoleScale = scale; }
    inline float getSinkHoleScale(){ return mSinkHoleScale; }
    
    ~LandscapeComponent();
    
private:
    
    LandscapeComponent( ec::Actor* context );
    
    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );
    void handleReloadGlslProg( ec::EventDataRef );
    void handleMicVolumeEvent( ec::EventDataRef );
    
    void registerHandlers();
    void unregisterHandlers();
    
    ec::ComponentUId mId;
    bool             mShuttingDown;
    
    ci::gl::BatchRef    mLandscape, mLandscapeShadow, mHead, mHeadShadow, mTeeth, mTeethShadow,mFoot, mFootShadow;
    ci::gl::GlslProgRef mLandscapeRender, mLandscapeShadowRender, mObjectRender;
    ci::gl::Texture2dRef    mMap;
    float mColorScale;
    float mSinkHoleScale;
    ci::Anim<float> mFogDensity;
    
    //TODO: make this provate for transform component and make a real scenegraph...
    
    ec::TransformComponent::Transformables mHeadTransform, mFootTransform, mTeethTransform;
    
    
};
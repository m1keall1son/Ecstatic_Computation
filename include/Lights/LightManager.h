//
//  LightManager.h
//  System_test
//
//  Created by Mike Allison on 5/21/15.
//
//

#pragma once

#include "AppCommon.h"
#include "Common.h"
#include "cinder/gl/Ubo.h"
#include "Light.h"
#include "ShadowMap.h"
#include "cinder/Json.h"

class LightManager {
    
public:

    static ci::Light::Type parseLightType( const std::string &type );
    static std::string parseLightTypeToString( const ci::Light::Type &type );
    
    inline std::vector< ec::ActorUId >& getLights(){ return mLights; }
    inline int getLightUboLocation(){ return mLightUboLocation; }
    
    void initShadowMap( const ci::JsonTree& init );
    
    inline ShadowMapRef getShadowMap(){ return mShadowMap; }
    
    void update();
    
    ~LightManager();
    
private:
    
    struct Lights {
        ci::Light::Data     data[24];
        int                 numLights;
        ci::vec4            upDirection;
    };
    
    LightManager();
    
    void handleLightRegistration( ec::EventDataRef );
    void handleShutDown( ec::EventDataRef );
    
    std::vector< ec::ActorUId > mLights;
    ci::gl::UboRef              mLightUbo;
    int                         mLightUboLocation;
    ShadowMapRef                mShadowMap;
    bool                        mShuttingDown;
    
    friend class AppSceneBase;
    
};
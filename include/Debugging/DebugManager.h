//
//  DebugManager.h
//  System_test
//
//  Created by Mike Allison on 5/21/15.
//
//

#pragma once

#include "AppCommon.h"
#include "Common.h"
#include "cinder/Json.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"

class DebugManager {
    
public:
    
    inline ec::IdType getId(){ return mId; }
    inline ci::gl::Texture2dRef getDebugTexture(){ return mDebugTexture; }
    
    ~DebugManager();
    
private:
    
    DebugManager();
    
    void handleDebugComponentRegistration( ec::EventDataRef );
    void handleShutDown( ec::EventDataRef );
    void initDebug( ec::EventDataRef );
    void handleDeferredDebugDraw( ec::EventDataRef );
    void handleDebugDraw( ec::EventDataRef );
    void handleRiftDebugDraw( ec::EventDataRef );

    std::map< ec::ActorUId, DebugComponentRef > mDebugComponents;
    
    bool                        mShuttingDown;
    ec::IdType                  mId;
    
    ci::gl::FboRef              mDebugFbo;
    ci::gl::Texture2dRef        mDebugTexture;
    
    friend class AppSceneBase;
    
};
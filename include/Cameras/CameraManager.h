//
//  CameraManager.h
//  System_test
//
//  Created by Mike Allison on 5/21/15.
//
//

#pragma once

#include "AppCommon.h"
#include "Common.h"
#include "cinder/MayaCamUI.h"
#include "CameraComponent.h"

class CameraManager {
    
public:
    
    const ci::CameraPersp& getCamera( const CameraComponent::CameraType& cam_type );
    const ci::CameraPersp& getActiveCamera();
    
    inline ec::IdType getId(){ return mId; }
    
    ~CameraManager();
    
private:
    
    CameraManager();
    
    void handleCameraRegistration( ec::EventDataRef );
    void handleShutDown( ec::EventDataRef );
    void handleSwitchCamera( ec::EventDataRef );
    
    void updateCamera( ec::EventDataRef );
    
    std::map< CameraComponent::CameraType, ec::ActorUId >    mCameras;
    ci::CameraPersp                         mDefaultCamera;
    ci::MayaCamUI                           mUI;
    bool                                    mShuttingDown;
    bool                                    mCameraSet;
    CameraComponent::CameraType             mCurrentCamera;
    ec::IdType                              mId;
    
    friend class AppSceneBase;
    
};
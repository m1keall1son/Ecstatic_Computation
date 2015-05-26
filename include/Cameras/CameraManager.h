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

class CameraManager {
    
public:
    
    enum CameraType { MAIN_CAMERA, DEBUG_CAMERA };

    static CameraType parseCameraType( const ec::ActorTypeQualifier &qualifier );
    
    const ci::CameraPersp& getCamera( const CameraType& cam_type );
    const ci::CameraPersp& getActiveCamera();

    ~CameraManager();
    
private:
    
    CameraManager();
    
    void handleCameraRegistration( ec::EventDataRef );
    void handleShutDown( ec::EventDataRef );
    void handleSwitchCamera( ec::EventDataRef );
    
    std::map< CameraType, ec::ActorUId >    mCameras;
    ci::CameraPersp                         mDefaultCamera;
    ci::MayaCamUI                           mUI;
    bool                                    mShuttingDown;
    bool                                    mCameraSet;
    CameraType                              mCurrentCamera;
    
    friend class AppSceneBase;
    
};
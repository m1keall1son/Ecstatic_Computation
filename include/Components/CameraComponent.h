//
//  CameraComponent.h
//  System_test
//
//  Created by Mike Allison on 5/21/15.
//
//

#pragma once

#include "AppCommon.h"
#include "Common.h"
#include "ComponentBase.h"
#include "TransformComponent.h"


class CameraComponent : public ec::ComponentBase {
    
public:
    
    enum CameraType { MAIN_CAMERA, DEBUG_CAMERA };
    
    static ec::ComponentType TYPE;
    
    static CameraType parseCameraType( const std::string &qualifier );
    static std::string parseCameraType( const CameraType &qualifier );

    static CameraComponentRef create( ec::Actor * context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    bool                          postInit()override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    void                          cleanup()override;
    

    void update( ec::EventDataRef );
    
    inline ci::CameraPersp& getCamera(){ return mCamera; }
    inline CameraType getCamType(){ return mCamType; }
    
    ~CameraComponent();

private:

    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );

    void registerHandlers();
    void unregisterHandlers();

    void updateCameraParams();

    CameraComponent( ec::Actor * context );
    ci::CameraPersp mCamera;
    ec::ComponentUId mId;
    CameraType mCamType;
    

    float mFov, mNear, mFar;
    ci::vec3 mInterestPoint;

    bool mShuttingDown;

};

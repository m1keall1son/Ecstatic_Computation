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
#include "cinder/MayaCamUI.h"

class CameraComponent : public ec::ComponentBase {
    
public:
    
    static ec::ComponentType TYPE;
    
    static CameraComponentRef create( ec::Actor * context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    bool                          postInit()override{ return true; }
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    
    void update( ec::EventDataRef );
    
    inline const ci::CameraPersp& getCamera(){ return mUI.getCamera(); }

    ~CameraComponent();

private:
    
    CameraComponent( ec::Actor * context );
    ci::CameraPersp mCamera;
    ci::MayaCamUI mUI;
    ec::ComponentUId mId;
    
};

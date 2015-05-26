//
//  RoomComponent.h
//  System_test
//
//  Created by Mike Allison on 5/23/15.
//
//

#pragma once
#include "cinder/gl/Batch.h"
#include "cinder/gl/Shader.h"
#include "ComponentFactory.h"
#include "ComponentBase.h"
#include "AppCommon.h"

class RoomComponent : public ec::ComponentBase {
    
public:
    
    static ec::ComponentType TYPE;
    
    static RoomComponentRef create( ec::Actor* context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;

    bool postInit()override;
    
    void drawShadow( ec::EventDataRef );
    
    void update( ec::EventDataRef );
    void draw( ec::EventDataRef );

    ~RoomComponent();

private:

    RoomComponent( ec::Actor* context );
    
    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );
    
    void registerListeners();
    void unregisterListeners();
    
    bool mShuttingDown;
    ci::gl::BatchRef mRoom;
    ci::gl::BatchRef mRoomShadow;
    float mRoomSize;
    ec::ComponentUId mId;

};
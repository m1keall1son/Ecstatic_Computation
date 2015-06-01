//
//  GeomTeapotComponentComponent.h
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

class GeomTeapotComponent : public ec::ComponentBase {
    
public:
    
    static ec::ComponentType TYPE;
    
    static GeomTeapotComponentRef create( ec::Actor* context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    void                           cleanup()override;
    bool postInit()override;


    void draw( ec::EventDataRef event );
    void update( ec::EventDataRef );
    void drawShadow( ec::EventDataRef );

    ~GeomTeapotComponent();

private:

    GeomTeapotComponent( ec::Actor* context );
    
    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );
    
    void registerHandlers();
    void unregisterHandlers();
    
    ci::gl::BatchRef mTeapot, mTeapotShadow;
    ec::ComponentUId mId;
    bool             mShuttingDown;

};
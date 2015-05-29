//
//  AppSceneBase.h
//  System_test
//
//  Created by Mike Allison on 5/22/15.
//
//

#pragma once
#include "Scene.h"
#include "LightManager.h"
#include "CameraManager.h"

///Common application layer scene class

class AppSceneBase : public ec::Scene {
    
public:
    
    inline LightManagerRef lights(){ return mLights; }
    inline CameraManagerRef cameras(){ return mCameras; }
    
    virtual void initialize( const ci::JsonTree& init )override;
    
    virtual void postInit()override = 0;
    
    virtual void update()override;
    virtual void preDraw() = 0;
    virtual void draw() = 0;
    virtual void postDraw() = 0;
    virtual void shutDown( ec::EventDataRef ) = 0;
    
    virtual ~AppSceneBase();
    
protected:
    
    AppSceneBase( const std::string& name );
    
    void handleSaveScene( ec::EventDataRef );
    void initGUI( const ec::GUIManagerRef &gui_manager )override;
    
        
    LightManagerRef mLights;
    CameraManagerRef mCameras;
    
};
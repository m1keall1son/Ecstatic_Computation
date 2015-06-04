//
//  IntroScene.h
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/28/15.
//
//

#pragma once

#include "AppCommon.h"
#include "Common.h"
#include "AppSceneBase.h"

class IntroScene : public AppSceneBase {
    
public:
    
    static IntroSceneRef create( const std::string& name );
    
    void initialize( const ci::JsonTree& init )override;
    
    void update() override;
    void preDraw() override;
    void draw() override;
    void postDraw() override;
    void shutDown( ec::EventDataRef )override;
    std::vector<ec::ActorUId> shutdown()override;
    
    void postInit()override;
    
    ~IntroScene();
    
private:
    
    void initGUI( const ec::GUIManagerRef &gui_manager )override;
    void handlePresentScene( ec::EventDataRef );
    
    IntroScene( const std::string& name );
    float mScale;
    bool mEnableSinkHole;

};
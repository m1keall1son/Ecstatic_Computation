//
//  TunnelScene.h
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/28/15.
//
//

#pragma once

#include "AppCommon.h"
#include "Common.h"
#include "AppSceneBase.h"

class TunnelScene : public AppSceneBase {
    
public:
    
    static TunnelSceneRef create( const std::string& name );
    
    void update() override;
    void preDraw() override;
    void draw() override;
    void postDraw() override;
    void shutDown( ec::EventDataRef )override;
    
    void postInit()override;
    
private:
    
    void initGUI( const ec::GUIManagerRef &gui_manager )override;
    
    TunnelScene( const std::string& name );
    
    float mTunnelSpeed;
    float mTunnelSamplePt;
    float mTunnelAccel;
    bool mScrubTunnel;
};
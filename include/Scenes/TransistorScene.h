//
//  TransistorScene.h
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/28/15.
//
//

#pragma once

#include "AppCommon.h"
#include "Common.h"
#include "AppSceneBase.h"

class TransistorScene : public AppSceneBase {
    
public:
    
    static TransistorSceneRef create( const std::string& name );
    
    void initialize( const ci::JsonTree& init )override;

    void update() override;
    void preDraw() override;
    void draw() override;
    void postDraw() override;
    std::vector<ec::ActorUId> shutdown()override;
    void shutDown( ec::EventDataRef )override;
    
    void postInit()override;
    
    ~TransistorScene();
    
private:
    
    void initGUI( const ec::GUIManagerRef &gui_manager )override;
    void handlePresentScene( ec::EventDataRef );
    
    TransistorScene( const std::string& name );
    
};
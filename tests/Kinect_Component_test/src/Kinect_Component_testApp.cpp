#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Json.h"
#include "cinder/Log.h"

#include "Controller.h"
#include "Scenes.h"
#include "Components.h"
#include "Scene.h"
#include "Events.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Kinect_Component_testApp : public App {
public:
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void keyUp( KeyEvent event ) override;
    void update() override;
    void draw() override;
    void cleanup()override;
    bool mDebug = false;
    bool mGUI = false;
    ec::ControllerRef mController;
    
};

void Kinect_Component_testApp::setup()
{
    //configure system with my concrete scene factory and component factory
    mController = ec::Controller::create( SceneFactory::create(), ComponentFactory::create() );
    //init system
    mController->initialize( JsonTree( loadAsset("configs/config.json") ) );
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
}

void Kinect_Component_testApp::mouseDown( MouseEvent event )
{
}

void Kinect_Component_testApp::keyUp( KeyEvent event )
{
    
    if(event.getChar() == KeyEvent::KEY_d){
        mDebug = !mDebug;
        mController->enableDebug( mDebug );
        if(mDebug)
            mController->scene().lock()->manager()->queueEvent( SwitchCameraEvent::create( CameraManager::CameraType::DEBUG_CAMERA ) );
        else
            mController->scene().lock()->manager()->queueEvent( SwitchCameraEvent::create( CameraManager::CameraType::MAIN_CAMERA ) );
    }
    else if( event.getChar() == KeyEvent::KEY_r ){
        mController->scene().lock()->manager()->queueEvent( ReloadGlslProgEvent::create() );
        
    }
    else if( event.getChar() == KeyEvent::KEY_s ){
        mController->scene().lock()->manager()->queueEvent( SaveSceneEvent::create() );
    }
    else if( event.getChar() == KeyEvent::KEY_g ){
        mGUI = !mGUI;
        mController->enableGUI( mGUI );
    }
}

void Kinect_Component_testApp::update()
{
    mController->update();
}

void Kinect_Component_testApp::draw()
{
    gl::clear( ColorA( 0,0,0,1 ) );
    mController->draw();
}

void Kinect_Component_testApp::cleanup()
{
  mController->eventManager()->triggerEvent(ec::ShutDownEvent::create());
}

void prepareSettings( App::Settings*settings )
{
    settings->setFullScreen();
}

CINDER_APP( Kinect_Component_testApp, RendererGl, prepareSettings )

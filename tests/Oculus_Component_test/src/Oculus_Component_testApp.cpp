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

class Oculus_Component_test : public App {
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

void Oculus_Component_test::setup()
{
    //configure system with my concrete scene factory and component factory
    mController = ec::Controller::create( this, SceneFactory::create(), ComponentFactory::create() );
    //init system
    mController->initialize( JsonTree( loadAsset("configs/config.json") ) );
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
}

void Oculus_Component_test::mouseDown( MouseEvent event )
{
}

void Oculus_Component_test::keyUp( KeyEvent event )
{
    
    if(event.getChar() == KeyEvent::KEY_d){
        mDebug = !mDebug;
        mController->enableDebug( mDebug );
        if(mDebug)
            mController->scene().lock()->manager()->queueEvent( SwitchCameraEvent::create( CameraComponent::CameraType::DEBUG_CAMERA ) );
        else
            mController->scene().lock()->manager()->queueEvent( SwitchCameraEvent::create( CameraComponent::CameraType::MAIN_CAMERA ) );
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

void Oculus_Component_test::update()
{
    mController->update();
    if(getElapsedFrames()%30==0)cout<<getAverageFps()<<endl;
}

void Oculus_Component_test::draw()
{
    mController->draw();    
}

void Oculus_Component_test::cleanup()
{
    mController->eventManager()->triggerEvent(ec::ShutDownEvent::create());
}

void prepareSettings( App::Settings*settings )
{
    ec::Controller::initializeRift();
    
    if( ec::Controller::isRiftEnabled() ){
        settings->disableFrameRate();
        settings->setWindowSize( 1920, 1080 );
    }else{
        settings->setFullScreen();
    }
    
}

CINDER_APP( Oculus_Component_test, RendererGl( RendererGl::Options().msaa(0) ), prepareSettings )

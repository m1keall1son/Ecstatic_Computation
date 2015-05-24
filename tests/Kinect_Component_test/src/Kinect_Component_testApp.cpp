#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Json.h"
#include "cinder/Log.h"

#include "Controller.h"
#include "Scenes.h"
#include "Components.h"

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
    bool mDebug = false;
};

void Kinect_Component_testApp::setup()
{
    //configure system with my concrete scene factory and component factory
    ec::Controller::create( SceneFactory::create(), ComponentFactory::create() );
    //init system
    ec::Controller::get()->initialize( JsonTree( loadAsset("configs/config.json") ) );
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
}

void Kinect_Component_testApp::mouseDown( MouseEvent event )
{
}

void Kinect_Component_testApp::keyUp( KeyEvent event )
{
    mDebug = !mDebug;
    ec::Controller::get()->enableDebug( mDebug );
}

void Kinect_Component_testApp::update()
{
    ec::Controller::get()->update();
}

void Kinect_Component_testApp::draw()
{
    gl::clear( ColorA( 0,0,0,1 ) );
    ec::Controller::get()->draw();
}

void prepareSettings( App::Settings*settings )
{
    settings->setFullScreen();
}

CINDER_APP( Kinect_Component_testApp, RendererGl, prepareSettings )

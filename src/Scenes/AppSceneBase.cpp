//
//  AppSceneBase.cpp
//  System_test
//
//  Created by Mike Allison on 5/22/15.
//
//

#include "AppSceneBase.h"
#include "Controller.h"
#include "EventManager.h"
#include "Events.h"
#include "cinder/Log.h"
#include "Actor.h"
#include "SystemEvents.h"
#include "cinder/params/Params.h"
#include "GUIManager.h"
#include "GBuffer.h"

using namespace ci;
using namespace ci::app;

AppSceneBase::~AppSceneBase()
{
    
}

AppSceneBase::AppSceneBase( const std::string& name ):ec::Scene(name)
{
    mLights = LightManagerRef( new LightManager );
    mCameras = CameraManagerRef( new CameraManager );
        
    mSceneManager->addListener(fastdelegate::MakeDelegate(this, &AppSceneBase::handleSaveScene), SaveSceneEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mCameras.get(), &CameraManager::handleSwitchCamera), SwitchCameraEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mCameras.get(), &CameraManager::updateCamera), UpdateEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mCameras.get(), &CameraManager::handleCameraRegistration), ComponentRegistrationEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mLights.get(), &LightManager::handleLightRegistration), ComponentRegistrationEvent::TYPE);
    
    ///CREATE GBUFFER
    
    ///albedo
    gl::Texture2d::Format colorData = gl::Texture2d::Format()
    .internalFormat( GL_RGB10_A2 )
    .magFilter( GL_NEAREST )
    .minFilter( GL_NEAREST )
    .wrap( GL_CLAMP_TO_EDGE )
    .dataType( GL_FLOAT );
    
    ///encoded normal and material id
    gl::Texture2d::Format data = gl::Texture2d::Format()
    .internalFormat( GL_RGBA16F )
    .magFilter( GL_NEAREST )
    .minFilter( GL_NEAREST )
    .wrap( GL_CLAMP_TO_EDGE )
    .dataType( GL_HALF_FLOAT );
    
    gl::Texture2d::Format depthFormat;
    depthFormat.setInternalFormat( GL_DEPTH_COMPONENT32F );
    depthFormat.setMagFilter( GL_LINEAR );
    depthFormat.setMinFilter( GL_LINEAR );
    depthFormat.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
    depthFormat.setCompareMode( GL_COMPARE_REF_TO_TEXTURE );
    depthFormat.setCompareFunc( GL_LEQUAL );
    depthFormat.setLabel("uGBufferDepthTexture");
    
    GBuffer::Format fmt;
    fmt.attachment(GL_COLOR_ATTACHMENT0, colorData, "uAlbedo");
    fmt.attachment(GL_COLOR_ATTACHMENT1, data, "uData");
    fmt.depthTexture(depthFormat);
    
    mGBuffer = GBuffer::create(fmt);

}

void AppSceneBase::initialize(const ci::JsonTree &init)
{
    try {
        auto shadow_map = init["shadow_map"];
        mLights->initShadowMap(shadow_map);
        CI_LOG_V("initialized shadow map");

    } catch (const ci::JsonTree::ExcChildNotFound &e) {
        
    }
}

void AppSceneBase::update()
{
    mLights->update();
    ec::Scene::update();
}

void AppSceneBase::initGUI(const ec::GUIManagerRef &gui_manager)
{
    
    //add any special thing to gui for scene?
    auto scene_params = ci::params::InterfaceGl::create(ci::app::getWindow(), "Scene: "+getName() + " GUI", ci::vec2(100,100));
    auto saveFn = [&]{ mSceneManager->triggerEvent(SaveSceneEvent::create()); };
    scene_params->addButton("Save", saveFn);
    scene_params->hide();
    
    gui_manager->instertGUI(getId(), scene_params);
    
    //tell everyone else to load up
    for( auto & actor : mActors ){
        if(auto actor_strong = actor.second.lock()){
            gui_manager->instertGUI(actor_strong->getId(), actor_strong->initGUI());
        }
    }
}

void AppSceneBase::handleSaveScene(ec::EventDataRef)
{
    ci::JsonTree save = ci::JsonTree::makeObject();
    save.addChild(ci::JsonTree("name", getName()));
    save.addChild(ci::JsonTree("id", (uint64_t)getId()));
    
    if( mLights->getShadowMap() ){
        auto shadow_map = ci::JsonTree::makeObject( "shadow_map" );
        shadow_map.addChild( ci::JsonTree("size", mLights->getShadowMap()->getSize().x ) );
        //todo:: more params??
        save.addChild(shadow_map);
    }
    
    auto actors = ci::JsonTree::makeArray("actors");
    
    for( auto & actor: mActors ){
        
        auto actor_strong = actor.second.lock();
        if(actor_strong){
            actors.addChild( actor_strong->serialize() );
        }
    }
    
    save.addChild(actors);
    save.serialize();
    
    std::vector<std::string> allowed_extensions( 1, "json" );
    
    auto save_path = ci::app::getSaveFilePath( ci::app::getAssetDirectories()[0], allowed_extensions );
    
    if(save_path != "")
        save.write( save_path, ci::JsonTree::WriteOptions().indented());
    
}


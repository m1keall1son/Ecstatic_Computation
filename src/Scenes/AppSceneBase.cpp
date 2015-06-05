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
#include "DebugManager.h"

using namespace ci;
using namespace ci::app;

AppSceneBase::~AppSceneBase()
{
    
}

AppSceneBase::AppSceneBase( const std::string& name ):ec::Scene(name)
{

}

void AppSceneBase::initialize(const ci::JsonTree &init)
{
    
    mLights = LightManagerRef( new LightManager );
    mCameras = CameraManagerRef( new CameraManager );
    mDebug = DebugManagerRef(new DebugManager );
    
    mSceneManager->addListener(fastdelegate::MakeDelegate(this, &AppSceneBase::handleSaveScene), SaveSceneEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mCameras.get(), &CameraManager::handleSwitchCamera), SwitchCameraEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mCameras.get(), &CameraManager::updateCamera), UpdateEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mCameras.get(), &CameraManager::handleCameraRegistration), ComponentRegistrationEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mLights.get(), &LightManager::handleLightRegistration), ComponentRegistrationEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mDebug.get(), &DebugManager::handleDebugComponentRegistration), ComponentRegistrationEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mDebug.get(), &DebugManager::initDebug), ShareGeometryDepthTextureEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mDebug.get(), &DebugManager::handleDebugDraw), DrawDebugEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mDebug.get(), &DebugManager::handleDeferredDebugDraw), DrawDeferredDebugEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mDebug.get(), &DebugManager::handleRiftDebugDraw), DrawToRiftBufferEvent::TYPE);
    
}

void AppSceneBase::update()
{
    mLights->update();
    ec::Scene::update();
}

void AppSceneBase::initGUI( ec::GUIManager* gui_manager)
{
    
    //add any special thing to gui for scene?
    auto gui = gui_manager->getMainGui();
    auto saveFn = [&]{ mSceneManager->triggerEvent(SaveSceneEvent::create()); };
    gui->addButton("Save", saveFn);
    gui->hide();
    
//    gui_manager->instertGUI(getId(), scene_params);
    
    //tell everyone else to load up
    for( auto & actor : mActors ){
        if(auto actor_strong = actor.second.lock()){
            //gui_manager->instertGUI(actor_strong->getId(), );
            actor_strong->initGUI( gui_manager );
        }
    }
}

void AppSceneBase::handleSaveScene(ec::EventDataRef)
{
    ci::JsonTree save = ci::JsonTree::makeObject();
    save.addChild(ci::JsonTree("name", getName()));
    save.addChild(ci::JsonTree("id", (uint64_t)getId()));
        
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


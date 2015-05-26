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


AppSceneBase::~AppSceneBase()
{
    
}

AppSceneBase::AppSceneBase( const std::string& name ):ec::Scene(name)
{
    mLights = LightManagerRef( new LightManager );
    mCameras = CameraManagerRef( new CameraManager );
    mSceneManager->addListener(fastdelegate::MakeDelegate(this, &AppSceneBase::handleSaveScene), SaveSceneEvent::TYPE);
    mSceneManager->addListener(fastdelegate::MakeDelegate(mCameras.get(), &CameraManager::handleSwitchCamera), SwitchCameraEvent::TYPE);

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
    save.write( ci::app::getAssetDirectories()[0] / "saved_scenes" / ci::fs::path(getName()+"_saved_config.json"), ci::JsonTree::WriteOptions().indented() );
    
}


//
//  Scenes.cpp
//  System_test
//
//  Created by Mike Allison on 5/18/15.
//
//

#include "cinder/Log.h"
#include "Scenes.h"
#include "SystemEvents.h"
#include "Actor.h"
#include "Components.h"
#include "CameraManager.h"
#include "ActorManager.h"
#include "Light.h"
#include "LightComponent.h"
#include "Controller.h"
#include "Scene.h"
#include "AppSceneBase.h"
#include "Events.h"

using namespace ci;
using namespace ci::app;

SceneFactoryRef SceneFactory::create()
{
    return SceneFactoryRef( new SceneFactory );
}

ec::SceneRef SceneFactory::createScene( const ci::JsonTree& init )
{
    std::string name = "";
    
    try{
        
        name = init.getValueForKey("name");
        
    } catch (const ci::JsonTree::ExcChildNotFound &e) {
        CI_LOG_E( e.what() );
    }
    
    if (name == "intro")
    {
        CI_LOG_V("parsed intro scene");
        return IntroScene::create(name);
    }else{
        return nullptr;
    }

}

IntroSceneRef IntroScene::create( const std::string& name )
{
    return IntroSceneRef( new IntroScene(name) );
}

IntroScene::IntroScene( const std::string& name ):AppSceneBase(name)
{
    //initialize stuff
    CI_LOG_V("Intro scene constructed");
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &IntroScene::shutDown), ec::ShutDownEvent::TYPE);
}

void IntroScene::shutDown(ec::EventDataRef)
{
    CI_LOG_V( "intro_scene handle shutdown");
}

void IntroScene::update()
{
    //do stuff
    CI_LOG_V("Intro scene updating");
    
    CI_LOG_V("update components event triggered");
    mSceneManager->triggerEvent( UpdateEvent::create() );

    //update the superclass
    AppSceneBase::update();
}

void IntroScene::preDraw()
{
    CI_LOG_V("cull visible event triggered");
    mSceneManager->triggerEvent( CullEvent::create() );
}

void IntroScene::draw()
{
    
    ///DRAW SHADOWS
    CI_LOG_V("Drawing into shadowbuffers");
    
    {
        gl::ScopedFramebuffer shadow_buffer( mLights->getShadowMap()->getFbo() );
        
        gl::clear();

        for( auto & light_id : mLights->getLights() ){
            
            auto light_actor = ec::ActorManager::get()->retreiveUnique(light_id).lock();
            
            if(light_actor){
                
                auto light_component = light_actor->getComponent<LightComponent>().lock();
                auto light = light_component->getLight();
                if( light->hasShadows() )
                {
                    if( light->getType() == Light::Type::Spot ){
                        auto spot_light = std::dynamic_pointer_cast<SpotLight>(light);
                        gl::ScopedMatrices pushMatrix;
                        gl::setViewMatrix( spot_light->getViewMatrix() );
                        gl::setProjectionMatrix( spot_light->getProjectionMatrix() );
                        gl::setModelMatrix(mat4());
                        auto shadow_view_mapping = spot_light->getMapping();
                        gl::ScopedViewport shadow_view( vec2( shadow_view_mapping.x, shadow_view_mapping.y ), vec2( shadow_view_mapping.x + shadow_view_mapping.z, shadow_view_mapping.y + shadow_view_mapping.w ) );
                        
                        mSceneManager->triggerEvent( DrawShadowEvent::create() );
                        
                        
                    }
                    
                }
            }
            
        }
        
    }
    
    //do stuff
    CI_LOG_V("Intro scene preDrawing");
    
    {
        gl::ScopedMatrices pushMatrix;
        
        gl::setMatrices(mCameras->getActiveCamera());
        
        gl::ScopedTextureBind shadowMap( mLights->getShadowMap()->getTexture(), 3 );
    
        CI_LOG_V("draw visible event triggered");
        mSceneManager->triggerEvent( DrawToMainBufferEvent::create() );

        if( ec::Controller::get()->debugEnabled() )
            mSceneManager->triggerEvent( DrawDebugEvent::create() );
    }

    
}

void IntroScene::postDraw()
{
    //do stuff post draw
    
}

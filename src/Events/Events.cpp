//
//  Events.cpp
//  System_test
//
//  Created by Mike Allison on 4/28/15.
//
//

#include "Events.h"
#include "Common.h"

//RELOAD GLSL PROGS -------------------------------------------------------------/

ec::EventType ReloadGlslProgEvent::TYPE = ec::getHash("reload_glslprogs");

ReloadGlslProgEventRef ReloadGlslProgEvent::create()
{
    return ReloadGlslProgEventRef( new ReloadGlslProgEvent() );
}

ReloadGlslProgEvent::ReloadGlslProgEvent() : ec::EventData( cinder::app::getElapsedSeconds() ){}

const char* ReloadGlslProgEvent::getName() const
{
    return "reload_glslprogs";
}

ec::EventType ReloadGlslProgEvent::getEventType() const
{
    return ReloadGlslProgEvent::TYPE;
}


//DRAW DEBUG -------------------------------------------------------------/

ec::EventType DrawDebugEvent::TYPE = ec::getHash("draw_debug_event");

DrawDebugEventRef DrawDebugEvent::create()
{
    return DrawDebugEventRef( new DrawDebugEvent() );
}

DrawDebugEvent::DrawDebugEvent() : ec::EventData( cinder::app::getElapsedSeconds() ){}

const char* DrawDebugEvent::getName() const
{
    return "draw_debug_event";
}

ec::EventType DrawDebugEvent::getEventType() const
{
    return DrawDebugEvent::TYPE;
}


//UPDATE -------------------------------------------------------------/

ec::EventType CullEvent::TYPE = ec::getHash("cull_event");

CullEventRef CullEvent::create()
{
    return CullEventRef( new CullEvent() );
}

CullEvent::CullEvent() : ec::EventData( cinder::app::getElapsedSeconds() ){}

const char* CullEvent::getName() const
{
    return "cull_event";
}

ec::EventType CullEvent::getEventType() const
{
    return CullEvent::TYPE;
}

//UPDATE -------------------------------------------------------------/

ec::EventType UpdateEvent::TYPE = ec::getHash("update_event");

UpdateEventRef UpdateEvent::create()
{
    return UpdateEventRef( new UpdateEvent() );
}

UpdateEvent::UpdateEvent() : ec::EventData( cinder::app::getElapsedSeconds() ){}

const char* UpdateEvent::getName() const
{
    return "update_event";
}

ec::EventType UpdateEvent::getEventType() const
{
    return UpdateEvent::TYPE;
}

//DRAW TO MAIN BUFFER -------------------------------------------------------------/

ec::EventType DrawToMainBufferEvent::TYPE = ec::getHash("draw_to_main_buffer_event");

DrawToMainBufferEventRef DrawToMainBufferEvent::create()
{
    return DrawToMainBufferEventRef( new DrawToMainBufferEvent() );
}

DrawToMainBufferEvent::DrawToMainBufferEvent() : ec::EventData( cinder::app::getElapsedSeconds() ){}

const char* DrawToMainBufferEvent::getName() const
{
    return "draw_to_main_buffer_event";
}

ec::EventType DrawToMainBufferEvent::getEventType() const
{
    return DrawToMainBufferEvent::TYPE;
}

//DRAW SHADOW -------------------------------------------------------------/

ec::EventType DrawShadowEvent::TYPE = ec::getHash("draw_shadow_event");

DrawShadowEventRef DrawShadowEvent::create()
{
    return DrawShadowEventRef( new DrawShadowEvent() );
}

DrawShadowEvent::DrawShadowEvent() : ec::EventData( cinder::app::getElapsedSeconds() ){}

const char* DrawShadowEvent::getName() const
{
    return "draw_shadow_event";
}

ec::EventType DrawShadowEvent::getEventType() const
{
    return DrawShadowEvent::TYPE;
}

//SAVE SCENE -------------------------------------------------------------/

ec::EventType SaveSceneEvent::TYPE = ec::getHash("save_scene_event");

SaveSceneEventRef SaveSceneEvent::create()
{
    return SaveSceneEventRef( new SaveSceneEvent() );
}

SaveSceneEvent::SaveSceneEvent() : ec::EventData( cinder::app::getElapsedSeconds() ){}

const char* SaveSceneEvent::getName() const
{
    return "save_scene_event";
}

ec::EventType SaveSceneEvent::getEventType() const
{
    return SaveSceneEvent::TYPE;
}


//SWITCH CAMERA -------------------------------------------------------------/

ec::EventType SwitchCameraEvent::TYPE = ec::getHash("switch_camera_event");

SwitchCameraEventRef SwitchCameraEvent::create( const CameraComponent::CameraType& type )
{
    return SwitchCameraEventRef( new SwitchCameraEvent(type) );
}

SwitchCameraEvent::SwitchCameraEvent(const CameraComponent::CameraType& type) : ec::EventData( cinder::app::getElapsedSeconds() ), mType(type){}

const char* SwitchCameraEvent::getName() const
{
    return "switch_camera_event";
}

ec::EventType SwitchCameraEvent::getEventType() const
{
    return SwitchCameraEvent::TYPE;
}

//COMPONENT REGISTRATION EVENT -------------------------------------------------------------/

ec::EventType ComponentRegistrationEvent::TYPE = ec::getHash("component_registration_event");

ComponentRegistrationEventRef ComponentRegistrationEvent::create( const ComponentRegistrationEvent::RegistrationType& type, const ec::ActorUId& actor, ec::ComponentBase* component  )
{
    return ComponentRegistrationEventRef( new ComponentRegistrationEvent(type, actor, component) );
}

ComponentRegistrationEvent::ComponentRegistrationEvent(const ComponentRegistrationEvent::RegistrationType& type, const ec::ActorUId& actor, ec::ComponentBase* component  ) : ec::EventData( cinder::app::getElapsedSeconds() ), mType(type), mActor(actor), mComponent(component){}

const char* ComponentRegistrationEvent::getName() const
{
    return "component_registration_event";
}

ec::EventType ComponentRegistrationEvent::getEventType() const
{
    return ComponentRegistrationEvent::TYPE;
}


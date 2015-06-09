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

//DRAW DEBUG -------------------------------------------------------------/

ec::EventType DrawDeferredDebugEvent::TYPE = ec::getHash("draw_deferred_debug_event");

DrawDeferredDebugEventRef DrawDeferredDebugEvent::create()
{
    return DrawDeferredDebugEventRef( new DrawDeferredDebugEvent() );
}

DrawDeferredDebugEvent::DrawDeferredDebugEvent() : ec::EventData( cinder::app::getElapsedSeconds() ){}

const char* DrawDeferredDebugEvent::getName() const
{
    return "draw_deferred_debug_event";
}

ec::EventType DrawDeferredDebugEvent::getEventType() const
{
    return DrawDeferredDebugEvent::TYPE;
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


//DRAW TO MAIN BUFFER -------------------------------------------------------------/

ec::EventType DrawToRiftBufferEvent::TYPE = ec::getHash("draw_to_rift_buffer_event");

DrawToRiftBufferEventRef DrawToRiftBufferEvent::create(const Style& style, int eye )
{
    return DrawToRiftBufferEventRef( new DrawToRiftBufferEvent(style, eye) );
}

DrawToRiftBufferEvent::DrawToRiftBufferEvent(const Style& style, int eye ) : ec::EventData( cinder::app::getElapsedSeconds() ),mStyle(style), mEye(eye){}

const char* DrawToRiftBufferEvent::getName() const
{
    return "draw_to_rift_buffer_event";
}

ec::EventType DrawToRiftBufferEvent::getEventType() const
{
    return DrawToRiftBufferEvent::TYPE;
}

//DRAW EVENT -------------------------------------------------------------/

ec::EventType DrawEvent::TYPE = ec::getHash("draw__event");

DrawEventRef DrawEvent::create()
{
    return DrawEventRef( new DrawEvent() );
}

DrawEvent::DrawEvent() : ec::EventData( cinder::app::getElapsedSeconds() ){}

const char* DrawEvent::getName() const
{
    return "draw_event";
}

ec::EventType DrawEvent::getEventType() const
{
    return DrawEvent::TYPE;
}

//DRAW GEOMETRY -------------------------------------------------------------/

ec::EventType DrawGeometryEvent::TYPE = ec::getHash("draw_geometry_event");

DrawGeometryEventRef DrawGeometryEvent::create()
{
    return DrawGeometryEventRef( new DrawGeometryEvent() );
}

DrawGeometryEvent::DrawGeometryEvent() : ec::EventData( cinder::app::getElapsedSeconds() ){}

const char* DrawGeometryEvent::getName() const
{
    return "draw_geometry_event";
}

ec::EventType DrawGeometryEvent::getEventType() const
{
    return DrawGeometryEvent::TYPE;
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

ComponentRegistrationEventRef ComponentRegistrationEvent::create(  const Registration& reg, const ComponentRegistrationEvent::RegistrationType& type, const ec::ActorUId& actor, ec::ComponentBaseRef component  )
{
    return ComponentRegistrationEventRef( new ComponentRegistrationEvent(reg, type, actor, component) );
}

ComponentRegistrationEvent::ComponentRegistrationEvent( const Registration& reg, const ComponentRegistrationEvent::RegistrationType& type, const ec::ActorUId& actor, ec::ComponentBaseRef component  ) : ec::EventData( cinder::app::getElapsedSeconds() ),mReg(reg), mType(type), mActor(actor), mComponent(component){}

const char* ComponentRegistrationEvent::getName() const
{
    return "component_registration_event";
}

ec::EventType ComponentRegistrationEvent::getEventType() const
{
    return ComponentRegistrationEvent::TYPE;
}

//FINISH RENDER SCENE -------------------------------------------------------------/

ec::EventType FinishRenderEvent::TYPE = ec::getHash("finish_render_event");

FinishRenderEventRef FinishRenderEvent::create(const ci::gl::Texture2dRef &final)
{
    return FinishRenderEventRef( new FinishRenderEvent(final) );
}

FinishRenderEvent::FinishRenderEvent(const ci::gl::Texture2dRef &final) : ec::EventData( cinder::app::getElapsedSeconds() ), mFinalTexture(final){}

const char* FinishRenderEvent::getName() const
{
    return "finish_render_event";
}

ec::EventType FinishRenderEvent::getEventType() const
{
    return FinishRenderEvent::TYPE;
}

//SHARE DEPTH TEXTURE -------------------------------------------------------------/

ec::EventType ShareGeometryDepthTextureEvent::TYPE = ec::getHash("share_geometry_depth_texture_event");

ShareGeometryDepthTextureEventRef ShareGeometryDepthTextureEvent::create(const ci::gl::Texture2dRef &depth)
{
    return ShareGeometryDepthTextureEventRef( new ShareGeometryDepthTextureEvent(depth) );
}

ShareGeometryDepthTextureEvent::ShareGeometryDepthTextureEvent(const ci::gl::Texture2dRef &depth) : ec::EventData( cinder::app::getElapsedSeconds() ), mDepthTexture(depth){}

const char* ShareGeometryDepthTextureEvent::getName() const
{
    return "share_geometry_depth_texture_event";
}

ec::EventType ShareGeometryDepthTextureEvent::getEventType() const
{
    return ShareGeometryDepthTextureEvent::TYPE;
}

//MIC VOLUME -------------------------------------------------------------/

ec::EventType MicVolumeEvent::TYPE = ec::getHash("mic_volume_event");

MicVolumeEventRef MicVolumeEvent::create(const float &vol)
{
    return MicVolumeEventRef( new MicVolumeEvent(vol) );
}

MicVolumeEvent::MicVolumeEvent(const float &vol) : ec::EventData( cinder::app::getElapsedSeconds() ), mVolume(vol){}

const char* MicVolumeEvent::getName() const
{
    return "mic_volume_event";
}

ec::EventType MicVolumeEvent::getEventType() const
{
    return MicVolumeEvent::TYPE;
}

//ADVANCE EVENT -------------------------------------------------------------/

ec::EventType AdvanceEvent::TYPE = ec::getHash("advance_event");

AdvanceEventRef AdvanceEvent::create()
{
    return AdvanceEventRef( new AdvanceEvent() );
}

AdvanceEvent::AdvanceEvent() : ec::EventData( cinder::app::getElapsedSeconds() ){}

const char* AdvanceEvent::getName() const
{
    return "advance_event";
}

ec::EventType AdvanceEvent::getEventType() const
{
    return AdvanceEvent::TYPE;
}


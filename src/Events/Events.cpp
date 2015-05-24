//
//  Events.cpp
//  System_test
//
//  Created by Mike Allison on 4/28/15.
//
//

#include "Events.h"
#include "Common.h"


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

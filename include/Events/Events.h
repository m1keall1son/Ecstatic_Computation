//
//  Events.h
//  System_test
//
//  Created by Mike Allison on 4/28/15.
//
//

#pragma once

#include "BaseEventData.h"
#include "AppCommon.h"
    
class DrawDebugEvent : public ec::EventData {
    public:
    
    static ec::EventType TYPE;
    
    static DrawDebugEventRef create();
    
    ~DrawDebugEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
    private:
        DrawDebugEvent();
};

class CullEvent : public ec::EventData {
public:
    
    static ec::EventType TYPE;
    
    static CullEventRef create();
    
    ~CullEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
private:
    CullEvent();
};


class UpdateEvent : public ec::EventData {
public:
    
    static ec::EventType TYPE;
    
    static UpdateEventRef create();
    
    ~UpdateEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
private:
    UpdateEvent();
};


class DrawToMainBufferEvent : public ec::EventData {
public:
    
    static ec::EventType TYPE;
    
    static DrawToMainBufferEventRef create();
    
    ~DrawToMainBufferEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
private:
    DrawToMainBufferEvent();
};

class DrawShadowEvent : public ec::EventData {
public:
    
    static ec::EventType TYPE;
    
    static DrawShadowEventRef create();
    
    ~DrawShadowEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
private:
    DrawShadowEvent();
};


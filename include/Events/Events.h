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
#include "CameraManager.h"

class ReloadGlslProgEvent : public ec::EventData {
public:
    
    static ec::EventType TYPE;
    
    static ReloadGlslProgEventRef create();
    
    ~ReloadGlslProgEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
private:
    ReloadGlslProgEvent();
};

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

class DrawDeferredDebugEvent : public ec::EventData {
public:
    
    static ec::EventType TYPE;
    
    static DrawDeferredDebugEventRef create();
    
    ~DrawDeferredDebugEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
private:
    DrawDeferredDebugEvent();
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

class DrawToRiftBufferEvent : public ec::EventData {
public:
    
    enum Style { TWICE, STEREO };
    
    static ec::EventType TYPE;
    
    static DrawToRiftBufferEventRef create( const Style& style, int eye = 0 );
    
    ~DrawToRiftBufferEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
    inline Style getStyle(){ return mStyle; }
    inline int getEye(){ return mEye; }
    
private:
    DrawToRiftBufferEvent(const Style& style, int eye );
    Style mStyle;
    int mEye;
};



class DrawEvent : public ec::EventData {
public:
    
    static ec::EventType TYPE;
    
    static DrawEventRef create();
    
    ~DrawEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
private:
    DrawEvent();
};

class DrawGeometryEvent : public ec::EventData {
public:
    
    static ec::EventType TYPE;
    
    static DrawGeometryEventRef create();
    
    ~DrawGeometryEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
private:
    DrawGeometryEvent();
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


class SaveSceneEvent : public ec::EventData {
public:
    
    static ec::EventType TYPE;
    
    static SaveSceneEventRef create();
    
    ~SaveSceneEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
private:
    SaveSceneEvent();
};


class SwitchCameraEvent : public ec::EventData {
public:
    
    static ec::EventType TYPE;
    
    static SwitchCameraEventRef create( const CameraComponent::CameraType &type );
    
    ~SwitchCameraEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
    inline CameraComponent::CameraType& getType(){ return mType; }
    
private:
    SwitchCameraEvent( const CameraComponent::CameraType &type  );
    CameraComponent::CameraType mType;
};

class ComponentRegistrationEvent : public ec::EventData {
public:
    
    enum RegistrationType { CAMERA, LIGHT, PASS, DEBUG_COMPONENT };
    enum Registration { REGISTER, UNREGISTER };

    static ec::EventType TYPE;
    
    ///TODO: this is the only unsafe pplace for components
    
    static ComponentRegistrationEventRef create( const Registration& reg, const RegistrationType &type, const ec::ActorUId& actor, ec::ComponentBaseRef component );
    
    ~ComponentRegistrationEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
    inline RegistrationType& getType(){ return mType; }
    inline ec::ActorUId& getActorUId(){ return mActor; }
    inline ec::ComponentBaseRef getComponentBase(){ return mComponent; }
    inline Registration getRegistration(){ return mReg; }

private:
    ComponentRegistrationEvent(  const Registration& reg, const RegistrationType &type, const ec::ActorUId& actor, ec::ComponentBaseRef component );
    RegistrationType mType;
    ec::ActorUId mActor;
    ec::ComponentBaseRef mComponent;
    Registration mReg;
};

class FinishRenderEvent : public ec::EventData {
public:
    
    static ec::EventType TYPE;
    
    static FinishRenderEventRef create( const ci::gl::Texture2dRef &final );
    
    ~FinishRenderEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}

    inline ci::gl::Texture2dRef getFinalTexture(){ return mFinalTexture; }
    
private:
    FinishRenderEvent(const ci::gl::Texture2dRef &final);
    ci::gl::Texture2dRef mFinalTexture;
};

class ShareGeometryDepthTextureEvent : public ec::EventData {
public:
    
    static ec::EventType TYPE;
    
    static ShareGeometryDepthTextureEventRef create( const ci::gl::Texture2dRef &depth );
    
    ~ShareGeometryDepthTextureEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
    inline ci::gl::Texture2dRef getDepthTexture(){ return mDepthTexture; }
    
private:
    ShareGeometryDepthTextureEvent(const ci::gl::Texture2dRef &final);
    ci::gl::Texture2dRef mDepthTexture;
};


class MicVolumeEvent : public ec::EventData {
public:
    
    static ec::EventType TYPE;
    
    static MicVolumeEventRef create( const float &vol );
    
    ~MicVolumeEvent(){}
    ec::EventDataRef copy(){ return ec::EventDataRef(); }
    const char* getName() const;
    ec::EventType getEventType() const;
    
    void serialize( ci::Buffer &streamOut ){}
    void deSerialize( const ci::Buffer &streamIn ){}
    
    inline float getVolume(){ return mVolume; }
    
private:
    MicVolumeEvent(const float &vol);
    float mVolume;
};

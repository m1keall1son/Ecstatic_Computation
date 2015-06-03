//
//  RoomParticlesComponent.h
//  System_test
//
//  Created by Mike Allison on 5/23/15.
//
//

#pragma once
#include "Common.h"
#include "ComponentFactory.h"
#include "ComponentBase.h"
#include "AppCommon.h"
#include "ParticleSystemComponent.h"

class RoomParticlesComponent : public ec::ComponentBase {
    
public:
    
    enum Style { Noise, Seek };
    
    static ec::ComponentType TYPE;
    
    static RoomParticlesComponentRef create( ec::Actor* context );
    
    bool                          initialize( const ci::JsonTree &tree )override;
    ci::JsonTree                  serialize()override;
    const ec::ComponentNameType   getName() const override;
    const ec::ComponentUId        getId() const override;
    const ec::ComponentType       getType() const override;
    void                          loadGUI( const ci::params::InterfaceGlRef &gui )override;
    void                          cleanup()override;
    bool                          postInit()override;
    void                          drawTF( ec::EventDataRef );

    ~RoomParticlesComponent();
    
private:
    
    RoomParticlesComponent( ec::Actor* context );
    
    
    void reloadGlslProg();
    void initTF();
    
    void handleShutDown( ec::EventDataRef );
    void handleSceneChange( ec::EventDataRef );
    
    void registerHandlers();
    void unregisterHandlers();
    
    ec::ComponentUId    mId;
    bool                mShuttingDown;
    
    Style               mStyle;
    
    GLenum                          mPrimitive;
    int                             mMaxParticles;
    int                             mCurrent;
    
    ci::gl::VaoRef                  mVaos[2];
    ci::gl::TransformFeedbackObjRef mTF[2];
    
    ci::gl::GlslProgRef mUpdateSeek;
    ci::gl::GlslProgRef mUpdateNoise, mRender;
    
    ci::gl::VboRef      mPositions[2];
    ci::gl::VboRef      mData[2];
    
    ci::Colorf          mColor;
    
};
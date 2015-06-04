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
#include "cinder/gl/BufferTexture.h"
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
    void                          drawRift( ec::EventDataRef );
    void                          drawShadows( ec::EventDataRef );
    void                          sampleTexture( bool sample = true ){ mSampleTexture = true; }
    void                          draw(ec::EventDataRef event);

    inline ci::gl::BufferTextureRef getPositions(){ return mPositionsTexture[mCurrent]; }
    
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
    bool                            mSampleTexture;
    
    ci::gl::VaoRef                  mVaos[2];
    ci::gl::BufferTextureRef        mPositionsTexture[2], mVelocitiesTexture[2];
    ci::gl::TransformFeedbackObjRef mTF[2];
    
    ci::gl::BatchRef    mMesh, mMeshShadow;
    ci::gl::GlslProgRef mMeshRender,mShadowRender;

    ci::gl::GlslProgRef mUpdateNoise, mFillBuffer;
    
    ci::gl::VboRef      mPositions[2];
    ci::gl::VboRef      mVelocities[2];
    ci::gl::VboRef      mTexCoords;

    ci::Colorf          mColor;
    
    ci::vec3            mTarget;
    float               mDec;
};
//
//  ParticleSystemComponent.h
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
#include "cinder/gl/TransformFeedbackObj.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Vbo.h"

class ParticleSystemComponent : public ec::ComponentBase {
    
public:
    
    
    virtual bool                          initialize( const ci::JsonTree &tree )override;
    virtual ci::JsonTree                  serialize()override;
    virtual void                          cleanup()override;
    
    virtual bool                          postInit()override;
    
    virtual void                          draw( ec::EventDataRef );
    virtual void                          update( ec::EventDataRef );
    virtual void                          drawTF( ec::EventDataRef );
    
    virtual ~ParticleSystemComponent();
    
protected:
    
    ParticleSystemComponent( ec::Actor* context );
    
    virtual void handleShutDown( ec::EventDataRef );
    virtual void handleSceneChange( ec::EventDataRef );
    
    virtual void registerHandlers();
    virtual void unregisterHandlers();
    
    ec::ComponentUId mId;
    bool             mShuttingDown;
    
    GLenum                          mPrimitive;
    int                             mMaxParticles;
    int                             mCurrent;
    ci::gl::VaoRef                  mVaos[2];
    ci::gl::TransformFeedbackObjRef mTF[2];

    
    
};
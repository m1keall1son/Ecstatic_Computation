//
//  ParticleSystemComponent.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#include "ParticleSystemComponent.h"
#include "Actor.h"
#include "Scene.h"
#include "Controller.h"

using namespace ci;
using namespace ci::app;

void ParticleSystemComponent::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void ParticleSystemComponent::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void ParticleSystemComponent::registerHandlers()
{
}
void ParticleSystemComponent::unregisterHandlers()
{
}

bool ParticleSystemComponent::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    return true;
}

void ParticleSystemComponent::cleanup()
{
    unregisterHandlers();
}

bool ParticleSystemComponent::postInit()
{
    if(!mInitialized){
        
        mInitialized = true;
    }
    
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

ParticleSystemComponent::ParticleSystemComponent( ec::Actor* context ): ec::ComponentBase( context ),mShuttingDown(false),mPrimitive(GL_POINTS),mMaxParticles(0), mCurrent(1)
{
    
    registerHandlers();
    
}

ParticleSystemComponent::~ParticleSystemComponent()
{
}


ci::JsonTree ParticleSystemComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    
    return save;
    
}

void ParticleSystemComponent::update( ec::EventDataRef )
{
    
    mCurrent = 1 - mCurrent;
    
    ci::gl::ScopedVao		vaoScope( mVaos[mCurrent] );
    ci::gl::ScopedState		stateScope( GL_RASTERIZER_DISCARD, true );
    
    mTF[1 - mCurrent]->bind();
    ci::gl::beginTransformFeedback( GL_POINTS );
    ci::gl::drawArrays( GL_POINTS, 0, mMaxParticles );
    ci::gl::endTransformFeedback();
    mTF[1 - mCurrent]->unbind();
    
}

void ParticleSystemComponent::drawTF( ec::EventDataRef )
{
    
    ci::gl::ScopedVao		vaoScope( mVaos[mCurrent] );
    
    mTF[1 - mCurrent]->bind();
    ci::gl::beginTransformFeedback( GL_POINTS );
    ci::gl::drawArrays( GL_POINTS, 0, mMaxParticles );
    ci::gl::endTransformFeedback();
    mTF[1 - mCurrent]->unbind();
    
    mCurrent = 1 - mCurrent;
    
}

void ParticleSystemComponent::draw( ec::EventDataRef )
{
    ci::gl::ScopedVao vao( mVaos[mCurrent] );
    ci::gl::drawArrays( GL_POINTS, 0, mMaxParticles );
}



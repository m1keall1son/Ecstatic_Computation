//
//  ShadowMap.cpp
//  System_test
//
//  Created by Mike Allison on 5/23/15.
//
//

#include "ShadowMap.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"

using namespace ci;
using namespace ci::app;

ShadowMap::~ShadowMap()
{
}

ShadowMapRef ShadowMap::create( int size )
{
    return ShadowMapRef( new ShadowMap{ size } );
}

ShadowMap::ShadowMap( int size )
{
    reset( size );
}

void ShadowMap::reset( int size )
{
    gl::Texture2d::Format depthFormat;
    depthFormat.setInternalFormat( GL_DEPTH_COMPONENT32F );
    depthFormat.setMagFilter( GL_LINEAR );
    depthFormat.setMinFilter( GL_LINEAR );
    depthFormat.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
    depthFormat.setCompareMode( GL_COMPARE_REF_TO_TEXTURE );
    depthFormat.setCompareFunc( GL_LEQUAL );
    mTextureShadowMap = gl::Texture2d::create( size, size, depthFormat );
    
    gl::Fbo::Format fboFormat;
    fboFormat.attachment( GL_DEPTH_ATTACHMENT, mTextureShadowMap );
    fboFormat.disableColor();
    mShadowMap = gl::Fbo::create( size, size, fboFormat );
}

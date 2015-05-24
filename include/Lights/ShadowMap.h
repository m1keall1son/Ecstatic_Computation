//
//  ShadowMap.h
//  System_test
//
//  Created by Mike Allison on 5/23/15.
//
//

#pragma once 

#include "Common.h"
#include "AppCommon.h"

class ShadowMap {
public:
    ~ShadowMap();
    
    static ShadowMapRef create( int size );
    
    ShadowMap( int size );
    void reset( int size );
    
    inline const ci::gl::FboRef&		getFbo() const { return mShadowMap; }
    inline const ci::gl::Texture2dRef&	getTexture() const { return mTextureShadowMap; }
    
    inline float                        getAspectRatio() const { return mShadowMap->getAspectRatio(); }
    inline ci::ivec2                    getSize() const { return mShadowMap->getSize(); }
    
private:
    ci::gl::FboRef          mShadowMap;
    ci::gl::Texture2dRef	mTextureShadowMap;
};
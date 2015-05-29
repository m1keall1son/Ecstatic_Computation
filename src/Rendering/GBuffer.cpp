//
//  GBuffer.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#include "GBuffer.h"

using namespace ci;
using namespace ci::app;

GBufferRef GBuffer::create(const GBuffer::Format &format){
    return GBufferRef( new GBuffer(format) );
}


GBuffer::GBuffer( const Format& format ):mFormat(format){ resize(); }

void GBuffer::resize(){
    
    int width = getWindowWidth();
    int height = getWindowHeight();
    
    gl::Fbo::Format fboFormat;
    
    for( Attachment a : mFormat.mAttachments ){
        
        gl::TextureRef texture = gl::Texture::create( width, height, a.getFormat() );
        mAttachments.push_back( texture );
        fboFormat.attachment( a.getLocation(), mAttachments.back() );
    }
    
    if(mFormat.mDepthTexture){
        mDepthTexture = mFormat.mDepthTexture;
        fboFormat.attachment( GL_DEPTH_ATTACHMENT, mDepthTexture);
        
    }else{
        mDepthTexture = gl::Texture2d::create(width,height,mFormat.mDepthFormat);
        fboFormat.attachment( GL_DEPTH_ATTACHMENT, mDepthTexture );
    }
    
    mGBuffer = gl::Fbo::create( width, height, fboFormat );
    
}

ci::gl::TextureRef GBuffer::getTexture( const std::string& label ){
    
    ci::gl::Texture2dRef tex;
    
    auto it = mAttachments.begin();
    auto end = mAttachments.end();
    while( it!=end ){
        if( (*it)->getLabel() == label ){
            tex = *it;
            break;
        }
        ++it;
    }
    
    if(!tex){
        CI_LOG_E("couldn't find texture by the name: "+label);
    }
    return tex;
}

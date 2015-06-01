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

GBufferRef GBuffer::create( const ci::vec2 &size, const GBuffer::Format &format){
    return GBufferRef( new GBuffer(size, format) );
}


GBuffer::GBuffer( const ci::vec2 &size, const Format& format ):mFormat(format),mSize(size){ resize(); }

void GBuffer::resize(){
    
    gl::Fbo::Format fboFormat;
    
    for( Attachment a : mFormat.mAttachments ){
        
        gl::TextureRef texture = gl::Texture::create( mSize.x, mSize.y, a.getFormat() );
        mAttachments.push_back( texture );
        fboFormat.attachment( a.getLocation(), mAttachments.back() );
    }
    
    if(mFormat.mDepthTexture){
        mDepthTexture = mFormat.mDepthTexture;
        fboFormat.attachment( GL_DEPTH_ATTACHMENT, mDepthTexture);
        
    }else{
        fboFormat.depthTexture( mFormat.mDepthFormat );
    }
    
    mGBuffer = gl::Fbo::create( mSize.x, mSize.y, fboFormat );
    
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

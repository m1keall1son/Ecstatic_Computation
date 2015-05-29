//
//  GBuffer.h
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#pragma once
#include "Common.h"
#include "AppCommon.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Log.h"

class GBuffer {
    
public:
    
    struct Attachment {
    public:
        
        Attachment(const GLint& loc = GL_COLOR_ATTACHMENT0, const ci::gl::Texture2d::Format& tex_fmt = ci::gl::Texture2d::Format(), const std::string& label = "" ) : mLocation(loc), mFormat(tex_fmt){ if(label != "") mFormat.setLabel(label); else CI_LOG_E("ATTACHMNET NEEDS LABEL!"); }
        
        Attachment& textureFormat( const ci::gl::Texture2d::Format& fmt ){ std::string label = mFormat.getLabel(); mFormat = fmt; mFormat.setLabel(label); return *this; }
        Attachment& label( const std::string& label ){ mFormat.setLabel(label); return *this; }
        Attachment& location( const int& location ){ mLocation = location; return *this; }
        
        const GLint& getLocation() { return mLocation; }
        const std::string& getLabel() { return mFormat.getLabel(); }
        ci::gl::Texture2d::Format& getFormat() { return mFormat; }

    protected:
        GLint mLocation;
        ci::gl::Texture2d::Format mFormat;
    };
    
    struct Format {
        
    public:
        
        Format(){
            
            mAttachments.clear();
            
        }
        
        Format& attachment( const GLint& loc, const ci::gl::Texture2d::Format& tex_fmt, const std::string& label ){ mAttachments.push_back( Attachment( loc, tex_fmt, label ) ); return *this; }
        Format& depthTexture( const ci::gl::Texture2dRef& depth_tex ){ mDepthTexture = depth_tex; return *this; }
        Format& depthTexture( const ci::gl::Texture2d::Format& depth_tex_fmt ){ mDepthFormat = depth_tex_fmt; return *this; }

        std::list<Attachment> mAttachments;
        ci::gl::Texture2dRef mDepthTexture;
        ci::gl::Texture2d::Format mDepthFormat;
        
    };
    
    static GBufferRef create( const Format& format );
    void resize();
    
    ci::gl::Texture2dRef getTexture( const std::string& label );
    inline ci::gl::Texture2dRef getDepthTexture(){ return mFormat.mDepthTexture; }
    inline ci::gl::FboRef getFbo(){ return mGBuffer; }
    
private:
    
    GBuffer( const Format& format );
    
    ci::gl::FboRef mGBuffer;
    std::list<ci::gl::Texture2dRef> mAttachments;
    ci::gl::Texture2dRef mDepthTexture;
    Format mFormat;
    
};
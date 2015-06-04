//
//  RoomParticlesComponent.cpp
//  Kinect_Component_test
//
//  Created by Mike Allison on 5/29/15.
//
//

#include "RoomParticlesComponent.h"
#include "Actor.h"
#include "Scene.h"
#include "Controller.h"
#include "AppSceneBase.h"
#include "Events.h"
#include "RoomComponent.h"
#include "cinder/Rand.h"
#include "cinder/Perlin.h"
#include "ActorManager.h"
#include "KinectComponent.h"

using namespace ci;
using namespace ci::app;

const int POSITION_LOC = 0;
const int DATA_LOC = 1;
const int TEXCOORD_LOC = 2;

ec::ComponentType RoomParticlesComponent::TYPE = ec::getHash("Room_particles_component");

RoomParticlesComponentRef RoomParticlesComponent::create( ec::Actor* context )
{
    return RoomParticlesComponentRef( new RoomParticlesComponent(context) );
}

void RoomParticlesComponent::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}
void RoomParticlesComponent::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void RoomParticlesComponent::registerHandlers()
{
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &RoomParticlesComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener( fastdelegate::MakeDelegate( this, &RoomParticlesComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    
}
void RoomParticlesComponent::unregisterHandlers()
{
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &RoomParticlesComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &RoomParticlesComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    
}

bool RoomParticlesComponent::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    
    try {
        mMaxParticles = tree["max_particles"].getValue<int>();
    } catch (ci::JsonTree::ExcChildNotFound e) {
        CI_LOG_W("max particles not provided defaulting 100");
        mMaxParticles = 640*480/2.;
    }
    
    try {
        auto primitive = tree["gl_primitive"].getValue();
        
        if( primitive == "GL_TRIANGLES" )mPrimitive = GL_TRIANGLES;
        else if( primitive == "GL_POINTS" )mPrimitive = GL_POINTS;
        
    } catch (ci::JsonTree::ExcChildNotFound e) {
        CI_LOG_W("primitive not found, defualting to points");
        mPrimitive = GL_POINTS;
    }
    
    try {
        auto axis = tree["color"].getChildren();
        auto end = axis.end();
        ci::vec3 final;
        int i = 0;
        for( auto it = axis.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        mColor = Colorf( final.x, final.y, final.z );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find axis, not setting");
    }
    
    return true;
}

void RoomParticlesComponent::cleanup()
{
    unregisterHandlers();
}

void RoomParticlesComponent::reloadGlslProg()
{
    try {
        std::vector<std::string> transformFeedbackVaryings;
        ci::gl::GlslProg::Format updateFmt;
        
        updateFmt.vertex( loadAsset("shaders/room_particles_update_noise.vert") );
        
        transformFeedbackVaryings.push_back( "outPosition" );
        transformFeedbackVaryings.push_back( "outVelocity" );
        
        updateFmt.attribLocation("inPosition", POSITION_LOC);
        updateFmt.attribLocation("inVelocity", DATA_LOC);
        updateFmt.attribLocation("inTexCoord", TEXCOORD_LOC);
        
        updateFmt.feedbackFormat( GL_SEPARATE_ATTRIBS );
        updateFmt.feedbackVaryings( transformFeedbackVaryings ).preprocess(true);
        
        mUpdateNoise = ci::gl::GlslProg::create( updateFmt );
        
    }
    catch( ci::gl::GlslProgCompileExc e ) {
        CI_LOG_E( "Room particles update " + std::string(e.what()) );
    }
    
    try {
        std::vector<std::string> transformFeedbackVaryings;
        ci::gl::GlslProg::Format updateFmt;
        
        updateFmt.vertex( loadAsset("shaders/room_particles_update_fill.vert") );
        
        transformFeedbackVaryings.push_back( "outPosition" );
        transformFeedbackVaryings.push_back( "outVelocity" );
        
        updateFmt.attribLocation("inPosition", POSITION_LOC);
        updateFmt.attribLocation("inVelocity", DATA_LOC);
        updateFmt.attribLocation("inTexCoord", TEXCOORD_LOC);
        
        updateFmt.feedbackFormat( GL_SEPARATE_ATTRIBS );
        updateFmt.feedbackVaryings( transformFeedbackVaryings ).preprocess(true);
        
        mFillBuffer = ci::gl::GlslProg::create( updateFmt );
        
    }
    catch( ci::gl::GlslProgCompileExc e ) {
        CI_LOG_E( "Room particles fill buffer" + std::string(e.what()) );
    }
    
    try{//
        mMeshRender = ci::gl::GlslProg::create( ci::gl::GlslProg::Format().vertex(loadAsset("shaders/particle_mesh.vert")).geometry(loadAsset("shaders/particle_mesh.geom")).fragment(loadAsset("shaders/particle_mesh.frag")).preprocess(true) );
        
    }catch( ci::gl::GlslNullProgramExc e ){
        CI_LOG_E("kinect particles mesh render" + std::string(e.what()));
    }
    
    try{
        mShadowRender = ci::gl::GlslProg::create( ci::gl::GlslProg::Format().vertex(loadAsset("shaders/particle_mesh_shadow.vert")).geometry(loadAsset("shaders/particle_mesh_shadow.geom")).fragment(loadAsset("shaders/particle_mesh_shadow.frag")).preprocess(true) );
        
    }catch( ci::gl::GlslNullProgramExc e ){
        CI_LOG_E("kinect particles mesh render" + std::string(e.what()));
    }
    
    auto kinect_comp = mContext->getComponent<KinectComponent>().lock();
    auto kinect = kinect_comp->getKinect();
    
    mFillBuffer->uniform( "uRef_pix_size", kinect->getZeroPlanePixelSize() );
    mFillBuffer->uniform( "uRef_distance", kinect->getZeroPlaneDistance() );
    mFillBuffer->uniform( "uConst_shift", kinect->getRegistrationConstShift() );
    mFillBuffer->uniform( "uDcmos_emitter_dist", kinect->getDcmosEmitterDist() );
    
    mMeshRender->uniformBlock("uLights", 0);
    mMeshRender->uniform("uShadowMap", 3);
    mMeshRender->uniform("uPositions", 7);
    mMeshRender->uniform("uColorMap", 8);
    mMeshRender->uniform("uVelocities", 9);

    mMeshRender->uniform( "uThresholds", kinect_comp->getThresholds() );
    mMeshRender->uniform( "uTriangleCutoff", kinect_comp->getTriagleCutoff() );
    
    mShadowRender->uniform( "uThresholds", kinect_comp->getThresholds() );
    mShadowRender->uniform( "uTriangleCutoff", kinect_comp->getTriagleCutoff() );
    
    mShadowRender->uniform("uPositions", 7);
    mShadowRender->uniform("uVelocities", 9);

    if(mMesh)mMesh->replaceGlslProg(mMeshRender);
    if(mMeshShadow)mMesh->replaceGlslProg(mShadowRender);

}

void RoomParticlesComponent::initTF()
{
    
    std::vector<vec4> positions;
    std::vector<vec4> velocities;
    std::vector<vec2> texcoords;
    for( int i = 0; i < mMaxParticles; i++ ){
        positions.push_back( vec4(0,0,0,randFloat(20,200)) );
        velocities.push_back(vec4(0));
    
    }
    
    auto mesh = TriMesh(geom::Plane().size(vec2(640,480)).origin(vec3(getWindowCenter(),0)).subdivisions(vec2(640,480)/1.5f));
    
    int num = mesh.getNumVertices();
    
    mTexCoords = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec2)*num, mesh.getTexCoords0<2>(), GL_STATIC_DRAW );

    mPositions[0] = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec4)*positions.size(), positions.data(), GL_STATIC_DRAW );
    mPositions[1] = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec4)*positions.size(), nullptr, GL_STATIC_DRAW );
    
    mVelocities[0] = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec4)*velocities.size(), velocities.data(), GL_STATIC_DRAW );
    mVelocities[1] = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec4)*velocities.size(), nullptr, GL_STATIC_DRAW );
    
    mPositionsTexture[0] = gl::BufferTexture::create(mPositions[0], GL_RGBA32F);
    mPositionsTexture[1] = gl::BufferTexture::create(mPositions[1], GL_RGBA32F);
    
    mVelocitiesTexture[0] = gl::BufferTexture::create(mVelocities[0], GL_RGBA32F);
    mVelocitiesTexture[1] = gl::BufferTexture::create(mVelocities[1], GL_RGBA32F);
    
    for( int i  = 0; i < 2; i++ ){
        
        mVaos[i] = gl::Vao::create();
        mVaos[i]->bind();
        
        mPositions[i]->bind();
        gl::vertexAttribPointer(POSITION_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);
        gl::enableVertexAttribArray(POSITION_LOC);
        
        mVelocities[i]->bind();
        gl::vertexAttribPointer(DATA_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);
        gl::enableVertexAttribArray(DATA_LOC);
        
        mTexCoords->bind();
        gl::vertexAttribPointer(TEXCOORD_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);
        gl::enableVertexAttribArray(TEXCOORD_LOC);
        
        mTF[i] = gl::TransformFeedbackObj::create();
        
        mTF[i]->bind();
        gl::bindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, POSITION_LOC, mPositions[i]);
        gl::bindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, DATA_LOC, mVelocities[i]);
        mTF[i]->unbind();
        
    }
    
    CI_CHECK_GL();
    
}

bool RoomParticlesComponent::postInit()
{
    if(!mInitialized){
        
        reloadGlslProg();
        initTF();
        
        {
            auto tmesh = TriMesh(geom::Plane().size(vec2(640,480)).origin(vec3(getWindowCenter(),0)).subdivisions(vec2(640,480)/1.5f), TriMesh::formatFromSource(geom::Plane().size(vec2(640,480)).origin(vec3(getWindowCenter(),0)).subdivisions(vec2(640,480)/1.5f)));
            
            auto positions = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec3)*tmesh.getNumVertices(), tmesh.getPositions<3>(), GL_STATIC_DRAW);
            auto texcoords = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec2)*tmesh.getNumVertices(), tmesh.getTexCoords0<2>(), GL_STATIC_DRAW);
            
            std::vector<std::pair<geom::BufferLayout, ci::gl::VboRef> > vertexArrayBuffers;
            
            geom::BufferLayout position_layout;
            position_layout.append(geom::Attrib::POSITION, 4, 0, 0 );
            
            geom::BufferLayout tc_layout;
            tc_layout.append(geom::Attrib::TEX_COORD_0, 2, 0, 0 );
            
            vertexArrayBuffers.push_back( std::make_pair(position_layout, positions) );
            vertexArrayBuffers.push_back( std::make_pair(tc_layout, texcoords) );
            
            auto mesh = gl::VboMesh::create(tmesh.getNumVertices(), GL_POINTS, vertexArrayBuffers);
            CI_CHECK_GL();
            
            mMesh = gl::Batch::create( mesh, mMeshRender );
            CI_CHECK_GL();
        }
        {
            auto tmesh = TriMesh(geom::Plane().size(vec2(640,480)).origin(vec3(getWindowCenter(),0)).subdivisions(vec2(640,480)/1.5f), TriMesh::formatFromSource(geom::Plane().size(vec2(640,480)).origin(vec3(getWindowCenter(),0)).subdivisions(vec2(640,480)/1.5f)));
            
            auto positions = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec3)*tmesh.getNumVertices(), tmesh.getPositions<3>(), GL_STATIC_DRAW);
            auto texcoords = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec2)*tmesh.getNumVertices(), tmesh.getTexCoords0<2>(), GL_STATIC_DRAW);
            
            std::vector<std::pair<geom::BufferLayout, ci::gl::VboRef> > vertexArrayBuffers;
            
            geom::BufferLayout position_layout;
            position_layout.append(geom::Attrib::POSITION, 4, 0, 0 );
            
            geom::BufferLayout tc_layout;
            tc_layout.append(geom::Attrib::TEX_COORD_0, 2, 0, 0 );
            
            vertexArrayBuffers.push_back( std::make_pair(position_layout, positions) );
            vertexArrayBuffers.push_back( std::make_pair(tc_layout, texcoords) );
            
            auto mesh = gl::VboMesh::create(tmesh.getNumVertices(), GL_POINTS, vertexArrayBuffers);
            CI_CHECK_GL();
            
            mMeshShadow = gl::Batch::create( mesh, mShadowRender );
            CI_CHECK_GL();
        }        CI_CHECK_GL();
        
 
        mInitialized = true;
    }
    
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

RoomParticlesComponent::RoomParticlesComponent( ec::Actor* context ): ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "_Room_particles_component" ) ),mShuttingDown(false), mStyle(Noise),mCurrent(1),mSampleTexture(true),mDec(10)
{
    
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

RoomParticlesComponent::~RoomParticlesComponent()
{
}

const ec::ComponentNameType RoomParticlesComponent::getName() const
{
    return "Room_particles_component";
}

const ec::ComponentUId RoomParticlesComponent::getId() const
{
    return mId;
}

const ec::ComponentType RoomParticlesComponent::getType() const
{
    return TYPE;
}

ci::JsonTree RoomParticlesComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    save.addChild( ci::JsonTree( "max_particls", mMaxParticles ) );
    if( mPrimitive == GL_TRIANGLES )
        save.addChild( ci::JsonTree( "gl_primitive", "GL_TRIANGLES" ) );
    else if( mPrimitive == GL_POINTS )
        save.addChild( ci::JsonTree( "gl_primitive", "GL_POINTS" ) );
    
    auto color = ci::JsonTree::makeArray( "color" );
    for( int i = 0; i<3; i++ ){
        color.addChild( JsonTree( "", mColor[i] ) );
    }
    save.addChild(color);
    
    return save;
    
}

void RoomParticlesComponent::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
    gui->addParam("color", &mColor);
    gui->addButton("reload glsl", std::bind(&RoomParticlesComponent::reloadGlslProg, this));
}

void RoomParticlesComponent::drawTF(ec::EventDataRef event )
{
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" drawTF");
    {
        
        ci::gl::Texture2dRef kTex;
        
        if(mSampleTexture){
            auto c = mContext->getComponent<KinectComponent>().lock();
            kTex = c->getKinectTexture();
            gl::ScopedTextureBind tex(kTex,14);
            mFillBuffer->uniform( "uDepthTexture", 14);
            mFillBuffer->uniform( "uThresholds", c->getThresholds() );

            gl::ScopedGlslProg glsl(mFillBuffer);
            gl::setDefaultShaderVars();
            
            mCurrent = 1 - mCurrent;
            
            ci::gl::ScopedVao		vaoScope( mVaos[mCurrent] );
            gl::ScopedState		stateScope( GL_RASTERIZER_DISCARD, true );
            
            mTF[1 - mCurrent]->bind();
            
            ci::gl::beginTransformFeedback( GL_POINTS );
            
            ci::gl::drawArrays( GL_POINTS, 0, mMaxParticles );
            
            ci::gl::endTransformFeedback();
            
            mSampleTexture = false;
            

        }else{
            
            //mDec -= .01;
            
            Perlin p;
            
            auto circle = p.noise(getElapsedSeconds()*.1)*.1;
            
            mTarget = vec3( mDec*cos( getElapsedSeconds()*(1.-circle) ), mDec*sin(getElapsedSeconds()*( 1 - p.noise(getElapsedSeconds()*.2)*.2 ) ), mDec*cos(getElapsedSeconds()*(1.-circle)) );
            
            gl::ScopedGlslProg glsl(mUpdateNoise);
            mUpdateNoise->uniform( "uDeltaTime", (float)ec::getFrameTimeStep() );
            
            auto trans = mContext->getComponent<ec::TransformComponent>().lock();
            
            mUpdateNoise->uniform("uTarget", vec3(inverse(trans->getModelMatrix()) *vec4( mTarget,1.0)));
            CI_CHECK_GL();
            
            gl::setDefaultShaderVars();
            
            mCurrent = 1 - mCurrent;
            
            ci::gl::ScopedVao		vaoScope( mVaos[mCurrent] );
            gl::ScopedState		stateScope( GL_RASTERIZER_DISCARD, true );
            
            mTF[1 - mCurrent]->bind();
            
            ci::gl::beginTransformFeedback( GL_POINTS );
            ci::gl::drawArrays( GL_POINTS, 0, mMaxParticles );
            ci::gl::endTransformFeedback();

        }
        

//
    }
    CI_CHECK_GL();
}

void RoomParticlesComponent::drawShadows(ec::EventDataRef event)
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw shadow");
    {
        gl::ScopedModelMatrix pushModel;
        auto trans = mContext->getComponent<ec::TransformComponent>().lock();
        gl::multModelMatrix( trans->getModelMatrix() );
        gl::ScopedTextureBind posTex( mPositionsTexture[mCurrent]->getTarget(),  mPositionsTexture[mCurrent]->getId(), 7 );
        gl::ScopedTextureBind velTex( mVelocitiesTexture[mCurrent]->getTarget(),  mVelocitiesTexture[mCurrent]->getId(), 9 );

        mMeshShadow->draw();
        CI_CHECK_GL();

    }
}

void RoomParticlesComponent::draw(ec::EventDataRef event)
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw");

    {
        gl::ScopedModelMatrix pushModel;
        auto trans = mContext->getComponent<ec::TransformComponent>().lock();
        gl::multModelMatrix( trans->getModelMatrix() );
        gl::ScopedTextureBind colorTex(mContext->getComponent<KinectComponent>().lock()->getKinectColorTexture(), 8);
        gl::ScopedTextureBind posTex( mPositionsTexture[mCurrent]->getTarget(),  mPositionsTexture[mCurrent]->getId(), 7 );
        gl::ScopedTextureBind velTex( mVelocitiesTexture[mCurrent]->getTarget(),  mVelocitiesTexture[mCurrent]->getId(), 9 );

        mMesh->draw();
        CI_CHECK_GL();

    }
}

void RoomParticlesComponent::drawRift(ec::EventDataRef event)
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" draw rift");
    
    auto e = std::dynamic_pointer_cast<DrawToRiftBufferEvent>(event);
    
    switch (e->getStyle()) {
        case DrawToRiftBufferEvent::TWICE:
        {
            draw( nullptr );
        }
            break;
        case DrawToRiftBufferEvent::STEREO:
        {
        }
            break;
        default:
            break;
    }
}

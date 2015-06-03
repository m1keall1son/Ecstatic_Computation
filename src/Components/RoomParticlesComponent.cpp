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

using namespace ci;
using namespace ci::app;

const int POSITION_LOC = 0;
const int DATA_LOC = 1;

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
    
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &RoomParticlesComponent::drawTF), DrawToMainBufferEvent::TYPE);
    
}
void RoomParticlesComponent::unregisterHandlers()
{
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &RoomParticlesComponent::handleShutDown), ec::ShutDownEvent::TYPE);
    ec::Controller::get()->eventManager()->removeListener( fastdelegate::MakeDelegate( this, &RoomParticlesComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    
    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &RoomParticlesComponent::drawTF), DrawToMainBufferEvent::TYPE);
    
}

bool RoomParticlesComponent::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    
    try {
        mMaxParticles = tree["max_particles"].getValue<int>();
    } catch (ci::JsonTree::ExcChildNotFound e) {
        CI_LOG_W("max particles not provided defaulting 100");
        mMaxParticles = 100;
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
        transformFeedbackVaryings.push_back( "outData" );
        
        updateFmt.attribLocation("inPosition", POSITION_LOC);
        updateFmt.attribLocation("inData", DATA_LOC);
        
        updateFmt.feedbackFormat( GL_SEPARATE_ATTRIBS );
        updateFmt.feedbackVaryings( transformFeedbackVaryings );
        
        mUpdateNoise = ci::gl::GlslProg::create( updateFmt );

    }
    catch( ci::gl::GlslProgCompileExc e ) {
        CI_LOG_E( "Room particles update noise" + std::string(e.what()) );
    }
    
    try {
        ci::gl::GlslProg::Format updateFmt;
        updateFmt.vertex( loadAsset("shaders/room_particles.vert") )
        .fragment( loadAsset("shaders/room_particles.frag") )
        .attribLocation("inPosition", POSITION_LOC)
        .attribLocation("inData", DATA_LOC);
    
        mRender = ci::gl::GlslProg::create( updateFmt );
        
    }
    catch( ci::gl::GlslProgCompileExc e ) {
        CI_LOG_E( "Room particles render" + std::string(e.what()) );
    }

//    try {
//        std::vector<std::string> transformFeedbackVaryings;
//        ci::gl::GlslProg::Format updateFmt;
//        
//        updateFmt.feedbackFormat( GL_SEPARATE_ATTRIBS )
//        .vertex( loadAsset("shaders/room_particles_seek.vert") )
//        .fragment( loadAsset("shaders/room_particles.frag") );
//        
//        transformFeedbackVaryings.push_back( "outPosition" );
//        transformFeedbackVaryings.push_back( "outData" );
//        updateFmt.attribLocation("inPosition", 0);
//        updateFmt.attribLocation("inData", 1);
//        
//        updateFmt.feedbackVaryings( transformFeedbackVaryings );
//        
//        mUpdateSeekRender = ci::gl::GlslProg::create( updateFmt );
//    }
//    catch( ci::gl::GlslProgCompileExc e ) {
//        CI_LOG_E( "Room particles " + std::string(e.what()) );
//    }

}

void RoomParticlesComponent::initTF()
{
    gl::enable( GL_PROGRAM_POINT_SIZE );
    CI_CHECK_GL();
    
    
    std::vector<vec3> positions;
    std::vector<vec4> data;
    
    for( int i = 0; i < mMaxParticles; i++ ){
        positions.push_back( randVec3() );
        vec4 dataPt = vec4(0);
        dataPt.w = randFloat(200.,300.);
        data.push_back(dataPt);
    }
    
    mData[0] = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec4)* data.size(), data.data(), GL_STATIC_DRAW );
    CI_CHECK_GL();

    mData[1] = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec4)* data.size(), nullptr, GL_STATIC_DRAW );
    CI_CHECK_GL();


    mPositions[0] = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec3)*positions.size(), positions.data(), GL_STATIC_DRAW );
    CI_CHECK_GL();

    mPositions[1] = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec3)*positions.size(), nullptr, GL_STATIC_DRAW );
    CI_CHECK_GL();

    
    for( int i  = 0; i < 2; i++ ){
        
        mVaos[i] = gl::Vao::create();
        mVaos[i]->bind();
        CI_CHECK_GL();

        mPositions[i]->bind();
        gl::vertexAttribPointer(POSITION_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);
        gl::enableVertexAttribArray(POSITION_LOC);
        CI_CHECK_GL();

        
        mData[i]->bind();
        gl::vertexAttribPointer(DATA_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);
        gl::enableVertexAttribArray(DATA_LOC);
        CI_CHECK_GL();

        mTF[i] = gl::TransformFeedbackObj::create();
        CI_CHECK_GL();

        mTF[i]->bind();
        gl::bindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, POSITION_LOC, mPositions[i]);
        gl::bindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, DATA_LOC, mData[i]);
        mTF[i]->unbind();
        CI_CHECK_GL();

    }
    
    CI_CHECK_GL();
    
}

bool RoomParticlesComponent::postInit()
{
    if(!mInitialized){
        
        reloadGlslProg();
        initTF();
        
        mInitialized = true;
    }
    
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
    
    ///this could reflect errors...
    return true;
}

RoomParticlesComponent::RoomParticlesComponent( ec::Actor* context ): ec::ComponentBase( context ), mId( ec::getHash( context->getName() + "_Room_particles_component" ) ),mShuttingDown(false), mStyle(Noise),mCurrent(1)
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

    gl::disableDepthWrite();
    gl::enableAdditiveBlending();
    
    gl::ScopedModelMatrix pushModel;
    auto trans = mContext->getComponent<ec::TransformComponent>().lock();
    gl::multModelMatrix( trans->getModelMatrix() );
    
    if( mStyle == Style::Noise ){
        {
            gl::ScopedGlslProg glsl( mUpdateNoise );
            mUpdateNoise->uniform( "uDeltaTime", (float)ec::getFrameTimeStep());
            gl::setDefaultShaderVars();
            
            mCurrent = 1 - mCurrent;
            
            ci::gl::ScopedVao		vaoScope( mVaos[mCurrent] );
            gl::ScopedState		stateScope( GL_RASTERIZER_DISCARD, true );
            
            mTF[1 - mCurrent]->bind();
            ci::gl::beginTransformFeedback( GL_POINTS );
            ci::gl::drawArrays( GL_POINTS, 0, mMaxParticles );
            ci::gl::endTransformFeedback();
        }
        
        {
            gl::ScopedGlslProg glsl( mRender );
            mRender->uniform( "uColor", mColor );
            gl::setDefaultShaderVars();
            ci::gl::ScopedVao		vaoScope( mVaos[mCurrent] );
            ci::gl::drawArrays( GL_POINTS, 0, mMaxParticles );
        }
        
        
    }else if( mStyle == Style::Seek ){
        gl::ScopedGlslProg glsl( mUpdateSeek );
        mUpdateSeek->uniform( "uDeltaTime", (float)ec::getFrameTimeStep() );
        mUpdateSeek->uniform( "uColor", mColor );
        gl::setDefaultShaderVars();
    }
    
    gl::disableAlphaBlending();
    gl::enableDepthWrite();
    
}


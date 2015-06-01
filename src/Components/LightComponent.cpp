//
//  LightComponent.cpp
//  System_test
//
//  Created by Mike Allison on 5/21/15.
//
//

#include "LightComponent.h"
#include "DebugComponent.h"
#include "Actor.h"
#include "LightManager.h"
#include "FrustumCullComponent.h"
#include "AppSceneBase.h"
#include "Controller.h"
#include "Scene.h"
#include "Events.h"

using namespace ci;

ec::ComponentType LightComponent::TYPE = ec::getHash("light_component");

LightComponentRef LightComponent::create( ec::Actor* context )
{
    return LightComponentRef( new LightComponent(context) );
}

LightComponent::LightComponent( ec::Actor * context ): ec::ComponentBase( context ), mNeedsUpdate(true), mLight( nullptr ), mId( ec::getHash(context->getName()+"_light_component")),mShuttingDown(false)
{
    registerHandlers();
    CI_LOG_V( mContext->getName() + " : "+getName()+" constructed");
    
}

LightComponent::~LightComponent()
{
}

void LightComponent::cleanup()
{
    auto scene = ec::Controller::get()->scene().lock();
    scene->manager()->triggerEvent( ComponentRegistrationEvent::create( ComponentRegistrationEvent::UNREGISTER, ComponentRegistrationEvent::RegistrationType::LIGHT, mContext->getUId(), shared_from_this()) );
    unregisterHandlers();
}

void LightComponent::handleShutDown( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle shutdown");
    mShuttingDown = true;
    
}

void LightComponent::handleSceneChange( ec::EventDataRef )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" handle scene change");
    if(mContext->isPersistent())registerHandlers();
}

void LightComponent::registerHandlers()
{
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &LightComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    ec::Controller::get()->eventManager()->addListener(fastdelegate::MakeDelegate(this, &LightComponent::handleShutDown), ec::ShutDownEvent::TYPE);

    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->addListener(fastdelegate::MakeDelegate(this, &LightComponent::handleUpdate), UpdateEvent::TYPE);
}
void LightComponent::unregisterHandlers()
{
    ec::Controller::get()->eventManager()->removeListener(fastdelegate::MakeDelegate(this, &LightComponent::handleSceneChange), ec::SceneChangeEvent::TYPE);
    ec::Controller::get()->eventManager()->removeListener(fastdelegate::MakeDelegate(this, &LightComponent::handleShutDown), ec::ShutDownEvent::TYPE);

    auto scene = std::dynamic_pointer_cast<AppSceneBase>( ec::Controller::get()->scene().lock() );
    scene->manager()->removeListener(fastdelegate::MakeDelegate(this, &LightComponent::handleUpdate), UpdateEvent::TYPE);
}

bool LightComponent::postInit()
{
    if(!mInitialized){
        ///get bounding box;
        auto & aab_debug = mContext->getComponent<DebugComponent>().lock()->getAxisAlignedBoundingBox();
        geom::Cube() >> geom::Bounds( &aab_debug );
        CI_LOG_V( mContext->getName() + " : "+getName()+" post init");
        mInitialized = true;
    }
    return true;
}

static void initLightBase( const LightRef& light, const ci::JsonTree& init )
{
    try {
        auto color = init["color"].getChildren();
        auto end = color.end();
        ci::vec3 final;
        int i = 0;
        for( auto it = color.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        light->setColor( Color( final.x, final.y, final.z ) );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find color, setting default vec3(.01)");
        light->setColor( Color( 1., 1., 1. ) );
    }
    
    try {
        //mapping is .xy offset and .zw size for using a single map in the shader shared among lights
        auto mapping = init["mapping"].getChildren();
        auto end = mapping.end();
        ci::vec4 final;
        int i = 0;
        for( auto it = mapping.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        light->setMapping( final.x, final.y, final.z, final.w );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find mapping, not setting ");
    }
    
    try {
        auto intensity = init["intensity"].getValue<float>();
        light->setIntensity(intensity);
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find mapping, not setting ");
        light->setIntensity(1.);
    }
    
    try {
        auto visible = init["visible"].getValue<bool>();
        light->setVisible(visible);
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find visible, setting visible");
        light->setVisible(true);
    }
    
    //http://en.wikipedia.org/wiki/Color_temperature
    try {
        auto temp = init["temperature"].getValue<float>();
        light->setColorTemperature(temp);
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find temperature, not setting");
    }
    
}

static void serializeLightBase( const ci::LightRef& base, ci::JsonTree& save )
{
    
    save.addChild( JsonTree("type", LightManager::parseLightTypeToString( base->getType() ) ) );
    
    auto color = ci::JsonTree::makeArray( "color" );
    for( int i = 0; i<3; i++ ){
        color.addChild( JsonTree( "", base->getColor()[i] ) );
    }
    save.addChild(color);
    auto mapping = ci::JsonTree::makeArray( "mapping" );
    for( int i = 0; i<4; i++ ){
        mapping.addChild( JsonTree( "", base->getMapping()[i] ) );
    }
    save.addChild(mapping);
    
    save.addChild( JsonTree( "intensity", base->getIntensity() ) );
    save.addChild( JsonTree( "visible", base->isVisible() ) );
    
}

static void initDirectionalLight( const ci::DirectionalLightRef& light, const ci::JsonTree& init )
{
    
    initLightBase( light, init);
    
    try {
        auto dir = init["direction"].getChildren();
        auto end = dir.end();
        ci::vec3 final;
        int i = 0;
        for( auto it = dir.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        light->setDirection( final );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find direction, setting default down");
        light->setDirection(vec3(0,-1,0));
    }
    
}

static void serializeDirectionalLight( const ci::DirectionalLightRef& light, ci::JsonTree& save  ){
    
    serializeLightBase(light, save);
    
    auto dir = ci::JsonTree::makeArray( "direction" );
    for( int i = 0; i<3; i++ ){
        dir.addChild( JsonTree( "", light->getDirection()[i] ) );
    }
    save.addChild(dir);
    
}

static void initPointLight( const ci::PointLightRef& light, const ci::JsonTree& init ){
    
    initLightBase( light, init);
    
    try {
        auto dir = init["direction"].getChildren();
        auto end = dir.end();
        ci::vec3 final;
        int i = 0;
        for( auto it = dir.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        light->setDirection( final );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find direction, setting default down");
        light->setDirection(vec3(0,-1,0));
    }
    
    try {
        auto pos = init["position"].getChildren();
        auto end = pos.end();
        ci::vec3 final;
        int i = 0;
        for( auto it = pos.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        light->setPosition( final );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find position, setting default vec3(0,1,0)");
        light->setPosition(vec3(0,1,0));
    }
    
    try {
        auto range = init["range"].getValue<float>();
        light->setRange(range);
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find range, not setting");
    }
    
    try {
        auto atten = init["attenuation"].getChildren();
        auto end = atten.end();
        ci::vec2 final;
        int i = 0;
        for( auto it = atten.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        light->setAttenuation( final );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find position, setting default vec2(0,0.01)");
        light->setAttenuation(vec2(0,0.01));
    }
    
    
    try {
        auto dir = init["point_at"].getChildren();
        auto end = dir.end();
        ci::vec3 final;
        int i = 0;
        for( auto it = dir.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        light->pointAt( final );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find point_at, using direction");
    }
    
    try {
        auto shadows = init["enable_shadows"].getValue<bool>();
        light->enableShadows(shadows);
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find enable_shadows, setting false");
        light->enableShadows(false);
    }
    
    //light->calcRange();
    light->enableModulation(false);
    //light->calcIntensity();
    
}

static void serializePointLight( const ci::PointLightRef& light, ci::JsonTree& save  ){
    
    serializeLightBase(light, save);
    
    auto dir = ci::JsonTree::makeArray( "direction" );
    for( int i = 0; i<3; i++ ){
        dir.addChild( JsonTree( "", light->getDirection()[i] ) );
    }
    save.addChild(dir);
    
    auto position = ci::JsonTree::makeArray( "position" );
    for( int i = 0; i<3; i++ ){
        position.addChild( JsonTree( "", light->getPosition()[i] ) );
    }
    save.addChild(position);
    
    auto attenuation = ci::JsonTree::makeArray( "attenuation" );
    for( int i = 0; i<2; i++ ){
        attenuation.addChild( JsonTree( "", light->getAttenuation()[i] ) );
    }
    save.addChild(attenuation);
    
    save.addChild( JsonTree( "enable_shadows", light->hasShadows() ) );
    save.addChild( JsonTree( "range", light->getRange() ) );
    
}

static void initSpotLight( const ci::SpotLightRef &light, const ci::JsonTree& init ){
    
    initLightBase( light, init);
    
    try {
        auto dir = init["direction"].getChildren();
        auto end = dir.end();
        ci::vec3 final;
        int i = 0;
        for( auto it = dir.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        light->setDirection( final );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find direction, setting default down");
        light->setDirection(vec3(0,-1,0));
    }
    
    try {
        auto dir = init["point_at"].getChildren();
        auto end = dir.end();
        ci::vec3 final;
        int i = 0;
        for( auto it = dir.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        light->pointAt( final );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find point_at, using direction");
    }
    
    try {
        auto pos = init["position"].getChildren();
        auto end = pos.end();
        ci::vec3 final;
        int i = 0;
        for( auto it = pos.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        light->setPosition( final );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find position, setting default vec3(0,1,0)");
        light->setPosition(vec3(0,1,0));
    }
    
    try {
        auto range = init["range"].getValue<float>();
        light->setRange(range);
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find range, not setting");
    }
    
    try {
        auto atten = init["attenuation"].getChildren();
        auto end = atten.end();
        ci::vec2 final;
        int i = 0;
        for( auto it = atten.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        light->setAttenuation( final );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find position, setting default vec2(0,0.01)");
        light->setAttenuation(vec2(0,0.01));
    }
    
    try {
        auto shadows = init["enable_shadows"].getValue<bool>();
        light->enableShadows(shadows);
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find enable_shadows, setting false");
        light->enableShadows(false);
    }
    
    //radians
    try {
        auto angle = init["spot_angle"].getValue<float>();
        light->setSpotAngle(angle);
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find spot angle, not setting");
    }
    
    try {
        auto ratio = init["spot_ratio"].getValue<float>();
        light->setSpotRatio(ratio);
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find spot angle, not setting");
    }
    
    //radians
    try {
        auto angle = init["hotspot_angle"].getValue<float>();
        light->setHotspotAngle(angle);
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find spot angle, not setting");
    }
    
    try {
        auto ratio = init["hotspot_ratio"].getValue<float>();
        light->setHotspotRatio(ratio);
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find spot angle, not setting");
    }
    
    
}

static void serializeSpotLight( const ci::SpotLightRef& light, ci::JsonTree& save  ){
    
    serializeLightBase(light, save);
    
    auto dir = ci::JsonTree::makeArray( "direction" );
    for( int i = 0; i<3; i++ ){
        dir.addChild( JsonTree( "", light->getDirection()[i] ) );
    }
    save.addChild(dir);
    
    auto position = ci::JsonTree::makeArray( "position" );
    for( int i = 0; i<3; i++ ){
        position.addChild( JsonTree( "", light->getPosition()[i] ) );
    }
    save.addChild(position);
    
    auto attenuation = ci::JsonTree::makeArray( "attenuation" );
    for( int i = 0; i<2; i++ ){
        attenuation.addChild( JsonTree( "", light->getAttenuation()[i] ) );
    }
    save.addChild(attenuation);
    
    save.addChild( JsonTree( "enable_shadows", light->hasShadows() ) );
    save.addChild( JsonTree( "range", light->getRange() ) );
    save.addChild( JsonTree( "spot_angle", light->getSpotAngle() ) );
    save.addChild( JsonTree( "spot_ratio", light->getSpotRatio() ) );
    save.addChild( JsonTree( "hotspot_angle", light->getHotspotAngle() ) );
    save.addChild( JsonTree( "hotspot_ratio", light->getHotspotRatio() ) );
    
}

static void initWedgeLight( const ci::WedgeLightRef &light, const ci::JsonTree& init ){
    
    initSpotLight( light, init);
    
    try {
        auto length = init["length"].getValue<float>();
        light->setLength(length);
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find legth, not setting");
    }
    
    
    try {
        auto axis = init["axis"].getChildren();
        auto end = axis.end();
        ci::vec3 final;
        int i = 0;
        for( auto it = axis.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        light->setAxis( final );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find axis, not setting");
    }
    
}

static void serializeWedgeLight( const ci::WedgeLightRef& light, ci::JsonTree& save  ){
    
    serializeSpotLight( light, save );
    
    save.addChild( JsonTree( "length", light->getLength() ) );
    
    auto axis = ci::JsonTree::makeArray( "axis" );
    for( int i = 0; i<3; i++ ){
        axis.addChild( JsonTree( "", light->getAxis()[i] ) );
    }
    save.addChild(axis);
    
}

static void initCapsuleLight( const ci::CapsuleLightRef &light, const ci::JsonTree& init ){
    
    initPointLight( light, init);
    
    try {
        auto length = init["length"].getValue<float>();
        light->setLength(length);
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find legth, not setting");
    }
    
    
    try {
        auto axis = init["axis"].getChildren();
        auto end = axis.end();
        ci::vec3 final;
        int i = 0;
        for( auto it = axis.begin(); it != end; ++it ) {
            final[i++] = (*it).getValue<float>();
        }
        light->setAxis( final );
        
    } catch ( const ci::JsonTree::ExcChildNotFound &ex	) {
        CI_LOG_W("didn't find axis, not setting");
    }
    
}

static void serializeCapsuleLight( const ci::CapsuleLightRef& light, ci::JsonTree& save  ){
    
    serializePointLight( light, save );
    
    save.addChild( JsonTree( "length", light->getLength() ) );
    
    auto axis = ci::JsonTree::makeArray( "axis" );
    for( int i = 0; i<3; i++ ){
        axis.addChild( JsonTree( "", light->getAxis()[i] ) );
    }
    save.addChild(axis);
    
}

void LightComponent::handleUpdate(ec::EventDataRef event)
{
    
    CI_LOG_V( mContext->getName() + " : "+getName()+" update");
    ///i dunno
    if( mContext->hasComponent(FrustumCullComponent::TYPE) ){
        auto cull = mContext->getComponent<FrustumCullComponent>().lock();
        mLight->setVisible( cull->isVisible() );
    }
    
    if( mContext->hasComponent(ec::TransformComponent::TYPE) ){
        
        //TODO: transform must have target (pointer to another actor location), lookat, view direction,
        auto transform = mContext->getComponent<ec::TransformComponent>().lock();
        auto t = transform->getTranslation();
        
        switch (mLight->getType()) {
            case ci::Light::Type::Point:
            {
                auto l = std::dynamic_pointer_cast<PointLight>(mLight);
                l->setPosition( t + l->getPosition() );
                auto dir = normalize( l->getPosition() * transform->getRotation() );
                l->setDirection(dir);
            }
                break;
                
            case ci::Light::Type::Capsule:
            {
                auto l = std::dynamic_pointer_cast<CapsuleLight>(mLight);
                l->setPosition( t );
                auto dir = normalize( l->getPosition() * transform->getRotation() );
                l->setDirection(dir);
            }
                break;
                
            case ci::Light::Type::Spot:
            {
                auto l = std::dynamic_pointer_cast<SpotLight>(mLight);
                l->setPosition( t );
                auto dir = normalize( l->getPosition() * transform->getRotation() );
                l->setDirection(dir);
            }
                break;
                
            case ci::Light::Type::Wedge:
            {
                auto l = std::dynamic_pointer_cast<WedgeLight>(mLight);
                l->setPosition( t );
                auto dir = normalize( l->getPosition() * transform->getRotation() );
                l->setDirection(dir);
            }
                break;
                
            default:
                break;
        }
    }
    
}

bool LightComponent::initialize( const ci::JsonTree &tree )
{
    CI_LOG_V( mContext->getName() + " : "+getName()+" initialize");
    
    try{
        auto light = tree["light"];
        auto l_type = light["type"].getValue();
        auto type = LightManager::parseLightType( l_type );
        
        
        switch (type) {
            case Light::Type::Directional: { mLight = Light::createDirectional(); initDirectionalLight( std::dynamic_pointer_cast<DirectionalLight>(mLight), light ); } break;
            case Light::Type::Point: { mLight = Light::createPoint(); initPointLight( std::dynamic_pointer_cast<PointLight>(mLight), light ); } break;
            case Light::Type::Spot: { mLight = Light::createSpot(); initSpotLight( std::dynamic_pointer_cast<SpotLight>(mLight), light ); } break;
            case Light::Type::Wedge: { mLight = Light::createWedge(); initWedgeLight( std::dynamic_pointer_cast<WedgeLight>(mLight), light ); } break;
            case Light::Type::Capsule: { mLight = Light::createCapsule(); initCapsuleLight( std::dynamic_pointer_cast<CapsuleLight>(mLight), light ); } break;
            default: mLight = nullptr; break;
        }
    }catch( ci::JsonTree::ExcChildNotFound e ){
        CI_LOG_E(e.what());
        CI_LOG_E("Couldn't find light tree");
    }
    
    if(!mInitialized)
    {
        ec::Controller::get()->scene().lock()->manager()->queueEvent(  ComponentRegistrationEvent::create( ComponentRegistrationEvent::REGISTER,ComponentRegistrationEvent::RegistrationType::LIGHT, mContext->getUId(), shared_from_this() ) );
        
    }
    
    return true;
}
ci::JsonTree LightComponent::serialize()
{
    auto save = ci::JsonTree();
    save.addChild( ci::JsonTree( "type", getName() ) );
    save.addChild( ci::JsonTree( "id", (uint64_t)getId() ) );
    
    auto light = ci::JsonTree::makeObject("light");
    
    switch (mLight->getType()) {
        case Light::Type::Directional: serializeDirectionalLight( std::dynamic_pointer_cast<DirectionalLight>(mLight), light ); break;
        case Light::Type::Point: serializePointLight( std::dynamic_pointer_cast<PointLight>(mLight), light ); break;
        case Light::Type::Spot: serializeSpotLight( std::dynamic_pointer_cast<SpotLight>(mLight), light ); break;
        case Light::Type::Wedge: serializeWedgeLight( std::dynamic_pointer_cast<WedgeLight>(mLight), light ); break;
        case Light::Type::Capsule: serializeCapsuleLight( std::dynamic_pointer_cast<CapsuleLight>(mLight), light ); break;
        default: break;
    }
    
    save.addChild(light);
    
    return save;
}

const ec::ComponentNameType   LightComponent::getName() const
{
    return "light_component";
}
const ec::ComponentUId        LightComponent::getId() const
{
    return mId;
}
const ec::ComponentType       LightComponent::getType() const
{
    return TYPE;
}

void LightComponent::loadGUI(const ci::params::InterfaceGlRef &gui)
{
    gui->addSeparator();
    gui->addText(getName());
    
    gui->addParam("visible", &mLight->mVisible);
    gui->addParam("intensity", &mLight->mIntensity );
    gui->addParam("color", &mLight->mColor );
    gui->addText("mapping");
    gui->addParam("mapping.x", &mLight->mMapping.x);
    gui->addParam("mapping.y", &mLight->mMapping.y);
    gui->addParam("mapping.w", &mLight->mMapping.z);
    gui->addParam("mapping.h", &mLight->mMapping.w);
    
    
    switch (mLight->getType()) {
        case Light::Type::Directional:
        {
            auto directional = std::dynamic_pointer_cast<DirectionalLight>(mLight);
            gui->addParam("direction", &directional->mDirection);
            
        }
            break;
        case Light::Type::Point:
        {
            auto point = std::dynamic_pointer_cast<PointLight>(mLight);
            gui->addParam("position", &point->mPosition);
            gui->addParam("direction", &point->mDirection);
            gui->addParam("point at", &point->mPointAt);
            gui->addParam("range", &point->mRange);
            gui->addParam("attenuation linear", &point->mAttenuation.x);
            gui->addParam("attenuation quadratic", &point->mAttenuation.y);
            
            std::weak_ptr< PointLight > weak_light = std::weak_ptr< PointLight >(point);
            auto enableShadows = std::bind([](std::weak_ptr< PointLight >& weak_point){
                if (auto strong_point = weak_point.lock()) strong_point->enableShadows( strong_point->mHasShadows );
            }, std::move(weak_light));
            
            gui->addParam("enable shadows", &point->mHasShadows).updateFn(enableShadows);
            
        }
            break;
        case Light::Type::Spot:
        {
            auto spot = std::dynamic_pointer_cast<SpotLight>(mLight);
            gui->addParam("position", &spot->mPosition);
            gui->addParam("direction", &spot->mDirection);
            gui->addParam("point at", &spot->mPointAt);
            gui->addParam("range", &spot->mRange);
            gui->addParam("attenuation linear", &spot->mAttenuation.x);
            gui->addParam("attenuation quadratic", &spot->mAttenuation.y);
            gui->addParam("enable shadows", &spot->mHasShadows);
            gui->addParam("spot ratio", &spot->mSpotRatio);
            gui->addParam("hotspot ratio", &spot->mHotspotRatio);
            
            std::weak_ptr< SpotLight > weak_light = std::weak_ptr< SpotLight >(spot);
            auto enableShadows = std::bind([](std::weak_ptr< SpotLight >& weak_light){
                if (auto strong_light = weak_light.lock()) strong_light->enableShadows( strong_light->mHasShadows );
            }, std::move(weak_light));
            
            gui->addParam("enable shadows", &spot->mHasShadows).updateFn(enableShadows);
            
        }
            break;
        case Light::Type::Capsule:
        {
            auto capsule = std::dynamic_pointer_cast<CapsuleLight>(mLight);
            gui->addParam("position", &capsule->mPosition);
            gui->addParam("direction", &capsule->mDirection);
            gui->addParam("point at", &capsule->mPointAt);
            gui->addParam("range", &capsule->mRange);
            gui->addParam("attenuation linear", &capsule->mAttenuation.x);
            gui->addParam("attenuation quadratic", &capsule->mAttenuation.y);
            
            std::weak_ptr< CapsuleLight > weak_light = std::weak_ptr< CapsuleLight >(capsule);
            auto enableShadows = std::bind([](std::weak_ptr< CapsuleLight >& weak_light){
                if (auto strong_light = weak_light.lock()) strong_light->enableShadows( strong_light->mHasShadows );
            }, std::move(weak_light));
            
            gui->addParam("enable shadows", &capsule->mHasShadows).updateFn(enableShadows);
            
            gui->addParam("length", &capsule->mLength);
            gui->addParam("axis", &capsule->mAxis);
            
        }
            break;
        case Light::Type::Wedge:
        {
            auto wedge = std::dynamic_pointer_cast<WedgeLight>(mLight);
            gui->addParam("position", &wedge->mPosition);
            gui->addParam("direction", &wedge->mDirection);
            gui->addParam("point at", &wedge->mPointAt);
            gui->addParam("range", &wedge->mRange);
            gui->addParam("attenuation linear", &wedge->mAttenuation.x);
            gui->addParam("attenuation quadratic", &wedge->mAttenuation.y);
            
            std::weak_ptr< WedgeLight > weak_light = std::weak_ptr< WedgeLight >(wedge);
            auto enableShadows = std::bind([](std::weak_ptr< WedgeLight >& weak_light){
                if (auto strong_light = weak_light.lock()) strong_light->enableShadows( strong_light->mHasShadows );
            }, std::move(weak_light));
            
            gui->addParam("enable shadows", &wedge->mHasShadows).updateFn(enableShadows);
            
            gui->addParam("spot ratio", &wedge->mSpotRatio);
            gui->addParam("hotspot ratio", &wedge->mHotspotRatio);
            gui->addParam("length", &wedge->mLength);
            gui->addParam("axis", &wedge->mAxis);
        }
            break;
            
        default:
            break;
    }
    
}



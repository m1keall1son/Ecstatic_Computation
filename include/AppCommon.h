//
//  AppCommon.h
//  System_test
//
//  Created by Mike Allison on 5/21/15.
//
//

#pragma once

///SCENES

using SceneFactoryRef = std::shared_ptr<class SceneFactory>;
using IntroSceneRef = std::shared_ptr<class IntroScene>;

///COMPONENTS

using ComponentFactoryRef = std::shared_ptr<class ComponentFactory>;
using FrustumCullComponentRef = std::shared_ptr< class FrustumCullComponent >;
using DebugComponentRef = std::shared_ptr< class DebugComponent >;

///CAMERAS
using CameraManagerRef = std::shared_ptr< class CameraManager >;
using CameraComponentRef = std::shared_ptr< class CameraComponent >;

///LIGHTS

using LightManagerRef = std::shared_ptr< class LightManager >;
using LightComponentRef = std::shared_ptr< class LightComponent >;
using ShadowMapRef = std::shared_ptr<class ShadowMap>;


///RENDERABLE

using GeomTeapotComponentRef = std::shared_ptr< class GeomTeapotComponent >;
using RoomComponentRef = std::shared_ptr< class RoomComponent >;

///App EVENTS

using CullEventRef = std::shared_ptr<class CullEvent>;
using UpdateEventRef = std::shared_ptr<class UpdateEvent>;
using DrawToMainBufferEventRef = std::shared_ptr<class DrawToMainBufferEvent>;
using DrawDebugEventRef = std::shared_ptr<class DrawDebugEvent>;
using DrawShadowEventRef = std::shared_ptr<class DrawShadowEvent>;

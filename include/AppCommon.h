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
using TunnelSceneRef = std::shared_ptr<class TunnelScene>;

///COMPONENTS
using ComponentTemplateRef = std::shared_ptr<class ComponentTemplate>;

using ComponentFactoryRef = std::shared_ptr<class ComponentFactory>;
using FrustumCullComponentRef = std::shared_ptr< class FrustumCullComponent >;
using DebugComponentRef = std::shared_ptr< class DebugComponent >;
using KinectComponentRef = std::shared_ptr< class KinectComponent >;
using BitComponentRef = std::shared_ptr<class BitComponent>;
using TunnelComponentRef = std::shared_ptr< class TunnelComponent >;
using GeomTeapotComponentRef = std::shared_ptr< class GeomTeapotComponent >;
using RoomComponentRef = std::shared_ptr< class RoomComponent >;
using OculusRiftComponentRef = std::shared_ptr< class OculusRiftComponent >;

///CAMERAS
using CameraManagerRef = std::shared_ptr< class CameraManager >;
using CameraComponentRef = std::shared_ptr< class CameraComponent >;

///LIGHTS

using LightManagerRef = std::shared_ptr< class LightManager >;
using LightComponentRef = std::shared_ptr< class LightComponent >;
using ShadowMapRef = std::shared_ptr<class ShadowMap>;

///DEBUG
using DebugManagerRef = std::shared_ptr< class DebugManager >;

///RENDERING
using RenderManagerRef = std::shared_ptr<class RenderManager>;
using GBufferRef = std::shared_ptr<class GBuffer>;
using PostProcessingManagerRef = std::shared_ptr< class PostProcessingManager >;
using PassBaseRef = std::shared_ptr< class PassBase >;
using PassPriority = uint32_t;
using LightPassRef = std::shared_ptr< class LightPass >;
using ShadowPassRef = std::shared_ptr< class ShadowPass >;
using GBufferPassRef = std::shared_ptr< class GBufferPass >;
using FXAAPassRef = std::shared_ptr< class FXAAPass >;
using ForwardPassRef = std::shared_ptr< class ForwardPass >;

///App EVENTS

using CullEventRef = std::shared_ptr<class CullEvent>;
using UpdateEventRef = std::shared_ptr<class UpdateEvent>;
using DrawEventRef = std::shared_ptr<class DrawEvent>;
using DrawToMainBufferEventRef = std::shared_ptr<class DrawToMainBufferEvent>;
using DrawToRiftBufferEventRef = std::shared_ptr<class DrawToRiftBufferEvent>;
using DrawDebugEventRef = std::shared_ptr<class DrawDebugEvent>;
using DrawGeometryEventRef = std::shared_ptr<class DrawGeometryEvent>;
using DrawShadowEventRef = std::shared_ptr<class DrawShadowEvent>;
using ReloadGlslProgEventRef = std::shared_ptr< class ReloadGlslProgEvent >;
using SaveSceneEventRef = std::shared_ptr< class SaveSceneEvent >;
using SwitchCameraEventRef = std::shared_ptr<class SwitchCameraEvent>;
using LoadGUIEventRef = std::shared_ptr<class LoadGUIEvent>;
using ComponentRegistrationEventRef = std::shared_ptr<class ComponentRegistrationEvent>;
using FinishRenderEventRef = std::shared_ptr<class FinishRenderEvent>;
using ShareGeometryDepthTextureEventRef = std::shared_ptr<class ShareGeometryDepthTextureEvent>;


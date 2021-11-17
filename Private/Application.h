/***********************************************************************************
*   Copyright 2021 Marcos S�nchez Torrent.                                         *
*   All Rights Reserved.                                                           *
***********************************************************************************/

#pragma once

#ifndef CORE_APPLICATION_H
#define CORE_APPLICATION_H 1

#include "ImplPrerequisites.h"
#include <Core/IApplication.h>
#include <Core/Event.h>

namespace greaper::core
{
	class Application final : public IApplication
	{
		IGreaperLibrary* m_Library;
		ApplicationConfig m_Config;
		OnCloseEvent_t m_OnClose;
		InitializationEvt_t m_OnInitialization;
		ActivationEvt_t m_OnActivation;
		bool m_IsActive;
		bool m_IsInitialized;
		bool m_HasToStop;

		void LoadConfigLibraries();

	public:
		Application();
		~Application();

		const Uuid& GetInterfaceUUID()const override { return InterfaceUUID; }

		const StringView& GetInterfaceName()const override { return InterfaceName; }

		IGreaperLibrary* GetLibrary()const override { return m_Library; }

		void Initialize(IGreaperLibrary* library)override;

		void Deinitialize()override;

		void OnActivate()override;

		void OnDeactivate()override;

		bool IsActive()const override { return m_IsActive; }

		bool IsInitialized()const override { return m_IsInitialized; }

		void PreUpdate()override;

		void Update()override;

		void PostUpdate()override;

		void FixedUpdate()override;

		InitializationEvt_t* const GetInitializationEvent()override { return &m_OnInitialization; }

		ActivationEvt_t* const GetActivationEvent()override { return &m_OnActivation; }

		void SetConfig(ApplicationConfig config)override;

		const ApplicationConfig& GetConfig()const override { return m_Config; }

		void OnChangingDefault(IApplication* newDefault)override
		{
			UNUSED(newDefault);
			Break("Cannot change the full application at runtime.");
		}

		ChangingDefaultEvt_t* const GetChangingDefaultEvent() { return nullptr; }

		EmptyResult RegisterGreaperLibrary(IGreaperLibrary* library)override;

		Result<IGreaperLibrary*> RegisterGreaperLibrary(const WStringView& libPath)override;

		Result<IGreaperLibrary*> GetGreaperLibrary(const WStringView& libraryName)override;

		Result<IGreaperLibrary*> GetGreaperLibrary(const Uuid& libraryUUID)override;

		EmptyResult UnregisterGreaperLibrary(IGreaperLibrary* library)override;

		EmptyResult RegisterInterface(IInterface* interface)override;

		EmptyResult UnregisterInterface(IInterface* interface)override;

		void MakeInterfaceDefault(IInterface* interface)override;

		Result<IInterface*> GetInterface(const Uuid& interfaceUUID)const  override;

		Result<IInterface*> GetInterface(const StringView& interfaceName)const override;

		Result<IInterface*> GetInterface(const Uuid& interfaceUUID, const Uuid& libraryUUID)const override;

		Result<IInterface*> GetInterface(const StringView& interfaceName, const StringView& libraryName)const override;

		Result<IInterface*> GetInterface(const Uuid& interfaceUUID, const StringView& libraryName)const override;

		Result<IInterface*> GetInterface(const StringView& interfaceName, const Uuid& libraryUUID)const override;

		void StartApplication()override;

		bool AppHasToStop()const override { return m_HasToStop; }

		void StopApplication()override;

		OnCloseEvent_t* const GetOnCloseEvent()override { return &m_OnClose; }

		const StringView& GetApplicationName()const override { return m_Config.ApplicationName; }

		int32 GetApplicationVersion()const override { return m_Config.ApplicationVersion; }

		int32 GetGreaperVersion()const override { return GREAPER_CORE_VERSION; }

		const StringView& GetCompilationInfo()const override
		{
			static constexpr StringView gCompilationInfo = 
#if GREAPER_DEBUG
			"DEBUG"sv;
#elif GREAPER_FRELEASE
			"PUBLIC"sv;
#else
			"RELEASE"sv;
#endif
			return gCompilationInfo;
		}
	};
}

#endif /* CORE_APPLICATION_H */

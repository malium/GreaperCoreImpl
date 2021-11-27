/***********************************************************************************
*   Copyright 2021 Marcos Sánchez Torrent.                                         *
*   All Rights Reserved.                                                           *
***********************************************************************************/

#ifndef GREAPER_CORE_DLL_H
#define GREAPER_CORE_DLL_H 1

#pragma once

#if GREAPER_CORE_DLL

#include "ImplPrerequisites.h"
#include <Core/IGreaperLibrary.h>
#include <Core/ILogManager.h>

namespace greaper
{
	class IInterface;
}

namespace greaper::core
{
	class GreaperCoreLibrary : public IGreaperLibrary
	{
		EmptyResult RegisterProperty(IProperty* property)override;
		IApplication* m_Application = nullptr;
		Library m_Library;
		ILogManager* m_LogManager = nullptr;
		bool m_LogManagerActivated = false;
		EventHandler<IInterface*> m_OnNewLogManager;
		EventHandler<bool> m_OnLogActivation;
		void DumpLogsToLogManager();
		void OnNewLogManager(greaper::IInterface* nlog);
		void OnLogActivation(bool activated);
		Vector<greaper::LogData> m_InitLogs;

		UnorderedMap<StringView, sizet> m_PropertyMap;
		Vector<IProperty*> m_Properties;

	public:
		static constexpr Uuid LibraryUUID = Uuid{ 0xDAC703FC, 0x16BD4F59, 0xB62D28ED, 0x3C9DE087 };
		static constexpr StringView LibraryName = StringView{ "GreaperCoreImpl" };
		
		GreaperCoreLibrary() = default;

		void InitLibrary(Library lib, IApplication* app)override;

		void InitManagers()override;

		void InitProperties()override;

		void InitReflection()override;

		void DeinitReflection()override;

		void DeinitManagers()override;

		void DeinitLibrary()override;

		const Uuid& GetLibraryUuid()const override { return LibraryUUID; }

		const StringView& GetLibraryName()const override { return LibraryName; }

		IApplication* GetApplication()const override { return m_Application; }

		const Library* GetOSLibrary()const override { return &m_Library; }

		CRange<IProperty*> GetPropeties()const override;

		Result<IProperty*> GetProperty(const StringView& name)const override;

		void LogVerbose(const String& message)override;

		void Log(const String& message)override;

		void LogWarning(const String& message)override;

		void LogError(const String& message)override;

		void LogCritical(const String& message)override;
	};
}


#endif // GREAPER_CORE_DLL

#endif /* GREAPER_CORE_DLL_H */
/***********************************************************************************
*   Copyright 2021 Marcos Sánchez Torrent.                                         *
*   All Rights Reserved.                                                           *
***********************************************************************************/

#include "Application.h"
#include <Core/IGreaperLibrary.h>

using namespace greaper;
using namespace greaper::core;

void Application::AddGreaperLibrary(IGreaperLibrary* library)
{
	const auto id = m_Libraries.size();
	m_LibraryNameMap.insert_or_assign(library->GetLibraryName(), id);
	m_LibraryUuidMap.insert_or_assign(library->GetLibraryUuid(), id);
	LibInfo info;
	info.Lib = library;
	m_Libraries.push_back(std::move(info));
}

void Application::LoadConfigLibraries()
{
	if (m_Config.GreaperLibraries == nullptr)
		return;
	for (uint32 i = 0; i < m_Config.GreaperLibraryCount; ++i)
	{
		auto res = RegisterGreaperLibrary(m_Config.GreaperLibraries[i]);
		if (res.HasFailed())
		{
			m_Library->LogWarning(res.GetFailMessage());
			continue;
		}
	}
}

Application::Application()
	:m_Library(nullptr)
	,m_OnClose("OnClose"sv)
	,m_IsActive(false)
	,m_IsInitialized(false)
	,m_HasToStop(false)
{

}

Application::~Application()
{
	// No-op
}

void Application::Initialize(IGreaperLibrary* library)
{
	if (m_IsInitialized)
		return;

	m_Library = library;

	// Initialize...

	m_OnInitialization.Trigger(true);
	m_IsInitialized = true;
}

void Application::Deinitialize()
{
	if (!m_IsInitialized)
		return;
	
	// Deinitialize...

	m_OnInitialization.Trigger(false);
	m_IsInitialized = false;
}

void Application::OnActivate()
{
	if (m_IsActive)
		return;

	// Activate...

	m_OnActivation.Trigger(true);
	m_IsActive = true;
}

void Application::OnDeactivate()
{
	if (!m_IsActive)
		return;

	// Deactivate...

	m_OnActivation.Trigger(false);
	m_IsActive = false;
}

void Application::PreUpdate()
{
	// No-op
}

void Application::Update()
{
	// No-op
}

void Application::PostUpdate()
{
	// No-op
}

void Application::FixedUpdate()
{
	// No-op
}

void Application::SetConfig(ApplicationConfig config)
{
	m_Config = std::move(config);
	// Apply config
	LoadConfigLibraries();
}

EmptyResult Application::RegisterGreaperLibrary(IGreaperLibrary* library)
{
	return CreateFailure<EmptyStruct>("Unfinished function '" FUNCTION_FULL "'.");
}

Result<IGreaperLibrary*> Application::RegisterGreaperLibrary(const WStringView& libPath)
{
	Library lib{ libPath };
	if (!lib.IsOpen())
	{
		return CreateFailure<IGreaperLibrary*>(Format(
			"Trying to register a GreaperLibrary with path '%S', but couldn't be openned.", libPath.data()));
	}
	auto fn = lib.GetFunctionT<void*>("_Greaper"sv);
	if (fn == nullptr)
	{
		return CreateFailure<IGreaperLibrary*>(Format(
			"Trying to register a GreaperLibrary with path '%S', but does not comply with Greaper modular protocol.",
			libPath.data()));
	}
	auto gLib = reinterpret_cast<IGreaperLibrary*>(fn());
	if (gLib == nullptr)
	{
		return CreateFailure<IGreaperLibrary*>(Format(
			"Trying to register a GreaperLibrary with path '%S', but the library returned a nullptr GreaperLibrary.",
			libPath.data()));
	}
	auto uLib = GetGreaperLibrary(gLib->GetLibraryUuid());
	if (uLib.IsOk() && uLib.GetValue() != nullptr)
	{
		return CreateFailure<IGreaperLibrary*>(Format(
			"Trying to register a GreaperLibrary with path '%S', but its UUID '%s' its already registered.",
			libPath.data(), gLib->GetLibraryUuid().ToString().c_str()));
	}
	auto nLib = GetGreaperLibrary(gLib->GetLibraryName());
	if (nLib.IsOk() && nLib.GetValue() != nullptr)
	{
		return CreateFailure<IGreaperLibrary*>(Format(
			"Trying to register a GreaperLibrary with path '%S', but its name '%s' its already registered.",
			libPath.data(), gLib->GetLibraryName().data()));
	}

	AddGreaperLibrary(gLib);
	gLib->InitLibrary(std::move(lib), this);
	return CreateResult(gLib);
}

Result<IGreaperLibrary*> Application::GetGreaperLibrary(const StringView& libraryName)
{
	if (auto findIT = m_LibraryNameMap.find(libraryName); findIT != m_LibraryNameMap.end())
	{
		if (m_Libraries.size() <= findIT->second)
		{
			return CreateFailure<IGreaperLibrary*>(Format("A GreaperLibrary with name '%s' was found, but the library list didn't have that library.", libraryName.data()));
		}
		auto& libInfo = m_Libraries[findIT->second];
		return CreateResult(libInfo.Lib);
	}

	return CreateFailure<IGreaperLibrary*>(Format("Couldn't find the GreaperLibrary '%s'.", libraryName.data()));
}

Result<IGreaperLibrary*> Application::GetGreaperLibrary(const Uuid& libraryUUID)
{
	if (const auto findIT = m_LibraryUuidMap.find(libraryUUID); findIT != m_LibraryUuidMap.end())
	{
		if (m_Libraries.size() <= findIT->second)
		{
			return CreateFailure<IGreaperLibrary*>(Format("A GreaperLibrary with UUID '%s' was found, but the library list didn't have that library.", libraryUUID.ToString().c_str()));
		}
		auto& libInfo = m_Libraries[findIT->second];
		return CreateResult(libInfo.Lib);
	}

	return CreateFailure<IGreaperLibrary*>(Format("Couldn't find the GreaperLibrary '%s'.", libraryUUID.ToString().c_str()));
}

EmptyResult Application::UnregisterGreaperLibrary(IGreaperLibrary* library)
{
	if (library == nullptr)
		return CreateFailure<EmptyStruct>("Trying to unregister a nullptr GreaperLibrary"sv);
	
	const auto nameIT = m_LibraryNameMap.find(library->GetLibraryName());
	const auto uuidIT = m_LibraryUuidMap.find(library->GetLibraryUuid());

	sizet nIndex = std::numeric_limits<sizet>::max();
	sizet uIndex = std::numeric_limits<sizet>::max();
	sizet index = std::numeric_limits<sizet>::max();
	if (nameIT != m_LibraryNameMap.end())
	{
		nIndex = nameIT->second;
		m_LibraryNameMap.erase(nameIT);
	}
	if (uuidIT != m_LibraryUuidMap.end())
	{
		uIndex = uuidIT->second;
		m_LibraryUuidMap.erase(uuidIT);
	}

	if (nIndex != uIndex)
	{
		m_Library->LogWarning(Format("Trying to unregister a GreaperLibrary '%s', but its name and uuid points to different indices.", library->GetLibraryName().data()));
		if (m_Libraries.size() > nIndex)
			index = nIndex;
		else if (m_Libraries.size() > uIndex)
			index = uIndex;
	}

	if (m_Libraries.size() <= index)
		return CreateFailure<EmptyStruct>(Format("Trying to unregister a GreaperLibrary '%s', but it was not registered.", library->GetLibraryName().data()));

	auto& libInfo = m_Libraries[index];
	m_Library->Log(Format("Unregistering GraeperLibrary '%s'.", library->GetLibraryName().data()));
	for (auto*& iface : libInfo.Interfaces)
	{
		if (iface == nullptr)
			continue;

		if (iface->IsActive())
		{
			StopInterfaceDefault(iface->GetInterfaceUUID());
		}

	}


	return CreateFailure<EmptyStruct>(Format("Trying to unregister a GreaperLibrary named '%s', but was not registered.", library->GetLibraryName().data()));
}

EmptyResult Application::RegisterInterface(IInterface* interface)
{
	return CreateFailure<EmptyStruct>("Unfinished function '" FUNCTION_FULL "'.");
}

EmptyResult Application::UnregisterInterface(IInterface* interface)
{
	return CreateFailure<EmptyStruct>("Unfinished function '" FUNCTION_FULL "'.");
}

EmptyResult Application::MakeInterfaceDefault(IInterface* interface)
{
	return CreateFailure<EmptyStruct>("Unfinished function '" FUNCTION_FULL "'.");
}

EmptyResult Application::StopInterfaceDefault(const Uuid& interfaceUUID)
{
	return CreateFailure<EmptyStruct>("Unfinished function '" FUNCTION_FULL "'.");
}

EmptyResult Application::StopInterfaceDefault(const StringView& interfaceName)
{
	return CreateFailure<EmptyStruct>("Unfinished function '" FUNCTION_FULL "'.");
}

Result<IInterface*> Application::GetInterface(const Uuid& interfaceUUID) const
{
	return CreateFailure<IInterface*>("Unfinished function '" FUNCTION_FULL "'.");
}

Result<IInterface*> Application::GetInterface(const StringView& interfaceName) const
{
	return CreateFailure<IInterface*>("Unfinished function '" FUNCTION_FULL "'.");
}

Result<IInterface*> Application::GetInterface(const Uuid& interfaceUUID, const Uuid& libraryUUID) const
{
	return CreateFailure<IInterface*>("Unfinished function '" FUNCTION_FULL "'.");
}

Result<IInterface*> Application::GetInterface(const StringView& interfaceName, const StringView& libraryName) const
{
	return CreateFailure<IInterface*>("Unfinished function '" FUNCTION_FULL "'.");
}

Result<IInterface*> Application::GetInterface(const Uuid& interfaceUUID, const StringView& libraryName) const
{
	return CreateFailure<IInterface*>("Unfinished function '" FUNCTION_FULL "'.");
}

Result<IInterface*> Application::GetInterface(const StringView& interfaceName, const Uuid& libraryUUID) const
{
	return CreateFailure<IInterface*>("Unfinished function '" FUNCTION_FULL "'.");
}

void Application::StartApplication()
{

}

void Application::StopApplication()
{

}
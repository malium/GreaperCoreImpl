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
	return CreateFailure<IGreaperLibrary*>("Unfinished function '" FUNCTION_FULL "'.");
}

Result<IGreaperLibrary*> Application::GetGreaperLibrary(const Uuid& libraryUUID)
{
	return CreateFailure<IGreaperLibrary*>("Unfinished function '" FUNCTION_FULL "'.");
}

EmptyResult Application::UnregisterGreaperLibrary(IGreaperLibrary* library)
{
	return CreateFailure<EmptyStruct>("Unfinished function '" FUNCTION_FULL "'.");
}

EmptyResult Application::RegisterInterface(IInterface* interface)
{
	return CreateFailure<EmptyStruct>("Unfinished function '" FUNCTION_FULL "'.");
}

EmptyResult Application::UnregisterInterface(IInterface* interface)
{
	return CreateFailure<EmptyStruct>("Unfinished function '" FUNCTION_FULL "'.");
}

void Application::MakeInterfaceDefault(IInterface* interface)
{

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
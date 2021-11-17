/***********************************************************************************
*   Copyright 2021 Marcos Sánchez Torrent.                                         *
*   All Rights Reserved.                                                           *
***********************************************************************************/

#include "Application.h"
#include <Core/IGreaperLibrary.h>

using namespace greaper;
using namespace greaper::core;

void Application::LoadConfigLibraries()
{

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
	return CreateFailure<IGreaperLibrary*>("Unfinished function '" FUNCTION_FULL "'.");
}

Result<IGreaperLibrary*> Application::GetGreaperLibrary(const WStringView& libraryName)
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
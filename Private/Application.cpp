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
	gLib->InitManagers();
	gLib->InitProperties();
	gLib->InitReflection();
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
		return CreateEmptyFailure("Trying to unregister a nullptr GreaperLibrary"sv);
	
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
	else
	{
		index = nIndex;
	}

	if (m_Libraries.size() <= index)
		return CreateEmptyFailure(Format("Trying to unregister a GreaperLibrary '%s', but it was not registered.", library->GetLibraryName().data()));

	auto& libInfo = m_Libraries[index];
	m_Library->Log(Format("Unregistering GraeperLibrary '%s'.", library->GetLibraryName().data()));
	for (auto*& iface : libInfo.Interfaces)
	{
		if (iface == nullptr)
			continue;

		if (iface->IsActive())
		{
			DeactivateInterface(iface->GetInterfaceUUID());
		}
		if (iface->IsInitialized())
		{
			iface->Deinitialize();
		}
	}
	libInfo.Lib->DeinitReflection();
	libInfo.Lib->DeinitManagers();
	libInfo.Lib->DeinitLibrary();
	libInfo.Lib = nullptr;
	libInfo.IntefaceNameMap.clear();
	libInfo.InterfaceUuidMap.clear();
	libInfo.Interfaces.clear();
	return CreateEmptyResult();
}

EmptyResult Application::RegisterInterface(IInterface* interface)
{
	if (interface == nullptr)
		return CreateEmptyFailure("Trying to register a nullptr interface.");

	auto* glib = interface->GetLibrary();
	if (glib == nullptr)
	{
		return CreateEmptyFailure("Trying to register an Interface without GreaperLibrary.");
	}
	
	const auto findIT = m_LibraryUuidMap.find(glib->GetLibraryUuid());
	if (findIT == m_LibraryUuidMap.end())
		return CreateEmptyFailure("Trying to register an Interface with a non-registered GreaperLibrary.");
	
	auto& libInfo = m_Libraries[findIT->second];
	
	if (libInfo.Lib != glib)
		return CreateEmptyFailure("Trying to register an Interface with a GreaperLibrary which UUID points to another GreaperLibrary.");
	
	if (const auto nameIT = libInfo.IntefaceNameMap.find(interface->GetInterfaceName()); nameIT != libInfo.IntefaceNameMap.end())
	{
		return CreateEmptyFailure("Trying to register an Interface, but that GreaperLibrary already has an Interface registered with the same name.");
	}

	if (const auto uuidIT = libInfo.InterfaceUuidMap.find(interface->GetInterfaceUUID()); uuidIT != libInfo.InterfaceUuidMap.end())
	{
		return CreateEmptyFailure("Trying to register an Interface, but that GreaperLibrary already has an Interface registered with the same UUID.");
	}

	const auto index = libInfo.Interfaces.size();
	libInfo.Interfaces.push_back(interface);
	libInfo.IntefaceNameMap.insert_or_assign(interface->GetInterfaceName(), index);
	libInfo.InterfaceUuidMap.insert_or_assign(interface->GetInterfaceUUID(), index);

	if (!interface->IsInitialized())
		interface->Initialize(glib);
	return CreateEmptyResult();
}

EmptyResult Application::UnregisterInterface(IInterface* interface)
{
	if (interface == nullptr)
		return CreateEmptyFailure("Trying to unregister a nullptr interface.");

	auto* glib = interface->GetLibrary();
	if (glib == nullptr)
	{
		return CreateEmptyFailure("Trying to unregister an Interface without GreaperLibrary.");
	}

	const auto findIT = m_LibraryUuidMap.find(glib->GetLibraryUuid());
	if (findIT == m_LibraryUuidMap.end())
		return CreateEmptyFailure("Trying to unregister an Interface with a non-registered GreaperLibrary.");

	auto& libInfo = m_Libraries[findIT->second];

	if (libInfo.Lib != glib)
		return CreateEmptyFailure("Trying to unregister an Interface with a GreaperLibrary which UUID points to another GreaperLibrary.");

	const auto nameIT = libInfo.IntefaceNameMap.find(interface->GetInterfaceName());
	const auto uuidIT = libInfo.InterfaceUuidMap.find(interface->GetInterfaceUUID());

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
		m_Library->LogWarning(Format("Trying to unregister an Interface '%s', but its name and uuid points to different indices.", interface->GetInterfaceName().data()));
		if (libInfo.Interfaces.size() > nIndex)
			index = nIndex;
		else if (libInfo.Interfaces.size() > uIndex)
			index = uIndex;
	}
	else
	{
		index = nIndex;
	}

	if (libInfo.Interfaces.size() <= index)
		return CreateEmptyFailure(Format("Trying to unregister an Interface '%s', but it was not registered.", interface->GetInterfaceName().data()));

	m_Library->Log(Format("Unregistering an Interface '%s' from '%s' GreaperLibrary.", interface->GetInterfaceName().data(), glib->GetLibraryName().data()));

	if (interface->IsActive())
		DeactivateInterface(interface->GetInterfaceUUID());
	if (interface->IsInitialized())
		interface->Deinitialize();
	libInfo.Interfaces[index] = nullptr;
	return CreateEmptyResult();
}

EmptyResult Application::ActivateInterface(IInterface* interface)
{
	if (interface == nullptr)
		return CreateEmptyFailure("Trying to make default a nullptr interface, if you want to remove an Active interface call StopInterfaceDefault.");

	LOCK(m_ToAddMutex);
	
	if(Contains(m_InterfacesToAdd, interface))
	{
		return CreateEmptyResult();
	}

	m_InterfacesToAdd.push_back(interface);

	auto* iface = interface;
	m_OnInterfaceActivation.Trigger(std::move(iface));
	
	return CreateEmptyResult();
}

EmptyResult Application::DeactivateInterface(const Uuid& interfaceUUID)
{
	LOCK(m_ActiveMutex);
	const auto uuidIT = m_ActiveInterfaceUuidMap.find(interfaceUUID);
	if (uuidIT == m_ActiveInterfaceUuidMap.end())
		return CreateEmptyFailure(Format("Trying to deactivate an Interface with UUID '%s', but was not active.", interfaceUUID.ToString().c_str()));
	
	const auto index = uuidIT->second;

	if (index >= m_ActiveInterfaces.size())
		return CreateEmptyFailure(Format("Trying to deactivate an Interface with UUID '%s', but its index was out of bounds.", interfaceUUID.ToString().c_str()));

	auto* iface = m_ActiveInterfaces[index];
	if (iface == nullptr)
		return CreateEmptyResult(); // Was already stopped

	iface->OnChangingDefault(nullptr);

	return CreateEmptyResult();
}

EmptyResult Application::DeactivateInterface(const StringView& interfaceName)
{
	LOCK(m_ActiveMutex);
	const auto nameIT = m_ActiveInterfaceNameMap.find(interfaceName);
	if (nameIT == m_ActiveInterfaceNameMap.end())
		return CreateEmptyFailure(Format("Trying to deactivate an Interface with name '%s', but was not active.", interfaceName.data()));

	const auto index = nameIT->second;

	if (index >= m_ActiveInterfaces.size())
		return CreateEmptyFailure(Format("Trying to deactivate an Interface with name '%s', but its index was out of bounds.", interfaceName.data()));

	auto* iface = m_ActiveInterfaces[index];
	if (iface == nullptr)
		return CreateEmptyResult(); // Was already stopped

	iface->OnChangingDefault(nullptr);

	return CreateEmptyResult();
}

Result<IInterface*> Application::GetActiveInterface(const Uuid& interfaceUUID) const
{
	LOCK(m_ActiveMutex);

	const auto uuidIT = m_ActiveInterfaceUuidMap.find(interfaceUUID);
	if (uuidIT == m_ActiveInterfaceUuidMap.end())
		return CreateFailure<IInterface*>(Format("Couldn't find an active Interface with UUID '%s'.", interfaceUUID.ToString().c_str()));

	const auto index = uuidIT->second;

	if (index >= m_ActiveInterfaces.size())
		return CreateFailure<IInterface*>(Format("Interface with UUID '%s', its index was out of bounds.", interfaceUUID.ToString().c_str()));

	auto* iface = m_ActiveInterfaces[index];

	return CreateResult(iface);
}

Result<IInterface*> Application::GetActiveInterface(const StringView& interfaceName) const
{
	LOCK(m_ActiveMutex);

	const auto nameIT = m_ActiveInterfaceNameMap.find(interfaceName);
	if (nameIT == m_ActiveInterfaceNameMap.end())
		return CreateFailure<IInterface*>(Format("Couldn't find an active Interface with name '%s'.", interfaceName.data()));

	const auto index = nameIT->second;

	if (index >= m_ActiveInterfaces.size())
		return CreateFailure<IInterface*>(Format("Interface with name '%s', its index was out of bounds.", interfaceName.data()));

	auto* iface = m_ActiveInterfaces[index];

	return CreateResult(iface);
}

Result<IInterface*> Application::GetInterface(const Uuid& interfaceUUID, const Uuid& libraryUUID) const
{
	const auto libUuidIT = m_LibraryUuidMap.find(libraryUUID);
	if (libUuidIT == m_LibraryUuidMap.end())
	{
		return CreateFailure<IInterface*>(Format("Trying to get an interface with UUID '%s' from a library with UUID '%s', but the library is not registered.", interfaceUUID.ToString().c_str(), libraryUUID.ToString().c_str()));
	}

	if(libUuidIT->second >= m_Libraries.size())
		return CreateFailure<IInterface*>(Format("Trying to get an interface with UUID '%s' from a library with UUID '%s', but the library index is outside of bounds.", interfaceUUID.ToString().c_str(), libraryUUID.ToString().c_str()));

	auto& libInfo = m_Libraries[libUuidIT->second];

	const auto ifaceUuidIT = libInfo.InterfaceUuidMap.find(interfaceUUID);

	if(ifaceUuidIT == libInfo.InterfaceUuidMap.end())
		return CreateFailure<IInterface*>(Format("Trying to get an interface with UUID '%s' from a library with UUID '%s', but the interface is not registered.", interfaceUUID.ToString().c_str(), libraryUUID.ToString().c_str()));

	if(ifaceUuidIT->second >= libInfo.Interfaces.size())
		return CreateFailure<IInterface*>(Format("Trying to get an interface with UUID '%s' from a library with UUID '%s', but the interface index is outside of bounds.", interfaceUUID.ToString().c_str(), libraryUUID.ToString().c_str()));

	auto* iface = libInfo.Interfaces[ifaceUuidIT->second];
	return CreateResult(iface);
}

Result<IInterface*> Application::GetInterface(const StringView& interfaceName, const StringView& libraryName) const
{
	const auto libNameIT = m_LibraryNameMap.find(libraryName);
	if (libNameIT == m_LibraryNameMap.end())
	{
		return CreateFailure<IInterface*>(Format("Trying to get an interface with name '%s' from a library with name '%s', but the library is not registered.", interfaceName.data(), libraryName.data()));
	}

	if (libNameIT->second >= m_Libraries.size())
		return CreateFailure<IInterface*>(Format("Trying to get an interface with name '%s' from a library with name '%s', but the library index is outside of bounds.", interfaceName.data(), libraryName.data()));

	auto& libInfo = m_Libraries[libNameIT->second];

	const auto ifaceNameIT = libInfo.IntefaceNameMap.find(interfaceName);

	if (ifaceNameIT == libInfo.IntefaceNameMap.end())
		return CreateFailure<IInterface*>(Format("Trying to get an interface with name '%s' from a library with name '%s', but the interface is not registered.", interfaceName.data(), libraryName.data()));

	if (ifaceNameIT->second >= libInfo.Interfaces.size())
		return CreateFailure<IInterface*>(Format("Trying to get an interface with name '%s' from a library with name '%s', but the interface index is outside of bounds.", interfaceName.data(), libraryName.data()));

	auto* iface = libInfo.Interfaces[ifaceNameIT->second];
	return CreateResult(iface);
}

Result<IInterface*> Application::GetInterface(const Uuid& interfaceUUID, const StringView& libraryName) const
{
	const auto libNameIT = m_LibraryNameMap.find(libraryName);
	if (libNameIT == m_LibraryNameMap.end())
	{
		return CreateFailure<IInterface*>(Format("Trying to get an interface with UUID '%s' from a library with name '%s', but the library is not registered.", interfaceUUID.ToString().c_str(), libraryName.data()));
	}

	if (libNameIT->second >= m_Libraries.size())
		return CreateFailure<IInterface*>(Format("Trying to get an interface with UUID '%s' from a library with name '%s', but the library index is outside of bounds.", interfaceUUID.ToString().c_str(), libraryName.data()));

	auto& libInfo = m_Libraries[libNameIT->second];

	const auto ifaceUuidIT = libInfo.InterfaceUuidMap.find(interfaceUUID);

	if (ifaceUuidIT == libInfo.InterfaceUuidMap.end())
		return CreateFailure<IInterface*>(Format("Trying to get an interface with UUID '%s' from a library with name '%s', but the interface is not registered.", interfaceUUID.ToString().c_str(), libraryName.data()));

	if (ifaceUuidIT->second >= libInfo.Interfaces.size())
		return CreateFailure<IInterface*>(Format("Trying to get an interface with UUID '%s' from a library with name '%s', but the interface index is outside of bounds.", interfaceUUID.ToString().c_str(), libraryName.data()));

	auto* iface = libInfo.Interfaces[ifaceUuidIT->second];
	return CreateResult(iface);
}

Result<IInterface*> Application::GetInterface(const StringView& interfaceName, const Uuid& libraryUUID) const
{
	const auto libUuidIT = m_LibraryUuidMap.find(libraryUUID);
	if (libUuidIT == m_LibraryUuidMap.end())
	{
		return CreateFailure<IInterface*>(Format("Trying to get an interface with name '%s' from a library with UUID '%s', but the library is not registered.", interfaceName.data(), libraryUUID.ToString().c_str()));
	}

	if (libUuidIT->second >= m_Libraries.size())
		return CreateFailure<IInterface*>(Format("Trying to get an interface with name '%s' from a library with UUID '%s', but the library index is outside of bounds.", interfaceName.data(), libraryUUID.ToString().c_str()));

	auto& libInfo = m_Libraries[libUuidIT->second];

	const auto ifaceNameIT = libInfo.IntefaceNameMap.find(interfaceName);

	if (ifaceNameIT == libInfo.IntefaceNameMap.end())
		return CreateFailure<IInterface*>(Format("Trying to get an interface with name '%s' from a library with UUID '%s', but the interface is not registered.", interfaceName.data(), libraryUUID.ToString().c_str()));

	if (ifaceNameIT->second >= libInfo.Interfaces.size())
		return CreateFailure<IInterface*>(Format("Trying to get an interface with name '%s' from a library with UUID '%s', but the interface index is outside of bounds.", interfaceName.data(), libraryUUID.ToString().c_str()));

	auto* iface = libInfo.Interfaces[ifaceNameIT->second];
	return CreateResult(iface);
}

void Application::StartApplication()
{
	m_Library->Log(Format("Starting %s...", m_Config.ApplicationName.data()));

}

void Application::StopApplication()
{
	m_Library->Log(Format("Stopping %s...", m_Config.ApplicationName.data()));

	m_HasToStop = true;
	m_OnClose.Trigger();
}
/***********************************************************************************
*   Copyright 2021 Marcos Sánchez Torrent.                                         *
*   All Rights Reserved.                                                           *
***********************************************************************************/

#include "GreaperCoreDLL.h"
#include "Application.h"
#include <Core/Property.h>

#if GREAPER_CORE_DLL
extern greaper::core::GreaperCoreLibrary* gCoreLibrary = nullptr;
#if PLT_WINDOWS
#define DLL_PROCESS_ATTACH   1
#define DLL_THREAD_ATTACH    2
#define DLL_THREAD_DETACH    3
#define DLL_PROCESS_DETACH   0

int __stdcall DllMain(HINSTANCE hModule,
	unsigned long  ul_reason_for_call,
	void* lpReserved
)
{
	UNUSED(hModule);
	UNUSED(lpReserved);
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return 1;
}
#endif
BEGIN_C
DLLEXPORT void* CDECL _Greaper();
END_C

void* _Greaper()
{
	if (gCoreLibrary == nullptr)
	{
		gCoreLibrary = greaper::Construct<greaper::core::GreaperCoreLibrary>();
	}
	return gCoreLibrary;
}

greaper::EmptyResult greaper::core::GreaperCoreLibrary::RegisterProperty(IProperty* property)
{
	const auto nameIT = m_PropertyMap.find(property->GetPropertyName());
	if (nameIT != m_PropertyMap.end())
	{
		const auto index = nameIT->second;
		if (index < m_Properties.size())
		{
			auto* oProp = m_Properties[index];
			if (oProp == nullptr)
			{
				m_Properties[index] = property;
				return CreateEmptyResult();
			}
			return CreateEmptyFailure(Format("Trying to register a Property '%s', but its already registered.", property->GetPropertyName().c_str()));
		}
		m_PropertyMap.erase(nameIT);
	}
	const auto index = m_Properties.size();
	m_Properties.push_back(property);
	m_PropertyMap.insert_or_assign(property->GetPropertyName(), index);
	return CreateEmptyResult();
}

void greaper::core::GreaperCoreLibrary::DumpLogsToLogManager()
{
	if (m_LogManager != nullptr && m_LogManagerActivated)
	{
		for (const auto& logData : m_InitLogs)
			m_LogManager->_Log(logData);
		m_InitLogs.clear();
	}
}

void greaper::core::GreaperCoreLibrary::OnNewLogManager(greaper::IInterface* nLog)
{
	if (nLog == nullptr || nLog->GetInterfaceUUID() != ILogManager::InterfaceUUID)
		return;
	m_LogManager = reinterpret_cast<ILogManager*>(nLog);
	m_LogManagerActivated = m_LogManager->IsActive();

	m_OnLogActivation.Disconnect();
	m_LogManager->GetActivationEvent()->Connect(m_OnLogActivation,
		std::bind(&GreaperCoreLibrary::OnLogActivation, this, std::placeholders::_1));

	DumpLogsToLogManager();
}

void greaper::core::GreaperCoreLibrary::OnLogActivation(bool activated)
{
	m_LogManagerActivated = activated;
	DumpLogsToLogManager();
}

void greaper::core::GreaperCoreLibrary::InitLibrary(Library lib, IApplication* app)
{
	m_Library = std::move(lib);
	m_Application = app;
}

void greaper::core::GreaperCoreLibrary::InitManagers()
{
	if (m_Application != nullptr)
	{
		LogError(Format("Trying to Initialize the Managers of the GreaperLibrary '%s', but IApplication was already initialized.", LibraryName.data()));
		return;
	}
	m_Application = Construct<Application>();
	m_Application->GetOnInterfaceActivationEvent()->Connect(m_OnNewLogManager,
		std::bind(&GreaperCoreLibrary::OnNewLogManager, this, std::placeholders::_1));
	m_Application->Initialize(this);
}

void greaper::core::GreaperCoreLibrary::InitProperties()
{

}

void greaper::core::GreaperCoreLibrary::InitReflection()
{
}

void greaper::core::GreaperCoreLibrary::DeinitReflection()
{
}

void greaper::core::GreaperCoreLibrary::DeinitManagers()
{
	m_Application->Deinitialize();
}

void greaper::core::GreaperCoreLibrary::DeinitLibrary()
{
	
}

greaper::CRange<greaper::IProperty*> greaper::core::GreaperCoreLibrary::GetPropeties() const
{
	return CreateRange(m_Properties);
}

greaper::Result<greaper::IProperty*> greaper::core::GreaperCoreLibrary::GetProperty(const StringView& name) const
{
	const auto nameIT = m_PropertyMap.find(name);
	if (nameIT == m_PropertyMap.end())
		return CreateFailure<greaper::IProperty*>(Format("Couldn't find the property '%s' registered in the GreaperLibrary '%s'.", name.data(), GetLibraryName().data()));
	if(nameIT->second >= m_Properties.size())
		return CreateFailure<greaper::IProperty*>(Format("The property '%s' registered in the GreaperLibrary '%s', is registered to an out of bounds index.", name.data(), GetLibraryName().data()));
	auto* prop = m_Properties[nameIT->second];
	return CreateResult(prop);
}

void greaper::core::GreaperCoreLibrary::LogVerbose(const String& message)
{
	if (m_LogManager != nullptr && m_LogManagerActivated)
		m_LogManager->Log(LogLevel_t::VERBOSE, message);
	else
		m_InitLogs.push_back(LogData{ message, Clock_t::now(), LogLevel_t::VERBOSE });
}

void greaper::core::GreaperCoreLibrary::Log(const String& message)
{
	if (m_LogManager != nullptr && m_LogManagerActivated)
		m_LogManager->Log(LogLevel_t::INFORMATIVE, message);
	else
		m_InitLogs.push_back(LogData{ message, Clock_t::now(), LogLevel_t::INFORMATIVE });
}

void greaper::core::GreaperCoreLibrary::LogWarning(const String& message)
{
	if (m_LogManager != nullptr && m_LogManagerActivated)
		m_LogManager->Log(LogLevel_t::WARNING, message);
	else
		m_InitLogs.push_back(LogData{ message, Clock_t::now(), LogLevel_t::WARNING });
}

void greaper::core::GreaperCoreLibrary::LogError(const String& message)
{
	if (m_LogManager != nullptr && m_LogManagerActivated)
		m_LogManager->Log(LogLevel_t::LL_ERROR, message);
	else
		m_InitLogs.push_back(LogData{ message, Clock_t::now(), LogLevel_t::LL_ERROR });
}

void greaper::core::GreaperCoreLibrary::LogCritical(const String& message)
{
	if (m_LogManager != nullptr && m_LogManagerActivated)
		m_LogManager->Log(LogLevel_t::CRITICAL, message);
	else
		m_InitLogs.push_back(LogData{ message, Clock_t::now(), LogLevel_t::CRITICAL });
}

#else

#endif
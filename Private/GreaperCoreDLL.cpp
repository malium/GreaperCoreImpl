/***********************************************************************************
*   Copyright 2021 Marcos Sánchez Torrent.                                         *
*   All Rights Reserved.                                                           *
***********************************************************************************/

#include "GreaperCoreDLL.h"
#include "Application.h"

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

bool greaper::core::GreaperCoreLibrary::RegisterProperty(IProperty* property)
{
	return false;
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
}

void greaper::core::GreaperCoreLibrary::InitProperties()
{
}

void greaper::core::GreaperCoreLibrary::InitReflection()
{
}

void greaper::core::GreaperCoreLibrary::DeinitLibrary()
{
	m_Application->Deinitialize();
}

greaper::Vector<greaper::IProperty*> greaper::core::GreaperCoreLibrary::GetPropeties() const
{
	return Vector<IProperty*>();
}

greaper::Result<greaper::IProperty*> greaper::core::GreaperCoreLibrary::GetProperty(const String& name) const
{
	return CreateFailure<greaper::IProperty*>("Unfinished function '" FUNCTION_FULL "'.");
}

void greaper::core::GreaperCoreLibrary::LogVerbose(const String& message)
{
}

void greaper::core::GreaperCoreLibrary::Log(const String& message)
{
}

void greaper::core::GreaperCoreLibrary::LogWarning(const String& message)
{
}

void greaper::core::GreaperCoreLibrary::LogError(const String& message)
{
}

void greaper::core::GreaperCoreLibrary::LogCritical(const String& message)
{
}

#else

#endif
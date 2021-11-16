/***********************************************************************************
*   Copyright 2021 Marcos S�nchez Torrent.                                         *
*   All Rights Reserved.                                                           *
***********************************************************************************/

#include "Application.h"
#include <Core/IGreaperLibrary.h>

using namespace greaper::core;

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

}
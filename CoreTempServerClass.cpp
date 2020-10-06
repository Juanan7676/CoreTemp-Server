#include <iostream>
#include <Windows.h>
#include "CoreTempServerClass.h"

using namespace std;

CoreTempServerClass::CoreTempServerClass(void)
{
	this->configurePlugin();
}

CoreTempServerClass::~CoreTempServerClass(void)
{
	cout << "Bye bye from CoreTempServer\n";
	delete m_CoreTempPlugin.pluginInfo;
}

int CoreTempServerClass::Start()
{
	cout << "The start function has been called\n";

	this->latest = new core_temp_shared_data;
	this->stop = false;

	// Return 0 for success, non-zero for failure.
	return 0;
}

void CoreTempServerClass::Update(const LPCoreTempSharedData data)
{
	if (data != NULL)
	{
		this->latest->fCPUSpeed = data->fCPUSpeed;
		this->latest->fFSBSpeed = data->fFSBSpeed;
		this->latest->fMultiplier = data->fMultiplier;
		memcpy(this->latest->fTemp,data->fTemp,sizeof(data->fTemp));
		this->latest->fVID = data->fVID;
		memcpy(this->latest->sCPUName,data->sCPUName,sizeof(data->sCPUName));
		this->latest->ucDeltaToTjMax = data->ucDeltaToTjMax;
		this->latest->ucFahrenheit = data->ucFahrenheit;
		this->latest->uiCoreCnt = data->uiCoreCnt;
		this->latest->uiCPUCnt = data->uiCPUCnt;
		memcpy(this->latest->uiLoad,data->uiLoad,sizeof(data->uiLoad));
		memcpy(this->latest->uiTjMax,data->uiTjMax,sizeof(data->uiTjMax));
	}
}

void CoreTempServerClass::Stop()
{
	cout << "The plugin has stopped\n";
	this->stop = true;
}

int CoreTempServerClass::Configure()
{
	// Return 0 for not-implemented, non-zero for "Handled".
	return 0;
}

void CoreTempServerClass::Remove(const wchar_t* path)
{
	cout << "Cleanup should be performed at " << path << "\n";
}

LPCoreTempPlugin CoreTempServerClass::GetPluginInstance(HMODULE hModule)
{
	return &this->m_CoreTempPlugin;
}

void CoreTempServerClass::configurePlugin()
{
	LPCoreTempPluginInfo pluginInfo = new CoreTempPluginInfo;
	m_CoreTempPlugin.pluginInfo = pluginInfo;

	pluginInfo->name = const_cast<wchar_t*>(L"Core Temp Server Plugin");
	pluginInfo->description = const_cast<wchar_t*>(L"Provides CPU temp info via a socket");
	pluginInfo->version = const_cast<wchar_t*>(L"1.0");

	// Interface version should be 1 for current plugin API.
	m_CoreTempPlugin.interfaceVersion = 1;
	m_CoreTempPlugin.type = General_Type;
}
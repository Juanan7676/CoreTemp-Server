#pragma once
#include "lib/CoreTempPlugin.h"
class CoreTempServerClass
{
public:
	CoreTempServerClass(void);
	virtual ~CoreTempServerClass(void);

	int Start();
	void Update(const LPCoreTempSharedData data);
	void Stop();
	int Configure();
	void Remove(const wchar_t* path);
	bool stop;

	LPCoreTempSharedData latest;

	LPCoreTempPlugin GetPluginInstance(HMODULE hModule);

protected:

	CoreTempPlugin m_CoreTempPlugin;
	void configurePlugin();
};


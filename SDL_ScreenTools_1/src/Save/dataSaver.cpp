#include "dataSaver.h"

#include "../app.h"

#include "../globalData.h"

void SaveConfig(App* app) {
	Json::Value ConfigData;

	ConfigData["timeColor"]["R"] = gData::timeColor.r;
	ConfigData["timeColor"]["G"] = gData::timeColor.g;
	ConfigData["timeColor"]["B"] = gData::timeColor.b;
	ConfigData["timeColor"]["A"] = gData::timeColor.a;

	ConfigData["windowPos"]["X"] = gData::windowPos.x;
	ConfigData["windowPos"]["Y"] = gData::windowPos.y;

	ConfigData["timeTextSize"] = gData::timeTextSize;

	if (!saveJsonFile(*app->GetJsonConfigPath(), ConfigData)) {
		LOG_WARNING_STREAM << "要保存的目标路径:" << *app->GetJsonConfigPath();
		LOG_WARNING(u8"配置文件保存失败 Config file save failed");
	}
}

void LoadConfig(App* app) {
	Json::Value ConfigData;

	if (!readJsonFile(*app->GetJsonConfigPath(), ConfigData)) {
		LOG_WARNING_STREAM << "要读取的目标路径:" << *app->GetJsonConfigPath();
		LOG_WARNING(u8"配置文件读取失败,将使用默认值 Failed to read config file, will use defaults");
		return;
	}

	if (ConfigData.isMember("timeColor")){
		const auto& color = ConfigData["timeColor"];
		gData::timeColor.r = color["R"].asInt();
		gData::timeColor.g = color["G"].asInt();
		gData::timeColor.b = color["B"].asInt();
		gData::timeColor.a = color["A"].asInt();
	}

	if (ConfigData.isMember("windowPos")){
		const auto& pos = ConfigData["windowPos"];
		gData::windowPos.x = pos["X"].asFloat();
		gData::windowPos.y = pos["Y"].asFloat();
	}

	if (ConfigData.isMember("timeTextSize")) {
		gData::timeTextSize = ConfigData["timeTextSize"].asInt();
	}
}
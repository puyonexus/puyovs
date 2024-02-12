// Copyright (c) 2024, LossRain
// SPDX-License-Identifier: GPLv3

#include "AssetManager.h"

namespace ppvs {

// This macro walks the bundles and stops at 0 return value, overwrites `target` variable
#define DELEGATE(FUNCTION)             \
	for (auto bundle : m_bundleList) { \
		if (!bundle->FUNCTION) {       \
			success = true;            \
			break;                     \
		}                              \
		delete target;                 \
		target = nullptr;              \
	}

// This awful macro iterates through the bundles and performs the function, waiting for a zero return value.
#define ITERATE_TARGET(FUNCTION, TYPE)                                                \
	TYPE target = nullptr;                                                            \
	bool success = false;                                                             \
	DELEGATE(FUNCTION);                                                               \
	if (!success) {                                                                   \
		m_debug->log("Error loading (TYPE) token " + token, DebugMessageType::Error); \
	}                                                                                 \
	return target;

AssetManager::AssetManager(ppvs::Frontend* fe, DebugLog* dbg)
	: m_front(fe)
	, m_debug(dbg)
{
	m_debug->log("Asset manager loaded", DebugMessageType::Debug);
};

AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
	m_debug->log("Asset manager destroyed", DebugMessageType::Debug);
}

void AssetManager::init(ppvs::Frontend* fe, ppvs::DebugLog* dbg)
{
	m_front = fe;
	m_debug = dbg;
	reloadBundles();
}

int AssetManager::loadBundle(ppvs::AssetBundle* bundle, int priority)
{
	bundle->init(m_front);
	bundle->reload();
	bundle->m_debug = m_debug;
	// TODO: implement priority
	m_bundleList.push_back(bundle);
	return 0;
}

int AssetManager::deleteBundle(ppvs::AssetBundle* bundle)
{
	m_bundleList.remove(bundle);
	delete bundle;
	return false;
}

// TODO: merge into a macro
FeImage* AssetManager::loadImage(const std::string& token)
{
	FeImage* target = nullptr;
	for (auto* bundle : m_bundleList) {
		target = bundle->loadImage(std::string());
		if (!target->error()) {
			break;
		} else {
			target = nullptr;
			break;
		}
	}
	if (target == nullptr || target->error()) {
		m_debug->log("Error loading image token " + token, DebugMessageType::Error);
	}
	return target;
}

FeSound* AssetManager::loadSound(const std::string& token)
{
	FeSound* target = nullptr;
	for (auto* bundle : m_bundleList) {
		target = bundle->loadSound(token);
		if (!target->error()) {
			// target->play();
			break;
		} else {
			target = nullptr;
			break;
		}
	}
	if (target == nullptr || target->error()) {
		m_debug->log("Error loading sound token " + token, DebugMessageType::Error);
	}
	return target;
}

std::vector<std::string> AssetManager::listBackgrounds()
{
	return {};
};

int AssetManager::reloadBundles()
{
	int number_of_loaded = 0;
	for (auto bundle : m_bundleList) {
		bundle->reload(m_front);
		number_of_loaded++;
	}
	return number_of_loaded;
}

} // ppvs
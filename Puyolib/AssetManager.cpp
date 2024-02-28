// Copyright (c) 2024, LossRain
// SPDX-License-Identifier: GPLv3

#include "AssetManager.h"

namespace ppvs {

AssetManager* AssetManager::clone()
{
	AssetManager* current = new AssetManager;
	for (auto bundle : m_bundleList) {
		current->loadBundle(bundle->clone());
	}
	current->activate(m_front, m_debug);
	return current;
}

AssetManager::AssetManager(ppvs::Frontend* fe, DebugLog* dbg)
	: m_front(fe)
	, m_debug(dbg)
{
	if (m_debug != nullptr) {
		m_debug->log("Asset manager loaded", DebugMessageType::Debug);
	}
}

AssetManager::AssetManager()
	= default;

AssetManager::~AssetManager()
{
	if (m_debug != nullptr) {
		m_debug->log("Asset manager destroyed", DebugMessageType::Debug);
	}
}

void AssetManager::activate(ppvs::Frontend* fe, ppvs::DebugLog* dbg)
{
	m_front = fe;
	m_debug = dbg;
	reloadBundles();
	// Do not activate unless we know we won't call nullptr
	if (m_front != nullptr && m_debug != nullptr) {
		activated = true;
	} else {
		activated = false;
	}
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

int AssetManager::unloadAll()
{
	if (m_bundleList.empty()) {
		return 1;
	}
	// This approach is required to remove instances from the list we are iterating over
	auto i = m_bundleList.begin();
	while (i != m_bundleList.end()) {
		if (!(*i)->active) { // Check for tombstones
			m_bundleList.erase(i++);
		} else {
			i++;
		}
	}
	return 0;
}

FeImage* AssetManager::loadImage(const std::string& token, const std::string& custom)
{
	FeImage* target {};
	for (auto* bundle : m_bundleList) {
		target = bundle->loadImage(std::string(token), custom);
		if (target != nullptr && !target->error()) {
			break;
		}
	}
	if (target == nullptr || target->error()) {
		m_debug->log("Error loading image token " + token + " custom " + custom, DebugMessageType::Error);
	}
	return target;
}

FeImage* AssetManager::loadImage(const std::string& token, PuyoCharacter character)
{
	FeImage* target {};
	for (auto* bundle : m_bundleList) {
		target = bundle->loadCharImage(std::string(token), character);
		if (target != nullptr && !target->error()) {
			break;
		}
	}
	if (target == nullptr || target->error()) {
		m_debug->log("Error loading image token " + token + " character " + std::to_string(static_cast<int>(character)), DebugMessageType::Error);
	}
	return target;
}

FeSound* AssetManager::loadSound(const std::string& token, const std::string& custom)
{
	FeSound* target = {};
	for (auto* bundle : m_bundleList) {
		target = bundle->loadSound(token, custom);
		if (!target->error()) {
			// target->play();
			break;
		} else {
			target = nullptr;
		}
	}
	if (target == nullptr || target->error()) {
		m_debug->log("Error loading sound token " + token + " custom " + custom, DebugMessageType::Error);
	}
	return target;
}

FeSound* AssetManager::loadSound(const std::string& token, PuyoCharacter character)
{
	FeSound* target = {};
	for (auto* bundle : m_bundleList) {
		target = bundle->loadCharSound(token, character);
		if (!target->error()) {
			break;
		} else {
			target = nullptr;
		}
	}
	if (target == nullptr || target->error()) {
		m_debug->log("Error loading sound token " + token + " character " + std::to_string(static_cast<int>(character)), DebugMessageType::Error);
	}
	return target;
}

std::string AssetManager::getAnimationFolder(ppvs::PuyoCharacter character)
{
	std::string target;
	for (auto bundle : m_bundleList) {
		target = bundle->getCharAnimationsFolder(character);
		if (!target.empty()) {
			break;
		}
	}
	if (target.empty()) {
		m_debug->log("Error loading animation script character " + std::to_string(static_cast<int>(character)), DebugMessageType::Error);
	}
	return target;
}

std::string AssetManager::getAnimationFolder(std::string token, const std::string& script_name)
{
	std::string target;
	for (auto bundle : m_bundleList) {
		target = bundle->getAnimationFolder(token, script_name);
		if (!target.empty()) {
			break;
		}
	}
	if (target.empty()) {
		m_debug->log("Error loading animation script token " + token, DebugMessageType::Error);
	}
	return target;
}

std::set<std::string> AssetManager::listPuyoSkins()
{
	std::set<std::string> result {};
	for (auto bundle : m_bundleList) {
		for (auto value : bundle->listPuyoSkins()) {
			result.insert(value);
		}
	}
	return result;
}

std::set<std::string> AssetManager::listBackgrounds()
{
	std::set<std::string> result {};
	for (auto bundle : m_bundleList) {
		for (auto value : bundle->listBackgrounds()) {
			result.insert(value);
		}
	}
	return result;
}

std::set<std::string> AssetManager::listCharacterSkins()
{
	std::set<std::string> result {};
	for (auto bundle : m_bundleList) {
		for (auto value : bundle->listCharacterSkins()) {
			result.insert(value);
		}
	}
	return result;
}

std::set<std::string> AssetManager::listSfx()
{
	std::set<std::string> result {};
	for (auto bundle : m_bundleList) {
		for (auto value : bundle->listSfx()) {
			result.insert(value);
		}
	}
	return result;
}

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
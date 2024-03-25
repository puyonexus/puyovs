// Copyright (c) 2024, LossRain
// SPDX-License-Identifier: GPLv3

#include "AssetManager.h"

namespace ppvs {

AssetManagerPriv* AssetManager::generateGamePriv(Frontend* frontend, DebugLog* m_dbg)
{
	return new AssetManagerPriv(this, frontend, m_dbg ? m_dbg : m_debug);
}

AssetManager::AssetManager(DebugLog* dbg)
	: m_debug(dbg)
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

bool AssetManager::loadBundle(AssetBundle* bundle, int priority)
{
	bundle->reload();
	bundle->m_debug = m_debug;
	// TODO: implement priority
	m_bundleList.push_back(bundle);
	return false;
}

bool AssetManager::deleteBundle(AssetBundle* bundle)
{
	m_bundleList.remove(bundle);
	delete bundle;
	return false;
}

bool AssetManager::unloadAll()
{
	if (m_bundleList.empty()) {
		return true;
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
	return false;
}

FeImage* AssetManager::loadImage(ImageToken token, const std::string& custom, Frontend* frontend)
{
	FeImage* target {};
	for (auto* bundle : m_bundleList) {
		target = bundle->loadImage(token, custom, frontend);
		if (target != nullptr && !target->error()) {
			break;
		}
	}
	return target;
}

FeImage* AssetManager::loadImage(const ImageToken token, PuyoCharacter character, Frontend* frontend)
{
	FeImage* target {};
	for (auto* bundle : m_bundleList) {
		target = bundle->loadCharImage(token, character, frontend);
		if (target != nullptr && !target->error()) {
			break;
		}
	}
	return target;
}

FeSound* AssetManager::loadSound(const SoundEffectToken token, const std::string& custom, Frontend* frontend)
{
	FeSound* target = {};
	for (auto* bundle : m_bundleList) {
		target = bundle->loadSound(token, custom, frontend);
		if (!target->error()) {
			// target->play();
			break;
		} else {
			target = nullptr;
		}
	}
	return target;
}

FeSound* AssetManager::loadSound(const SoundEffectToken token, PuyoCharacter character, Frontend* frontend)
{
	FeSound* target = {};
	for (auto* bundle : m_bundleList) {
		target = bundle->loadCharSound(token, character, frontend);
		if (!target->error()) {
			break;
		} else {
			target = nullptr;
		}
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
	return target;
}

std::string AssetManager::getAnimationFolder(AnimationToken token, const std::string& script_name)
{
	std::string target;
	for (auto bundle : m_bundleList) {
		target = bundle->getAnimationFolder(token, script_name);
		if (!target.empty()) {
			break;
		}
	}
	return target;
}

std::set<std::string> AssetManager::listPuyoSkins()
{
	std::set<std::string> result {};
	for (auto bundle : m_bundleList) {
		for (const auto& value : bundle->listPuyoSkins()) {
			result.insert(value);
		}
	}
	return result;
}

std::set<std::string> AssetManager::listBackgrounds()
{
	std::set<std::string> result {};
	for (auto bundle : m_bundleList) {
		for (const auto& value : bundle->listBackgrounds()) {
			result.insert(value);
		}
	}
	return result;
}

std::set<std::string> AssetManager::listCharacterSkins()
{
	std::set<std::string> result {};
	for (auto bundle : m_bundleList) {
		for (const auto& value : bundle->listCharacterSkins()) {
			result.insert(value);
		}
	}
	return result;
}

std::set<std::string> AssetManager::listSfx()
{
	std::set<std::string> result {};
	for (auto bundle : m_bundleList) {
		for (const auto& value : bundle->listSfx()) {
			result.insert(value);
		}
	}
	return result;
}

bool AssetManager::reloadBundles()
{
	int number_of_loaded = 0;
	for (auto bundle : m_bundleList) {
		assert(bundle!= nullptr);
		bundle->reload();
		number_of_loaded++;
	}
	return number_of_loaded;
}
bool AssetManager::reloadBundles(GameAssetSettings* settings) {
	int number_of_loaded = 0;
	for (auto bundle: m_bundleList) {
		assert(bundle!= nullptr);
		if (bundle->affectedByUser()) {
			bundle->reload(settings);
			number_of_loaded++;
		}
	}
	assert(number_of_loaded);
	return number_of_loaded;
}

AssetManagerPriv::AssetManagerPriv(AssetManager* parent, Frontend* frontend, DebugLog* dbg)
	: own_parent(parent)
	, m_frontend(frontend)
	, m_debug(dbg)
{
	allow_loading();
};

void AssetManagerPriv::activate(ppvs::Frontend* fe, ppvs::DebugLog* debug)
{
	m_frontend = fe;
	m_debug = debug;
	// Do not activate unless we know we won't call nullptr
	if (m_frontend != nullptr && m_debug != nullptr) {
		activated = true;
	} else {
		activated = false;
	}
}

// Attempts to load the image with the token (see FolderAssetBundle in AssetBundle.h)
// parameter `custom` will replace %custom% tag in pseudo-filename
FeImage* AssetManagerPriv::loadImage(ImageToken token, const std::string& custom)
{
	while (load_lock) { };
	auto* target = own_parent->loadImage(token, custom, m_frontend);
	if (target == nullptr || target->error()) {
		m_debug->log("Error loading image token " + toString(static_cast<int>(token)) + " custom " + custom, DebugMessageType::Error);
	}
	return target;
}

// Attempts to load the sound with the token (see FolderAssetBundle in AssetBundle.h)
// parameter `custom` will replace %custom% tag in pseudo-filename
FeSound* AssetManagerPriv::loadSound(SoundEffectToken token, const std::string& custom)
{
	while (load_lock) { };
	auto* target = own_parent->loadSound(token, custom, m_frontend);
	if (target == nullptr || target->error()) {
		m_debug->log("Error loading sound token " + toString(static_cast<int>(token)) + " custom " + custom, DebugMessageType::Error);
	}
	return target;
}

// Attempts to load the character-based image with the token (see FolderAssetBundle in AssetBundle.h)
FeImage* AssetManagerPriv::loadImage(ImageToken token, PuyoCharacter character)
{
	while (load_lock) { };
	auto* target = own_parent->loadImage(token, character, m_frontend);
	if (target == nullptr || target->error()) {
		m_debug->log("Error loading image token " + toString(static_cast<int>(token)) + " character " + std::to_string(static_cast<int>(character)), DebugMessageType::Error);
	}
	return target;
};
// Attempts to load the character-based sound with the token (see FolderAssetBundle in AssetBundle.h)
FeSound* AssetManagerPriv::loadSound(SoundEffectToken token, PuyoCharacter character)
{
	while (load_lock) { };
	auto* target = own_parent->loadSound(token, character, m_frontend);
	if (target == nullptr || target->error()) {
		m_debug->log("Error loading sound token " + toString(static_cast<int>(token)) + " character " + std::to_string(static_cast<int>(character)), DebugMessageType::Error);
	}
	return target;
};

// Attempts to locate a viable folder for animation files
// Takes a token and filename of script
std::string AssetManagerPriv::getAnimationFolder(AnimationToken token, const std::string& script_name)
{
	auto target = own_parent->getAnimationFolder(token, script_name);
	if (target.empty()) {
		m_debug->log("Error loading animation script token " + toString(static_cast<int>(token)), DebugMessageType::Error);
	}
	return target;
}
// Attempts to locate a viable folder for animation files
// Takes a character, setup is assumed
std::string AssetManagerPriv::getAnimationFolder(ppvs::PuyoCharacter character)
{
	auto target = own_parent->getAnimationFolder(character);
	if (target.empty()) {
		m_debug->log("Error loading animation script character " + toString(static_cast<int>(character)), DebugMessageType::Error);
	}
	return target;
}

} // ppvs

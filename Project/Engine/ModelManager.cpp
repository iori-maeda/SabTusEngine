#include "ModelManager.h"

#include <cassert>
#include <format>

#include "StringUtility.h"
#include "Logger.h"
#include "DirectXCommon.h"

uint32_t ModelManager::modelCount = 0;
std::string ModelManager::defaultDirectoryPath = "Resources/Models/";

ModelManager &ModelManager::GetInstace()
{
	static ModelManager instance;
	return instance;
}

void ModelManager::Initialize(DirectXCommon *dxCommon)
{
	assert(dxCommon != nullptr);
	dxCommon_ = dxCommon;
}

void ModelManager::Finalize()
{
	models_.clear();
}

Model *ModelManager::Load(const std::string &fileName)
{

	// 読み込み済みなら要素を返して早期リターン
	auto modelData = models_.find(fileName);
	if (modelData != models_.end())
	{
		Logger::Log(std::format("ModelManager::Search::{}\n", fileName));
		return modelData->second.get();
	}

	Logger::Log(std::format("ModelManager::Loading::{}\n", fileName));
	Model* loadModel = Load(defaultDirectoryPath, fileName);

	models_.emplace(fileName, std::move(loadModel));
	modelCount++;

	return loadModel;
}

Model *ModelManager::Load(const std::string &directoryPath, const std::string &fileName)
{

	// 読み込み済みなら要素を返して早期リターン
	auto modelData = models_.find(fileName);
	if (modelData != models_.end())
	{
		Logger::Log(std::format("ModelManager::Search::{}\n", fileName));
		return modelData->second.get();
	}

	Logger::Log(std::format("ModelManager::Loading::{}\n", fileName));
	std::unique_ptr<Model> loadModel = std::make_unique<Model>();
	loadModel->LoadModelFile(directoryPath , fileName);
	loadModel->Initialize(dxCommon_);

	models_.emplace(fileName, std::move(loadModel));
	modelCount++;

	return GetModel(fileName);
}

Model *ModelManager::GetModel(const std::string &fileName)
{
	// 読み込み済みなら要素を返して早期リターン
	auto modelData = models_.find(fileName);
	if (modelData != models_.end())
	{
		Logger::Log(std::format("ModelManager::SearchSuccess::{}\n", fileName));
		return modelData->second.get();
	}

	Logger::Log(std::format("ModelManager::SearchFailed::{}\n", fileName));
	assert(nullptr);
	return nullptr;
}
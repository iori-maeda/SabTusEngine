#include "DxCommand.h"

#include "DxDevice.h"
#include "../Logger.h"
#include "../StringUtility.h"
#include <cassert>

void DxCommand::Initialize(DxDevice* device)
{
	// コマンドキュー生成
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	HRESULT hr = device->GetDevice()->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
	// コマンドキュー生成確認
	assert(SUCCEEDED(hr));
	Logger::Log("CreateCommandQueue\n");
	// コマンドアロケータ生成
	hr = device->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	// コマンドアロケータ生成確認
	assert(SUCCEEDED(hr));
	Logger::Log("CreateCommandAllocator\n");
	// コマンドリスト生成
	hr = device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
	// コマンドリスト生成確認
	assert(SUCCEEDED(hr));
	Logger::Log("CreateCommandList\n");
}

void DxCommand::CommandListCloseAndExecute()
{
	// コマンドリスト積込み終了
	HRESULT hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	// GPUにコマンドリストの実行依頼
	ID3D12CommandList* commandLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(1, commandLists);
}

void DxCommand::CommandListReset()
{
	// 次の準備
	HRESULT hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
}

ID3D12CommandQueue* DxCommand::GetCommandQueue()
{
	return commandQueue_.Get();
}

ID3D12GraphicsCommandList* DxCommand::GetCommandList()
{
	return commandList_.Get();
}

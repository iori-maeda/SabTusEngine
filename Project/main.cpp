#include <Windows.h>
#include <memory>

// MyCrassies
#include "GameClass/BaseGame.h"

// Utility
#include "Logger.h"
#include "StringUtility.h"

#include <xaudio2.h>
#pragma comment (lib, "xaudio2.lib")
#include <fstream>
#include <wrl.h>
#include <cstdint>
#include <string>
#include <cassert>

struct ChunkHeader
{
	char id[4]{}; //ChunkID
	int32_t size; // ChunkSize
};

struct RIFFHeader
{
	ChunkHeader chunk{};//RIFF
	char type[4]{};		// WAVE
};

struct FomratChunk
{
	ChunkHeader chunk{};	// "fmt"
	WAVEFORMATEX fmt{};		// 波形フォーマット
};

struct SoundData
{
	WAVEFORMATEX wfex{};		// 波形フォーマット
	BYTE *pBuffer = nullptr;	// バッファ先頭アドレス
	int32_t bufferSize = 0;	// バッファサイズ
};

SoundData LoadSoundWave(const std::string &filePath)
{
	//HRESULT hr;

	// ファイル展開
	std::ifstream file{};
	// ファイルをバイナリデータとして展開
	file.open(filePath, std::ios_base::binary);
	assert(file.is_open());

	// .wave読み込み
#pragma region RIFF
	// RIFFの読込
	RIFFHeader RIFF{};
	file.read((char *)&RIFF, sizeof(RIFF));
	// RIFFか確認
	if (std::strncmp(RIFF.chunk.id, "RIFF", 4) != 0)
	{
		assert(0 && "not read RIFF");
	}
	// .waveか確認
	if (std::strncmp(RIFF.type, "WAVE", 4) != 0)
	{
		assert(0 && "not open WAVE");
	}
#pragma endregion

#pragma region Format
	FomratChunk format{};
	// fmtか確認
	file.read((char *)&format, sizeof(ChunkHeader));

	if (std::strncmp(format.chunk.id, "fmt ", 4) != 0)
	{
		assert(0 && "not read fmt");
	}

	// Chunk本体の読込
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char *)&format.fmt, format.chunk.size);
#pragma endregion

#pragma region DataChunk
	ChunkHeader data{};
	file.read((char *)&data, sizeof(data));

	if (std::strncmp(data.id, "JUNK", 4) == 0)
	{
		// JUNK検出時は終了まで読み飛ばし
		file.seekg(data.size, std::ios_base::cur);

		// 再読み込み
		file.read((char *)&data, sizeof(data));
	}

	if (std::strncmp(data.id, "data", 4) != 0)
	{
		assert(0 && "not read data");
	}

	// Dataチャンクの波形データの取得
	char *pBuffer = new char[data.size];
	file.read(pBuffer, data.size);
#pragma endregion
	// ファイルクローズ
	file.close();

	// サウンドデータの返却
	SoundData soundData{
		.wfex = format.fmt,
		.pBuffer = reinterpret_cast<BYTE *>(pBuffer),
		.bufferSize = data.size
	};

	return soundData;
}

void SoundUnload(SoundData *soundData)
{
	// バッファのメモリ解放
	delete[] soundData->pBuffer;

	soundData = {};
}

void SoundPlayWave(IXAudio2 *xAudio2, const SoundData &soundData)
{
	HRESULT hr;

	// 波形データのフォーマットに従ってソースの生成
	IXAudio2SourceVoice *voiceSource = nullptr;
	hr = xAudio2->CreateSourceVoice(&voiceSource, &soundData.wfex);
	assert(SUCCEEDED(hr));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buff{};
	buff.pAudioData = soundData.pBuffer;
	buff.AudioBytes = soundData.bufferSize;
	buff.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	hr = voiceSource->SubmitSourceBuffer(&buff);
	hr = voiceSource->Start();
}

using namespace std;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{

	unique_ptr<BaseGame> baseGame = make_unique<BaseGame>();
	baseGame->Initialize();

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2 = nullptr;
	IXAudio2MasteringVoice *masterVoice = nullptr;
	// XAudio2エンジンのインスタンス生成	
	HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	// マスターボイスの生成
	hr = xAudio2->CreateMasteringVoice(&masterVoice);

	SoundData soundData = LoadSoundWave("Resources/Sounds/Alarm01.wav");

	SoundPlayWave(xAudio2.Get(), soundData);

	// GameLoop
	while (!baseGame->EndRequest())
	{
		baseGame->Upate();
		baseGame->Draw();
	}

	xAudio2.Reset();
	SoundUnload(&soundData);
	baseGame->Finalize();
	return 0;
}
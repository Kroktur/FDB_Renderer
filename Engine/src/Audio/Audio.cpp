#include "Audio/Audio.h"

#include <stdexcept>

void FDB::Audio::Soloud::Init()
{
	m_music.init();
}

void FDB::Audio::Soloud::Destroy()
{
	m_music.deinit();
}

FDB::Audio::Wav::Wav()
{
}

FDB::Audio::Wav::Wav(SoLoud::Wav& wav) : m_wav(&wav)
{

}

void FDB::Audio::Wav::SetWav(SoLoud::Wav& wav)
{
	m_wav = &wav;
}

void FDB::Audio::Wav::SetLoop(bool isLooping)
{
	ErrorValidWav();
	m_wav->setLooping(isLooping);
}

bool FDB::Audio::Wav::IsOver() const
{
	ErrorValidWav();
	return !IsPlaying();
}

void FDB::Audio::Wav::SetVolume(float volume)
{
	ErrorValidWav();
	Soloud::m_music.setVolume(m_handle, volume);
}

float FDB::Audio::Wav::GetVolume() const
{
	return Soloud::m_music.getVolume(m_handle);
}

void FDB::Audio::Wav::Play()
{
	ErrorValidWav();
	if (IsPlaying())
		throw std::out_of_range("already playing");

	m_handle = static_cast<int>(Soloud::m_music.play(*m_wav));
}

void FDB::Audio::Wav::Pause()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	if (!IsPause())
		Soloud::m_music.setPause(m_handle, true);
}

void FDB::Audio::Wav::Resume()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	if (IsPause())
		Soloud::m_music.setPause(m_handle, false);
}

void FDB::Audio::Wav::Stop()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	Soloud::m_music.stop(m_handle);
	m_handle = -1;
}

bool FDB::Audio::Wav::IsPause() const
{
	return Soloud::m_music.getPause(m_handle);
}

bool FDB::Audio::Wav::IsPlaying() const
{
	return m_handle != -1 && Soloud::m_music.isValidVoiceHandle(m_handle);
}

void FDB::Audio::Wav::ErrorValidWav() const
{
	if (!m_wav)
		throw std::runtime_error("no wav set");
}

void FDB::Audio::Wav::ErrorMusicNotPlay() const
{
	if (!IsPlaying())
		throw std::out_of_range("music not play");
}



FDB::Audio::WavStream::WavStream()
{

}

FDB::Audio::WavStream::WavStream(SoLoud::WavStream& wav) : m_wav(&wav)
{

}

void FDB::Audio::WavStream::SetWav(SoLoud::WavStream& wav)
{
	m_wav = &wav;
}

void FDB::Audio::WavStream::SetLoop(bool isLooping)
{
	ErrorValidWav();
	m_wav->setLooping(isLooping);
}

bool FDB::Audio::WavStream::IsOver() const
{
	ErrorValidWav();
	return !IsPlaying();
}

void FDB::Audio::WavStream::SetVolume(float volume)
{
	ErrorValidWav();
	Soloud::m_music.setVolume(m_handle, volume);
}

float FDB::Audio::WavStream::GetVolume() const
{
	return Soloud::m_music.getVolume(m_handle);
}

void FDB::Audio::WavStream::Play()
{
	ErrorValidWav();
	if (IsPlaying())
		throw std::out_of_range("already playing");

	m_handle = static_cast<int>(Soloud::m_music.play(*m_wav));
}

void FDB::Audio::WavStream::Pause()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	if (!IsPause())
		Soloud::m_music.setPause(m_handle, true);
}

void FDB::Audio::WavStream::Resume()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	if (IsPause())
		Soloud::m_music.setPause(m_handle, false);
}

void FDB::Audio::WavStream::Stop()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	Soloud::m_music.stop(m_handle);
	m_handle = -1;
}

bool FDB::Audio::WavStream::IsPause() const
{
	return Soloud::m_music.getPause(m_handle);
}

bool FDB::Audio::WavStream::IsPlaying() const
{
	return m_handle != -1 && Soloud::m_music.isValidVoiceHandle(m_handle);
}

void FDB::Audio::WavStream::ErrorValidWav() const
{
	if (!m_wav)
		throw std::runtime_error("no wav set");
}

void FDB::Audio::WavStream::ErrorMusicNotPlay() const
{	
	if (!IsPlaying())
		throw std::out_of_range("music not play");
}

std::unique_ptr<SoLoud::WavStream> FDB::Audio::AudioHelper::LoadWavStream(const std::filesystem::path& path)
{
	std::unique_ptr<SoLoud::WavStream> sound = std::make_unique<SoLoud::WavStream>();
	if (sound->load(path.string().c_str()) != SoLoud::SO_NO_ERROR)
	{
		throw std::runtime_error("file not found");
	}
	return std::move(sound);
}

std::unique_ptr<SoLoud::Wav> FDB::Audio::AudioHelper::LoadWav(const std::filesystem::path& path)
{
	std::unique_ptr<SoLoud::Wav> sound = std::make_unique<SoLoud::Wav>();
	if (sound->load(path.string().c_str()) != SoLoud::SO_NO_ERROR)
	{
		throw std::runtime_error("file not found");
	}
	return std::move(sound);
}


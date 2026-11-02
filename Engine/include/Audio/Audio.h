#ifndef FDBAUDIOAUDIOF
#define FDBAUDIOAUDIOF

#include <memory>

#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"

#include "Core/ResourceManager.h"

namespace FDB::Audio
{

    struct Soloud
    {
        static void Init();
        static void Destroy();
        static SoLoud::Soloud m_music;
    };
	inline SoLoud::Soloud Soloud::m_music = SoLoud::Soloud{};
    struct Wav
    {
        
        Wav();

       
        Wav(SoLoud::Wav& wav);

       
        void SetWav(SoLoud::Wav& wav);

        
        void SetLoop(bool isLooping);

        
        bool IsOver() const;

       
        void SetVolume(float volume);

        
        float GetVolume() const;

       
        void Play();

        void Pause();

       
        void Resume();

       
        void Stop();

       
        bool IsPause() const;

        
        bool IsPlaying() const;

      
       

    private:
       
        void ErrorValidWav() const;
        void ErrorMusicNotPlay() const;

        int m_handle = -1;              ///< Playback handle returned by SoLoud.
        SoLoud::Wav* m_wav = nullptr;   ///< Pointer to the associated WAV resource.

    };



   

    struct WavStream
    {
      
        WavStream();
        WavStream(SoLoud::WavStream& wav);

        void SetWav(SoLoud::WavStream& wav);
        void SetLoop(bool isLooping);

        bool IsOver() const;

       
        void SetVolume(float volume);

      
        float GetVolume() const;

        void Play();

       
        void Pause();

        
        void Resume();

       
        void Stop();

      
        bool IsPause() const;

       
        bool IsPlaying() const;

        

    private:
        void ErrorValidWav() const;
        void ErrorMusicNotPlay() const;
        int m_handle = -1;                   
        SoLoud::WavStream* m_wav = nullptr; 
           
    };


   

    struct AudioHelper
    {
	    static std::unique_ptr<SoLoud::WavStream> LoadWavStream(const std::filesystem::path& path);
        static std::unique_ptr<SoLoud::Wav> LoadWav(const  std::filesystem::path& path);
    };

    using wavManager_t = FDB::Core::resourceManager_t<SoLoud::Wav, FDB::Core::TypeWrapper<>, AudioHelper::LoadWav>;
    using wavStreamManager_t = FDB::Core::resourceManager_t<SoLoud::WavStream, FDB::Core::TypeWrapper<>, AudioHelper::LoadWavStream>;
}

#endif

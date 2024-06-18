#include "Audio/Sound.hpp"

#include "Core/Exception.hpp"
#include "Audio/AudioController.hpp"

#define DR_MP3_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION
#include "dr_mp3.h"
#include "dr_wav.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
#include "opusfile.h"

#include <filesystem>
#include <fstream>

#define RETHROW_EXCEPTION(x)                              \
	try {                                                 \
		x                                                 \
	} catch(...) {                                        \
		std::rethrow_exception(std::current_exception()); \
	}

//This is constant for now across all Opus files but this could potentially change in the future
#define OPUS_SAMPLE_RATE 48000

namespace Cacao {
	Sound::Sound(std::string filePath)
	  : Asset(false), filePath(filePath) {
		CheckException(std::filesystem::exists(filePath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot load sound from nonexistent file!")

		//Determine audio file format by reading header

		//Read the first four bytes of the file
		std::string header(4, '\0');
		std::ifstream file(filePath, std::ios::binary);
		if(!file.is_open()) {
			CheckException(false, Exception::GetExceptionCodeFromMeaning("FileOpenFailure"), "Failed to open the sound file!");
		}
		file.read(header.data(), header.size());

		bool goodFormat = false;

		//Check for MP3
		//These can either just start with a frame or have ID3 data, so we check both
		{
			std::string mp3 = header.substr(0, 3);

			//The weird binary bit is checking for the sync instruction that all MP3 frames start with
			if(mp3.compare("ID3") == 0 || (static_cast<unsigned char>(header[0]) == 0xFF && (static_cast<unsigned char>(header[1]) & 0xE0) == 0xE0)) {
				goodFormat = true;
				RETHROW_EXCEPTION(_InitMP3();)
			}
		}

		//Check for WAV
		if(header.compare("RIFF") == 0) {
			//Read a bit more to confirm WAV
			std::string waveHeader(4, '\0');
			file.seekg(8, std::ios::beg);
			file.read(&waveHeader[0], waveHeader.size());
			if(waveHeader == "WAVE") {
				goodFormat = true;
				RETHROW_EXCEPTION(_InitWAV();)
			}
		}

		//Check for Ogg stream (Vorbis/Opus)
		if(header.compare("OggS") == 0) {
			//Read the Ogg Header
			std::string oggHeader(64, '\0');
			file.seekg(0, std::ios::beg);
			file.read(&oggHeader[0], oggHeader.size());
			file.close();

			if(oggHeader.find("vorbis") != std::string::npos) {
				goodFormat = true;
				RETHROW_EXCEPTION(_InitVorbis();)
			} else if(oggHeader.find("OpusHead") != std::string::npos) {
				goodFormat = true;
				RETHROW_EXCEPTION(_InitOpus();)
			}
		}

		//Close the file
		file.close();

		//Now we handle the potential bad header circumstance because we have closed the file
		CheckException(goodFormat, Exception::GetExceptionCodeFromMeaning("WrongType"), "The provided sound file is of an unsupported format!")
	}

	std::shared_future<void> Sound::Compile() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled sound!")
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to compile a sound!")

		//Create buffer object
		alGenBuffers(1, &buf);

		//Load buffer with audio data
		alBufferData(buf, channelCount > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, audioData.data(), audioData.size() * sizeof(signed short), sampleRate);

		//Register a release event
		sec = new SignalEventConsumer([this](Event& e, std::promise<void>& p) {
			this->Release();
			p.set_value();
		});
		EventManager::GetInstance()->SubscribeConsumer("AudioShutdown", sec);

		compiled = true;

		//Return an already completed future
		std::promise<void> p;
		p.set_value();
		return p.get_future().share();
	}

	void Sound::Release() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled sound!")
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to compile a sound!")

		//Delete buffer object
		alDeleteBuffers(1, &buf);

		//Unregister release event
		EventManager::GetInstance()->UnsubscribeConsumer("AudioShutdown", sec);
		delete sec;

		compiled = false;
	}

	void Sound::_InitMP3() {
		//Open the MP3
		drmp3 mp3;
		CheckException(drmp3_init_file(&mp3, filePath.c_str(), NULL), Exception::GetExceptionCodeFromMeaning("IO"), "Failed to load MP3 sound file!");

		//Get file info
		sampleRate = mp3.sampleRate;
		channelCount = mp3.channels;

		//Read PCM frames
		std::vector<drmp3_int16> pcm;
		drmp3_uint64 totalFrames = drmp3_get_pcm_frame_count(&mp3);
		drmp3_uint64 framesRead = drmp3_read_pcm_frames_s16(&mp3, totalFrames, pcm.data());
		sampleCount = framesRead * mp3.channels;
		CheckException(framesRead == totalFrames, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to read MP3 frames!");

		//Close the MP3
		drmp3_uninit(&mp3);
	}

	void Sound::_InitWAV() {
		//Open the WAV
		drwav wave;
		CheckException(drwav_init_file(&wave, filePath.c_str(), NULL), Exception::GetExceptionCodeFromMeaning("IO"), "Failed to load WAV sound file!");

		//Get file info
		sampleRate = wave.sampleRate;
		channelCount = wave.channels == 1;

		//Read PCM frames
		std::vector<drmp3_int16> pcm;
		drmp3_uint64 framesRead = drwav_read_pcm_frames_s16(&wave, wave.totalPCMFrameCount, pcm.data());
		sampleCount = framesRead * wave.channels;
		CheckException(framesRead == wave.totalPCMFrameCount, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to read WAV frames!");

		//Close the WAV
		drwav_uninit(&wave);
	}

	void Sound::_InitVorbis() {
		//Define some variables for reading the file
		OggVorbis_File vf;
		int currentSection;

		//Open the file
		FILE* f = fopen(filePath.c_str(), "rb");
		CheckException(ov_open(f, &vf, NULL, 0) >= 0, Exception::GetExceptionCodeFromMeaning("FileOpenFailure"), "Failed to open Ogg Vorbis sound file!");

		//Get file info
		vorbis_info* info = ov_info(&vf, -1);
		sampleRate = info->rate;
		channelCount = info->channels;
		sampleCount = ov_pcm_total(&vf, -1);

		//Read the audio data
		while(true) {
			signed short pcm[4096];
			long readResult = ov_read(&vf, (char*)pcm, sizeof(pcm), 0, 2, 1, &currentSection);

			//Should neither of these conditions be met, everything is fine
			if(readResult == 0) {
				//End of file, so exit the loop
				break;
			} else if(readResult < 0) {
				//Oh no, an error!
				ov_clear(&vf);
				CheckException(false, Exception::GetExceptionCodeFromMeaning("IO"), std::string("Ogg Vorbis file reading error, result ") + std::to_string(readResult))
			}

			//Add the PCM data to the end
			audioData.insert(audioData.end(), std::begin(pcm), std::end(pcm));
		}

		//Clean up Ogg Vorbis
		ov_clear(&vf);
	}

	void Sound::_InitOpus() {
		//Open the file
		int openError;
		OggOpusFile* opus = op_open_file(filePath.c_str(), &openError);

		//Get file info
		sampleRate = OPUS_SAMPLE_RATE;
		channelCount = op_channel_count(opus, -1);
		sampleCount = op_pcm_total(opus, -1);

		//Read the audio data
		while(true) {
			signed short pcm[4096];
			long readResult = op_read_stereo(opus, pcm, sizeof(pcm));

			//Should neither of these conditions be met, everything is fine
			if(readResult == 0) {
				//End of file, so exit the loop
				break;
			} else if(readResult < 0) {
				//Oh no, an error!
				op_free(opus);
				CheckException(false, Exception::GetExceptionCodeFromMeaning("IO"), std::string("Opus file reading error, result ") + std::to_string(readResult))
			}

			//Add the PCM data to the end
			audioData.insert(audioData.end(), std::begin(pcm), std::end(pcm));
		}

		//Clean up opusfile
		op_free(opus);
	}
}
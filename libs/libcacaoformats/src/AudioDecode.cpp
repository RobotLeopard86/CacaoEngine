#include "AudioDecode.hpp"

#include "CheckException.hpp"
#include "libcacaoformats/libcacaoformats.hpp"

#define DR_MP3_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION
#include "dr_mp3.h"
#include "dr_wav.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
#include "opusfile.h"

#include <fstream>

//This is constant for now across all Opus files when using libopusfile but this could potentially change in the future
#define OPUSFILE_SAMPLE_RATE 48000

namespace libcacaoformats {
	AudioBuffer MP3Decode(std::vector<unsigned char> encoded) {
		//Create return object
		AudioBuffer abuf;

		//Open the MP3
		drmp3 mp3;
		CheckException(drmp3_init_memory(&mp3, encoded.data(), encoded.size(), nullptr), "Failed to load MP3 sound data!");

		//Get file info
		abuf.sampleRate = mp3.sampleRate;
		abuf.channelCount = mp3.channels;
		drmp3_uint64 totalPCMFrameCount = drmp3_get_pcm_frame_count(&mp3);
		abuf.sampleCount = totalPCMFrameCount * abuf.channelCount;

		//Read PCM frames
		abuf.data.resize(abuf.sampleCount);
		drmp3_uint64 framesRead = drmp3_read_pcm_frames_s16(&mp3, totalPCMFrameCount, abuf.data.data());
		CheckException(framesRead == totalPCMFrameCount, "Failed to read MP3 PCM frames!");

		//Close the MP3
		drmp3_uninit(&mp3);

		return abuf;
	}

	AudioBuffer WAVDecode(std::vector<unsigned char> encoded) {
		//Create return object
		AudioBuffer abuf;

		//Open the WAV
		drwav wave;
		CheckException(drwav_init_memory(&wave, encoded.data(), encoded.size(), nullptr), "Failed to load WAV sound data!");

		//Get file info
		abuf.sampleRate = wave.sampleRate;
		abuf.channelCount = wave.channels;
		drwav_uint64 totalPCMFrameCount = wave.totalPCMFrameCount;
		abuf.sampleCount = totalPCMFrameCount * abuf.channelCount;

		//Read PCM frames
		abuf.data.resize(abuf.sampleCount);
		drwav_uint64 framesRead = drwav_read_pcm_frames_s16(&wave, totalPCMFrameCount, abuf.data.data());
		CheckException(framesRead == totalPCMFrameCount, "Failed to read WAV frames!");

		//Close the WAV
		drwav_uninit(&wave);

		return abuf;
	}

	AudioBuffer VorbisDecode(std::vector<unsigned char> encoded) {
		//Create return object
		AudioBuffer abuf;

		//Define some variables for decoding
		OggVorbis_File vf;
		int currentSection;

		//Create a tracker for the Vorbis data (fake file so we can use libvorbisfile instead of the low-level Vorbis API)
		struct VorbisTracker {
			const unsigned char* data;
			std::size_t size;
			std::size_t offset;
		} tracker {.data = encoded.data(), .size = encoded.size(), .offset = 0};

		//Define Vorbis IO read callback
		const auto read_callback = [](void* out, size_t size, size_t elems, void* src) {
			VorbisTracker* tracker = static_cast<VorbisTracker*>(src);

			//Calculate byte read count
			size_t readBytes = size * elems;
			if(tracker->offset + readBytes > tracker->size) readBytes = tracker->size - tracker->offset;

			//Copy memory
			std::memcpy(out, tracker->data + tracker->offset, readBytes);

			//Advance offset and return
			tracker->offset += readBytes;
			return (tracker->offset == tracker->size ? 0 : readBytes / size);
		};

		//Open the Vorbis data
		ov_callbacks cb = OV_CALLBACKS_STREAMONLY_NOCLOSE;
		cb.read_func = read_callback;
		ov_open_callbacks(&tracker, &vf, nullptr, 0, cb);

		//Get file info
		vorbis_info* info = ov_info(&vf, -1);
		abuf.sampleRate = info->rate;
		abuf.channelCount = info->channels;
		abuf.sampleCount = ov_pcm_total(&vf, -1);

		//Read the audio data
		while(true) {
			short pcm[4096];
			long samplesRead = ov_read(&vf, reinterpret_cast<char*>(pcm), sizeof(pcm), 0, 2, 1, &currentSection);

			if(samplesRead == 0) {
				//End of file, so exit the loop
				break;
			} else if(samplesRead < 0) {
				//Oh no, an error!
				ov_clear(&vf);
				CheckException(false, "Failed to read Ogg Vorbis file!");
			} else {
				//Add the PCM data to the end
				abuf.data.insert(abuf.data.end(), pcm, pcm + (samplesRead / abuf.channelCount));
			}
		}

		//Clean up Ogg Vorbis
		ov_clear(&vf);

		return abuf;
	}

	AudioBuffer OpusDecode(std::vector<unsigned char> encoded) {
		//Create return object
		AudioBuffer abuf;

		//Open the file
		int openError;
		OggOpusFile* opus = op_open_memory(encoded.data(), encoded.size(), &openError);

		//Get file info
		abuf.sampleRate = OPUSFILE_SAMPLE_RATE;
		const OpusHead* head = op_head(opus, 0);
		abuf.channelCount = head->channel_count;
		abuf.sampleCount = op_pcm_total(opus, -1);

		//Read the audio data
		abuf.data.reserve(abuf.sampleCount * abuf.channelCount);
		while(true) {
			short pcm[4096];
			long samplesRead = op_read_stereo(opus, pcm, sizeof(pcm));

			if(samplesRead == 0) {
				//End of file, so exit the loop
				break;
			} else if(samplesRead < 0) {
				//Oh no, an error!
				op_free(opus);
				CheckException(false, "Failed to read Opus file!");
			} else {
				//Add the PCM data to the end
				abuf.data.insert(abuf.data.end(), pcm, pcm + (samplesRead * abuf.channelCount));
			}
		}

		//Clean up opusfile
		op_free(opus);

		return abuf;
	}

	AudioBuffer DecodeAudio(std::vector<unsigned char> encoded) {
		//Determine audio format by reading header

		//Grab the first four header bytes
		std::string header(4, '\0');
		std::memcpy(header.data(), encoded.data(), 4);

		//Check for MP3
		//These can either just start with a frame or have ID3 data, so we check both
		{
			std::string mp3 = header.substr(0, 3);

			//The weird binary bit is checking for the sync instruction that all MP3 frames start with
			if(mp3.compare("ID3") == 0 || (static_cast<unsigned char>(header[0]) == 0xFF && (static_cast<unsigned char>(header[1]) & 0xE0) == 0xE0)) {
				return MP3Decode(encoded);
			}
		}

		//Check for WAV
		if(header.compare("RIFF") == 0) {
			//Read a bit more to confirm WAV
			std::string waveHeader(4, '\0');
			std::memcpy(waveHeader.data(), encoded.data() + 4, 4);
			if(waveHeader == "WAVE") {
				return WAVDecode(encoded);
			}
		}

		//Check for Ogg stream (Vorbis/Opus)
		if(header.compare("OggS") == 0) {
			//Read the Ogg Header
			std::string oggHeader(64, '\0');
			std::memcpy(oggHeader.data(), encoded.data(), 64);

			if(oggHeader.find("vorbis") != std::string::npos) {
				return VorbisDecode(encoded);
			} else if(oggHeader.find("OpusHead") != std::string::npos) {
				return OpusDecode(encoded);
			}
		}

		//Now we throw the exception
		CheckException(false, "The provided sound data is of an unsupported format!");

		return {};
	}
}
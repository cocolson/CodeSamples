/* ======================================================================== /
/!
\file AudioSystem.cpp
\par Solaris Rift
\author Coby Colson
\par coby.colson@digipen.edu
\brief Implementation of the AudioSystem class.
This file contains the declaration of the AudioSystem class, which represents
the audio system in the application. The AudioSystem class is responsible for
managing and controlling all audio-related operations, including sound loading,
playing, and event handling.
\par
Course: GAM200-F20
COPYRIGHT (C) 2020-2021 DigiPen, All rights reserved.
/
/ ======================================================================== */

#include <FMOD/fmod_errors.h>
#include <EventSystem.h>
#include <BuiltInEvents.h>
#include <GameObject.h>
#include <Space.h>
#include <Component.h>
#include <EventSystem.h>
#include <InputManager.h>
#include <SpaceManager.h>
#include <InputEvents.h>
#include <Engine.h>
#include <Window.h>
#include <stdafx.h>
#include <AudioSystem.h>
#include <Logger.h>
#include <Utilities.h>

namespace DeckedOut
{
	static const char GUID_BANK_ID[] = "bank:/";
	static const char GUID_EVENT_ID[] = "event:/";
	static const char GUID_BUS_ID[] = "bus:/";
	static const char GUIDS_PATH[] = "Assets/Audio/Project/Build/GUIDs.txt";
	static const char BANKS_DIRECTORY_PATH[] = "Assets/Audio/Project/Build/Desktop/";
	static const char BANKS_FILE_EXT[] = ".bank";
	static constexpr int FMOD_MAX_CHANNELS = 64;
	static constexpr float OCTAVE_RATIO = 2.0f; //! Frequency ratio of an octave in a 12-tone temperament
	static constexpr float SEMITONE_RATIO = 1.0595f; //! Frequency ratio of a semitone in 12-tone temperament

	ChangeVolumeEvent::ChangeVolumeEvent(const std::string& busName, float volume) :
		Event("ChangeVolumeEvent"), busName_(busName), volume_(volume)
	{
	}

	AudioSystem::AudioSystem() :
		DeckedOutObject("AudioSystem"),
		sounds_(),
		sys_(nullptr),
		sysLow_(nullptr),
		channelGroups_()
	{
	}

	AudioSystem::~AudioSystem()
	{

	}

	void AudioSystem::Initialize()
	{
		// Create the FMOD system
		ReportFMODError(
			FMOD::Studio::System::create(&sys_));
#ifdef _DEBUG
		// Initialize with live-update settings if in debug mode
		ReportFMODError(
			sys_->initialize(FMOD_MAX_CHANNELS, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL, 0));
#endif
#ifndef _DEBUG
		// Initialize with normal settings if in release mode
		ReportFMODError(
			sys_->initialize(FMOD_MAX_CHANNELS, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0));
#endif
		// Access the the low-level system API
		ReportFMODError(
			sys_->getCoreSystem(&sysLow_));
		// Access the master channel group
		ReportFMODError(
			sysLow_->getMasterChannelGroup(&channelGroups_[(int)AudioChannelGroup::Master]));
		// Create a sound effects channel
		ReportFMODError(
			sysLow_->createChannelGroup("Sound", &channelGroups_[(int)AudioChannelGroup::Sound]));
		// Create a streamed audio channel
		ReportFMODError(
			sysLow_->createChannelGroup("Stream", &channelGroups_[(int)AudioChannelGroup::Stream]));

		InitializeStudio();

		unsigned int ver;
		sysLow_->getVersion(&ver);
		// Initialize the event systems.
		EventSystem::ConnectEvent(this, this, "ChangeVolumeEvent", &AudioSystem::HandleVolumeEvent);
		EventSystem::ConnectEvent(&InputManager::Instance(), this, "KeyTriggered", &AudioSystem::OnKeyTriggered);
		EventSystem::ConnectEvent(&SpaceManager::Instance(), this, "PauseScreenClosed", &AudioSystem::OnPauseScreenClosed);
	}
	
	void AudioSystem::InitializeStudio()
	{
		std::ifstream guidsFile(GUIDS_PATH);
		std::string line;

		if (guidsFile.is_open())
		{
			while (std::getline(guidsFile, line))
			{
				// Skip the unique identifier, advance to the named portion
				size_t begin = line.find('}') + 2;
				line = line.substr(begin);

				if (line.find(GUID_BANK_ID) != std::string::npos)
				{
					CreateBankPathFromGUID(line);
					FMOD::Studio::Bank* bank;
					ReportFMODError(
						sys_->loadBankFile(line.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank));
					banks_[line] = bank;
				}

				if (line.find(GUID_EVENT_ID) != std::string::npos)
				{
					// Load the event description
					FMOD::Studio::EventDescription* eventDescription;
					sys_->getEvent(line.c_str(), &eventDescription);
					eventDescriptions_[line] = eventDescription;

					// Load the event instance
					FMOD::Studio::EventInstance* eventInstance;
					eventDescriptions_[line]->createInstance(&eventInstance);
					eventInstances_[line] = eventInstance;
				}

				if (line.find(GUID_BUS_ID) != std::string::npos)
				{
					// Load the bus
					FMOD::Studio::Bus* bus;
					sys_->getBus(line.c_str(), &bus);
					buses_[line] = bus;
				}
			}
		}
		else
		{
			LogCritical("Invalid GUIDs file");
		}
		guidsFile.close();
	}

	void AudioSystem::Update(float dt)
	{
		UNREFERENCED_PARAMETER(dt);
		ReportFMODError(
			sys_->update());
	}

	void AudioSystem::Shutdown()
	{
		// Release all sounds
		auto sounds_it = sounds_.begin();
		while (sounds_it != sounds_.end())
		{
			ReportFMODError(
				sounds_it->second->release());
			sounds_it->second = nullptr;
			sounds_it++;
		}

		// Release all event descriptions/instances
		auto eventDescriptions_it = eventDescriptions_.begin();
		while (eventDescriptions_it != eventDescriptions_.end())
		{
			if (eventDescriptions_it->second != nullptr)
			{
				ReportFMODError(
					eventDescriptions_it->second->releaseAllInstances());
			}
			eventDescriptions_it->second = nullptr;
			eventDescriptions_it++;
		}

		// Release all banks
		auto banks_it = banks_.begin();
		while (banks_it != banks_.end())
		{
			ReportFMODError(
				banks_it->second->unload());
			banks_it->second = nullptr;
			banks_it++;
		}
		
		ReportFMODError(
			sysLow_->release());
	}
	
	void AudioSystem::LoadSound(const std::string& filename, bool loop, bool stream)
	{
		if (SoundIsLoaded(filename))
		{
			return;
		}

		FMOD_MODE mode = FMOD_DEFAULT;
		mode |= loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		mode |= stream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

		FMOD::Sound* sound = nullptr;
		ReportFMODError(
			sysLow_->createSound(filename.c_str(), mode, nullptr, &sound));
		if (sound)
		{
			sounds_[filename] = sound;
		}
	}

	void AudioSystem::UnloadSound(const std::string& filename)
	{
		if (sounds_.find(filename) == sounds_.end())
		{
			LogWarning("Tried to unload the sound '", filename, "', but this sound had never been loaded.");
			return;
		}

		ReportFMODError(
			sounds_[filename]->release());
		sounds_.erase(filename);
	}

	bool AudioSystem::SoundIsLoaded(const std::string& filename) const
	{
		return (sounds_.find(filename) != sounds_.end());
	}

	FMOD::Channel* AudioSystem::PlaySound(const std::string& filename, float volume, float pitch)
	{
		FMOD::Sound* sound = sounds_[filename];
		FMOD_MODE mode;
		ReportFMODError(
			sound->getMode(&mode));
		FMOD::ChannelGroup* channelGroup = (mode - FMOD_CREATESTREAM - FMOD_LOOP_NORMAL == FMOD_DEFAULT) ?
			channelGroups_[(int)AudioChannelGroup::Stream] : channelGroups_[(int)AudioChannelGroup::Sound];
		FMOD::Channel* channel;
		float frequency, newFrequency;

		ReportFMODError(
			sysLow_->playSound(sound, channelGroup, true, &channel));
		ReportFMODError(
			channel->setVolume(volume));
		ReportFMODError(
			channel->getFrequency(&frequency));
		newFrequency = ChangeSemitone(frequency, pitch);
		ReportFMODError(
			channel->setFrequency(newFrequency));
		ReportFMODError(
			channel->setPaused(false));

		return channel;
	}

	void AudioSystem::PlayEvent(const std::string& event)
	{
		if (eventInstances_.find(event) != eventInstances_.end())
		{
			ReportFMODError(
				eventInstances_[event]->start());
		}
		else
		{
			const char* eventWarning = "Tried to play an unknown FMOD studio event.";
			LogWarning(eventWarning);
			throw std::exception(eventWarning);
		}
	}

	void AudioSystem::StopEvent(const std::string& event)
	{
		if (eventInstances_.find(event) != eventInstances_.end())
		{
			ReportFMODError(
				eventInstances_[event]->stop(FMOD_STUDIO_STOP_IMMEDIATE));
		}
		else
		{
			const char* eventWarning = "Tried to stop an unknown FMOD studio event.";
			LogWarning(eventWarning);
			throw std::exception(eventWarning);
		}
	}

	void AudioSystem::StopAllEvents()
	{
		for (auto it = eventInstances_.begin(); it != eventInstances_.end(); ++it)
		{
			if (it->second != nullptr)
			{
				ReportFMODError(
					it->second->stop(FMOD_STUDIO_STOP_IMMEDIATE));
			}
		}
	}

	void AudioSystem::SetEventParameter(const std::string& event, const std::string& parameter, float value)
	{
		if (eventInstances_.find(event) != eventInstances_.end())
		{
			ReportFMODError(
				eventInstances_[event]->setParameterByName(parameter.c_str(), value));
			
		}
		else
		{
			const char* eventWarning = "Tried to play an unknown FMOD studio event.";
			LogWarning(eventWarning);
			//throw std::exception(eventWarning);
		}
	}

	bool AudioSystem::GetEventPlaying(const std::string& event)
	{
		if (eventInstances_.find(event) != eventInstances_.end())
		{
			FMOD_STUDIO_PLAYBACK_STATE state;
			ReportFMODError(
				eventInstances_[event]->getPlaybackState(&state));
			return (state != FMOD_STUDIO_PLAYBACK_STOPPED);

		}
		else
		{
			const char* eventWarning = "Tried to get data about an unknown FMOD studio event.";
			LogWarning(eventWarning);
			throw std::exception(eventWarning);
		}
	}

	void AudioSystem::SetChannelGroupVolume(AudioChannelGroup channelGroup, float volume)
	{
		volume = Clamp(volume, 0.0f, 1.0f);
		channelGroups_[(int)channelGroup]->setVolume(volume);
	}

	void AudioSystem::SetBusVolume(const std::string& bus, float volume)
	{
		if (buses_.find(bus) != buses_.end())
		{
			volume = Clamp(volume, 0.0f, 1.0f);
			buses_[bus]->setVolume(volume);
		}
		else
		{
			const char* eventWarning = "Tried to modify an unkown FMOD studio bus.";
			LogWarning(eventWarning);
			throw std::exception(eventWarning);
		}
	}

	float AudioSystem::GetBusVolume(const std::string& bus)
	{
		float volume = -1.0f;
		if (buses_.find(bus) != buses_.end())
		{
			buses_[bus]->getVolume(&volume);
		}
		else
		{
			const char* eventWarning = "Tried to read data from an unkown FMOD studio bus.";
			LogWarning(eventWarning);
			throw std::exception(eventWarning);
		}
		return volume;
	}

	void AudioSystem::SetBusPaused(const std::string& bus, bool pause)
	{
		if (buses_.find(bus) != buses_.end())
		{
			bool paused;
			buses_[bus]->getPaused(&paused);
			if (paused != pause)
			{
				buses_[bus]->setPaused(pause);
			}
		}
		else
		{
			const char* eventWarning = "Tried to modify an unkown FMOD studio bus.";
			LogWarning(eventWarning);
			throw std::exception(eventWarning);
		}
	}

	bool AudioSystem::GetBusPaused(const std::string& bus)
	{
		bool paused = false;
		if (buses_.find(bus) != buses_.end())
		{
			buses_[bus]->getPaused(&paused);
		}
		else
		{
			const char* eventWarning = "Tried to read data from an unkown FMOD studio bus.";
			LogWarning(eventWarning);
			throw std::exception(eventWarning);
		}
		return paused;
	}

	void AudioSystem::BusStopAllEvents(const std::string& bus)
	{
		if (buses_.find(bus) != buses_.end())
		{
			buses_[bus]->stopAllEvents(FMOD_STUDIO_STOP_IMMEDIATE);
		}
		else
		{
			const char* eventWarning = "Tried to modify an unkown FMOD studio bus.";
			LogWarning(eventWarning);
			throw std::exception(eventWarning);
		}
	}

	AudioSystem& AudioSystem::Instance()
	{
        static AudioSystem audioSystem;
		return audioSystem;
	}

	void AudioSystem::ReportFMODError(FMOD_RESULT err)
	{
		if (err != FMOD_OK)
		{
			const char* FMODError = FMOD_ErrorString(err);
			LogCritical("FMOD ERROR: ", FMODError);
			throw std::exception(FMODError);
		}
	}

	void AudioSystem::CreateBankPathFromGUID(std::string& guid)
	{
		size_t pathSuffix = guid.find_last_of('/') + 1;
		guid = BANKS_DIRECTORY_PATH + guid.substr(pathSuffix) + BANKS_FILE_EXT;
	}

	float AudioSystem::ChangeOctave(float frequency, float variation)
	{
		return frequency * pow(OCTAVE_RATIO, variation);
	}

	float AudioSystem::ChangeSemitone(float frequency, float variation)
	{
		return frequency * pow(SEMITONE_RATIO, variation);
	}

	void AudioSystem::OnKeyTriggered(const KeyboardButtonEvent* event)
	{
		if (event->key_ == InputManager::InputButton::KEY_ESCAPE)
		{
			SetEventParameter("event:/MUSIC/ForestMap/AdaptiveForestMusic", "parameter:/Pausing", 1.0f);
			SetEventParameter("event:/MUSIC/BossMap/Boss Music", "parameter:/Pausing", 1.0f);
		}
	}

	void AudioSystem::OnPauseScreenClosed(const NamedEvent* event)
	{
		UNREFERENCED_PARAMETER(event);
		SetEventParameter("event:/MUSIC/ForestMap/AdaptiveForestMusic", "parameter:/Pausing", 0.0f);
		SetEventParameter("event:/MUSIC/BossMap/Boss Music", "parameter:/Pausing", 0.0f);
	}

	void AudioSystem::MuteAllBuses()
	{
		for (auto bus = buses_.begin(); bus != buses_.end(); ++bus)
		{
			float volume = 0.0f;
			bus->second->getVolume(&volume);
			busVolumes_.push(std::make_pair(bus->first, volume));
			bus->second->setVolume(0.0f);
		}
	}

	void AudioSystem::UnmuteAllBuses()
	{
		while (!busVolumes_.empty())
		{
			std::pair<std::string, float> busVolume = busVolumes_.top();
			buses_[busVolume.first]->setVolume(busVolume.second);
			busVolumes_.pop();
		}
	}

	void AudioSystem::HandleVolumeEvent(const ChangeVolumeEvent* event)
	{
		SetBusVolume(event->busName_, event->volume_);
	}
}
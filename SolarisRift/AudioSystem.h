/* ======================================================================== /
/!
\file AudioSystem.h
\par Solaris Rift
\author Coby Colson
\par coby.colson@digipen.edu
\brief Header file for the AudioSystem class.
This file contains the declaration of the AudioSystem class, which represents
the audio system in the application. The AudioSystem class is responsible for
managing and controlling all audio-related operations, including sound loading,
playing, and event handling.
\par
Course: GAM200-F20
COPYRIGHT (C) 2020-2021 DigiPen, All rights reserved.
/
/ ======================================================================== */

#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <FMOD/fmod_studio.hpp>
#include <array>
#include <stack>
#include <Event.h>
#include <DeckedOutObject.h>
#include <AudioChannel.h>

namespace DeckedOut
{
	/**
	 * \brief Struct representing a keyboard button event.
	 */
	struct KeyboardButtonEvent;

	/**
	 * \brief Struct representing a named event.
	 */
	struct NamedEvent;

	/**
	 * \brief Enumeration representing different audio channel groups.
	 */
	enum class AudioChannelGroup
	{
		Master, //!< Master audio channel group.
		Sound,  //!< Sound audio channel group.
		Stream  //!< Stream audio channel group.
	};

	/**
	 * \brief Struct representing a volume change event.
	 *
	 * This event is triggered when the volume of an audio bus needs to be changed.
	 */
	struct ChangeVolumeEvent : public Event
	{
	public:
		std::string busName_; //!< Name of the audio bus.
		float volume_;        //!< New volume value.

		/**
		 * \brief Constructor for ChangeVolumeEvent.
		 * \param busName The name of the audio bus.
		 * \param volume The new volume value.
		 */
		ChangeVolumeEvent(const std::string& busName, float volume);
	};

	/**
	 * \brief Class representing the audio system.
	 *
	 * This class manages the audio system and provides functionalities to load, play, and control audio.
	 */
	class AudioSystem : public DeckedOutObject
	{
	public:
		/**
		 * \brief Initializes the audio system.
		 */
		void Initialize() override;

		/**
		 * \brief Updates the audio system.
		 * \param dt The time elapsed since the last update.
		 */
		void Update(float dt) override;

		/**
		 * \brief Shuts down the audio system.
		 */
		void Shutdown() override;

		/**
		 * \brief Loads a sound from a file.
		 * \param filename The name of the sound file to load.
		 * \param loop Specifies whether the sound should loop.
		 * \param stream Specifies whether the sound should be streamed.
		 */
		void LoadSound(const std::string& filename, bool loop = false, bool stream = false);

		/**
		 * \brief Unloads a previously loaded sound.
		 * \param filename The name of the sound file to unload.
		 */
		void UnloadSound(const std::string& filename);

		/**
		 * \brief Plays a sound.
		 * \param filename The name of the sound file to play.
		 * \param volume The volume of the sound.
		 * \param pitch The pitch of the sound.
		 * \return A pointer to the FMOD::Channel object representing the playing sound.
		 */
		FMOD::Channel* PlaySound(const std::string& filename, float volume, float pitch);

		/**
		 * \brief Checks if a sound is loaded.
		 * \param filename The name of the sound file to check.
		 * \return True if the sound is loaded, false otherwise.
		 */
		bool SoundIsLoaded(const std::string& filename) const;

		/**
		 * \brief Sets the volume of an audio channel group.
		 * \param channelGroup The audio channel group.
		 * \param volume The volume value.
		 */
		void SetChannelGroupVolume(AudioChannelGroup channelGroup, float volume);

		/**
		 * \brief Sets the volume of an audio bus.
		 * \param bus The name of the audio bus.
		 * \param volume The volume value.
		 */
		void SetBusVolume(const std::string& bus, float volume);

		/**
		 * \brief Gets the volume of an audio bus.
		 * \param bus The name of the audio bus.
		 * \return The volume value.
		 */
		float GetBusVolume(const std::string& bus);

		/**
		 * \brief Sets the paused state of an audio bus.
		 * \param bus The name of the audio bus.
		 * \param pause The paused state.
		 */
		void SetBusPaused(const std::string& bus, bool pause);

		/**
		 * \brief Gets the paused state of an audio bus.
		 * \param bus The name of the audio bus.
		 * \return The paused state.
		 */
		bool GetBusPaused(const std::string& bus);

		/**
		 * \brief Stops all events on an audio bus.
		 * \param bus The name of the audio bus.
		 */
		void BusStopAllEvents(const std::string& bus);

		/**
		 * \brief Plays an event in FMOD Studio.
		 * \param event The name of the event to play.
		 */
		void PlayEvent(const std::string& event);

		/**
		 * \brief Stops a playing event in FMOD Studio.
		 * \param event The name of the event to stop.
		 */
		void StopEvent(const std::string& event);

		/**
		 * \brief Stops all playing events in FMOD Studio.
		 */
		void StopAllEvents();

		/**
		 * \brief Sets a parameter value for an event in FMOD Studio.
		 * \param event The name of the event.
		 * \param parameter The name of the parameter.
		 * \param value The value to set for the parameter.
		 */
		void SetEventParameter(const std::string& event, const std::string& parameter, float value);

		/**
		 * \brief Checks if an event is currently playing in FMOD Studio.
		 * \param event The name of the event to check.
		 * \return True if the event is playing, false otherwise.
		 */
		bool GetEventPlaying(const std::string& event);

		/**
		 * \brief Initializes the FMOD Studio system.
		 */
		void InitializeStudio();

		/**
		 * \brief Reports an FMOD error.
		 * \param err The FMOD_RESULT error code.
		 * \throw An exception with the error message.
		 */
		static void ReportFMODError(FMOD_RESULT err) noexcept(false);

		/**
		 * \brief Creates a bank path from a GUID.
		 * \param guid The GUID string to create the bank path from.
		 */
		static void CreateBankPathFromGUID(std::string& guid);

		/**
		 * \brief Changes the octave of a frequency value.
		 * \param frequency The original frequency.
		 * \param variation The variation to apply to the octave.
		 * \return The new frequency value.
		 */
		static float ChangeOctave(float frequency, float variation);

		/**
		 * \brief Changes the semitone of a frequency value.
		 * \param frequency The original frequency.
		 * \param variation The variation to apply to the semitone.
		 * \return The new frequency value.
		 */
		static float ChangeSemitone(float frequency, float variation);

		/**
		 * \brief Handles the event when a key is triggered on the keyboard. This will primarily handle
		 * pausing background music when the game is paused.
		 * \param event Pointer to the KeyboardButtonEvent object containing the event details.
		 */
		void OnKeyTriggered(const KeyboardButtonEvent* event);

		/**
		 * \brief Handles the event when the pause screen is closed. This will primarily handle
		 * playing background music when the game is unpaused.
		 * \param event Pointer to the NamedEvent object containing the event details.
		 */
		void OnPauseScreenClosed(const NamedEvent* event);

		/**
		 * \brief Mutes all audio buses in the system.
		 */
		void MuteAllBuses();

		/**
		 * \brief Unmutes all audio buses in the system.
		 */
		void UnmuteAllBuses();

		/**
		 * \brief Returns the instance of the AudioSystem.
		 * \return Reference to the AudioSystem instance.
		 */
		static AudioSystem& Instance();
	private:
		typedef std::unordered_map<std::string, FMOD::Sound*> SoundMap; //!< Map storing sound objects.
		typedef std::unordered_map<std::string, FMOD::Studio::Bank*> BankMap; //!< Map storing bank objects.
		typedef std::unordered_map<std::string, FMOD::Studio::EventDescription*> EventDescriptionMap; //!< Map storing event description objects.
		typedef std::unordered_map<std::string, FMOD::Studio::EventInstance*> EventInstanceMap; //!< Map storing event instance objects.
		typedef std::unordered_map<std::string, FMOD::Studio::Bus*> BusMap; //!< Map storing bus objects.
		SoundMap sounds_; //!< Map containing the loaded sound objects.
		BankMap banks_; //!< Map containing the loaded bank objects.
		EventDescriptionMap eventDescriptions_; //!< Map containing the event description objects.
		EventInstanceMap eventInstances_; //!< Map containing the event instance objects.
		BusMap buses_; //!< Map containing the bus objects.
		std::stack<std::pair<std::string, float>> busVolumes_; //!< Stack storing bus volume settings.
		FMOD::Studio::System* sys_; //!< Pointer to the FMOD Studio level system.
		FMOD::System* sysLow_; //!< Pointer to the low-level FMOD system.
		FMOD::ChannelGroup* channelGroups_[3]; //!< Array of channel groups.
		std::vector<AudioChannel*> channels_; //!< Vector storing audio channel objects.

		AudioSystem(); //!< Default constructor of the AudioSystem class.
		~AudioSystem(); //!< Destructor of the AudioSystem class.
		void HandleVolumeEvent(const ChangeVolumeEvent* event); //!< Handles the volume change event by setting bus volume.
	};
}

#endif // AUDIO_SYSTEM_H
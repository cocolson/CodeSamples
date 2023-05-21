//---------------------------------------------------------
// file:    Animation.h
// project: WONDERLIFT
// author:  Coby Colson
// email:   coby.colson@digipen.edu
// course:	GAM150 - Spring 2020
//
// Copyright © 2020 DigiPen, All rights reserved.
//---------------------------------------------------------

#pragma once
#include "Sprite.h"

typedef struct Sprite* SpritePtr;
typedef struct Animation* AnimationPtr;
typedef struct AnimationMachine* AnimationMachinePtr;

/**
\brief Creates a new AnimationMachine with the specified number of states.
The AnimationMachine is a data structure that represents an animation state machine.
It stores information about the current state, next state, linked states, pause status,
and animations for each state.
\param numStates The number of states in the animation machine.
\return A pointer to the newly created AnimationMachine, or NULL if memory allocation failed.
*/
AnimationMachinePtr animationMachineCreate(int numStates);

/**
\brief Adds an animation to the AnimationMachine for the specified state.
This function sets the properties of the animation for the given state, including the
sprite, initial frame index, maximum frame index, frame duration, loop status, and frame delay.
\param machine Pointer to the AnimationMachine.
\param state The state to which the animation will be added.
\param sprite_p Pointer to the Sprite object.
\param frameIndex The initial frame index.
\param frameIndexMax The maximum frame index.
\param frameDuration The duration of each frame.
\param loop The loop status of the animation.
*/
void animationMachineAdd(AnimationMachinePtr machine, int state, SpritePtr sprite_p, unsigned int frameIndex, unsigned int frameIndexMax, float frameDuration, int loop);

/**
\brief Checks if the AnimationMachine is currently playing animations.
This function returns a boolean value indicating whether the AnimationMachine is currently playing animations.
\param machine Pointer to the AnimationMachine.
\return 0 if the AnimationMachine is paused, 1 if it is playing.
*/
int animationMachineIsPlaying(AnimationMachinePtr machine);

/**
\brief Resumes playing animations for the AnimationMachine.
This function sets the isPaused flag of the AnimationMachine to 0, indicating that animations should be played.
\param machine Pointer to the AnimationMachine.
*/
void animationMachinePlay(AnimationMachinePtr machine);

/**
\brief Pauses animations for the AnimationMachine.
This function sets the isPaused flag of the AnimationMachine to 1, indicating that animations should be paused.
\param machine Pointer to the AnimationMachine.
*/
void animationMachinePause(AnimationMachinePtr machine);

/**
\brief Retrieves the current state of the AnimationMachine.
This function returns the index of the current state in the AnimationMachine.
\param machine Pointer to the AnimationMachine.
\return The index of the current state.
*/
int animationMachineGetState(AnimationMachinePtr machine);

/**
\brief Sets the state of the AnimationMachine to the specified state gracefully.
This function sets the stateNext field of the AnimationMachine to the specified state,
and if the current state is -1 (uninitialized), it also sets the stateCurr field to the specified state.
Additionally, it sets the isPaused flag to 0 to resume playing animations.
\param machine Pointer to the AnimationMachine.
\param state The state to set.
*/
void animationMachineSetState(AnimationMachinePtr machine, int state);

/**
\brief Sets the state of the AnimationMachine to the specified state forcefully.
This function sets the stateNext and stateCurr fields of the AnimationMachine to the specified state,
and sets the isPaused flag to 0 to resume playing animations.
\param machine Pointer to the AnimationMachine.
\param state The state to set.
*/
void animationMachineSetStateForced(AnimationMachinePtr machine, int state);

/**
\brief Links two animation states in the AnimationMachine.
This function establishes a link between anim1 and anim2 in the AnimationMachine.
It checks if the provided animation indices are valid and not the same,
and updates the linkedStates array accordingly.
\param machine Pointer to the AnimationMachine.
\param anim1 The index of the first animation state.
\param anim2 The index of the second animation state.
*/
void animationMachineLink(AnimationMachinePtr machine, int anim1, int anim2);

/**
\brief Links all animation states in the AnimationMachine to create a sequential chain.
This function establishes links between all animation states in the AnimationMachine,
starting from anim1 and ending at anim2. It checks if the provided animation indices are valid and not the same.
If anim2 is less than anim1, it wraps around to the beginning of the animation states.
It uses the animationMachineLink function to create the links.
\param machine Pointer to the AnimationMachine.
\param anim1 The index of the first animation state.
\param anim2 The index of the last animation state.
*/
void animationMachineLinkAllTo(AnimationMachinePtr machine, int anim1, int anim2);

/**
\brief Links all animation states in the AnimationMachine to create a sequential chain.
This function establishes links between all animation states in the AnimationMachine,
starting from the first animation state (index 0) and ending at the last animation state (index machine->states - 1).
It calls the animationMachineLinkAllTo function with the appropriate arguments.
\param machine Pointer to the AnimationMachine.
*/
void animationMachineLinkAll(AnimationMachinePtr machine);

/**
\brief Unlinks an animation state from the AnimationMachine.
This function removes the link associated with the specified animation state in the AnimationMachine,
if it exists. It checks if the provided animation index is valid.
\param machine Pointer to the AnimationMachine.
\param anim1 The index of the animation state to unlink.
*/
void animationMachineUnlink(AnimationMachinePtr machine, int anim1);

/**
\brief Unlinks all animation states between anim1 and anim2 in the AnimationMachine.
This function removes the links associated with all animation states between anim1 and anim2
in the AnimationMachine. It checks if the provided animation indices are valid and not the same.
If anim2 is less than anim1, it wraps around to the beginning of the animation states.
It calls the animationMachineUnlink function for each animation state to be unlinked.
\param machine Pointer to the AnimationMachine.
\param anim1 The index of the first animation state.
\param anim2 The index of the last animation state.
*/
void animationMachineUnlinkAllTo(AnimationMachinePtr machine, int anim1, int anim2);

/**
\brief Unlinks all animation states in the AnimationMachine.
This function removes all links between animation states in the AnimationMachine.
It calls the animationMachineUnlinkAllTo function with the appropriate arguments
to unlink all animation states from the first animation state (index 0) to the last animation state (index machine->states - 1).
\param machine Pointer to the AnimationMachine.
*/
void animationMachineUnlinkAll(AnimationMachinePtr machine);

/**
\brief Updates the AnimationMachine and advances animations.
This function updates the AnimationMachine and advances the animations based on the elapsed time.
If the AnimationMachine is not paused and the current state is valid,
it decreases the frameDelay and performs the necessary actions to set and advance animation frames.
If an animation finishes, it checks for queued animations, linked animations, looping, or pauses accordingly.
\param machine Pointer to the AnimationMachine.
*/
void animationMachineUpdate(AnimationMachinePtr machine);

/**
\brief Frees the memory occupied by an AnimationMachine object.
This function releases the memory allocated for an AnimationMachine object,
including its anims array and linkedStates array. It sets the pointer to the AnimationMachine object
to null after freeing the memory.
\param machine Pointer to the pointer to the AnimationMachine object.
*/
void animationMachineFree(AnimationMachinePtr* machine);
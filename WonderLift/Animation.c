//---------------------------------------------------------
// file:    Animation.c
// project: WONDERLIFT
// author:  Coby Colson
// email:   coby.colson@digipen.edu
// course:	GAM150 - Spring 2020
//
// Copyright © 2020 DigiPen, All rights reserved.
//---------------------------------------------------------

#include "stdafx.h"
#include "Animation.h"
#include "time.h"

typedef struct Animation
{
	SpritePtr sprite_p;
	unsigned int frameIndexInit;
	unsigned int frameIndex;
	unsigned int frameIndexMax;
	float frameDelay;
	float frameDuration;
	int isLooping;
} Animation;

typedef struct AnimationMachine
{
	int states;
	int stateCurr;
	int stateNext;
	int* linkedStates;
	int isPaused;
	AnimationPtr* anims;
} AnimationMachine;

AnimationMachinePtr animationMachineCreate(int numStates) {
	AnimationMachinePtr machine = calloc(1, sizeof(AnimationMachine));
	if (machine) {
		machine->states = numStates;
		machine->stateCurr = -1;
		machine->stateNext = -1;
		machine->isPaused = 0;
		machine->anims = calloc(numStates, sizeof(AnimationPtr));
		if (machine->anims) {
			for (int i = 0; i < numStates; i++) {
				machine->anims[i] = calloc(1, sizeof(Animation));
			}
		}
		machine->linkedStates = calloc(numStates, sizeof(int));
		if (machine->linkedStates) {
			for (int i = 0; i < numStates; i++) {
				machine->linkedStates[i] = -1;
			}
		}
		return machine;
	}
	return NULL;
}

void animationMachineAdd(AnimationMachinePtr machine, int state, SpritePtr sprite_p, unsigned int frameIndex, unsigned int frameIndexMax, float frameDuration, int loop) {
	machine->anims[state]->sprite_p = sprite_p;
	machine->anims[state]->frameIndexInit = frameIndex;
	machine->anims[state]->frameIndex = frameIndex;
	machine->anims[state]->frameIndexMax = frameIndexMax;
	machine->anims[state]->frameDuration = frameDuration;
	machine->anims[state]->isLooping = loop;
	machine->anims[state]->frameDelay = frameDuration;
}

int animationMachineIsPlaying(AnimationMachinePtr machine) {
	return !machine->isPaused;
}

void animationMachinePlay(AnimationMachinePtr machine) {
	machine->isPaused = 0;
}

void animationMachinePause(AnimationMachinePtr machine) {
	machine->isPaused = 1;
}

int animationMachineGetState(AnimationMachinePtr machine) {
	return machine->stateCurr;
}

void animationMachineSetState(AnimationMachinePtr machine, int state) {
	machine->stateNext = state;
	if (machine->stateCurr == -1) {
		machine->stateCurr = state;
	}
	machine->isPaused = 0;
}

void animationMachineSetStateForced(AnimationMachinePtr machine, int state) {
	machine->stateNext = state;
	machine->stateCurr = state;
	machine->isPaused = 0;
}

void animationMachineLink(AnimationMachinePtr machine, int anim1, int anim2) {
	if (anim1 < 0 || anim1 > machine->states - 1 ||
		anim2 < 0 || anim2 > machine->states - 1 ||
		anim1 == anim2) {
		return;
	}
	if (machine->linkedStates[anim1]) {
		machine->linkedStates[anim1] = anim2;
	}
}

void animationMachineLinkAllTo(AnimationMachinePtr machine, int anim1, int anim2) {
	if (anim1 < 0 || anim1 > machine->states - 1 ||
		anim2 < 0 || anim2 > machine->states - 1 ||
		anim1 == anim2) {
		return;
	}
	int links = 0;
	if (anim2 < anim1) {
		links = machine->states - abs(anim1 - anim2);
	}
	else {
		links = anim2 - anim1;
	}
	for (int i = 0; i <= links; i++) {
		int link1 = (anim1 + i) % machine->states;
		int link2 = (anim1 + i + 1) % machine->states;
		animationMachineLink(machine, link1, link2);
	}
}

void animationMachineLinkAll(AnimationMachinePtr machine) {
	animationMachineLinkAllTo(machine, 0, machine->states - 1);
}

void animationMachineUnlink(AnimationMachinePtr machine, int anim1) {
	if (anim1 < 0 || anim1 > machine->states - 1) {
		return;
	}
	if (machine->linkedStates[anim1]) {
		machine->linkedStates[anim1] = -1;
	}
}

void animationMachineUnlinkAllTo(AnimationMachinePtr machine, int anim1, int anim2) {
	if (anim1 < 0 || anim1 > machine->states - 1 ||
		anim2 < 0 || anim2 > machine->states - 1 ||
		anim1 == anim2) {
		return;
	}
	int links = 0;
	if (anim2 < anim1) {
		links = machine->states - abs(anim1 - anim2);
	}
	else {
		links = anim2 - anim1;
	}
	for (int i = 0; i <= links; i++) {
		int link1 = (anim1 + i) % machine->states;
		animationMachineUnlink(machine, link1);
	}
}

void animationMachineUnlinkAll(AnimationMachinePtr machine) {
	animationMachineUnlinkAllTo(machine, 0, machine->states - 1);
}

void animationMachineUpdate(AnimationMachinePtr machine) {
	if (!machine->isPaused) {
		int state = machine->stateCurr;
		if (state < 0 || state > machine->states - 1) {
			return;
		}
		machine->anims[state]->frameDelay -= dt();
		// Setting animation frame
		if (machine->anims[state]->sprite_p != NULL) {
			spriteSetFrame(machine->anims[state]->sprite_p, machine->anims[state]->frameIndex);
		}
		// Advancing animation frame
		if (machine->anims[state]->frameDelay <= 0.0f) {
			// Animation unfinished, increment the frame index.
			if (machine->anims[state]->frameIndex < machine->anims[state]->frameIndexMax) {
				machine->anims[state]->frameIndex++;
				machine->anims[state]->frameDelay = machine->anims[state]->frameDuration;
			}
			// Animation finished, is there a queued animation?
			else if (machine->stateCurr != machine->stateNext) {
				machine->stateCurr = machine->stateNext;
			}
			// Animation finished, is there a linked animation?
			else if (machine->linkedStates[machine->stateCurr] != -1) {
				animationMachineSetStateForced(machine, machine->linkedStates[machine->stateCurr]);
				int newState = machine->stateCurr;
				machine->anims[/*(old)*/state]->frameDelay = machine->anims[/*(old)*/state]->frameDuration;
				machine->anims[/*(old)*/state]->frameIndex = machine->anims[/*(old)*/state]->frameIndexInit;
				machine->anims[newState]->frameDelay = machine->anims[newState]->frameDuration;
				machine->anims[newState]->frameIndex = machine->anims[newState]->frameIndexInit;
			}
			// Animation finished, are we looping?
			else if (machine->anims[state]->isLooping) {
				machine->anims[state]->frameIndex = machine->anims[state]->frameIndexInit;
				machine->anims[state]->frameDelay = machine->anims[state]->frameDuration;
			}
			// Animation finished, pause animation.
			else {
				machine->anims[state]->frameIndex = machine->anims[state]->frameIndexMax;
				machine->anims[state]->frameDelay = 0.0f;
				animationMachinePause(machine);
			}
		}
	}
}

void animationMachineFree(AnimationMachinePtr* machine) {
	if (*machine) {
		if ((*machine)->anims) {
			for (int i = 0; i < (*machine)->states; i++) {
				if ((*machine)->anims[i]) {
					free((*machine)->anims[i]);
					(*machine)->anims[i] = ((void*)0);
				}
			}
			free((*machine)->anims);
			(*machine)->anims = ((void*)0);
		}
		if ((*machine)->linkedStates) {
			free((*machine)->linkedStates);
			(*machine)->linkedStates = ((void*)0);
		}
		free(*machine);
		*machine = ((void*)0);
	}
}
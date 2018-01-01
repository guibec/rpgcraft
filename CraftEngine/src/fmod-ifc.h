#pragma once

#include "fmod.h"

// FMOD streams are restricted to only being able to be played once, so it makes sense to have a custom
// FmodStream type that has a strong association between sound and channel.

struct FmodMusic
{
    FMOD_SOUND*     sndptr  = nullptr;
    FMOD_CHANNEL*   channel = nullptr;
};

struct FmodSound
{
    FMOD_SOUND*     sndptr  = nullptr;
};


extern void             fmod_CheckLib       ();
extern void             fmod_InitSystem     ();

extern void             fmod_CreateMusic    (FmodMusic& dest, const xString& fullpath);
extern void             fmod_CreateSound    (FmodSound& dest, const xString& fullpath);
extern void             fmod_Play           (FmodMusic& stream);
extern void             fmod_SetPause       (FmodMusic& stream, bool isPaused);
extern void             fmod_SetVolume      (const FmodMusic& stream, float vol);
extern void             fmod_SetVolume      (FMOD_CHANNEL* channel, float volume);

extern FMOD_CHANNEL*    fmod_PlaySound      (const FmodSound& sound);

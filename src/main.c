#include <stdio.h> //snprintf
#include <math.h> //fabsf, abs
#include <stdlib.h> //malloc

#include "screens.h"
#include "raylib.h"
#include "images.h"

SceneState* scenestate_create() {
    SceneState* state = malloc(sizeof(SceneState));
    *state = (SceneState){};

    state->persist_data = malloc(sizeof(PersistentSceneData));
    *(state->persist_data) = (PersistentSceneData){};
    
    state->persist_data->game_fps = 60;
    state->current_scene = SCENE_NONE;
    state->next_scene = SCENE_MENU;
    state->flags[SCENE_FLAG_SCENECHANGE] = true;
    
    return state;
}

void scenestate_destroy(SceneState* state){
    free(state->persist_data);
    free(state);
}

int main(char** argv, int argc) {
    SceneState* state = scenestate_create();
    
    InitWindow(100, 100, "c-snake");
    SetTargetFPS(state->persist_data->game_fps);
    init_snaketextures();
    
    while (!WindowShouldClose()) {
        if (state->flags[SCENE_FLAG_SCENECHANGE]) {
            switch(state->current_scene){
                case SCENE_MENU:
                    unload_menuscreen(state);
                    break;
                case SCENE_GAME:
                    unload_gamescreen(state);
                    break;
                default:
                    break;
            }
            
            switch(state->next_scene){
                case SCENE_MENU:
                    setup_menuscreen(state);
                    break;
                case SCENE_GAME:
                    setup_gamescreen(state);
                    break;
            }
            state->current_scene = state->next_scene;
            state->flags[SCENE_FLAG_SCENECHANGE] = false;
        }

        switch(state->current_scene) {
            case SCENE_MENU:
                update_menuscreen(state);
                draw_menuscreen(state);
                break;
            case SCENE_GAME:
                update_gamescreen(state);
                draw_gamescreen(state);
                break;
        }
    }
    
    // destroy_snake(&snake);
    scenestate_destroy(state);

    return 0;
}

// //Just dumping raylib sound functions here for now
// void play_sound() {
//     // RLAPI void InitAudioDevice(void);                                     // Initialize audio device and context
//     // RLAPI void CloseAudioDevice(void);                                    // Close the audio device and context
//     // RLAPI bool IsAudioDeviceReady(void);                                  // Check if audio device has been initialized successfully
//     // RLAPI void SetMasterVolume(float volume);                             // Set master volume (listener)
//     // RLAPI float GetMasterVolume(void);                                    // Get master volume (listener)
//     // RLAPI Wave LoadWave(const char *fileName);                            // Load wave data from file
//     // RLAPI Sound LoadSound(const char *fileName);                          // Load sound from file
//     // RLAPI void PlaySound(Sound sound);                                    // Play a sound
//     // RLAPI void StopSound(Sound sound);                                    // Stop playing a sound
//     // RLAPI void PauseSound(Sound sound);                                   // Pause a sound
//     // RLAPI void ResumeSound(Sound sound);                                  // Resume a paused sound
//     // RLAPI bool IsSoundPlaying(Sound sound);                               // Check if a sound is currently playing
//     // RLAPI void SetSoundVolume(Sound sound, float volume);                 // Set volume for a sound (1.0 is max level)

//     // RLAPI Music LoadMusicStream(const char *fileName);                    // Load music stream from file
//     // RLAPI Music LoadMusicStreamFromMemory(const char *fileType, const unsigned char *data, int dataSize); // Load music stream from data
//     // RLAPI bool IsMusicReady(Music music);                                 // Checks if a music stream is ready
//     // RLAPI void UnloadMusicStream(Music music);                            // Unload music stream
//     // RLAPI void PlayMusicStream(Music music);                              // Start music playing
//     // RLAPI void UpdateMusicStream(Music music);                            // Updates buffers for music streaming
//     // RLAPI void StopMusicStream(Music music);                              // Stop music playing
//     // RLAPI void PauseMusicStream(Music music);                             // Pause music playing
//     // RLAPI void ResumeMusicStream(Music music);                            // Resume playing paused music
// }


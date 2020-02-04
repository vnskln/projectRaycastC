#include <stdio.h>
#include <SDL2/SDL.h>
#include "config.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int windowSetup(SDL_Window **window, SDL_Renderer **renderer) {

    //function to initialize the SDL library
    //returns 0 if ok
    //else some error
    if (SDL_Init(SDL_INIT_EVERYTHING) !=0 ) {
        fprintf(stderr,"Error initializing SDL\n");
        return FALSE;
    };

    //initializing window object
    *window = SDL_CreateWindow("TheGame",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_WIDTH, WIN_HEIGHT, 0);
    if (!*window) {
        fprintf(stderr,"Error creating window");
        return FALSE;
    }

    //initializing rendere
    //-1 is default renderer
    *renderer = SDL_CreateRenderer(*window, -1, 0);
    if (!*renderer) {
        fprintf(stderr,"Error creating renderer\n");
        return FALSE;
    }

    //Choose rendering technique
    //BLEND mode
    //when pixel changes color instead it gives you a mixture
    //of the old pixel and the new one
    //based on how large 'alpha' value of the new pixel is
    SDL_SetRenderDrawBlendMode(*renderer, SDL_BLENDMODE_BLEND);
    return TRUE;
};

void windowDestruction(SDL_Window **window, SDL_Renderer **renderer) {
    SDL_DestroyRenderer(*renderer);
    SDL_DestroyWindow(*window);
    SDL_Quit();
}


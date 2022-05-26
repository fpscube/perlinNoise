
#include <SDL2/SDL.h>
 
int main(int argc, char *argv[])
{
 
    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    SDL_Window* win = SDL_CreateWindow("GAME",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       1000, 1000, 0);


    SDL_Event e;
    for (;;) {
        SDL_PollEvent(&e);
        switch (e.type) {
            case SDL_QUIT:
                SDL_Log("Program quit after %i ticks", e.quit.timestamp);
                exit(0);
                break;
            default:
                break;
        }
        SDL_Delay(10);
    }
}
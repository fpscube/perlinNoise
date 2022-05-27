
#include <SDL2/SDL.h>
#include "stdint.h"
#include "stdio.h"
extern void perlinGenGradiant();
extern void perlinGenTexture(uint32_t * pBuffer,int pWidth,int pHeight);
static SDL_Renderer *gSDLRenderer;
static SDL_Texture *texture;
#define WIDTH 1024
#define HEIGHT 768

typedef uint32_t T_PixelType;

T_PixelType gFrameBuffer[WIDTH*HEIGHT];

static void createWindow()
{
    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }

    SDL_Window* window = SDL_CreateWindow("GAME",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       WIDTH, HEIGHT, 0);


    gSDLRenderer = SDL_CreateRenderer(window, -1, 0);
#ifdef RENDER8888
    texture = SDL_CreateTexture(gSDLRenderer,
    SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);
#else
    texture = SDL_CreateTexture(gSDLRenderer,
    SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);
#endif

}
static void renderScene()
{
    SDL_RenderClear(gSDLRenderer);

    for(int i=0;i<20000;i++)
    {
        gFrameBuffer[i]=0x00FF00FF;
    }

    
    perlinGenTexture(gFrameBuffer,WIDTH,HEIGHT);

    SDL_UpdateTexture(texture , NULL, (const void *)gFrameBuffer, WIDTH * sizeof (T_PixelType));

    SDL_RenderCopy(gSDLRenderer, texture, NULL, NULL);
    SDL_RenderPresent(gSDLRenderer);
}





int main(int argc, char *argv[])
{

    createWindow();
    perlinGenGradiant();

    SDL_Event e;
    for (;;) {

        renderScene();

        SDL_PollEvent(&e);
        switch (e.type) {
            case SDL_QUIT:
                SDL_Log("Program quit after %i ticks", e.quit.timestamp);
                exit(0);
                break;
            case SDL_KEYDOWN:                
                perlinGenGradiant();
                break;
            default:
                break;
        }
        SDL_Delay(10);
    }
}
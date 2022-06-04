
#include <SDL2/SDL.h>
#include "stdint.h"
#include "stdio.h"
#include "map.h"
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

    SDL_Window* window = SDL_CreateWindow("Perlin Noise press any key",
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

    T_map_ctrl *lMapCtrl = map_getMapControl();

    SDL_UpdateTexture(texture , NULL, (const void *)gFrameBuffer, WIDTH * sizeof (T_PixelType));
    SDL_RenderCopy(gSDLRenderer, texture, NULL, NULL);
    for(int iRing=0;iRing<K_MAP_NB_RING;iRing++)
    {
        T_map_texture * lTextures = map_getMapTexture(iRing);
        for(int i=0;i<9;i++)
        {
            SDL_Rect lRect;
            lRect.x = lTextures[i].posX*lTextures[i].size - (int)lMapCtrl->posx + WIDTH/2;
            lRect.y = lTextures[i].posY*lTextures[i].size - (int)lMapCtrl->posy + HEIGHT/2;

            lRect.h=lTextures[i].size ;       
            lRect.w=lTextures[i].size ;    

            // clipping screen
            if(lRect.x<0){
                lRect.w = lRect.w + lRect.x;
                lRect.x=0;
            } 
            if(lRect.y<0){
                lRect.h = lRect.h + lRect.y;
                lRect.y=0;
            } 
            if((lRect.x+ lRect.w) > WIDTH){
                lRect.w = WIDTH-lRect.x;
            } 
            if((lRect.y+ lRect.h) > HEIGHT){
                lRect.h = HEIGHT-lRect.y;
            }     
            
            if(iRing==0)        
            {    
                SDL_UpdateTexture(texture , &lRect, (const void *)lTextures[i].buffer, lTextures[i].size  * sizeof (T_PixelType));

                SDL_RenderCopy(gSDLRenderer, texture, &lRect, &lRect);
            }
            else
            {
               SDL_SetRenderDrawColor(gSDLRenderer, 255,(iRing-1)*250, 0, 255);
               SDL_RenderDrawRect(gSDLRenderer, &lRect);
            }

        
        }
    }


    SDL_SetRenderDrawColor(gSDLRenderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(gSDLRenderer, WIDTH/2 - 10, HEIGHT/2, WIDTH/2 + 10, HEIGHT/2);
    SDL_SetRenderDrawColor(gSDLRenderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(gSDLRenderer, WIDTH/2, HEIGHT/2-10, WIDTH/2, HEIGHT/2 + 10);
    SDL_RenderPresent(gSDLRenderer);
}



int main(int argc, char *argv[])
{

    createWindow();
    perlinGenGradiant();
    T_map_ctrl *lMapCtrl = map_getMapControl();

    SDL_Event e;
    for (;;) {

        renderScene();

        while(SDL_PollEvent(&e))
        {
            switch (e.type) {
                case SDL_QUIT:
                    SDL_Log("Program quit after %i ticks", e.quit.timestamp);
                    exit(0);
                    break;
                case SDL_KEYDOWN:    
                    switch(e.key.keysym.sym){
                    case SDLK_UP : 
                        lMapCtrl->posy -=10;              
                        break;
                    case SDLK_DOWN :    
                        lMapCtrl->posy +=10;       
                        break;
                    case SDLK_LEFT :   
                        lMapCtrl->posx -=10;             
                        break;
                    case SDLK_RIGHT :    
                        lMapCtrl->posx +=10;      
                        break;
                    default:   
                        perlinGenGradiant();          
                        break;
                    }
                default:
                    break;
            }
        }
        SDL_Delay(10);

        for(int iRing=0;iRing<K_MAP_NB_RING;iRing++)
        {
            map_refresh(iRing);
        }
    }
}
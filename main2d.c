
#include <SDL2/SDL.h>
#include "stdint.h"
#include "stdio.h"
#include "map.h"

extern void perlinGenTexture(uint32_t * pBuffer,int pPosX,int pPosY,int pSize,int pTextureSize,int pGridSize);
static SDL_Renderer *gSDLRenderer;
static SDL_Texture *textureSGL[K_MAP_NB_RING][K_MAP_NB_TEXTURE_BY_RING];
#define WIDTH 1024
#define HEIGHT 768

typedef uint32_t T_PixelType;

T_PixelType gFrameBuffer[WIDTH*HEIGHT];
static int gPosX=0;
static int gPosY=0;


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

    for(int r=0;r<(K_MAP_NB_RING);r++)
    {
        for(int i=0;i<(K_MAP_NB_TEXTURE_BY_RING);i++)
        {
            textureSGL[r][i] = SDL_CreateTexture(gSDLRenderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);
        }
    }
}

#define K_TILE_RESOLUTION 100
#define K_TILE_BASE_SIZE 100


static void renderScene()
{
    SDL_RenderClear(gSDLRenderer);


    // clear frame buffer with  red color
    for(int i=0;i<20000;i++)
    {
        gFrameBuffer[i]=0x00FF00FF;
    }

    T_map * lMap = map_update(K_TILE_BASE_SIZE,gPosX,gPosY);
    // for all ring 9 textures
    // Draw first low res
    for(int iRing=(K_MAP_NB_RING-1);iRing>=0;iRing--)
    {
        for(int i=0;i<K_MAP_NB_TEXTURE_BY_RING;i++)
        {
            T_map_texture * lTexture = &lMap->ring[iRing].tex[i];
            SDL_Rect lRectSrc;
            lRectSrc.x = 0;
            lRectSrc.y = 0;

            lRectSrc.h=K_TILE_RESOLUTION;
            lRectSrc.w=K_TILE_RESOLUTION;


            // Projection of tecture using posx posy
            SDL_Rect lRectDst;
            lRectDst.x = lTexture->posX - (int)gPosX + WIDTH/2;
            lRectDst.y = lTexture->posY - (int)gPosY + HEIGHT/2;

            lRectDst.h=lTexture->size ;       
            lRectDst.w=lTexture->size ;  

            int ringCoef = pow(3,iRing);  

            // clipping screen
            if(lRectDst.x<0){
                float delta = -lRectDst.x;
                lRectSrc.x += delta/(ringCoef);
                lRectDst.x += delta;
                lRectSrc.w -= delta/(ringCoef);
                lRectDst.w -= delta;
            } 
            if(lRectDst.y<0){
                float delta = -lRectDst.y;
                lRectSrc.y += delta/(ringCoef);
                lRectDst.y += delta;
                lRectSrc.h -= delta/(ringCoef);
                lRectDst.h -= delta;
            } 
            if((lRectDst.x+ lRectDst.w) > WIDTH){
                float delta = lRectDst.x + lRectDst.w - WIDTH;
                lRectSrc.w -= delta/(ringCoef);
                lRectDst.w -= delta;
            } 
            if((lRectDst.y+ lRectDst.h) > HEIGHT){
                float delta = lRectDst.y + lRectDst.h - HEIGHT;
                lRectSrc.h -= delta/(ringCoef);
                lRectDst.h -= delta;
            }     
            
       

            SDL_Rect lRect;
            lRect.x = 0;
            lRect.y = 0;
            lRect.h = K_TILE_RESOLUTION;
            lRect.w = K_TILE_RESOLUTION;

            static uint32_t stc_perlinTexture[K_TILE_RESOLUTION*K_TILE_RESOLUTION];
            if(!lTexture->isUpToDate)
            {
                printf("genText ring:%d tex:%d ",iRing,i);
                printf(" x:%d y:%d\n",lTexture->posX,lTexture->posY);

                perlinGenTexture((uint32_t *)stc_perlinTexture,lTexture->posX,lTexture->posY,lTexture->size,K_TILE_RESOLUTION,50);
            
                SDL_UpdateTexture(textureSGL[iRing][i] , &lRect, (const void *)stc_perlinTexture, K_TILE_RESOLUTION  * sizeof (T_PixelType));
            }
            SDL_RenderCopy(gSDLRenderer, textureSGL[iRing][i] , &lRectSrc, &lRectDst);
            SDL_SetRenderDrawColor(gSDLRenderer, 255,(iRing-1)*250, 0, 255);
            SDL_RenderDrawRect(gSDLRenderer, &lRectDst);
                  
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
                        gPosY -=10;              
                        break;
                    case SDLK_DOWN :    
                        gPosY +=10;       
                        break;
                    case SDLK_LEFT :   
                        gPosX -=10;             
                        break;
                    case SDLK_RIGHT :    
                        gPosX +=10;      
                        break;
                    default:         
                        break;
                    }
                default:
                    break;
            }
        }
        SDL_Delay(10);

        
    }
}
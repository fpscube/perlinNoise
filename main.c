
#include <SDL2/SDL.h>
#include "stdint.h"
#include "stdio.h"
#include "map.h"

extern void perlinGenTexture(uint32_t * pBuffer,int pPosX,int pPosY,int pSize,int pTextureSize,int pGridSize);
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

    perlinGenTexture(gFrameBuffer,0,0,HEIGHT,HEIGHT,HEIGHT/10);

    T_map_ctrl *lMapCtrl = map_getMapControl();

    SDL_UpdateTexture(texture , NULL, (const void *)gFrameBuffer, HEIGHT * sizeof (T_PixelType));
    SDL_RenderCopy(gSDLRenderer, texture, NULL, NULL);
    for(int iRing=(K_MAP_NB_RING-1);iRing>=0;iRing--)
    {
        T_map_texture * lTextures = map_getMapTexture(iRing);
        for(int i=0;i<9;i++)
        {

            SDL_Rect lRectSrc;
            lRectSrc.x = 0;
            lRectSrc.y = 0;

            lRectSrc.h=K_MAP_TILE_RESOLUTION;
            lRectSrc.w=K_MAP_TILE_RESOLUTION;


            SDL_Rect lRectDst;
            lRectDst.x = lTextures[i].posX*lTextures[i].size- (int)lMapCtrl->posx + WIDTH/2;
            lRectDst.y = lTextures[i].posY*lTextures[i].size - (int)lMapCtrl->posy + HEIGHT/2;

            lRectDst.h=lTextures[i].size ;       
            lRectDst.w=lTextures[i].size ;  

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
            
            if(iRing>=0)        
            {    

                SDL_Rect lRect;
                lRect.x = 0;
                lRect.y = 0;
                lRect.h = K_MAP_TILE_RESOLUTION;
                lRect.w = K_MAP_TILE_RESOLUTION;

                SDL_UpdateTexture(texture , &lRect, (const void *)lTextures[i].buffer, K_MAP_TILE_RESOLUTION  * sizeof (T_PixelType));

                SDL_RenderCopy(gSDLRenderer, texture, &lRectSrc, &lRectDst);
               SDL_SetRenderDrawColor(gSDLRenderer, 255,(iRing-1)*250, 0, 255);
               SDL_RenderDrawRect(gSDLRenderer, &lRectDst);
            }
            {
               
            }

        
        }
    }


    SDL_SetRenderDrawColor(gSDLRenderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(gSDLRenderer, WIDTH/2 - 10, HEIGHT/2, WIDTH/2 + 10, HEIGHT/2);
    SDL_SetRenderDrawColor(gSDLRenderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(gSDLRenderer, WIDTH/2, HEIGHT/2-10, WIDTH/2, HEIGHT/2 + 10);
    SDL_RenderPresent(gSDLRenderer);
}

void map_computeTex(T_map_texture * pTexture)
{
      perlinGenTexture((uint32_t *)pTexture->buffer,pTexture->posX*pTexture->size,pTexture->posY*pTexture->size,pTexture->size,K_MAP_TILE_RESOLUTION,50);
}

int main(int argc, char *argv[])
{

    createWindow();
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
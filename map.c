#include "map.h"
#include <stdio.h>
#include <math.h>


static T_map stc_map={0};

const int cst_offsetToTileId[3][3] ={{1,4,6},{2,0,7},{3,5,8}};

const int cst_offsetRing[8][2] =
{
 {-1,-1},{0,-1},{1,-1},
 {-1, 0},       {1, 0},
 {-1, 1},{0, 1},{1, 1}
};

// Tile pattern 
// 9 Tiles (same resolution)
// 1  2  3
// 4  0  5
// 6  7  8
// tile 0 is always located on posx posy
// tile are located in a fix grid depending of the tile size

typedef struct 
{
    int posX;
    int posY;
    int textureId;
    /* data */
}T_tile;

static T_tile stc_map_grid[K_MAP_NB_RING][9]={0};

static void map__updateRing(int pRing,int pTileBaseSize,float pPosX,float pPosY)
{
    T_tile *lGrid = &stc_map_grid[pRing][0];
    T_map_texture *lTextures = &stc_map.ring[pRing].tex[0];


   // printf("%f,%f\n",stc_map_ctrl.posx,stc_map_ctrl.posy);

    //Update Tile position
    int lTileDimension = pTileBaseSize * powl(3,pRing);
    int posXint =  pPosX/lTileDimension;
    int posYint =  pPosY/lTileDimension;
    if(pPosX<0)posXint = posXint-1;
    if(pPosY<0)posYint = posYint-1;
    lGrid[0].posX = posXint;
    lGrid[0].posY = posYint;
    //First ring
    for(int i=0;i<8;i++)
    {
        lGrid[i+1].posX = posXint + cst_offsetRing[i][0];
        lGrid[i+1].posY = posYint + cst_offsetRing[i][1];
    }

    //Reset Texture and Tile association
    for(int i=0;i<9;i++)
    {
        lGrid[i].textureId = -1;
        lTextures[i].tileId = -1;
        lTextures[i].size = lTileDimension  ;   
        lTextures[i].isUpToDate = 1;
    }

    //Associate existing texture to corresponding tile
    for(int i=0;i<9;i++)
    {
        if(!lTextures[i].isInitialised) continue;
        int xOffset =  (lTextures[i].gridPosX - posXint);
        int yOffset =  (lTextures[i].gridPosY - posYint);
        if (xOffset<-1 || xOffset>1 || yOffset<-1 || yOffset>1) continue;
        int tileId = cst_offsetToTileId[xOffset+1][yOffset+1];
        lGrid[tileId].textureId = i;
        lTextures[i].tileId = tileId;
        //printf("tile %d re-use texture %d  x:%d y:%d\n",tileId,i,lTextures[i].posX,lTextures[i].posY);
    }
    
    //Associate new texture to tile 
    for(int iTile=0;iTile<9;iTile++)
    {
        int iTexture=0;
        if(lGrid[iTile].textureId >= 0 ) continue;
        for(;iTexture<9;iTexture++)
        {
            if(lTextures[iTexture].tileId == -1 )
            {
                lGrid[iTile].textureId = iTexture;
                lTextures[iTexture].tileId = iTile;
                lTextures[iTexture].posX = lGrid[iTile].posX  * lTileDimension;
                lTextures[iTexture].posY = lGrid[iTile].posY  * lTileDimension;
                lTextures[iTexture].gridPosX = lGrid[iTile].posX;
                lTextures[iTexture].gridPosY = lGrid[iTile].posY;
                lTextures[iTexture].computeId = 0;
                lTextures[iTexture].isUpToDate = 0;
                lTextures[iTexture].isInitialised=1;   
                //printf("tile %d new texture %d  x:%d y:%d\n",iTile,iTexture,lTextures[iTexture].posX,lTextures[iTexture].posY);
                break;
            }
        }
    }


}

T_map *map_update(int pTileBaseSize,int pPosX,int pPosY)
{
    for(int iRing=0;iRing<K_MAP_NB_RING;iRing++)
    {
        map__updateRing(iRing,pTileBaseSize,pPosX,pPosY);
    }
    return &stc_map;
}
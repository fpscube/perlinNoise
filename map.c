#include "map.h"
#include <stdio.h>
#include <math.h>


static T_map_ctrl stc_map_ctrl={0};
static T_map_texture stc_map_texture[K_MAP_NB_RING][9]={0};

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


T_map_ctrl * map_getMapControl(void)
{
    return &stc_map_ctrl;
}


T_map_texture * map_getMapTexture(int pRing)
{
    return &stc_map_texture[pRing][0];
}



void map_refresh(int pRing)
{
    T_tile *lGrid = &stc_map_grid[pRing][0];
    T_map_texture *lTextures = &stc_map_texture[pRing][0];

   // printf("%f,%f\n",stc_map_ctrl.posx,stc_map_ctrl.posy);

    //Update Tile position
    int lTileDimension = K_MAP_TILE_RESOLUTION * powl(3,pRing);
    int posXint =  (int)stc_map_ctrl.posx/lTileDimension;
    int posYint =  (int)stc_map_ctrl.posy/lTileDimension;
    if(stc_map_ctrl.posx<0)posXint = posXint-1;
    if(stc_map_ctrl.posy<0)posYint = posYint-1;
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
    }

    //Associate existing texture to corresponding tile
    for(int i=0;i<9;i++)
    {
        if(!lTextures[i].isInitialised) continue;
        int xOffset =  (lTextures[i].posX - posXint);
        int yOffset =  (lTextures[i].posY - posYint);
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
                lTextures[iTexture].posX = lGrid[iTile].posX;
                lTextures[iTexture].posY = lGrid[iTile].posY;
                lTextures[iTexture].computeId = 0;
                lTextures[iTexture].isUpToDate = 0;
                lTextures[iTexture].isInitialised=1;   
                printf("tile %d new texture %d  x:%d y:%d\n",iTile,iTexture,lTextures[iTexture].posX,lTextures[iTexture].posY);
                break;
            }
        }
    }

    //Compute texture by priority order 0-9 grid
    for(int iTile=0;iTile<9;iTile++)
    {
        int lTextureId = lGrid[iTile].textureId;
        if(!lTextures[lTextureId].isUpToDate)
        {
            map_computeTex(&lTextures[lTextureId]);
            lTextures[lTextureId].isUpToDate=1;

        }     
    }

        for(int i=0;i<9;i++)
    {
       // printf("tile %d => texture %d x:%d y:%d\n",i,lGrid[i].textureId,lGrid[i].posX*K_MAP_TILE_DIMENSION,lGrid[i].posY*K_MAP_TILE_DIMENSION);
    }


    for(int i=0;i<9;i++)
    {
      //  printf("text %d => tile %d x:%d y:%d\n",i,lTextures[i].tileId,lTextures[i].posX*K_MAP_TILE_DIMENSION,lTextures[i].posY*K_MAP_TILE_DIMENSION);
    }
}


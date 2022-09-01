#ifndef _MAP_
#define _MAP_

#define K_MAP_NB_RING 3
#define K_MAP_NB_TEXTURE_BY_RING 9

typedef struct 
{
    int posX; //x pos of the corner upper left of the tile
    int posY; //y pos of the corner upper left of the tile
    int size;
    int gridPosX; 
    int gridPosY; 
    int tileId;
    int computeId;
    int isUpToDate;
    int isInitialised;
}T_map_texture;

typedef struct
{
    T_map_texture tex[K_MAP_NB_TEXTURE_BY_RING];
}T_map_ring;

typedef struct
{
    T_map_ring ring[K_MAP_NB_RING];
}T_map;


/* update update regarding position */
T_map * map_update(int pTileBaseSize,int pPosX,int pPosY);


#endif
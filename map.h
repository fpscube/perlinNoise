#ifndef _MAP_
#define _MAP_

#define K_MAP_TILE_RESOLUTION 100
#define K_MAP_NB_RING 3

typedef struct 
{
    float posx;
    float posy;
}T_map_ctrl;


typedef struct 
{
    int posX;
    int posY;
    int size;
    int tileId;
    int computeId;
    int isUpToDate;
    int isInitialised;
    void * buffer[K_MAP_TILE_RESOLUTION*K_MAP_TILE_RESOLUTION];
    /* data */
}T_map_texture;

/* used to mov the map by writing T_map_ctrl data */
T_map_ctrl * map_getMapControl(void);
/* used to get texture info */
T_map_texture * map_getMapTexture(int pRing);
/* need to be implemented by perlin noise generator for example */
void map_computeTex(T_map_texture* pTexture);
/* update map regarding ctrl info */
void map_refresh(int pRing);



#endif
#ifndef _PERLIN_NOISE_
#define _PERLIN_NOISE_


extern void perlinGenTexture(uint32_t * pBuffer,int pPosX,int pPosY,int pSize,int pTextureSize,int pGridSize);
extern void perlinGenHeightMap(float * pBuffer,int pPosX,int pPosY,int pSize,int pTextureSize,int pGridSize);

#endif
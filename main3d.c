
#include "glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
 
#include "map.h"
#include "perlinNoise.h"
#include "linmath.h"
 
#include <stdlib.h>
#include <stdio.h>
#define K_TILE_RES_PX 100
#define K_TILE_SIZE 1000
typedef struct
{
    float x, y,z;
    float r, g, b;
}T_vertexbuffer;

static T_vertexbuffer vertexBuffer[K_TILE_RES_PX*K_TILE_RES_PX*6];
static int indexBuffer[K_TILE_RES_PX*K_TILE_RES_PX*6];
GLint mvp_location, vpos_location, vcol_location;
 
static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec3 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 1.0);\n"
"    color = vCol;\n"
"}\n";
 
static const char* fragment_shader_text =
"#version 110\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";
 
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
vec3 gCamPos = {0.0,40519.624023,0.0};
vec3 gCamDir = {-0.002699 , -0.842619, 0.538504};
vec4 gMvDir = {0.0,0.0,0.0,0.0};
float gCamSpeed=1000.0;
 
GLenum  gRenderType = GL_TRIANGLES;

 
typedef struct 
{
    int left;
    int right;
    int up;
    int down;
}T_media;

T_media gMedia;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if(key == GLFW_KEY_SPACE)
    {
        gCamPos[1]+=100;

    }        
    if(key == GLFW_KEY_L)
    {
        gRenderType = GL_LINES;
    }     
    if(key == GLFW_KEY_T)
    {
        gRenderType = GL_TRIANGLES;
    }
    if(key == GLFW_KEY_R)
    {

        gCamPos[0] = 0;
        gCamPos[1] = 0;
        gCamPos[2] = 0;
        gCamSpeed=1000.0;

    }      
    if(key == GLFW_KEY_KP_ADD)
    {
        gCamSpeed*=2;

    }         
    if(key == GLFW_KEY_KP_SUBTRACT)
    {
        gCamSpeed/=2;
    }           

    if(key == GLFW_KEY_LEFT || key == GLFW_KEY_A)
    {
        gMedia.left = (action != GLFW_RELEASE);
        vec3 mvDir3 = {gCamDir[0],0.0,gCamDir[2]};
        vec3_norm(mvDir3,mvDir3);
        vec4 mvDir = {mvDir3[0],0.0,mvDir3[2],1.0};
        mat4x4 R;
        mat4x4_identity(R);
        mat4x4_rotate_Y(R,R,1.57079632679);
        mat4x4_mul_vec4(gMvDir,R,mvDir);

    }
    else if(key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)
    {
        gMedia.right = (action != GLFW_RELEASE);

        vec3 mvDir3 = {gCamDir[0],0.0,gCamDir[2]};
        vec3_norm(mvDir3,mvDir3);
        vec4 mvDir = {mvDir3[0],0.0,mvDir3[2],1.0};
        mat4x4 R;
        mat4x4_identity(R);
        mat4x4_rotate_Y(R,R,-1.57079632679);
        mat4x4_mul_vec4(gMvDir,R,mvDir);
    }
    else if(key == GLFW_KEY_UP || key == GLFW_KEY_W)
    {
        gMedia.up = (action != GLFW_RELEASE);
        gMvDir[0]= gCamDir[0];
        gMvDir[1]= gCamDir[1];
        gMvDir[2]= gCamDir[2];
    }
    else if(key == GLFW_KEY_DOWN || key == GLFW_KEY_S)
    {
        gMedia.down = (action != GLFW_RELEASE);
        gMvDir[0]= -gCamDir[0];
        gMvDir[1]= -gCamDir[1];
        gMvDir[2]= -gCamDir[2];
        
    }

    if (action == GLFW_RELEASE && !gMedia.up && !gMedia.down && !gMedia.left  && !gMedia.right   )
    {
        gMvDir[0]=0.0;
        gMvDir[1]=0.0;
        gMvDir[2]=0.0;
    }

}

float groundGetY(float x,float z)
{
  float y = sin(x/50)*5+sin(z/50)*5 + sin(x/90)*9 + sin(z/90)*9 + sin(x/150)*15 + sin(z/150)*15 -(x/200)*(x/200) -(z/200)*(z/200);

  return(y);
}

void map_computeTex(T_map_texture * pTexture)
{
}
 
GLuint vertex_buffer;
GLuint elementbuffer;


typedef struct
{
    float  tex[K_TILE_RES_PX*K_TILE_RES_PX];
}T_MapTileTex;

/**
 * @brief generate and draw map tile
 * 
 * @param pCenterX_WC center X of the tile in world coord
 * @param pCenterY_WC center Y of the tile in world coord
 * @param pSizeWC  size of the tile in world coord
 * @param pGridSizeWC grid size in world coord
 * @param pRingId ring id 
 * @param pTileId position of the tile in the ring in order to manage resolution limit
 * tile 0/2/4/6 draw limit with inner tile
 *    7 0 1 
 *    6 x 2
 *    5 4 3 
 * @param pTextureTab List of texture (0=internal 8-15=ring1 16-23=ring2 24-31=ring3 ...)
 */
void drawMapTile(int pCenterX_WC,int pCenterY_WC,int pSizeWC,int pGridSizeWC,int pRingId,int pTileId,T_MapTileTex *pTextureTab)
{
   
    // get left bottom corner coord in order to generate texture at center
    int pOriginX = pCenterX_WC - pSizeWC/2;
    int pOriginY = pCenterY_WC - pSizeWC/2;       
    
    int vcount=0;
    int icount=0;
    float lsb =  ((float)pSizeWC)/((float)(K_TILE_RES_PX-1));

    // debug color to comment
    float colorR = ((float)(pOriginX%124))/255.0;
    float colorG = ((float)(pOriginY%168))/255.0;

    // get current Texture
    float *lTexture = pTextureTab[pRingId*8+pTileId].tex;
    float *lTextureHRLimit[3]={0,0,0};

    for (int z=0;z<K_TILE_RES_PX;z++)
    {
        for (int x=0;x<K_TILE_RES_PX;x++)
        {                
            /* tile 0/2/4/6 draw limit with inner tile
                *    7 0 1 
                *    6 x 2
                *    5 4 3 
                */
            int highReslimit= ((pTileId==0 && z==0 && pRingId>1)); //||
                     //   (pTileId==2 && z==0) ||
                     //   (pTileId==4 && x==(K_TILE_RES_PX-2)) ||
                     //   (pTileId==6 && z==(K_TILE_RES_PX-2))) && pRingId>1;
            
            // compute vertex color using perlin texture
            float y = lTexture[x+z*K_TILE_RES_PX];
           

            if(y<0)
            {
                vertexBuffer[vcount].r=(y + 1.0)/2.0 + colorR;
                vertexBuffer[vcount].g=(y + 1.0)/2.0 + colorG;
                vertexBuffer[vcount].b=(y + 1.0)/2.0 + 0.5;
            }
            else
            {
                vertexBuffer[vcount].r=(y + 1.0)/2.0 + colorR;
                vertexBuffer[vcount].g=(y + 1.0)/2.0 + 0.2 + colorG;
                vertexBuffer[vcount].b=(y + 1.0)/2.0 ;
            }

            //Add Vertex
            vertexBuffer[vcount].x=x*lsb + pOriginX;
            vertexBuffer[vcount].y=y*1000;
            vertexBuffer[vcount].z=z*lsb + pOriginY;
            vcount++;


            // //Add Triangles
            if (((x+1)<K_TILE_RES_PX) && ((z+1)<K_TILE_RES_PX))
            {
                //Avoid Limit
               if(highReslimit) continue;

                //triangle 1 
                // x--x
                // | /
                // |/
                // x
                indexBuffer[icount++] = vcount-1; 
                indexBuffer[icount++] = vcount;
                indexBuffer[icount++] = vcount-1 + K_TILE_RES_PX;

                //triangle 2
                //    x
                //   /| 
                //  / |
                // x--x
                indexBuffer[icount++] = vcount;
                indexBuffer[icount++] = vcount + K_TILE_RES_PX;
                indexBuffer[icount++] = vcount-1 + K_TILE_RES_PX;
               
            
            }
        }
    }


    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBuffer), vertexBuffer, GL_STATIC_DRAW);
 
   
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertexBuffer[0]), (void*) 0);
                          
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertexBuffer[0]), (void*) (sizeof(float) * 3));

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, icount* sizeof(unsigned int), &indexBuffer[0], GL_STATIC_DRAW);
          
    glDrawElements(gRenderType, K_TILE_SIZE*K_TILE_SIZE,GL_UNSIGNED_INT,(void*)0 );
}

void drawMapTilesRing(int pPosX_WC,int pPosY_WC,int pSizeWC,int pGridSizeWC)
{

    static T_MapTileTex perlinHeightMapTab[4*8];

    const int cstXOffset[]= {0.0, 1.0, 1.0, 1.0, 0.0,-1.0,-1.0,-1.0};
    const int cstYOffset[]= {1.0, 1.0, 0.0,-1.0,-1.0,-1.0, 0.0, 1.0};

    // 7 0 1
    // 6 x 2
    // 5 4 3

    perlinGenHeightMap(perlinHeightMapTab[0].tex,pPosX_WC,pPosY_WC,pSizeWC,K_TILE_RES_PX,pGridSizeWC);
    for(int ringId=1;ringId<4;ringId++)
    {
        int lRingSize_WC;
        lRingSize_WC= pow(3,ringId-1)*pSizeWC;
        for(int i=0;i<8;i++)
        {
            int xOffset = cstXOffset[i]*lRingSize_WC + pPosX_WC;
            int yOffset = cstYOffset[i]*lRingSize_WC + pPosY_WC;
            perlinGenHeightMap(perlinHeightMapTab[ringId*8 + i].tex,xOffset,yOffset,lRingSize_WC,K_TILE_RES_PX,pGridSizeWC);
        }
    }


    drawMapTile(pPosX_WC,pPosY_WC,pSizeWC,pGridSizeWC,0,0,&perlinHeightMapTab[0]);


    for(int ringId=1;ringId<4;ringId++)
    {
        int lRingSize_WC;
        lRingSize_WC= pow(3,ringId-1)*pSizeWC;
        for(int i=0;i<8;i++)
        {
            int xOffset = cstXOffset[i]*lRingSize_WC + pPosX_WC;
            int yOffset = cstYOffset[i]*lRingSize_WC + pPosY_WC;
            drawMapTile(xOffset,yOffset,lRingSize_WC,pGridSizeWC,ringId,i,&perlinHeightMapTab[0]) ;
        }
    }


}


int main(void)
{
    GLFWwindow* window;
    GLuint  vertex_shader, fragment_shader, program;
    int width, height;
    static double stc_lastTime;
    stc_lastTime = glfwGetTime();

 
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
 
    window = glfwCreateWindow(800, 800, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwGetFramebufferSize(window, &width, &height);
    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_HIDDEN);
    glfwSetCursorPos(window,width/2.0,height/2.0);
    glfwSetKeyCallback(window, key_callback);
 
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);
 
    // NOTE: OpenGL error checks have been omitted for brevity
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
 
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
 
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
 
    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");
     
 
    glEnableVertexAttribArray(vpos_location);
    glEnableVertexAttribArray(vcol_location);
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST); 
    //glEnable(GL_CULL_FACE);    

  while (!glfwWindowShouldClose(window))
  {
        double lCurrentTime = glfwGetTime();
        double lDeltaTime =  lCurrentTime-stc_lastTime;
        stc_lastTime =  lCurrentTime;

        float ratio;
        glfwGetFramebufferSize(window, &width, &height);
        mat4x4 m, p, mvp;
        double xcursor;
        double ycursor;
        glfwGetCursorPos(window,&xcursor,&ycursor);
        glfwSetCursorPos(window,width/2.0,height/2.0);
        double xDelta = width/2.0 - xcursor;
        double yDelta = height/2.0 - ycursor;

        {
            vec3 up = {0,1,0};
            vec3 hVector;
            vec3_mul_cross(hVector, gCamDir, up);
            vec3_norm(hVector,hVector);
            gCamDir[0] -= hVector[0]* xDelta/1000.0;
            gCamDir[1] +=  yDelta/1000.0;
            gCamDir[2] -= hVector[2]* xDelta/1000.0;
            vec3_norm(gCamDir,gCamDir);
        }

        gCamPos[0] += gMvDir[0]*lDeltaTime*gCamSpeed;
        gCamPos[1] += gMvDir[1]*lDeltaTime*gCamSpeed;
        gCamPos[2] += gMvDir[2]*lDeltaTime*gCamSpeed;

        //printf("%f,%f,%f-%f,%f,%f\n",gCamDir[0],gCamDir[1],gCamDir[2],gCamPos[0],gCamPos[1],gCamPos[2]);

        ratio = width / (float) height;
 
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
        mat4x4_identity(m);
        vec3 eye = {gCamPos[0], gCamPos[1], gCamPos[2]};
        vec3 center = {gCamDir[0]+gCamPos[0], gCamDir[1]+gCamPos[1], gCamDir[2]+gCamPos[2]};
        vec3 up = {0, 1.0, 0.0};
        mat4x4_translate_in_place(m, gCamPos[0],gCamPos[1],gCamPos[2] );
        mat4x4_look_at(m,eye,center,up);
       // mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        //mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 10.f, -10.f);
        //mat4x4_frustum(p, -ratio, ratio, -1.f, 1.f, 10.f, 10000.f);
        mat4x4_perspective(p,0.785398,ratio,1.0f,100000.f);
        mat4x4_mul(mvp, p, m);
 

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);



        // draw center at 0,0
        drawMapTilesRing(0,0,10000,1000); 

        //todo for ring 

        // implementation 
        // for(int iRing=0;iRing<K_MAP_NB_RING;iRing++)
        // {        
            
        //     for(int texId=0;texId<K_MAP_NB_TEXTURE_BY_RING;texId++)
        //     {
        //     //drawMapTile(&lMap->ring[iRing].tex[texId]);
        // }


        //void perlinGenHeightMap(float * pBuffer,int PosX_WC,int pPosY_WC,int pSizeWC,int pTextureSizePX,int pGridSizeWC)


        // T_map * lMap = map_update(K_TILE_SIZE*10,gCamPos[0],gCamPos[2]);



        // // Todo afficher de la couleur sur les texture en fonction du ring
        // for(int iRing=0;iRing<K_MAP_NB_RING;iRing++)
        // {
        //     for(int texId=0;texId<K_MAP_NB_TEXTURE_BY_RING;texId++)
        //     {
        //         drawMapTile(&lMap->ring[iRing].tex[texId]);
        //     }
        // }



        glfwSwapBuffers(window);
        glfwPollEvents();
        // printf(" - PollEvents %d\n",(counter++)%50);

    }
    
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}










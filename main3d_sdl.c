
#include "glad.h"
#define GLFW_INCLUDE_NONE
#include <SDL2/SDL.h>
 
#include "map.h"
#include "perlinNoise.h"
#include "linmath.h"
 
#include <stdlib.h>
#include <stdio.h>
#define K_TILE_RES 64
#define K_TILE_SIZE 1000
typedef struct
{
    float x, y,z;
    float r, g, b;
}T_vertexbuffer;

static T_vertexbuffer vertexBuffer[K_TILE_RES*K_TILE_RES*6];
static int indexBuffer[K_TILE_RES*K_TILE_RES*6];
GLint mvp_location, vpos_location, vcol_location;
 
static const char* vertex_shader_text =
"#version 330 core\n"
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
"#version 330 core\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
" gl_FragColor = vec4(vec3(gl_FragCoord.z), 1.0);"
//"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";
 
// static void error_callback(int error, const char* description)
// {
//     fprintf(stderr, "Error: %s\n", description);
// }
vec3 gCamPos = {486.554504,4519.624023,15592.208984};
vec3 gCamDir = {0.336166,-0.354075,-0.872710};
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

// static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
// {
//     if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//         glfwSetWindowShouldClose(window, GLFW_TRUE);
//     if(key == GLFW_KEY_SPACE)
//     {
//         gCamPos[1]+=100;

//     }        
//     if(key == GLFW_KEY_L)
//     {
//         gRenderType = GL_LINES;
//     }     
//     if(key == GLFW_KEY_T)
//     {
//         gRenderType = GL_TRIANGLES;
//     }
//     if(key == GLFW_KEY_R)
//     {

//         gCamPos[0] = 0;
//         gCamPos[1] = 0;
//         gCamPos[2] = 0;
//         gCamSpeed=1000.0;

//     }      
//     if(key == GLFW_KEY_KP_ADD)
//     {
//         gCamSpeed*=2;

//     }         
//     if(key == GLFW_KEY_KP_SUBTRACT)
//     {
//         gCamSpeed/=2;
//     }           

//     if(key == GLFW_KEY_LEFT || key == GLFW_KEY_A)
//     {
//         gMedia.left = (action != GLFW_RELEASE);
//         vec3 mvDir3 = {gCamDir[0],0.0,gCamDir[2]};
//         vec3_norm(mvDir3,mvDir3);
//         vec4 mvDir = {mvDir3[0],0.0,mvDir3[2],1.0};
//         mat4x4 R;
//         mat4x4_identity(R);
//         mat4x4_rotate_Y(R,R,1.57079632679);
//         mat4x4_mul_vec4(gMvDir,R,mvDir);

//     }
//     else if(key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)
//     {
//         gMedia.right = (action != GLFW_RELEASE);

//         vec3 mvDir3 = {gCamDir[0],0.0,gCamDir[2]};
//         vec3_norm(mvDir3,mvDir3);
//         vec4 mvDir = {mvDir3[0],0.0,mvDir3[2],1.0};
//         mat4x4 R;
//         mat4x4_identity(R);
//         mat4x4_rotate_Y(R,R,-1.57079632679);
//         mat4x4_mul_vec4(gMvDir,R,mvDir);
//     }
//     else if(key == GLFW_KEY_UP || key == GLFW_KEY_W)
//     {
//         gMedia.up = (action != GLFW_RELEASE);
//         gMvDir[0]= gCamDir[0];
//         gMvDir[1]= gCamDir[1];
//         gMvDir[2]= gCamDir[2];
//     }
//     else if(key == GLFW_KEY_DOWN || key == GLFW_KEY_S)
//     {
//         gMedia.down = (action != GLFW_RELEASE);
//         gMvDir[0]= -gCamDir[0];
//         gMvDir[1]= -gCamDir[1];
//         gMvDir[2]= -gCamDir[2];
        
//     }

//     if (action == GLFW_RELEASE && !gMedia.up && !gMedia.down && !gMedia.left  && !gMedia.right   )
//     {
//         gMvDir[0]=0.0;
//         gMvDir[1]=0.0;
//         gMvDir[2]=0.0;
//     }

// }

float groundGetY(float x,float z)
{
  float y = sin(x/50)*5+sin(z/50)*5 + sin(x/90)*9 + sin(z/90)*9 + sin(x/150)*15 + sin(z/150)*15 -(x/200)*(x/200) -(z/200)*(z/200);

  return(y);
}

void map_computeTex(T_map_texture * pTexture)
{
}
 
GLuint vao;
GLuint vbo;
GLuint ebo;

void drawMapTile(T_map_texture *pMapTexture)
{
    static float perlinHeightMap[K_TILE_RES*K_TILE_RES];
   // if(!lTexture->isUpToDate)
    {
        perlinGenHeightMap((float *)perlinHeightMap,pMapTexture->posX,pMapTexture->posY,pMapTexture->size,K_TILE_RES,1000);
    }
       
    int vcount=0;
    int icount=0;
    float lsb =  pMapTexture->size/K_TILE_RES;
    for (int x=0;x<(K_TILE_RES);x++)
    {
        for (int z=0;z<K_TILE_RES;z++)
        {
            float y = ((float *)(perlinHeightMap))[z+x*K_TILE_RES];
            if(y<0)
            {
                vertexBuffer[vcount].r=(y + 1.0)/2.0;
                vertexBuffer[vcount].g=(y + 1.0)/2.0;
                vertexBuffer[vcount].b=(y + 1.0)/2.0 + 0.5;
            }
            else
            {
                vertexBuffer[vcount].r=(y + 1.0)/2.0;
                vertexBuffer[vcount].g=(y + 1.0)/2.0 + 0.2;
                vertexBuffer[vcount].b=(y + 1.0)/2.0 ;
            }

            vertexBuffer[vcount].x=x*lsb + pMapTexture->posX;
            vertexBuffer[vcount].y=y*1000;
            vertexBuffer[vcount].z=z*lsb + pMapTexture->posY;
            vcount++;

            if (((x+1)<K_TILE_RES) && ((z+1)<K_TILE_RES))
            {
                //triangle 1
                indexBuffer[icount++] = z+x*(K_TILE_RES);
                indexBuffer[icount++] = z+1+x*(K_TILE_RES);
                indexBuffer[icount++] = z+(x+1)*(K_TILE_RES);

                //triangle 2
                indexBuffer[icount++] = z+1+x*(K_TILE_RES);
                indexBuffer[icount++] = z+1+(x+1)*(K_TILE_RES);
                indexBuffer[icount++] = z+(x+1)*(K_TILE_RES);
            
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

int main(void)
{
    GLuint  vertex_shader, fragment_shader, program;
    int width=800;
    int height=800;
    static double stc_lastTime;

    SDL_Window *window;   
    SDL_Init(SDL_INIT_VIDEO);
    stc_lastTime = SDL_GetTicks();


    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    window = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        width,                               // width, in pixels
        height,                               // height, in pixels
        SDL_WINDOW_OPENGL                  // flags - see below
    );
    printf("ERR1 %s\n", SDL_GetError());
    
    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    printf("ERR2 %s\n", SDL_GetError());
    SDL_GL_MakeCurrent(window,glcontext);
    printf("ERR3 %s\n", SDL_GetError());
    
    gladLoadGL();
    SDL_GL_SetSwapInterval(1);
    printf("ERR4 %s\n", SDL_GetError());
 
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

 
    glGenVertexArrays(1, &vao); 
    glBindVertexArray(vao);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");


 
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(vpos_location);
    glEnableVertexAttribArray(vcol_location);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LESS); 
    glEnable(GL_CULL_FACE);    

  while (1)
  {
        double lCurrentTime = SDL_GetTicks();
        double lDeltaTime =  lCurrentTime-stc_lastTime;
        stc_lastTime =  lCurrentTime;

        float ratio;
        mat4x4 m, p, mvp;
        double xcursor=0;
        double ycursor=0;
       // glfwGetCursorPos(window,&xcursor,&ycursor);
       // glfwSetCursorPos(window,width/2.0,height/2.0);
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
 

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type==SDL_QUIT)  exit(1);
        }

        glViewport(0, 0, width, height);
        glClearColor(1.0,0.0,0.0,1.0);
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
        mat4x4_perspective(p,0.785398,ratio,0.0f,10000.f);
        mat4x4_mul(mvp, p, m);
 

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);

        
        T_map * lMap = map_update(K_TILE_SIZE*10,gCamPos[0],gCamPos[2]);
        for(int iRing=0;iRing<1;iRing++)
        {
            for(int texId=0;texId<1;texId++)
            {
                drawMapTile(&lMap->ring[iRing].tex[texId]);
            }
        }
        // drawMapTile(&lMap->ring[0].tex[0]);
        // drawMapTile(&lMap->ring[0].tex[1]);

        SDL_GL_SwapWindow(window);
    }
    
 
    exit(EXIT_SUCCESS);
}










#include <stdio.h>
#include <stdlib.h>
#include "linmath.h"

#include "glad.h"
#include <GLFW/glfw3.h>

#define UNUSED(x) (void)(sizeof(x))

static const float VERTICES[] = {
     0.f,  -0.5f, 0.0,
    -0.5f, 0.5f, 1.0,
     0.5f, 0.5f, -1.0,
    -0.5f, -0.5f, 0.0,
     0.5f, -0.5f, 1.0,
     0.f,   0.5f, -1.0
};
 
static const char *VERTEX_SHADER_SOURCE =
    "#version 330 core\n"
    "in vec3 aPos;\n"
    "uniform mat4 MVP;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = MVP *  vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\n"
    "\0";
 
static const char *FRAGMENT_SHADER_SOURCE =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec4 gl_FragCoord ;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0f);\n"
    "}\n"
    "\0";


float   ratio=1.0;

static void error_callback(int err, const char *msg) {
    fprintf(stderr, "GLFW callback: %s (error code %d)\n", msg, err);
}

static void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    UNUSED(window);
    ratio = width / (float) height;
    glViewport(0, 0, width, height);
}

static unsigned compile_shader(GLenum shader_type, const char *shader_src) {
    int success;
    unsigned shader;

    shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_src, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "error: failed to compile vertex shader, %s\n", infoLog);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    return shader;
}

int main(void) {
    GLFWwindow *window;
    const char *msg;
    unsigned vertex_buffer, vertex_shader;
    int success;
    unsigned fragment_shader, program, VAO, VBO;

    fputs("Hello, Triangle!\n", stderr);

    glfwSetErrorCallback(error_callback);

    msg = glfwGetVersionString();
    if (!msg) {
        fputs("error: unable to determine GLFW version\n", stderr);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Compiled against GLFW %s\n", msg);

    if (!glfwInit()) {
        fputs("error: unable to initialize GLFW\n", stderr);
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(800, 600, "Hello, Triangle!", NULL, NULL);
    if (!window) {
        fputs("error: unable to create window\n", stderr);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fputs("error: unable to initialize GLAD\n", stderr);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    msg = (const char *)glGetString(GL_VERSION);
    if (!msg) {
        fputs("error: unable to determine OpenGL version\n", stderr);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    printf("OpenGL %s\n", msg);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

    vertex_shader   = compile_shader(GL_VERTEX_SHADER,   VERTEX_SHADER_SOURCE);
    fragment_shader = compile_shader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);


    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "error: failed to link shaders, %s\n", infoLog);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    
    	// Get a handle for our "MVP" uniform
	GLuint MVP = glGetUniformLocation(program, "MVP");
 
	// Get a handle for our buffers
	GLuint aPos = glGetAttribLocation(program, "aPos");


    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void *)0);
    glEnableVertexAttribArray(aPos);
      
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1);

    		
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST); 
    //glEnable(GL_CULL_FACE);    




    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.7f, 0.8f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        {
 
            mat4x4 m, p, mvp;
            mat4x4_identity(m);
            mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
            mat4x4_mul(mvp, p, m);
            glUniformMatrix4fv(MVP, 1, GL_FALSE, (const GLfloat*) mvp);
        }

        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

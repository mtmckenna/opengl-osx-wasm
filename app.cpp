#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <iostream>
#include <cmath>
#include <string>

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 400
#define PI 3.14159265

static SDL_Window *window = NULL;
static float attributes[] = {
   0.0f,  0.5f, 1.0f, 0.0f, 0.0f,
   0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
  -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
};
static GLuint length_of_attributes = sizeof(attributes) / sizeof(attributes[0]);
static GLuint previous_ticks = 0;
static GLuint max_fps = 15;
static GLfloat rotation_angle = -0.05;
static GLuint vbo;

static const char *vertexSource = R"glsl(
#version 100
attribute vec2 position;
attribute vec3 color;

varying vec3 vColor;

void main()
{
  vColor = color;
  gl_Position = vec4(position, 0.0, 1.0);
}
)glsl";

static const char *fragmentSource = R"glsl(
#version 100
precision mediump float;
varying vec3 vColor;
void main()
{
    gl_FragColor = vec4(vColor, 1.0);
}
)glsl";

#if __EMSCRIPTEN__
void loop(void *arg)
#else
int loop(void *arg)
#endif
{
  GLuint ticks = SDL_GetTicks();
  GLuint elapsed_ticks = (ticks - previous_ticks);

  if (elapsed_ticks > max_fps)
  {
    previous_ticks = ticks;
    for (int i = 0; i < length_of_attributes; i = i + 5)
    {
      float x = attributes[i];
      float y = attributes[i + 1];

      attributes[i]     = x * std::cosf(rotation_angle) - y * std::sinf(rotation_angle);
      attributes[i + 1] = y * std::cosf(rotation_angle) + x * std::sinf(rotation_angle);
    }
  }

  glBufferData(GL_ARRAY_BUFFER, sizeof(attributes), attributes, GL_DYNAMIC_DRAW);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  SDL_GL_SwapWindow(window);

  #if !__EMSCRIPTEN__
  return 0;
  #endif
}

int main()
{
  SDL_Init(SDL_INIT_EVERYTHING);

  #if __EMSCRIPTEN__
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  #else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  #endif

  window = SDL_CreateWindow("Cats", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  glewExperimental = GL_TRUE;
  glewInit();

  const GLubyte *version;
  version = glGetString(GL_VERSION);
  std::cout << "version: " << version << std::endl;

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(attributes), attributes, GL_DYNAMIC_DRAW);

  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertexSource, NULL);
  glCompileShader(vertex_shader);
  GLint vertex_shader_status;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_status);

  char vertexLogBuffer[512];
  glGetShaderInfoLog(vertex_shader, 512, NULL, vertexLogBuffer);
  std::cout << "vertex_shader_status: " << vertex_shader_status << " " << vertexLogBuffer << std::endl;

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragmentSource, NULL);
  glCompileShader(fragment_shader);
  GLint fragment_shader_status;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_status);

  char fragmentLogBuffer[512];
  glGetShaderInfoLog(fragment_shader, 512, NULL, fragmentLogBuffer);
  std::cout << "fragment_shader_status: " << fragment_shader_status << " " << fragmentLogBuffer << std::endl;

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  glUseProgram(shader_program);

  GLint pos_attrib = glGetAttribLocation(shader_program, "position");
  GLint color_attrib = glGetAttribLocation(shader_program, "color");

  glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
  glEnableVertexAttribArray(pos_attrib);

  glVertexAttribPointer(color_attrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid *)(2 * sizeof(float)));
  glEnableVertexAttribArray(color_attrib);

  glClearColor(1.0, 1.0, 1.0, 0.0);

  #if __EMSCRIPTEN__
  emscripten_set_main_loop_arg(loop, NULL, -1, 1);
  #else
  SDL_Event windowEvent;
  while (true)
  {
    if (SDL_PollEvent(&windowEvent))
    {
      if (windowEvent.type == SDL_QUIT) break;
    }
    loop(NULL);
  }
  #endif

  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}
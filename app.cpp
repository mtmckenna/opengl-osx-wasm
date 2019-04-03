#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 300

SDL_Window *window = NULL;

const char *vertexSource = R"glsl(
#version 100
attribute vec2 position;
void main()
{
  gl_Position = vec4(position, 0.0, 1.0);
}
)glsl";

const char *fragmentSource = R"glsl(
#version 100
void main()
  {
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
  }
)glsl";

#if __EMSCRIPTEN__
void loop()
#else
int loop()
#endif
{
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

  float vertices[] = { 0.0f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f };

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);
  GLint vertexShaderStatus;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexShaderStatus);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);
  GLint fragmentShaderStatus;
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentShaderStatus);

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");

  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(posAttrib);

  #if __EMSCRIPTEN__
  emscripten_set_main_loop(loop, -1, 1);
  #else
  SDL_Event windowEvent;
  while (true)
  {
    if (SDL_PollEvent(&windowEvent))
    {
      if (windowEvent.type == SDL_QUIT) break;
    }
    loop();
  }
  #endif

  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}
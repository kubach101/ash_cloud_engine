#pragma once

#include "context.h"
#include "headers.h"
#include "renderer.h"
#include "structs.h"
#include "utils.h"
#include "window.h"

void init(State *state) {
  setupErrorHandeling();
  logInfo();

  glfwInit();
  createContext(state);
  createWindow(state);
  createRenderer(state);
}
void loop(State *state) {
  while (!glfwWindowShouldClose(state->window.handle)) {
    glfwPollEvents();
  }
}
void cleanup(State *state) {
  destroyRenderer(state);
  destroyWindow(state);
  destroyContext(state);
}

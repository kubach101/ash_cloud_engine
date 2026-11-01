#pragma once

void init(State *state)
{
    setupErrorHandeling();
    logInfo();

    glfwInit();
    createContext(state);
    createWindow(state);
    createRenderer(state);
}
void loop(State *state)
{
    while (!glfwWindowShouldClose(state->window.handle))
    {
        glfwPollEvents();
    }
}
void cleanup(State *state)
{
    destroyRenderer(state);
    destroyWindow(state);
    destroyContext(state);
}

#include <pch.h>
#include <SDL.h>

SDL_Window * window = NULL;
SDL_Renderer * renderer = NULL;

void AppUpdate(void *userdata)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        /* Process SDL events here, as needed. */
    }

    /* Draw content: */
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    void *nativeWindow = NULL;

    /* Retrieve a pointer/handle to an OS-native window.  If specified,
     * this will be passed into SDL_CreateWindowFrom.
     */
    if (argc >= 3 && (SDL_strcmp("--native_window", argv[1]) == 0)) {
        nativeWindow = (void *)SDL_strtoull(argv[2], NULL, 10);
    }

    if (nativeWindow) {
        SDL_SetMainReady();
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return 1;
    }

    if (nativeWindow) {
        window = SDL_CreateWindowFrom(nativeWindow);
    } else {
        /* If you'd like AppInit() to support non-XAML window creation, then
         * put the usual SDL_CreateWindow() call here.
         * For now, just generate a fake error message.
         */
        SDL_SetError("TODO: create SDL_Window when nativeWindow is NULL");
    }
    if (!window) {
        SDL_Log("Unable to create SDL_Window: %s", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        SDL_Log("Unable to create SDL_Renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        window = NULL;
    }

#if __WINRT__
    if (nativeWindow) {
        if (SDL_WinRTXAMLSetAnimationCallback(&AppUpdate, NULL) != 0) {
            SDL_Log("Unable to set SDL's XAML animation callback: %s", SDL_GetError());
            SDL_DestroyRenderer(renderer);
            renderer = NULL;
            SDL_DestroyWindow(window);
            window = NULL;
            return 1;
        }
    }
#endif

    if (!nativeWindow) {
        /* If and when the app isn't running as a WinRT + XAML app, then
         * run a main loop.
         */
        while (true) {
            AppUpdate(NULL);
        }
    }

    return 0;
}

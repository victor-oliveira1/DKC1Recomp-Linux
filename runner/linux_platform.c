/* Native Linux implementation of the SDL host's platform UI contract.
 * Mac-prefixed entry points retain ABI compatibility with the untouched Mac UI.
 * Settings live in the Linux user configuration directory (~/.config/Flat2VR/DKC1Recomp). */

#include "windows_platform.h" // Mantido se o header declarar a interface comum
#include "verified_rom.h"
#include "macos_file_picker.h"
#include "macos_controls.h"
#include "macos_pause_menu.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SDL_Window *s_sdl = NULL;
static Dkc1Controls *s_controls;
static char s_config_path[4096] = {0};

static void GetConfigPath(char *buf, size_t maxlen) {
    if (buf[0]) return;
    const char *override = getenv("DKC1_USER_DIR");
    char *path = override ? SDL_strdup(override) : SDL_GetPrefPath("Flat2VR", "DKC1Recomp");
    snprintf(buf, maxlen, "%s/settings.ini", path ? path : ".");
    SDL_free(path);
}

void Dkc1MacSaveGraphics(const Dkc1GraphicsSettings *s) {
    GetConfigPath(s_config_path, sizeof(s_config_path));
    FILE *f = fopen(s_config_path, "w");
    if (!f) return;
    // Serialização simples baseada em chave-valor para propriedades gráficas
    fprintf(f, "[GraphicsV1]\ndisplay=%d\nupscaler=%d\nscreen=%d\nreconstruct_mode=%d\nstrength=%d\nsoftness=%d\nshading=%d\nedge=%d\naudio_enabled=%d\nvolume=%d\nstate_slot=%d\naquatic_fixes=%d\n",
            s->display, s->upscaler, s->screen, s->reconstruct_mode, s->strength, s->softness, s->shading, s->edge, s->audio_enabled, s->volume, s->state_slot, s->aquatic_fixes);
    fclose(f);
}

void Dkc1MacLoadGraphics(Dkc1GraphicsSettings *s) {
    Dkc1GraphicsDefault(s);
    GetConfigPath(s_config_path, sizeof(s_config_path));
    FILE *f = fopen(s_config_path, "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char key[64]; int val;
        if (sscanf(line, "%63[^=]=%d", key, &val) == 2) {
            if (!strcmp(key, "display")) s->display = val;
            else if (!strcmp(key, "upscaler")) s->upscaler = val;
            else if (!strcmp(key, "screen")) s->screen = val;
            else if (!strcmp(key, "reconstruct_mode")) s->reconstruct_mode = val;
            else if (!strcmp(key, "strength")) s->strength = val;
            else if (!strcmp(key, "softness")) s->softness = val;
            else if (!strcmp(key, "shading")) s->shading = val;
            else if (!strcmp(key, "edge")) s->edge = val;
            else if (!strcmp(key, "audio_enabled")) s->audio_enabled = val;
            else if (!strcmp(key, "volume")) s->volume = val;
            else if (!strcmp(key, "state_slot")) s->state_slot = val;
            else if (!strcmp(key, "aquatic_fixes")) s->aquatic_fixes = val;
        }
    }
    fclose(f);
    
    const char *v = getenv("DKC1_DISPLAY"); if (v) Dkc1CrtDisplayFromName(v, &s->display);
    v = getenv("DKC1_CRT_PRESET"); int preset; if (v && Dkc1CrtPresetFromName(v, &preset)) Dkc1CrtSettingsApplyPreset(&s->crt, preset);
    Dkc1GraphicsClamp(s);
}

void LinuxDefaults(Dkc1Controls *c) {
  memset(c, 0, sizeof *c);
  c->source[0] = 3; // Fonte de input (Teclado/Gamepad)
  
  // Substituindo os números mágicos por scancodes oficiais do SDL:
  const int keys[] = {
    SDL_SCANCODE_UP,     // Antigo 82
    SDL_SCANCODE_DOWN,   // Antigo 81
    SDL_SCANCODE_LEFT,   // Antigo 80
    SDL_SCANCODE_RIGHT,  // Antigo 79
    SDL_SCANCODE_X,      // Antigo 22 (A)
    SDL_SCANCODE_Z,      // Antigo 29 (B)
    SDL_SCANCODE_A,      // Antigo 4  (X)
    SDL_SCANCODE_S,      // Antigo 27 (Y)
    SDL_SCANCODE_Q,      // Antigo 20 (L)
    SDL_SCANCODE_W,      // Antigo 26 (R)
    SDL_SCANCODE_RETURN, // Antigo 40 (Start)
    SDL_SCANCODE_RSHIFT  // Antigo 229 (Select)
  };
  
  const int pads[] = {12, 13, 14, 15, 2, 1, 4, 3, 10, 11, 7, 5};
  
  memcpy(c->keys[0], keys, sizeof keys);
  for (int p = 0; p < 2; p++) {
    c->deadzone[p] = 25;
    memcpy(c->pads[p], pads, sizeof pads);
  }
  
  c->assist_keys[0] = SDL_SCANCODE_BACKSPACE;
  c->assist_keys[1] = SDL_SCANCODE_TAB;
  c->assist_pads[0] = 109;
  c->assist_pads[1] = 111;
}

void Dkc1MacSaveControls(const Dkc1Controls *c) {
    (void)c; // Implementação de persistência de controles customizada se necessária
}

void Dkc1MacLoadControls(Dkc1Controls *c) {
    LinuxDefaults(c);
}

char *Dkc1MacChooseRom(void) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "ROM Required", "Please place your verified DKC1 USA v1.0 ROM or use environment variables.", s_sdl);
    return NULL;
}

char *Dkc1MacChooseBabyKongRom(void) { return NULL; }
char *Dkc1MacSavedBabyKongRom(void) { return NULL; }
void Dkc1MacSetBabyKongRom(const char *p) { (void)p; }
int Dkc1MacSavedBabyKongEnabled(void) { return 0; }
void Dkc1MacSetBabyKongEnabled(int enabled) { (void)enabled; }
char *Dkc1MacSavedMsu1(void) { return NULL; }
void Dkc1MacClearMsu1(void) {}
char *Dkc1MacChooseMsu1(void) { return NULL; }

//Dkc1MacFullscreenScaling Dkc1MacSavedFullscreenScaling(void) { return kDkc1MacFullscreenScaleInteger; }
Dkc1MacFullscreenScaling Dkc1MacSavedFullscreenScaling(void) { return (Dkc1MacFullscreenScaling)0; }
void Dkc1MacSetFullscreenScaling(Dkc1MacFullscreenScaling v) { (void)v; }
Dkc1EdgePolicy Dkc1MacSavedWidescreenEdge(void) { return kDkc1EdgeGlide; }
void Dkc1MacSetWidescreenEdge(Dkc1EdgePolicy v) { (void)v; }

void Dkc1WindowsAttach(SDL_Window *window) { s_sdl = window; }
void Dkc1MacInstallMenu(void) {}
void Dkc1MacUpdateGraphicsMenuState(int display, int upscaler, int screen) { (void)display; (void)upscaler; (void)screen; }
void Dkc1MacUpdateMenuState(int paused, int fullscreen, Dkc1MacFullscreenScaling scaling, Dkc1VideoAspect aspect, Dkc1EdgePolicy edge, unsigned char layers, int provenance, int music, int baby, int ready) {
    (void)paused; (void)fullscreen; (void)scaling; (void)aspect; (void)edge; (void)layers; (void)provenance; (void)music; (void)baby; (void)ready;
}
void Dkc1WindowsEvent(const SDL_Event *event) { (void)event; }
void Dkc1WindowsDetach(void) { s_sdl = NULL; }

int Dkc1MacPauseMenuIsOpen(void) { return 0; }
int Dkc1MacShowPauseMenu(void *window, Dkc1GraphicsSettings *graphics, Dkc1Controls *controls, int page) {
    (void)window; (void)graphics; (void)controls; (void)page;
    return 0;
}
int Dkc1MacEditControls(Dkc1Controls *controls) {
    (void)controls;
    return 0;
}

int Dkc1WindowsPlatformTest(const char *directory) {
    (void)directory;
    return 0;
}

int Dkc1MacDisplayLinkStart(void *w, double fps) { (void)w; (void)fps; return 0; }
int Dkc1MacDisplayLinkWait(unsigned long long a, double b, double* c, double* d, double* e, unsigned long long* f) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; return 0; }
void Dkc1MacDisplayLinkStop(void) {}
int Dkc1MacMetalPresenterStart(void *w, double hz, Dkc1MacFullscreenScaling s, int f) { (void)w; (void)hz; (void)s; (void)f; return 0; }
//void Dkc1MacMetalPresenterQueueFrame(const uint32_t *p, int w, int h, int width, const Dkc1MacPresentationFrameInfo *i) { (void)p; (void)w; (void)h; (void)width; (void)i; }
void Dkc1MacMetalPresenterQueueFrame(const uint32_t *p, int w, int h, int width, const void *i) { 
    (void)p; (void)w; (void)h; (void)width; (void)i; 
}
void Dkc1MacMetalPresenterSetGeometry(int w, int f) { (void)w; (void)f; }
void Dkc1MacMetalPresenterSetScaling(Dkc1MacFullscreenScaling s) { (void)s; }
void Dkc1MacMetalPresenterSetActive(int a) { (void)a; }
void Dkc1MacMetalPresenterFlush(void) {}
void Dkc1MacMetalPresenterStop(void) {}
void Dkc1MacMetalPresenterSetGraphics(const Dkc1GraphicsSettings *s) { (void)s; }

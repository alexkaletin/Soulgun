/**
 * Heads-up display for Soulgun
 */

#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "humanoid.h"
#include "TextureManager.h"
#include <string.h>

#define HUD_WIDTH 250
#define HUD_HEIGHT 250
#define FONT_SIZE 24
#define TEXT_GAP 25

#define HUD_X 25
#define HUD_Y 1024 - 25 - HUD_HEIGHT

struct TimeUnits {
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    int ms = 0;
};

/**
 * Manages the heads-up display (timer, score, health, weapon)
 */
class HUD
{
public:
    HUD(SDL_Renderer *renderer, Humanoid *player, TextureManager *txMan);
    ~HUD(void);
    void refresh(void);

    void startTimer(void);
    void stopTimer(void);
    void resetTimer(void);
    TimeUnits getTime(void);
private:
    int renderText(std::string text, bool isBold, int offsetX);

    int lastTime;
    int elapsedTime;
    bool isPaused;

    SDL_Renderer *renderer;
    Humanoid *player;
    TextureManager *txMan;

    TTF_Font *fontNormal;
    TTF_Font *fontBold;
};

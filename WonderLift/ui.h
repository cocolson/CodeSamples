//---------------------------------------------------------
// file:    ui.h
// project: WONDERLIFT
// author:  Coby Colson
// email:   coby.colson@digipen.edu
// course:	GAM150 - Spring 2020
//
// Copyright © 2020 DigiPen, All rights reserved.
//---------------------------------------------------------

#pragma once

#include "uibuttondefines.h"

#define UI_SHADOW_OFFSET_64 12
#define UI_SHADOW_OFFSET_48 8
#define UI_SHADOW_OFFSET_32 4
#define UI_SHADOW_OFFSET_16 2
#define UI_SHADOW_OFFSET_8 2

typedef void (*Action)(void);

typedef enum UIOrientation {
	VERTICAL,
	HORIZONTAL
} UIOrientation;

typedef enum UIInteract {
	INTERACT,
	NO_INTERACT
} UIInteract;

typedef struct Button {
	AEGfxTexture* tex_p;
	Action action;
} Button;
typedef Button* ButtonPtr;

typedef struct ButtonSet {
	ButtonPtr buttons;
	UIOrientation orientation;
	int buttonCount;
	int activeButton;
	float buttonWidth;
	float buttonHeight;
	float padding;
	float inactiveAlpha;
} ButtonSet;
typedef ButtonSet* ButtonSetPtr;

typedef struct ProgressBar {
	u32 fillColor;
	u32 emptyColor;
	AEGfxVertexList* fill_verts_p;
	AEGfxVertexList* empty_verts_p;
	UIOrientation orientation;
	UIInteract interactable;
	float barWidth;
	float barHeight;
	int min;
	int max;
	int step;
	int* display;
	int displayChange;
	int displayTemp;
} ProgressBar;
typedef ProgressBar* ProgressBarPtr;

typedef struct Icon {
	AEGfxTexture* tex_p;
	int* display;
} Icon;
typedef Icon* IconPtr;

typedef struct IconSet {
	IconPtr icons;
	UIOrientation orientation;
	int iconCount;
	float iconWidth;
	float iconHeight;
	float padding;
} IconSet;
typedef IconSet* IconSetPtr;

typedef enum Font {
	P2P_WHITE,
	P2P_BLACK,
	P2P_YELLOW,
	P2P_GREEN,
	P2P_RED,
	P2P_BLUE,
	FONT_HEAD // Used only for initialization
} Font;

typedef enum FontSize {
	px8,
	px16,
	px32,
	px48,
	px64,
	FONTSIZE_HEAD // Used only for initialization
} FontSize;

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------- Resource Management---------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

/** \brief Initializes elements of the user interface.
Call this once in Main.c.
*/
void uiInit();

/** \brief Frees elements of the user interface.
Call this once before Main.c exits.
*/
void uiFree();

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------Buttons----------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

/** \brief Creates a button set which can store interactable buttons.
\param orientation orientation the orientation of the icon set (HORIZONTAL or VERTICAL)
\param buttonWidth the pixel width of the buttons
\param buttonHeight the pixel height of the buttons
\param verticalPadding the pixel padding between buttons
\param inactiveAlpha the alpha of buttons while inactive (0.0f - 1.0f)
\return a pointer to the ButtonSet
*/
ButtonSetPtr buttonSetCreate(UIOrientation orientation, float buttonWidth, float buttonHeight, float padding, float inactiveAlpha);

/** \brief Creates a button which resides inside of a button set.
\param buttonSet_p a pointer to the corresponding button set
\param resourcePath the path to the image to use for the button (NOTE: may become deprecated when UI no longer uses images)
\param action the void returning function defined in uibuttondefines.c
*/
void buttonCreate(ButtonSetPtr buttonSet_p, const char* resourcePath, Action action);

/** \brief Draws a button set.
Call this every frame in your update function if you want to draw the buttons.
\param buttonSet_p a pointer to the corresponding button set
\param x the x coordinate of the button set
\param y the y coordinate of the button set
*/
void buttonsDraw(ButtonSetPtr buttonSet_p, float x, float y);

/** \brief Updates a button set's active button.
Call this every frame in your update function if you want to be able to change the active button.
\param buttonSet_p a pointer to the corresponding button set
*/
void buttonsUpdate(ButtonSetPtr buttonSet_p);

/** \brief Unloads memory allocated by button sets.
\param buttonSet_p a pointer to the corresponding button set
*/
void buttonsUnload(ButtonSetPtr buttonSet_p);

/** \brief Returns the active button in a button set.
You do not need to call this to monitor button presses.
However, it might be useful for displaying/hiding certain effects.
\param buttonSet_p a pointer to the corresponding button set
\return the active button
*/
int buttonsGetActiveButton(ButtonSetPtr buttonSet_p);

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------Progress Bars--------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

/** \brief Creates a progress bar which can display the value of a desired integer.
\param orientation orientation the orientation of the progress bar (HORIZONTAL or VERTICAL)
\param interactable set to INTERACT if the player can alter the display value at runtime.
otherwise, set to NO_INTERACT.
\param fillColor the fill color of the progress bar
\param emptyColor the empty color of the progress bar
\param barWidth the pixel width of the entire progress bar
\param barHeight the pixel height of the entire progress bar
\param min the minimum value held by the display value. the display value will be clamped.
\param max the maximum value held by the display value. the display value will be clamped.
\param display a pointer to the integer to represent
\return a pointer to the ProgressBar
*/
ProgressBarPtr progressBarCreate(UIOrientation orientation, UIInteract interactable, u32 fillColor, u32 emptyColor,
	float barWidth, float barHeight, int min, int max, int* display);

/** \brief Draws a progress bar.
Call this every frame in your update function if you want to draw the progress bar.
\param progBar_p a pointer to the corresponding progress bar
\param x the x coordinate of the progress bar
\param y the y coordinate of the progress bar
*/
void progressBarDraw(ProgressBarPtr progBar_p, float x, float y);

/** \brief Updates a progress bar by monitoring input and recalculating meshes.
Call this every frame in your update function if you want to update the progress bar.
\param progBar_p a pointer to the corresponding progress bar
*/
void progressBarUpdate(ProgressBarPtr progBar_p);

/** \brief Unloads memory allocated by progress bars.
\param progBar_p a pointer to the corresponding progress bar
*/
void progressBarUnload(ProgressBarPtr progBar_p);

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------Icon Sets---------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

/** \brief Creates an icon set which can display multiple images with associated values.
\param orientation the orientation of the icon set (HORIZONTAL or VERTICAL)
\param iconWidth the pixel width of the icons
\param iconHeight the pixel height of the icons
\param padding the pixel padding between icons
\return a pointer to the new icon set
*/
IconSetPtr iconSetCreate(UIOrientation orientation, float iconWidth, float iconHeight, float padding);

/** \brief Creates an icon associated with an icon set.
\param iconSet_p a pointer to the corresponding icon set
\param resourcePath the path to the image to use for the icon (NOTE: may become deprecated when UI no longer uses images)
\param a pointer to the value displayed adjacent to the icon (NOTE: this feature will not work until text is working)
*/
void iconCreate(IconSetPtr iconSet_p, const char* resourcePath, int* display);

/** \brief Draws an icon set. Call this every frame in your update function.
\param iconSet_p a pointer to the corresponding icon set
\param x the x coordinate of the icon set
\param y the y coordinate of the icon set
*/
void iconsDraw(IconSetPtr iconSet_p, float x, float y);

/** \brief Unloads memory allocated by an icon set.
\param iconSet_p a pointer to the corresponding icon set
*/
void iconsUnload(IconSetPtr iconSet_p);

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------Text-------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

/** \brief Draws text to the screen.
\param text the text to draw to the screen
\param font the font to use (see ui.h for all fonts)
\param size the pixel size of the font (currently supports
px8, px16, px32, px48, px64)
\param x the x position of the text
\param y the y position of the text
*/
void text(const char* text, Font font, FontSize size, float x, float y, float alpha);

/** \brief Draws centered text to the screen.
\param text the text to draw to the screen
\param font the font to use (see ui.h for all fonts)
\param size the pixel size of the font (currently supports
px8, px16, px32, px48, px64)
\param x the x position of the text
\param y the y position of the text
*/
void centerText(const char* myText, Font font, FontSize size, float x, float y, float alpha);

/** \brief Draws wavy text to the screen.
\param text the text to draw to the screen
\param font the font to use (see ui.h for all fonts)
\param size the pixel size of the font (currently supports
px8, px16, px32, px48, px64)
\param x the x position of the text
\param y the y position of the text
\param waveHeight the height of the wave
*/
void waveText(const char* text, Font font, FontSize size, float x, float y, float waveHeight, float waveSpeed, float alpha);

/** \brief Draws centered wavy text to the screen.
\param text the text to draw to the screen
\param font the font to use (see ui.h for all fonts)
\param size the pixel size of the font (currently supports
px8, px16, px32, px48, px64)
\param x the x position of the text
\param y the y position of the text
\param waveHeight the height of the wave
*/
void centerWaveText(const char* text, Font font, FontSize size, float x, float y, float waveHeight, float waveSpeed, float alpha);

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------Gfx--------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

/** \brief Draws a stroked line.
\param verts The line to draw. I haven't tested this with other types of meshes.
\param x the x origin of the line
\param y the y origin of the line
\param weight the weight of the stroke
\param alpha the transparency of the line
*/
void strokeLine(AEGfxVertexList* verts, float x, float y, int weight, float alpha);
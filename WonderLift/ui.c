//---------------------------------------------------------
// file:    ui.c
// project: WONDERLIFT
// author:  Coby Colson
// email:   coby.colson@digipen.edu
// course:	GAM150 - Spring 2020
//
// Copyright © 2020 DigiPen, All rights reserved.
//---------------------------------------------------------

#include "stdafx.h"

#include "AEEngine.h"
#include "ui.h"
#include "copper_utilities.h"
#include "inputcontroller.h"
#include "Sprite.h"
#include "time.h"
#include "coby_utilities.h"

int pxSize[FONTSIZE_HEAD] = { 8, 16, 32, 48, 64 };

AEGfxVertexList* glyphMesh[FONTSIZE_HEAD];
AEGfxTexture* glyphTex[FONT_HEAD];
SpritesheetPtr glyphSheet[FONT_HEAD];
SpritePtr glyphSprite;

static float scrollingTextTimer = 0.0f;
static char* scrollingTextBuffer;
static int scrollingTextIndex = 0;

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------- Resource Management---------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

/** \brief Initializes elements of the user interface.
*	Call this once in Main.c.
*/
void uiInit() {
	glyphMesh[px8] = MeshCreateQuad(4.0f, 4.0f, 1.0f / 95, 1.0f);
	glyphMesh[px16] = MeshCreateQuad(8.0f, 8.0f, 1.0f / 95, 1.0f);
	glyphMesh[px32] = MeshCreateQuad(16.0f, 16.0f, 1.0f / 95, 1.0f);
	glyphMesh[px48] = MeshCreateQuad(24.0f, 24.0f, 1.0f / 95, 1.0f);
	glyphMesh[px64] = MeshCreateQuad(32.0f, 32.0f, 1.0f / 95, 1.0f);

	glyphTex[P2P_WHITE] = AEGfxTextureLoad("./Assets/Text/glyphs_white.png");
	glyphTex[P2P_BLACK] = AEGfxTextureLoad("./Assets/Text/glyphs_black.png");
	glyphTex[P2P_YELLOW] = AEGfxTextureLoad("./Assets/Text/glyphs_yellow.png");
	glyphTex[P2P_GREEN] = AEGfxTextureLoad("./Assets/Text/glyphs_green.png");
	glyphTex[P2P_RED] = AEGfxTextureLoad("./Assets/Text/glyphs_red.png");
	glyphTex[P2P_BLUE] = AEGfxTextureLoad("./Assets/Text/glyphs_blue.png");

	glyphSheet[P2P_WHITE] = spritesheetCreate(glyphTex[P2P_WHITE], 1, 95);
	glyphSheet[P2P_BLACK] = spritesheetCreate(glyphTex[P2P_BLACK], 1, 95);
	glyphSheet[P2P_YELLOW] = spritesheetCreate(glyphTex[P2P_YELLOW], 1, 95);
	glyphSheet[P2P_GREEN] = spritesheetCreate(glyphTex[P2P_GREEN], 1, 95);
	glyphSheet[P2P_RED] = spritesheetCreate(glyphTex[P2P_RED], 1, 95);
	glyphSheet[P2P_BLUE] = spritesheetCreate(glyphTex[P2P_BLUE], 1, 95);

	glyphSprite = spriteCreateEmpty();
	scrollingTextBuffer = malloc(256);
}

/** \brief Frees elements of the user interface.
	Call this once before Main.c exits.
*/
void uiFree() {
	spriteFree(&glyphSprite);
	for (int i = 0; i < FONT_HEAD; i++) {
		spritesheetFree(&glyphSheet[i]);
	}
	for (int i = 0; i < FONT_HEAD; i++) {
		AEGfxTextureUnload(glyphTex[i]);
	}
	for (int i = 0; i < FONTSIZE_HEAD; i++) {
		AEGfxMeshFree(glyphMesh[i]);
	}
	free(scrollingTextBuffer);
}

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------Buttons----------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

/** \brief Creates a button set which can store interactable buttons.
* \param orientation orientation the orientation of the icon set (HORIZONTAL or VERTICAL)
* \param buttonWidth the pixel width of the buttons
* \param buttonHeight the pixel height of the buttons
* \param verticalPadding the pixel padding between buttons
* \param inactiveAlpha the alpha of buttons while inactive (0.0f - 1.0f)
* \return a pointer to the ButtonSet
*/
ButtonSetPtr buttonSetCreate(UIOrientation orientation, float buttonWidth, float buttonHeight, float padding, float inactiveAlpha) {
	ButtonSetPtr buttonSet_p = calloc(1, sizeof(ButtonSet));
	if (buttonSet_p != NULL) {
		buttonSet_p->buttons = NULL;
		buttonSet_p->buttonCount = 0;
		buttonSet_p->activeButton = 0;
		buttonSet_p->orientation = orientation;
		buttonSet_p->buttonWidth = buttonWidth;
		buttonSet_p->buttonHeight = buttonHeight;
		buttonSet_p->padding = padding;
		buttonSet_p->inactiveAlpha = inactiveAlpha;
		return buttonSet_p;
	}
	AE_ASSERT_MESG(buttonSet_p, "Attempted to allocate memory for a button set unsuccessfully.");
	return NULL;
}

/** \brief Creates a button which resides inside of a button set.
* \param buttonSet_p a pointer to the corresponding button set
* \param resourcePath the path to the image to use for the button (NOTE: may become deprecated when UI no longer uses images)
* \param action the void returning function defined in uibuttondefines.c
*/
void buttonCreate(ButtonSetPtr buttonSet_p, const char* resourcePath, Action action) {
	if (buttonSet_p == NULL) {
		AE_ASSERT_MESG(buttonSet_p, "Attempted to create a button without a valid button set.");
		return;
	}
	ButtonPtr buttons = buttonSet_p->buttons;
	if (buttons == NULL) {
		buttons = calloc(1, sizeof(Button));
	}
	else {
		ButtonPtr temp_p = realloc(buttons, sizeof(Button) * (buttonSet_p->buttonCount + 1));
		buttons = (temp_p == NULL ? buttons : temp_p);
	}
	if (buttons != NULL) {
		(buttons + buttonSet_p->buttonCount)->tex_p = AEGfxTextureLoad(resourcePath);
		(buttons + buttonSet_p->buttonCount)->action = action;
		buttonSet_p->buttons = buttons;
		buttonSet_p->buttonCount++;
	}
}

/** \brief Updates a button set's active button.
*	Call this every frame in your update function if you want to be able to change the active button.
* \param buttonSet_p a pointer to the corresponding button set
*/
void buttonsUpdate(ButtonSetPtr buttonSet_p) {
	ButtonPtr buttons = buttonSet_p->buttons;
	if (buttonSet_p->orientation == HORIZONTAL) {
		if (CuInputCheckReleased(BTN_L_RIGHT) || CuInputCheckReleased(BTN_R_RIGHT)) {
			if (buttonSet_p->activeButton < buttonSet_p->buttonCount - 1) {
				buttonSet_p->activeButton++;
			}
			else {
				buttonSet_p->activeButton = 0;
			}
		}
		else if (CuInputCheckReleased(BTN_L_LEFT) || CuInputCheckReleased(BTN_R_LEFT)) {
			if (buttonSet_p->activeButton > 0) {
				buttonSet_p->activeButton--;
			}
			else {
				buttonSet_p->activeButton = buttonSet_p->buttonCount - 1;
			}
		}
	}
	else if (buttonSet_p->orientation == VERTICAL) {
		if (CuInputCheckReleased(BTN_L_UP) || CuInputCheckReleased(BTN_R_UP)) {
			if (buttonSet_p->activeButton > 0) {
				buttonSet_p->activeButton--;
			}
			else {
				buttonSet_p->activeButton = buttonSet_p->buttonCount - 1;
			}
		}
		else if (CuInputCheckReleased(BTN_L_DOWN) || CuInputCheckReleased(BTN_R_DOWN)) {
			if (buttonSet_p->activeButton < buttonSet_p->buttonCount - 1) {
				buttonSet_p->activeButton++;
			}
			else {
				buttonSet_p->activeButton = 0;
			}
		}
	}
	if (CuInputCheckReleased(BTN_START) || CuInputCheckReleased(BTN_SELECT)) {
		if ((buttons + buttonSet_p->activeButton)->action != NULL) {
			(buttons + buttonSet_p->activeButton)->action();
		}
	}
}

/** \brief Draws a button set.
*	Call this every frame in your update function if you want to draw the buttons.
* \param buttonSet_p a pointer to the corresponding button set
* \param x the x coordinate of the button set
* \param y the y coordinate of the button set
*/
void buttonsDraw(ButtonSetPtr buttonSet_p, float x, float y) {
	ButtonPtr buttons = buttonSet_p->buttons;
	float width = buttonSet_p->buttonWidth;
	float height = buttonSet_p->buttonHeight;
	float padding = buttonSet_p->padding;
	if (buttons != NULL) {
		for (int i = 0; i < buttonSet_p->buttonCount; i++) {
			float alpha = (buttonSet_p->activeButton == i ? 1.0f : buttonSet_p->inactiveAlpha);
			if (buttonSet_p->orientation == HORIZONTAL) {
				imageCorner((buttons + i)->tex_p, x + (padding * i), y, width, height, alpha, 0.0f);
			}
			else if (buttonSet_p->orientation == VERTICAL) {
				imageCorner((buttons + i)->tex_p, x, y + (padding * i), width, height, alpha, 0.0f);
			}
		}
	}
}

/** \brief Unloads memory allocated by button sets.
* \param buttonSet_p a pointer to the corresponding button set
*/
void buttonsUnload(ButtonSetPtr buttonSet_p) {
	if (buttonSet_p != NULL) {
		if (buttonSet_p->buttons != NULL) {
			free(buttonSet_p->buttons);
		}
		free(buttonSet_p);
		buttonSet_p = NULL;
	}
	else {
		AE_ASSERT_MESG(buttonSet_p, "Attempted to free unallocated memory for buttons.");
	}
}

/** \brief Returns the active button in a button set.
*	You do not need to call this to monitor button presses.
*	However, it might be useful for displaying/hiding certain effects.
* \param buttonSet_p a pointer to the corresponding button set
* \return the active button
*/
int buttonsGetActiveButton(ButtonSetPtr buttonSet_p) {
	return buttonSet_p->activeButton;
}

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------Progress Bars--------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

/** \brief Recalculates the mesh used by the fill portion of a
*	progress bar if the display value has been changed.
* \param progBar_p a pointer to the corresponding progress bar
*/
AEGfxVertexList* progressBarGetMesh(ProgressBarPtr progBar_p) {
	float fillPercent = (float) * (progBar_p->display) / (float)progBar_p->max;
	float w = progBar_p->barWidth;
	float h = progBar_p->barHeight;
	u32 fill = progBar_p->fillColor;
	AEGfxVertexList* mesh_p;
	AEGfxMeshStart();
	if (progBar_p->orientation == HORIZONTAL) {
		AEGfxTriAdd(0.0f, h, fill, 0.0f, 0.0f,
			w * fillPercent, h, fill, 0.0f, 0.0f,
			0.0f, 0.0f, fill, 0.0f, 0.0f);
		AEGfxTriAdd(w * fillPercent, h, fill, 0.0f, 0.0f,
			w * fillPercent, 0.0f, fill, 0.0f, 0.0f,
			0.0f, 0.0f, fill, 0.0f, 0.0f);
	}
	else if (progBar_p->orientation == VERTICAL) {
		AEGfxTriAdd(0.0f, h, fill, 0.0f, 0.0f,
			w, h, fill, 0.0f, 0.0f,
			0.0f, (1.0f - fillPercent) * h, fill, 0.0f, 0.0f);
		AEGfxTriAdd(w, h, fill, 0.0f, 0.0f,
			w, (1.0f - fillPercent) * h, fill, 0.0f, 0.0f,
			0.0f, (1.0f - fillPercent) * h, fill, 0.0f, 0.0f);
	}
	mesh_p = AEGfxMeshEnd();
	return mesh_p;
}

/** \brief Creates a progress bar which can display the value of a desired integer.
* \param orientation orientation the orientation of the progress bar (HORIZONTAL or VERTICAL)
* \param interactable set to INTERACT if the player can alter the display value at runtime.
	otherwise, set to NO_INTERACT.
* \param fillColor the fill color of the progress bar
* \param emptyColor the empty color of the progress bar
* \param barWidth the pixel width of the entire progress bar
* \param barHeight the pixel height of the entire progress bar
* \param min the minimum value held by the display value. the display value will be clamped.
* \param max the maximum value held by the display value. the display value will be clamped.
* \param display a pointer to the integer to represent
* \return a pointer to the ProgressBar
*/
ProgressBarPtr progressBarCreate(UIOrientation orientation, UIInteract interactable, u32 fillColor, u32 emptyColor,
	float barWidth, float barHeight, int min, int max, int* display) {
	ProgressBarPtr progBar_p = calloc(1, sizeof(ProgressBar));
	if (progBar_p != NULL) {
		progBar_p->fillColor = fillColor;
		progBar_p->emptyColor = emptyColor;
		progBar_p->barWidth = barWidth;
		progBar_p->barHeight = barHeight;
		progBar_p->min = min;
		progBar_p->max = max;
		progBar_p->display = display;
		progBar_p->orientation = orientation;
		progBar_p->interactable = interactable;
		progBar_p->fill_verts_p = progressBarGetMesh(progBar_p);
		AEGfxVertexList* empty_verts_p;
		AEGfxMeshStart();
		if (orientation == HORIZONTAL) {
			AEGfxTriAdd(0.0f, barHeight, emptyColor, 0.0f, 0.0f,
				barWidth, barHeight, emptyColor, 0.0f, 0.0f,
				0.0f, 0.0f, emptyColor, 0.0f, 0.0f);
			AEGfxTriAdd(barWidth, barHeight, emptyColor, 0.0f, 0.0f,
				barWidth, 0.0f, emptyColor, 0.0f, 0.0f,
				0.0f, 0.0f, emptyColor, 0.0f, 0.0f);
		}
		else if (orientation == VERTICAL) {
			AEGfxTriAdd(0.0f, barHeight, emptyColor, 0.0f, 0.0f,
				barWidth, barHeight, emptyColor, 0.0f, 0.0f,
				0.0f, 0.0f, emptyColor, 0.0f, 0.0f);
			AEGfxTriAdd(barWidth, barHeight, emptyColor, 0.0f, 0.0f,
				barWidth, 0.0f, emptyColor, 0.0f, 0.0f,
				0.0f, 0.0f, emptyColor, 0.0f, 0.0f);
		}
		empty_verts_p = AEGfxMeshEnd();
		progBar_p->empty_verts_p = empty_verts_p;
		return progBar_p;
	}
	else {
		AE_ASSERT_MESG(progBar_p, "Attempted to allocate memory for a progress bar unsuccessfully.");
		return NULL;
	}
}

/** \brief Updates a progress bar by monitoring input and recalculating meshes.
*	Call this every frame in your update function if you want to update the progress bar.
* \param progBar_p a pointer to the corresponding progress bar
*/
void progressBarUpdate(ProgressBarPtr progBar_p) {
	*(progBar_p->display) = clampInt(*(progBar_p->display), progBar_p->min, progBar_p->max);
	if (progBar_p->displayTemp != *(progBar_p->display)) {
		// Value has changed outside this loop, manually update
		progBar_p->displayChange = 1;
	}
	if (progBar_p->displayChange) {
		// Value has changed since last frame - update mesh
		progBar_p->fill_verts_p = progressBarGetMesh(progBar_p);
		progBar_p->displayChange = 0;
	}
	if (progBar_p->interactable == INTERACT) {
		if (progBar_p->orientation == HORIZONTAL) {
			if (CuInputCheckCurr(BTN_L_LEFT) || CuInputCheckCurr(BTN_R_LEFT)) {
				if (*(progBar_p->display) > progBar_p->min) {
					--* (progBar_p->display);
					progBar_p->displayChange = 1;
				}
			}
			else if (CuInputCheckCurr(BTN_L_RIGHT) || CuInputCheckCurr(BTN_R_RIGHT)) {
				if (*(progBar_p->display) < progBar_p->max) {
					++* (progBar_p->display);
					progBar_p->displayChange = 1;
				}
			}
		}
		else if (progBar_p->orientation == VERTICAL) {
			if (CuInputCheckCurr(BTN_L_UP) || CuInputCheckCurr(BTN_R_UP)) {
				if (*(progBar_p->display) < progBar_p->max) {
					++* (progBar_p->display);
					progBar_p->displayChange = 1;
				}
			}
			else if (CuInputCheckCurr(BTN_L_DOWN) || CuInputCheckCurr(BTN_R_DOWN)) {
				if (*(progBar_p->display) > progBar_p->min) {
					--* (progBar_p->display);
					progBar_p->displayChange = 1;
				}
			}
		}
	}
	progBar_p->displayTemp = *(progBar_p->display);
}

/** \brief Draws a progress bar.
*	Call this every frame in your update function if you want to draw the progress bar.
* \param progBar_p a pointer to the corresponding progress bar
* \param x the x coordinate of the progress bar
* \param y the y coordinate of the progress bar
*/
void progressBarDraw(ProgressBarPtr progBar_p, float x, float y) {
	float camX, camY;
	AEGfxGetCamPosition(&camX, &camY);
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
	AEGfxSetTransparency(1.0f);
	AEGfxSetPosition(x + camX, y + camY);
	AEGfxMeshDraw(progBar_p->empty_verts_p, AE_GFX_MDM_TRIANGLES);
	AEGfxMeshDraw(progBar_p->fill_verts_p, AE_GFX_MDM_TRIANGLES);
}

/** \brief Unloads memory allocated by progress bars.
* \param progBar_p a pointer to the corresponding progress bar
*/
void progressBarUnload(ProgressBarPtr progBar_p) {
	AEGfxMeshFree(progBar_p->fill_verts_p);
	AEGfxMeshFree(progBar_p->empty_verts_p);
	free(progBar_p);
}

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------Icon Sets ( WIP: Do not use yet )---------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

/** \brief Creates an icon set which can display multiple images with associated values.
* \param orientation the orientation of the icon set (HORIZONTAL or VERTICAL)
* \param iconWidth the pixel width of the icons
* \param iconHeight the pixel height of the icons
* \param padding the pixel padding between icons
* \return a pointer to the new icon set
*/
IconSetPtr iconSetCreate(UIOrientation orientation, float iconWidth, float iconHeight, float padding) {
	IconSetPtr iconSet_p = calloc(1, sizeof(IconSet));
	if (iconSet_p != NULL) {
		iconSet_p->icons = NULL;
		iconSet_p->iconCount = 0;
		iconSet_p->orientation = orientation;
		iconSet_p->iconWidth = iconWidth;
		iconSet_p->iconHeight = iconHeight;
		iconSet_p->padding = padding;
		return iconSet_p;
	}
	AE_ASSERT_MESG(iconSet_p, "Attempted to allocate memory for an icon set unsuccessfully.");
	return NULL;
}

/** \brief Creates an icon associated with an icon set.
* \param iconSet_p a pointer to the corresponding icon set
* \param resourcePath the path to the image to use for the icon (NOTE: may become deprecated when UI no longer uses images)
* \param a pointer to the value displayed adjacent to the icon (NOTE: this feature will not work until text is working)
*/
void iconCreate(IconSetPtr iconSet_p, const char* resourcePath, int* display) {
	if (iconSet_p == NULL) {
		AE_ASSERT_MESG(iconSet_p, "Attempted to create an icon without a valid icon set.");
		return;
	}
	IconPtr icons = iconSet_p->icons;
	if (icons == NULL) {
		icons = calloc(1, sizeof(Icon));
	}
	else {
		IconPtr temp_p = realloc(icons, sizeof(Icon) * (iconSet_p->iconCount + 1));
		icons = (temp_p == NULL ? icons : temp_p);
	}
	if (icons != NULL) {
		(icons + iconSet_p->iconCount)->tex_p = AEGfxTextureLoad(resourcePath);
		(icons + iconSet_p->iconCount)->display = display;
		iconSet_p->icons = icons;
		iconSet_p->iconCount++;
	}
}

/** \brief Draws an icon set. Call this every frame in your update function.
* \param iconSet_p a pointer to the corresponding icon set
* \param x the x coordinate of the icon set
* \param y the y coordinate of the icon set
*/
void iconsDraw(IconSetPtr iconSet_p, float x, float y) {
	IconPtr icons = iconSet_p->icons;
	float width = iconSet_p->iconWidth;
	float height = iconSet_p->iconHeight;
	float padding = iconSet_p->padding;
	if (icons != NULL) {
		for (int i = 0; i < iconSet_p->iconCount; i++) {
			if (iconSet_p->orientation == HORIZONTAL) {
				imageCorner((icons + i)->tex_p, x + (padding * i), y, width, height, 1.f, 0.0f);
			}
			else if (iconSet_p->orientation == VERTICAL) {
				imageCorner((icons + i)->tex_p, x, y + (padding * i), width, height, 1.f, 0.0f);
			}

		}
	}
}

/** \brief Unloads memory allocated by an icon set.
* \param iconSet_p a pointer to the corresponding icon set
*/
void iconsUnload(IconSetPtr iconSet_p) {
	if (iconSet_p != NULL) {
		if (iconSet_p->icons != NULL) {
			free(iconSet_p->icons);
		}
		free(iconSet_p);
		iconSet_p = NULL;
	}
	else {
		AE_ASSERT_MESG(iconSet_p, "Attempted to free unallocated memory for icons.");
	}
}

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------Text-------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */


/** \brief Draws text to the screen.
* \param text the text to draw to the screen
* \param font the font to use (see ui.h for all fonts)
* \param size the pixel size of the font (currently supports
	px8, px16, px32, px64)
* \param x the x position of the text
* \param y the y position of the text
*/
void text(const char* text, Font font, FontSize size, float x, float y, float alpha) {
	if (!text)
		return;
	float xOffset = 0;
	float yOffset = 0;
	spriteSetMesh(glyphSprite, glyphMesh[size]);
	spriteSetSpritesheet(glyphSprite, glyphSheet[font]);
	spriteSetAlpha(glyphSprite, alpha);
	for (int i = 0; *text != '\0'; i++, text++) {
		if (*text >= ' ' && *text <= 'z') {
			int frameIndex = *text - ' ';
			spriteSetFrame(glyphSprite, frameIndex);
			spriteDraw(glyphSprite, x + xOffset, y + yOffset);
			xOffset += pxSize[size];
		}
		else if (*text == '\n') {
			xOffset = 0;
			yOffset += pxSize[size];
		}
	}
}

/** \brief Draws centered text to the screen.
* \param text the text to draw to the screen
* \param font the font to use (see ui.h for all fonts)
* \param size the pixel size of the font (currently supports
	px8, px16, px32, px48, px64)
* \param x the x position of the text
* \param y the y position of the text
*/
void centerText(const char* myText, Font font, FontSize size, float x, float y, float alpha) {
	text(myText, font, size,
		x - pxSize[size] * ((strlen(myText) / 2.0f) / (newlineCount(myText) + 1)) + (pxSize[size] / 2.0f), y, alpha);
}

/** \brief Draws wavy text to the screen.
* \param text the text to draw to the screen
* \param font the font to use (see ui.h for all fonts)
* \param size the pixel size of the font (currently supports
	px8, px16, px32, px48, px64)
* \param x the x position of the text
* \param y the y position of the text
* \param waveHeight the height of the wave
*/
void waveText(const char* text, Font font, FontSize size, float x, float y, float waveHeight, float waveSpeed, float alpha) {
	if (!text)
		return;
	float xOffset = 0;
	float yOffset = 0;
	spriteSetMesh(glyphSprite, glyphMesh[size]);
	spriteSetSpritesheet(glyphSprite, glyphSheet[font]);
	spriteSetAlpha(glyphSprite, alpha);
	for (int i = 0; *text != '\0'; i++, text++) {
		if (*text >= ' ' && *text <= 'z') {
			int frameIndex = *text - ' ';
			spriteSetFrame(glyphSprite, frameIndex);
			spriteDraw(glyphSprite, x + xOffset, y + yOffset + waveHeight * sinf(waveSpeed * getGameStateTime() + i));
			xOffset += pxSize[size];
		}
		else if (*text == '\n') {
			xOffset = 0;
			yOffset += pxSize[size];
		}
	}
}

/** \brief Draws centered wavy text to the screen.
* \param text the text to draw to the screen
* \param font the font to use (see ui.h for all fonts)
* \param size the pixel size of the font (currently supports
	px8, px16, px32, px48, px64)
* \param x the x position of the text
* \param y the y position of the text
* \param waveHeight the height of the wave
*/
void centerWaveText(const char* text, Font font, FontSize size, float x, float y, float waveHeight, float waveSpeed, float alpha) {
	waveText(text, font, size,
		x - pxSize[size] * ((strlen(text) / 2) / (newlineCount(text) + 1)) + (pxSize[size] / 2), y, waveHeight, waveSpeed, alpha);
}

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------Gfx--------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

/** \brief Draws a stroked line.
* \param verts The line to draw. I haven't tested this with other types of meshes.
* \param x the x origin of the line
* \param y the y origin of the line
* \param weight the weight of the stroke
*/
void strokeLine(AEGfxVertexList* verts, float x, float y, int weight, float alpha) {
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxTextureSet(NULL, 0.0f, 0.0f);
	AEGfxSetBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
	AEGfxSetTransparency(alpha);
	for (int i = -weight; i < weight; i++) {
		AEGfxSetPosition(x - 0.5f * i, y - 0.5f * i);
		AEGfxMeshDraw(verts, AE_GFX_MDM_LINES_STRIP);
		AEGfxSetPosition(x + 0.5f * i, y + 0.5f * i);
		AEGfxMeshDraw(verts, AE_GFX_MDM_LINES_STRIP);
		AEGfxSetPosition(x + 0.5f * i, y - 0.5f * i);
		AEGfxMeshDraw(verts, AE_GFX_MDM_LINES_STRIP);
		AEGfxSetPosition(x - 0.5f * i, y + 0.5f * i);
		AEGfxMeshDraw(verts, AE_GFX_MDM_LINES_STRIP);
	}
}
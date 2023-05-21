//---------------------------------------------------------
// file:    ui.c
// project: WONDERLIFT
// author:  Coby Colson
// email:   coby.colson@digipen.edu
// course:	GAM150 - Spring 2020
//
// Copyright � 2020 DigiPen, All rights reserved.
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

void buttonsUpdate(ButtonSetPtr buttonSet_p) {
	ButtonPtr buttons = buttonSet_p->buttons;
	// Update active button based on input events for HORIZONTAL orientation
	if (buttonSet_p->orientation == HORIZONTAL) {
		if (CuInputCheckReleased(BTN_L_RIGHT) || CuInputCheckReleased(BTN_R_RIGHT)) {
			if (buttonSet_p->activeButton < buttonSet_p->buttonCount - 1) {
				buttonSet_p->activeButton++;
			} else {
				buttonSet_p->activeButton = 0;
			}
		} 
		else if (CuInputCheckReleased(BTN_L_LEFT) || CuInputCheckReleased(BTN_R_LEFT)) {
			if (buttonSet_p->activeButton > 0) {
				buttonSet_p->activeButton--;
			} else {
				buttonSet_p->activeButton = buttonSet_p->buttonCount - 1;
			}
		}
	}
	// Update active button based on input events for VERTICAL orientation
	else if (buttonSet_p->orientation == VERTICAL) {
		if (CuInputCheckReleased(BTN_L_UP) || CuInputCheckReleased(BTN_R_UP)) {
			if (buttonSet_p->activeButton > 0) {
				buttonSet_p->activeButton--;
			} else {
				buttonSet_p->activeButton = buttonSet_p->buttonCount - 1;
			}
		} 
		else if (CuInputCheckReleased(BTN_L_DOWN) || CuInputCheckReleased(BTN_R_DOWN)) {
			if (buttonSet_p->activeButton < buttonSet_p->buttonCount - 1) {
				buttonSet_p->activeButton++;
			} else {
				buttonSet_p->activeButton = 0;
			}
		}
	}
	
	// Execute action associated with the active button if start or select button is released
	if (CuInputCheckReleased(BTN_START) || CuInputCheckReleased(BTN_SELECT)) {
		if ((buttons + buttonSet_p->activeButton)->action != NULL) {
			(buttons + buttonSet_p->activeButton)->action();
		}
	}
}

void buttonsDraw(ButtonSetPtr buttonSet_p, float x, float y) {
	ButtonPtr buttons = buttonSet_p->buttons;
	float width = buttonSet_p->buttonWidth;
	float height = buttonSet_p->buttonHeight;
	float padding = buttonSet_p->padding;
	
	// Check if there are buttons to draw
	if (buttons != NULL) {
		for (int i = 0; i < buttonSet_p->buttonCount; i++) {
			float alpha = (buttonSet_p->activeButton == i ? 1.0f : buttonSet_p->inactiveAlpha);
			
			// Draw buttons in HORIZONTAL orientation
			if (buttonSet_p->orientation == HORIZONTAL) {
				imageCorner((buttons + i)->tex_p, x + (padding * i), y, width, height, alpha, 0.0f);
			}
			// Draw buttons in VERTICAL orientation
			else if (buttonSet_p->orientation == VERTICAL) {
				imageCorner((buttons + i)->tex_p, x, y + (padding * i), width, height, alpha, 0.0f);
			}
		}
	}
}

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

int buttonsGetActiveButton(ButtonSetPtr buttonSet_p) {
	return buttonSet_p->activeButton;
}

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------Progress Bars--------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

AEGfxVertexList* progressBarGetMesh(ProgressBarPtr progBar_p) {
	// Calculate the fill percentage based on the current display value and maximum value
	float fillPercent = (float) *(progBar_p->display) / (float) progBar_p->max;
	float w = progBar_p->barWidth;
	float h = progBar_p->barHeight;
	u32 fill = progBar_p->fillColor;
	AEGfxVertexList* mesh_p;

	// Start creating a new mesh
	AEGfxMeshStart();

	// Generate triangles based on the orientation of the progress bar
	if (progBar_p->orientation == HORIZONTAL) {
		// Add the left triangle for the filled portion
		AEGfxTriAdd(0.0f, h, fill, 0.0f, 0.0f,
					w * fillPercent, h, fill, 0.0f, 0.0f,
					0.0f, 0.0f, fill, 0.0f, 0.0f);

		// Add the right triangle for the unfilled portion
		AEGfxTriAdd(w * fillPercent, h, fill, 0.0f, 0.0f,
					w * fillPercent, 0.0f, fill, 0.0f, 0.0f,
					0.0f, 0.0f, fill, 0.0f, 0.0f);
	}
	else if (progBar_p->orientation == VERTICAL) {
		// Add the top triangle for the filled portion
		AEGfxTriAdd(0.0f, h, fill, 0.0f, 0.0f,
					w, h, fill, 0.0f, 0.0f,
					0.0f, (1.0f - fillPercent) * h, fill, 0.0f, 0.0f);

		// Add the bottom triangle for the unfilled portion
		AEGfxTriAdd(w, h, fill, 0.0f, 0.0f,
					w, (1.0f - fillPercent) * h, fill, 0.0f, 0.0f,
					0.0f, (1.0f - fillPercent) * h, fill, 0.0f, 0.0f);
	}

	// Finalize the mesh creation
	mesh_p = AEGfxMeshEnd();

	return mesh_p;
}

ProgressBarPtr progressBarCreate(UIOrientation orientation, UIInteract interactable, u32 fillColor, u32 emptyColor,
	float barWidth, float barHeight, int min, int max, int* display) {
	ProgressBarPtr progBar_p = calloc(1, sizeof(ProgressBar));
	if (progBar_p != NULL) {
		// Initialize the progress bar properties
		progBar_p->fillColor = fillColor;
		progBar_p->emptyColor = emptyColor;
		progBar_p->barWidth = barWidth;
		progBar_p->barHeight = barHeight;
		progBar_p->min = min;
		progBar_p->max = max;
		progBar_p->display = display;
		progBar_p->orientation = orientation;
		progBar_p->interactable = interactable;

		// Generate the vertex list for the filled portion of the progress bar
		progBar_p->fill_verts_p = progressBarGetMesh(progBar_p);

		AEGfxVertexList* empty_verts_p;
		AEGfxMeshStart();

		// Generate triangles for the empty portion of the progress bar based on the orientation
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

		// Finalize the empty vertex list
		empty_verts_p = AEGfxMeshEnd();
		progBar_p->empty_verts_p = empty_verts_p;

		return progBar_p;
	}
	else {
		AE_ASSERT_MESG(progBar_p, "Attempted to allocate memory for a progress bar unsuccessfully.");
		return NULL;
	}
}

void progressBarUpdate(ProgressBarPtr progBar_p) {
	// Update the display value, clamping it between min and max
	*(progBar_p->display) = clampInt(*(progBar_p->display), progBar_p->min, progBar_p->max);

	// Check if the display value has changed since the last frame
	if (progBar_p->displayTemp != *(progBar_p->display)) {
		progBar_p->displayChange = 1; // Set the display change flag
	}

	// If the display has changed, update the fill vertices of the progress bar mesh
	if (progBar_p->displayChange) {
		progBar_p->fill_verts_p = progressBarGetMesh(progBar_p);
		progBar_p->displayChange = 0; // Reset the display change flag
	}

	// Check if the progress bar is interactable
	if (progBar_p->interactable == INTERACT) {
		// Handle interaction based on the orientation of the progress bar
		if (progBar_p->orientation == HORIZONTAL) {
			// Handle left and right input for horizontal progress bar
			if (CuInputCheckCurr(BTN_L_LEFT) || CuInputCheckCurr(BTN_R_LEFT)) {
				if (*(progBar_p->display) > progBar_p->min) {
					--*(progBar_p->display); // Decrease the display value
					progBar_p->displayChange = 1; // Set the display change flag
				}
			} 
			else if (CuInputCheckCurr(BTN_L_RIGHT) || CuInputCheckCurr(BTN_R_RIGHT)) {
				if (*(progBar_p->display) < progBar_p->max) {
					++*(progBar_p->display); // Increase the display value
					progBar_p->displayChange = 1; // Set the display change flag
				}
			}
		}
		else if (progBar_p->orientation == VERTICAL) {
			// Handle up and down input for vertical progress bar
			if (CuInputCheckCurr(BTN_L_UP) || CuInputCheckCurr(BTN_R_UP)) {
				if (*(progBar_p->display) < progBar_p->max) {
					++*(progBar_p->display); // Increase the display value
					progBar_p->displayChange = 1; // Set the display change flag
				}
			} 
			else if (CuInputCheckCurr(BTN_L_DOWN) || CuInputCheckCurr(BTN_R_DOWN)) {
				if (*(progBar_p->display) > progBar_p->min) {
					--*(progBar_p->display); // Decrease the display value
					progBar_p->displayChange = 1; // Set the display change flag
				}
			}
		}
	}

	progBar_p->displayTemp = *(progBar_p->display); // Update the temporary display value
}

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

void progressBarUnload(ProgressBarPtr progBar_p) {
	AEGfxMeshFree(progBar_p->fill_verts_p);
	AEGfxMeshFree(progBar_p->empty_verts_p);
	free(progBar_p);
}

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------Icon Sets ( WIP: Do not use yet )---------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

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

void text(const char* text, Font font, FontSize size, float x, float y, float alpha) {
	if (!text) {
		// If the text is NULL, there is nothing to render, so we return from the function.
		return;
	}

	float xOffset = 0;
	float yOffset = 0;

	// Set the mesh, spritesheet, and alpha values for the glyph sprite.
	spriteSetMesh(glyphSprite, glyphMesh[size]);
	spriteSetSpritesheet(glyphSprite, glyphSheet[font]);
	spriteSetAlpha(glyphSprite, alpha);

	// Loop through each character in the text string.
	for (int i = 0; *text != '\0'; i++, text++) {
		if (*text >= ' ' && *text <= 'z') {
			// If the character is within the valid range of printable characters, render it.
			int frameIndex = *text - ' ';

			// Set the frame index of the glyph sprite and draw it at the specified position.
			spriteSetFrame(glyphSprite, frameIndex);
			spriteDraw(glyphSprite, x + xOffset, y + yOffset);

			// Increment the xOffset by the size of the character to position the next character.
			xOffset += pxSize[size];
		}
		else if (*text == '\n') {
			// If a newline character is encountered, move to the next line.
			xOffset = 0;
			yOffset += pxSize[size];
		}
	}
}


void centerText(const char* myText, Font font, FontSize size, float x, float y, float alpha) {
	text(myText, font, size,
		x - pxSize[size] * ((strlen(myText) / 2.0f) / (newlineCount(myText) + 1)) + (pxSize[size] / 2.0f), y, alpha);
}

void waveText(const char* text, Font font, FontSize size, float x, float y, float waveHeight, float waveSpeed, float alpha) {
	if (!text) {
		// If the text is NULL, there is nothing to render, so we return from the function.
		return;
	}

	float xOffset = 0;
	float yOffset = 0;

	// Set the mesh, spritesheet, and alpha values for the glyph sprite.
	spriteSetMesh(glyphSprite, glyphMesh[size]);
	spriteSetSpritesheet(glyphSprite, glyphSheet[font]);
	spriteSetAlpha(glyphSprite, alpha);

	// Loop through each character in the text string.
	for (int i = 0; *text != '\0'; i++, text++) {
		if (*text >= ' ' && *text <= 'z') {
			// If the character is within the valid range of printable characters, render it.
			int frameIndex = *text - ' ';

			// Set the frame index of the glyph sprite.
			spriteSetFrame(glyphSprite, frameIndex);

			// Calculate the vertical displacement using a sine wave with the given waveHeight and waveSpeed.
			float yOffsetWave = waveHeight * sinf(waveSpeed * getGameStateTime() + i);

			// Draw the glyph sprite at the specified position with the vertical displacement.
			spriteDraw(glyphSprite, x + xOffset, y + yOffset + yOffsetWave);

			// Increment the xOffset by the size of the character to position the next character.
			xOffset += pxSize[size];
		}
		else if (*text == '\n') {
			// If a newline character is encountered, move to the next line.
			xOffset = 0;
			yOffset += pxSize[size];
		}
	}
}

void centerWaveText(const char* text, Font font, FontSize size, float x, float y, float waveHeight, float waveSpeed, float alpha) {
	waveText(text, font, size,
		x - pxSize[size] * ((strlen(text) / 2) / (newlineCount(text) + 1)) + (pxSize[size] / 2), y, waveHeight, waveSpeed, alpha);
}

/* ------------------------------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------Gfx--------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------- */

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
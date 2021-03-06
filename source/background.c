#include <math.h>

#include "background.h"
#include "water.h"
#include "gfx.h"

#include "logo_bin.h"
#include "bubble_bin.h"

#define BG_WATER_CONTROLPOINTS (100)
#define BG_WATER_NEIGHBORHOODS (3)
#define BG_WATER_DAMPFACTOR (0.7f)
#define BG_WATER_SPRINGFACTOR (0.85f)
#define BG_WATER_WIDTH (500)
#define BG_WATER_OFFSET (0)

#define CONFIG_3D_SLIDERSTATE (*(float*)0x1FF81080)

static bubble_t bubbles[BUBBLE_COUNT];
static waterEffect_s waterEffect;
static int backgroundCnt;

float yposlogo=54;
int logodir=1; //1 is down -1 is up
float logomov=0.75; //actual movement!

void initBackground(void)
{
	int i = 0;
	for(i = 0;i < BUBBLE_COUNT;i += 1)
	{
		bubbles[i].x = rand() % 400;
		bubbles[i].y = rand() % 240;
		bubbles[i].fade = 15;
	}

	initWaterEffect(&waterEffect, BG_WATER_CONTROLPOINTS, BG_WATER_NEIGHBORHOODS, BG_WATER_DAMPFACTOR, BG_WATER_SPRINGFACTOR, BG_WATER_WIDTH, BG_WATER_OFFSET);
	backgroundCnt = 0;
}

void updateBubble(bubble_t* bubble)
{
	// Float up the screen.
	bubble->y += 2;

	// Check if faded away, then reset if gone.
	if(bubble->fade < 10)
	{
		bubble->x = rand() % 400;
		bubble->y = rand() % 10;
		bubble->fade = 15;
	}
	// Check if too far up screen and start fizzling away.
	else if(bubble->y >= 240 && bubble->y % 240 > 100)
	{
		bubble->fade -= 10;
	}
	// Otherwise make sure the bubble is visible.
	else if(bubble->fade < 255)
	{
		bubble->fade += 10;
	}
}

void drawBubbles(void)
{
	int i;
	//BUBBLES!!
	for(i = 0;i < BUBBLE_COUNT;i += 1)
	{
		// Then draw (no point in separating more because then we go through them all twice).
		gfxDrawSpriteAlphaBlendFade((bubbles[i].y >= 240) ? (GFX_TOP) : (GFX_BOTTOM), GFX_LEFT, (u8*)bubble_bin, 32, 32, 
			((bubbles[i].y >= 240) ? -64 : 0) + bubbles[i].y % 240, 
			((bubbles[i].y >= 240) ? 0 : -40) + bubbles[i].x, bubbles[i].fade);
		//Maybe just top is enough but gave weird thing just then!
		if (CONFIG_3D_SLIDERSTATE > 0){
			gfxDrawSpriteAlphaBlendFade((bubbles[i].y >= 240) ? (GFX_TOP) : (GFX_BOTTOM), GFX_RIGHT, (u8*)bubble_bin, 32, 32, 
				((bubbles[i].y >= 240) ? -64 : 0) + bubbles[i].y % 240, 
				((bubbles[i].y >= 240) ? 0 : -40) + bubbles[i].x, bubbles[i].fade);
		}
	}
}

float randomFloat()
{
	return (float)rand()/(float)(RAND_MAX);
}

void updateBackground(void)
{
	int i;
	for(i = 0;i < BUBBLE_COUNT;i += 1)
	{
		// Update first
		updateBubble(&bubbles[i]);
	}

	exciteWater(&waterEffect, sin(backgroundCnt*0.1f)*2.0f, 0, true);

	//TODO : improve
	if((hidKeysDown()&KEY_UP) || hidKeysDown()&KEY_DOWN)
	{
		exciteWater(&waterEffect, 0.2f+randomFloat()*2.0f, rand()%BG_WATER_CONTROLPOINTS, false);
	}else if((hidKeysDown()&KEY_LEFT) || hidKeysDown()&KEY_RIGHT)
	{
		float v=3.0f+randomFloat()*1.0f;
		if(rand()%2)v=-v;
		int l=rand()%BG_WATER_CONTROLPOINTS;
		int i; for(i=0;i<5;i++)exciteWater(&waterEffect, v, l-2+i, false);
	}

	updateWaterEffect(&waterEffect);

	backgroundCnt++;
}

void drawBackground(u8 bgColor[3], u8 waterBorderColor[3], u8 waterColor[3])
{
	

	// Finally draw the logo. (in 3D if needed!)
	//uses x then y (unkown reason?)
	float slider=CONFIG_3D_SLIDERSTATE;
	//Animate logo
	if (yposlogo < 64){logodir=1;}
	if (yposlogo > 64){logodir=-1;}
	if (logodir == 1 && logomov < 0.75){logomov+=0.025;}
	if (logodir == -1 && logomov > -0.75){logomov-=0.025;}
	yposlogo+=logomov;
	//top screen stuff
	//gfxFillColorGradient(GFX_TOP, GFX_LEFT, waterBorderColor, waterColor);
	gfxFillColor(GFX_TOP, GFX_LEFT, bgColor);
	gfxDrawWave(GFX_TOP, GFX_LEFT, waterBorderColor, waterColor, 135, 20, 5, (gfxWaveCallback)&evaluateWater, &waterEffect);
	gfxDrawWave(GFX_TOP, GFX_LEFT, waterColor, waterBorderColor, 130, 20, 0, (gfxWaveCallback)&evaluateWater, &waterEffect);

	//sub screen stuff
	gfxFillColorGradient(GFX_BOTTOM, GFX_LEFT, waterColor, waterBorderColor);

	drawBubbles();
	if (!slider > 0){
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)logo_bin, 113, 271, yposlogo, 80);
	}else{
		u16 fbWidth, fbHeight;
		u8* fbLeft=gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &fbWidth, &fbHeight);
		u8* fbRight=gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, &fbWidth, &fbHeight);
		memcpy(fbRight, fbLeft, 400*240*3); //Copy left fb to right fb
		//Draw Logo in 3D!
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)logo_bin, 113, 271, yposlogo, 80+7*slider);
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_RIGHT, (u8*)logo_bin, 113, 271, yposlogo, 80-7*slider);
	}

}


#library "newpaint"

#include "zcommon/zcommon.h"

#define PLAYERMAX 64
#define COLORCOUNT 8

int IsServer;

str PaintColorActors[COLORCOUNT] =
{
    "DrawBulletBlack",
    "DrawBulletRed",
    "DrawBulletGreen",
    "DrawBulletBlue",
    "DrawBulletYellow",
    "DrawBulletMagenta",
    "DrawBulletCyan",
    "DrawBulletWhite",
};

str PaintColorPuffs[COLORCOUNT] =
{
    "PaintPuffBlack",
    "PaintPuffRed",
    "PaintPuffGreen",
    "PaintPuffBlue",
    "PaintPuffYellow",
    "PaintPuffMagenta",
    "PaintPuffCyan",
    "PaintPuffWhite",
};

str PaintColorGraphics[COLORCOUNT] =
{
    "CM_BLACK",
    "CM_RED",
    "CM_GREEN",
    "CM_BLUE",
    "CM_YELLO",
    "CM_MGNTA",
    "CM_CYAN",
    "CM_WHITE",
};


#include "painter_funcs.h"
#include "painter_scripts.h"
#include "painter_viewlock.h"
#include "painter_colorpick.h"
#include "painter_lerp.h"
#library "newpaint"

#include "zcommon.acs"

#define COLORCOUNT 8

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
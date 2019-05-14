#define COLORMENU_FADETIME  6

#define COLORMENU_BGTIME    4
#define COLORMENU_BGALPHA   0.35

#define COLORMENU_BASEDIST      175.0
#define COLORMENU_DISTSCALAR    12.0

#define COLORMENU_BASEINDEX 7532
#define COLORMENU_BGINDEX   7632

#define COLORMENU_MOUSEDECAY 0.85
#define COLORMENU_SELECTDIST 0.3


function int CPicker_GetMenuAlpha(int t)
{
    return fractionMult(COLORMENU_BGALPHA, middle(0, t, COLORMENU_BGTIME), COLORMENU_FADETIME);
}

function int CPicker_ColorDistance(int t)
{
    t = max(0, COLORMENU_FADETIME - t);
    return COLORMENU_BASEDIST + (COLORMENU_DISTSCALAR * t * t);
}


// makes it easier to target diagonal colors
// only works with 8 colors
function int CPicker_AdjustAngle(int ang)
{
    int angQuadrant = ang % 0.25;
    int angAdjusted = angQuadrant;
    
    if (angQuadrant >= 0.04 && angQuadrant < 0.125)
    {
        angAdjusted = min(0.125, angQuadrant + 0.05);
    }
    
    if (angQuadrant > 0.125 && angQuadrant <= 0.21)
    {
        angAdjusted = max(0.125, angQuadrant - 0.05);
    }
    
    return ang + (angAdjusted - angQuadrant);
}


script "Golf_ColorPicker" enter clientside
{
    int pln = PlayerNumber();
    
    SetDBEntry("projtest", "iszandronum", 8972345);
    int isZandronum = GetDBEntry("projtest", "iszandronum") == 8972345;
    
    if (isZandronum && pln != ConsolePlayerNumber())
    {
        terminate;
    }
    
    int menuActive = false;
    int displayTic = -1;
    
    int lastAngle, lastPitch;
    int i;
    
    int mouseX = 0;
    int mouseY = 0;
    int mouseDist = 0;
    int mouseAng  = 0;
    
    int lastMoveAng     = -1;
    int colorSelected   = -1;
    
    int firstForw, firstSide;
    
    while (1)
    {
        int buttons = GetPlayerInput(-1, MODINPUT_BUTTONS);
        
        if (HasPainterOut() && (buttons & BT_ALTATTACK))
        {
            if (!menuActive)
            {
                lastAngle     = GetActorAngle(0);
                lastPitch     = GetActorPitch(0);
                colorSelected = CheckInventory("ColorType");
                firstForw     = 1.0 * (!!(buttons & BT_FORWARD)   - !!(buttons & BT_BACK));
                firstSide     = 1.0 * (!!(buttons & BT_MOVERIGHT) - !!(buttons & BT_MOVELEFT));
            }
            
            ChangeActorAngle(0, lastAngle, true);
            ChangeActorPitch(0, lastPitch, true);
            lastAngle  = GetActorAngle(0);
            lastPitch  = GetActorPitch(0);
            
            menuActive = true;
            displayTic = middle(1, COLORMENU_FADETIME, displayTic + 1);
            
            int decayX, decayY;
            
            mouseDist = VectorLength(mouseX, mouseY);
            mouseAng  = VectorAngle(mouseX, mouseY);
            
            if (mouseDist < COLORMENU_SELECTDIST)
            {
                mouseDist = max(0, mouseDist - FixedMul(COLORMENU_SELECTDIST, 1.0 - COLORMENU_MOUSEDECAY));
            }
            else
            {
                mouseDist = FixedMul(mouseDist, COLORMENU_MOUSEDECAY);
            }
            
            mouseX = FixedMul(mouseDist, cos(mouseAng));
            mouseY = FixedMul(mouseDist, sin(mouseAng));
            
            int screenW  = GetScreenWidth();
            int screenH  = GetScreenHeight();
            
            int deltaX = GetPlayerInput(-1, INPUT_YAW);
            int deltaY = FixedMul(itofDiv(screenW, screenH), GetPlayerInput(-1, INPUT_PITCH));
            int sens   = itofDiv(GetUserCVar(pln, "golf_colorsens") * 2, 25);
            
            mouseX -= FixedMul(deltaX, sens);
            mouseY -= FixedMul(deltaY, sens);
        }
        else
        {
            menuActive = false;
            displayTic = max(-1, displayTic - 1);
            
            mouseX = 0;
            mouseY = 0;
            lastMoveAng = -1;
        }
        
        if (displayTic <= 0)
        {
            if (displayTic == 0)
            {
                HudMessage(s:""; HUDMSG_PLAIN, COLORMENU_BGINDEX, 0,0,0,0);
                HudMessage(s:""; HUDMSG_PLAIN, COLORMENU_BASEINDEX + COLORCOUNT, 0,0,0,0);
            
                for (i = 0; i < COLORCOUNT; i++)
                {
                    HudMessage(s:""; HUDMSG_PLAIN, COLORMENU_BASEINDEX + i, 0,0,0,0);
                }
            }
            
            Delay(1);
            continue;
        }
        
        // from this point on, the menu is at least slightly visible
        //  (no guarantees on being active, though)
        
        if (menuActive)
        {
            int newColor = colorSelected;
            
            mouseDist = VectorLength(mouseX, mouseY);
            mouseAng  = CPicker_AdjustAngle(VectorAngle(mouseX, mouseY));
            
            if (mouseDist >= COLORMENU_SELECTDIST)
            {
                newColor = oldRound(mouseAng * COLORCOUNT) % COLORCOUNT;
            }
            
            int forw = 1.0 * (!!(buttons & BT_FORWARD)   - !!(buttons & BT_BACK));
            int side = 1.0 * (!!(buttons & BT_MOVERIGHT) - !!(buttons & BT_MOVELEFT));
            int moveAng = VectorAngle(side, -forw);
            int moveLen = VectorLength(forw, side);
            
            if (forw != firstForw || side != firstSide)
            {
                firstForw = -2.0;
                firstSide = -2.0;
                
                if (moveLen > 0.5 && moveAng == lastMoveAng)
                {
                    newColor = oldRound(moveAng * COLORCOUNT) % COLORCOUNT;
                }
            }
            
            if (newColor != colorSelected)
            {
                colorSelected = newColor;
                
                if (isZandronum && GameType() != GAME_SINGLE_PLAYER)
                {
                    NamedRequestScriptPuke("Golf_SelectColor", colorSelected);
                }
                else
                {
                    ACS_NamedExecuteWithResult("Golf_SelectColor", colorSelected);
                }
            }
            
            if (moveLen <= 0.5) { lastMoveAng = -1; }
            else                { lastMoveAng = moveAng; }
        }
        
        
        SetFont("CMENU_BG");
        SetHudSize(1, 1, true);
        HudMessage(s:"A"; HUDMSG_PLAIN | HUDMSG_ALPHA, COLORMENU_BGINDEX, CR_UNTRANSLATED, 0.4, 0.4, 0, CPicker_GetMenuAlpha(displayTic));
        
        int colorDist = CPicker_ColorDistance(displayTic);
        SetHudSize(640, 480, true);
        
        for (i = 0; i < COLORCOUNT; i++)
        {
            int colorAngle = fractionMult(1.0, i, COLORCOUNT);
            int colorX = FixedMul(cos(colorAngle), colorDist);
            int colorY = FixedMul(sin(colorAngle), colorDist);
            
            colorX = setFraction(320.0 + colorX, 0.4);
            colorY = setFraction(240.0 + colorY, 0.4);
            
            SetFont(PaintColorGraphics[i]);
            HudMessage(s:"A"; HUDMSG_PLAIN, COLORMENU_BASEINDEX + i, CR_UNTRANSLATED, colorX, colorY, 0);
            
            if (i == colorSelected)
            {
                SetFont("CM_HOVER");
                HudMessage(s:"A"; HUDMSG_PLAIN, COLORMENU_BASEINDEX + COLORCOUNT, CR_UNTRANSLATED, colorX, colorY, 0);
            }
        }
        
        if (colorSelected == -1)
        {
            HudMessage(s:""; HUDMSG_PLAIN, COLORMENU_BASEINDEX + COLORCOUNT, 0,0,0,0);
        }    
        
        Delay(1);
    }
}
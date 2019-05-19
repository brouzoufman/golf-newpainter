int AimVector[3];

// For these functions:
//  - the range of screenX and screenY are [-1.0, 1.0], and correspond to the
//     edges of the given field of view
//  - negative screenX values go left, positive values go right
//  - negative screenY values go up, positive values go down
//
//  - angle, pitch, and fov are standard ACS angles (fov cvar 90 = fov argument 0.25)
//  - if worldStretch is true, adjust coordinates as if the world is stretched 
//    vertically by 20%, otherwise adjust coordinates as if points on the screen
//    are stretched vertically by 20%
//
// ex. if you pass in an fov of 0.25 (FOV 90), a screenX of -1.0 is angled 45
//      degrees to the left of the given angle, and a screenY of 1.0 is angled 45
//      degrees down from the given pitch

function void ScreenToWorld_True3D(int screenX, int screenY, int angle, int pitch, int fov, int worldStretch)
{
    if (worldStretch)
    {
        // the renderer recalcs pitch as if it was aiming at a point 20% higher
        //  (or whatever the pixel ratio is), so by doing the same thing to the
        //  left and up directions here, it cancels that adjustment out
        int xyPitchLength = cos(pitch);
        int xyPitchHeight = FixedMul(sin(pitch), 1.2);
        pitch = VectorAngle(xyPitchLength, xyPitchHeight);
    }
    
    Rotate3D(1.0,0,0, angle, pitch);
    int forwardX = Rotate3D_Ret[0];
    int forwardY = Rotate3D_Ret[1];
    int forwardZ = Rotate3D_Ret[2];
    
    Rotate3D(0,-1.0,0, angle, pitch);
    int rightX = Rotate3D_Ret[0];
    int rightY = Rotate3D_Ret[1];
    int rightZ = Rotate3D_Ret[2];
    
    Rotate3D(0,0,1.0, angle, pitch);
    int upX = Rotate3D_Ret[0];
    int upY = Rotate3D_Ret[1];
    int upZ = Rotate3D_Ret[2];
    
    if (worldStretch)
    {
        forwardZ = FixedDiv(forwardZ, 1.2);
        rightZ   = FixedDiv(rightZ,   1.2);
        upZ      = FixedDiv(upZ,      1.2);
    }
    else
    {
        upX = FixedDiv(upX, 1.2);
        upY = FixedDiv(upY, 1.2);
        upZ = FixedDiv(upZ, 1.2);
    }
    
    int fovScalar = cot(fov/2);
    forwardX = FixedMul(forwardX, fovScalar);
    forwardY = FixedMul(forwardY, fovScalar);
    forwardZ = FixedMul(forwardZ, fovScalar);
    
    int aimX   = forwardX - FixedMul(upX, screenY) + FixedMul(rightX, screenX);
    int aimY   = forwardY - FixedMul(upY, screenY) + FixedMul(rightY, screenX);
    int aimZ   = forwardZ - FixedMul(upZ, screenY) + FixedMul(rightZ, screenX);
    int aimMag = VectorLength(VectorLength(aimX, aimY), aimZ);
    
    AimVector[0] = FixedDiv(aimX, aimMag);
    AimVector[1] = FixedDiv(aimY, aimMag);
    AimVector[2] = FixedDiv(aimZ, aimMag);
}


function void ScreenToWorld_Software(int screenX, int screenY, int angle, int pitch, int fov)
{
    // The software renderer always renders as if your aim is parallel with the
    //  horizon, and to look up or down, it shifts your rendering viewport up
    //  and down. Therefore, our up vector here always points straight up, and
    //  our forward and right vectors are always parallel to the floor.
    //
    // The True3D function should produce identical results to this (precision
    //  permitting) when pitch is 0.
    
    int fovScalar = cot(fov/2);
    
    int forwardX = FixedMul(fovScalar, cos(angle));
    int forwardY = FixedMul(fovScalar, sin(angle));
    
    int rightX   = cos(angle - 0.25);
    int rightY   = sin(angle - 0.25);
    
    // FOV factors in here because "one screen up" means different things
    //  depending on your FOV, and not handling that means this only works
    //  properly at FOV 90
    int zAdjust  = -FixedMul(tan(pitch), fovScalar);
    
    int aimX = forwardX + FixedMul(rightX, screenX);
    int aimY = forwardY + FixedMul(rightY, screenX);
    int aimZ = zAdjust  - FixedDiv(screenY, 1.2);
    int aimMag = VectorLength(VectorLength(aimX, aimY), aimZ);
    
    AimVector[0] = FixedDiv(aimX, aimMag);
    AimVector[1] = FixedDiv(aimY, aimMag);
    AimVector[2] = FixedDiv(aimZ, aimMag);
}


function int StatusBarHeight(int w, int h, int gzdoom)
{
    return 0;
}



#define PAINT_XHAIR   -8973

script "Golf_PainterViewLock" enter clientside
{
    int pln = PlayerNumber();
    
    SetDBEntry("projtest", "iszandronum", 8972345);
    int isZandronum = GetDBEntry("projtest", "iszandronum") == 8972345;
    
    if (isZandronum && pln != ConsolePlayerNumber())
    {
        terminate;
    }
    
    int hudX = 0;
    int hudY = 0;
    int aimX = 0;
    int aimY = 0;
    int oldBarRatio = 0;
    
    int lastShot = -3;
    
    int hasZScript = ScriptCall("GolfStuff", "HasZScript");
    int software;
    
    int angle = GetActorAngle(0);
    int pitch = GetActorPitch(0);
    
    SetFont("PNTXHAIR");
    
    while (1)
    {
        int buttons = GetPlayerInput(-1, MODINPUT_BUTTONS);
        
        if (!(HasPainterOut() && (buttons & BT_RELOAD)) || (buttons & BT_ALTATTACK))
        {
            HudMessage(s:""; HUDMSG_PLAIN, PAINT_XHAIR, 0,0,0,0);
            angle = GetActorAngle(0);
            pitch = GetActorPitch(0);
            oldBarRatio = 0;
            hudX = 0;
            hudY = 0;
            
            // for if we're spraying without locking the view
            if (HasPainterOut() && (buttons & BT_ATTACK))
            {
                SprayPaint(0,0,0, isZandronum);
            }
            
            Delay(1);
            continue;
        }
        
        int screenW  = GetScreenWidth();
        int screenH  = GetScreenHeight();
        
        int deltaX = GetPlayerInput(-1, INPUT_YAW);
        int deltaY = FixedMul(itofDiv(screenW, screenH), GetPlayerInput(-1, INPUT_PITCH));
        int sens   = itofDiv(GetUserCVar(pln, "golf_cl_paintsens") * 2, 25);
        
        int oldHudX = hudX;
        int oldHudY = hudY;
        
        hudX -= FixedMul(deltaX, sens);
        hudY -= FixedMul(deltaY, sens);
        
        ChangeActorAngle(0, angle, true);
        ChangeActorPitch(0, pitch, true);
        
        int barH     = StatusBarHeight(screenW, screenH, hasZScript);
        int barRatio = itofDiv(barH, screenH);
        
        if (oldBarRatio != barRatio)
        {
            hudY -= barRatio - oldBarRatio;
        }
        
        hudX = middle(-1.0, hudX, 1.0);
        hudY = middle(-1.0, hudY, 1.0 - (barRatio * 2));
        
        int aspect = itofDiv(screenW, screenH);
        int drawX  = setFraction(320.0 + (240 * FixedMul(hudX, aspect)), 0.4);
        int drawY  = setFraction(240.0 + (240 * hudY), 0.4);
        
        SetHudSize(640, 480, true);
        HudMessage(s:"a"; HUDMSG_PLAIN | HUDMSG_ALPHA, PAINT_XHAIR, CR_UNTRANSLATED, drawX, drawY, 0);
        
        oldBarRatio = barRatio;
        
        // hudX and hudY are in the [-1.0, 1.0] range, where -1.0 is the left/top
        //  of the screen and 1.0 is the right/bottom of the screen
        //
        // the below code transforms it so that -1.0 corresponds to looking X
        //  degrees to the left/up, and 1.0 corresponds to looking X degrees to
        //  the right/down, where X is half your FOV
        
        int oldAimX = aimX;
        int oldAimY = aimY;
        
        aimX = FixedMul(hudX, itofDiv(screenW * 3, screenH * 4));
        aimY = FixedMul(hudY + barRatio, 0.75);
        
        // we need to keep aim[XY] updated for the delta stuff to work, but why
        //  do the rest if we aren't painting
        if (!(buttons & BT_ATTACK))
        {
            Delay(1);
            continue;
        }
        
        int x = GetActorX(0);
        int y = GetActorY(0);
        int z = GetActorZ(0) + GetActorViewHeight(0);
        
        angle = GetActorAngle(0);
        pitch = GetActorPitch(0);
        int fov = GetPlayerInfo(pln, PLAYERINFO_FOV);
        
        if (hasZScript)
        {
            software = GetCVar("vid_rendermode") < 2;
        }
        else
        {
            software = GetCVar("vid_renderer") == 0;
        }
        
        if (software) { fov = itofDiv(middle(5, fov, 167), 360); }
        else          { fov = itofDiv(middle(5, fov, 170), 360); }
        
        // we don't need the old delta values anymore
        deltaX = oldAimX - aimX;
        deltaY = oldAimY - aimY;
        int deltaLen   = VectorLength(deltaX, deltaY);
        int deltaIters = oldRound(FixedMul(deltaLen * 100, tan(fov/2)));
        
        if (!IsServer)
        {
            int interpCap = GetCVar("golf_sv_paintinterpcap") - 1;
            if (interpCap >= 0) { deltaIters = min(deltaIters, max(0, interpCap)); }
        }
        
        for (int i = 0; i <= deltaIters; i++)
        {
            int aimStepX, aimStepY;
            
            if (deltaIters == 0)
            {
                aimStepX = aimX;
                aimStepY = aimY;
            }
            else
            {
                aimStepX = aimX + fractionMult(deltaX, i, deltaIters);
                aimStepY = aimY + fractionMult(deltaY, i, deltaIters);
            }
            
            if (software) { ScreenToWorld_Software(aimStepX, aimStepY, angle, pitch, fov); }
            else          { ScreenToWorld_True3D(  aimStepX, aimStepY, angle, pitch, fov, hasZScript); }
            
            SprayPaint(AimVector[0], AimVector[1], AimVector[2], isZandronum);
        }
        
        Delay(1);
    }
}


function void SprayPaint(int aimX, int aimY, int aimZ, int isZand)
{
    if (!CheckInventory("PaintingEnabled")) { return; }
    
    int angle, pitch;
    
    if (aimX == 0 && aimY == 0 && aimZ == 0)
    {
        angle = GetActorAngle(0);
        pitch = GetActorPitch(0);
        
        aimX =  FixedMul(cos(angle), cos(pitch));
        aimY =  FixedMul(sin(angle), cos(pitch));
        aimZ = -sin(pitch);
    }
    
    if (isZand && GameType() != GAME_SINGLE_PLAYER)
    {
        NamedRequestScriptPuke("Golf_SprayPaint", aimX, aimY, aimZ);
    }
    else
    {
        ACS_NamedExecuteWithResult("Golf_SprayPaint", aimX, aimY, aimZ);
    }
}
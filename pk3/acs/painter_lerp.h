int LastInterpPoint[PLAYERMAX];


script "Painter_MarkSpot" (void)
{
    int spotTID = UniqueTID();
    int spotX = GetActorX(0);
    int spotY = GetActorY(0);
    int spotZ = GetActorZ(0);
    
    SetActivator(0, AAPTR_TARGET);
    int pln = PlayerNumber();
    if (pln == -1) { terminate; }
    
    SpawnForced("InterpPoint", spotX, spotY, spotZ, spotTID);
    
    if (LastInterpPoint[pln] == 0)
    {
        LastInterpPoint[pln] = spotTID;
        ACS_NamedExecuteWithResult("Painter_Interpolate", pln, spotTID);
    }
    else
    {
        LastInterpPoint[pln] = spotTID;
    }
}


#define INTERP_LIFETIME 2

script "Painter_Interpolate" (int pln, int startID)
{
    SetActivator(startID);
    
    int prevID = 0;
    int curID  = startID;
    int ticsLeft   = INTERP_LIFETIME;
    
    while (true)
    {
        Delay(1);
        
        int nextID = LastInterpPoint[pln];
        
        // Build anchors if we have three points, then shift everything 
        //  (including activator) forward
        if (nextID != curID)
        {
            if (prevID != 0)
            {
                ACS_NamedExecuteWithResult("Painter_BuildAnchors", prevID, curID, nextID);
                ACS_NamedExecuteWithResult("Painter_DrawBezier",   prevID, curID, pln);    
                ACS_NamedExecuteWithResult("Painter_DeletePoint",  prevID);
            }
            
            SetPointer(AAPTR_TARGET, nextID);
            SetActivator(nextID);
            
            prevID   = curID;
            curID    = nextID;
            ticsLeft = INTERP_LIFETIME;
        }
        else
        {
            ticsLeft--;
            
            if (ticsLeft < 1)
            {
                SetActivator(0);
                
                if (prevID == 0)
                {
                    int pointX = GetActorX(curID);
                    int pointY = GetActorY(curID);
                    int pointZ = GetActorZ(curID);
                    
                    ActivatorToPlayer(pln);
                    ACS_NamedExecuteWithResult("Painter_InterpSpray", pointX, pointY, pointZ);
                }
                else
                {
                    ACS_NamedExecuteWithResult("Painter_DrawBezier", prevID, curID, pln);
                    ACS_NamedExecuteWithResult("Painter_DeletePoint", prevID);
                }
                
                ACS_NamedExecuteWithResult("Painter_DeletePoint", curID);
                LastInterpPoint[pln] = 0;
                terminate;
            }
        }
    }
}


script "Painter_DeletePoint" (int pointID)
{
    if (!IsTIDUsed(pointID)) { terminate; }
    int anchorID1 = 0;
    int anchorID2 = 0;
    
    if (SetActivator(pointID, AAPTR_MASTER)) { anchorID1 = ActivatorTID(); }
    if (SetActivator(pointID, AAPTR_TRACER)) { anchorID2 = ActivatorTID(); }
    
    SetActivator(0);
    if (anchorID1) { Thing_Remove(anchorID1); }
    if (anchorID2) { Thing_Remove(anchorID2); }
    Thing_Remove(pointID);
}


script "Painter_BuildAnchors" (int startID, int midID, int endID)
{
    int startX = GetActorX(startID);
    int startY = GetActorY(startID);
    int startZ = GetActorZ(startID);
    
    int midX = GetActorX(midID);
    int midY = GetActorY(midID);
    int midZ = GetActorZ(midID);
    
    int endX = GetActorX(endID);
    int endY = GetActorY(endID);
    int endZ = GetActorZ(endID);
    
    int startEndX   = endX - startX;
    int startEndY   = endY - startY;
    int startEndZ   = endZ - startZ;
    int startEndLen = VectorLength(VectorLength(startEndX, startEndY), startEndZ);
    
    int normSrtEndX = FixedDiv(startEndX, startEndLen);
    int normSrtEndY = FixedDiv(startEndY, startEndLen);
    int normSrtEndZ = FixedDiv(startEndZ, startEndLen);
    
    int midStartX = startX - midX;
    int midStartY = startY - midY;
    int midStartZ = startZ - midZ;
    
    int midEndX   = endX - midX;
    int midEndY   = endY - midY;
    int midEndZ   = endZ - midZ;
    
    int midStartProj = dot3(midStartX, midStartY, midStartZ, normSrtEndX, normSrtEndY, normSrtEndZ);
    int midEndProj   = dot3(midEndX,   midEndY,   midEndZ,   normSrtEndX, normSrtEndY, normSrtEndZ);
    
    int startAncX, startAncY, startAncZ;
    int endAncX,   endAncY,   endAncZ;
    
    // if both points are on the same side, that's because of a very sharp turn,
    //  so forget having the curve be smooth in those cases
    if (sign(midStartProj) == sign(midEndProj))
    {
        startAncX = midX + (midStartX / 2) - (startEndX / 5);
        startAncY = midY + (midStartY / 2) - (startEndY / 5);
        startAncZ = midZ + (midStartZ / 2) - (startEndZ / 5);
        
        endAncX   = midX + (midEndX / 2) + (startEndX / 5);
        endAncY   = midY + (midEndY / 2) + (startEndY / 5);
        endAncZ   = midZ + (midEndZ / 2) + (startEndZ / 5);
        
        //SpawnForced("DebugInterpPoint1", midX,      midY,      midZ);
        //SpawnForced("DebugInterpPoint2", startAncX, startAncY, startAncZ);
        //SpawnForced("DebugInterpPoint2", endAncX,   endAncY,   endAncZ);
    }
    else
    {
        startAncX = midX + FixedMul(normSrtEndX, FixedMul(midStartProj, 0.35));
        startAncY = midY + FixedMul(normSrtEndY, FixedMul(midStartProj, 0.35));
        startAncZ = midZ + FixedMul(normSrtEndZ, FixedMul(midStartProj, 0.35));
        
        endAncX   = midX + FixedMul(normSrtEndX, FixedMul(midEndProj, 0.35));
        endAncY   = midY + FixedMul(normSrtEndY, FixedMul(midEndProj, 0.35));
        endAncZ   = midZ + FixedMul(normSrtEndZ, FixedMul(midEndProj, 0.35));
    }
    
    int startAncID = UniqueTID();
    int endAncID   = UniqueTID();
    
    SpawnForced("AnchorPoint", startAncX, startAncY, startAncZ, startAncID);
    SpawnForced("AnchorPoint", endAncX,   endAncY,   endAncZ,   endAncID);
    
    SetActivator(midID);
    SetPointer(AAPTR_MASTER, startAncID);
    SetPointer(AAPTR_TRACER, endAncID);
}

int BezierAnchors[4][3];
int BezierMidPoints[4][3];

#define BEZIER_POINTDENSITY 3.0
#define BEZIER_POINTCOUNT   50
#define B_X     0
#define B_Y     1
#define B_Z     2
#define B_LEN   3

int BezierPoints[BEZIER_POINTCOUNT+1][4];

script "Painter_DrawBezier" (int leftID, int rightID, int pln)
{
    int anchorCount = 1;
    
    SetActivator(leftID);
    BezierAnchors[0][0] = GetActorX(0);
    BezierAnchors[0][1] = GetActorY(0);
    BezierAnchors[0][2] = GetActorZ(0);
    
    if (SetActivator(0, AAPTR_TRACER))
    {
        BezierAnchors[1][0] = GetActorX(0);
        BezierAnchors[1][1] = GetActorY(0);
        BezierAnchors[1][2] = GetActorZ(0);
        anchorCount++;
    }
    
    if (SetActivator(rightID, AAPTR_MASTER))
    {
        BezierAnchors[anchorCount][0] = GetActorX(0);
        BezierAnchors[anchorCount][1] = GetActorY(0);
        BezierAnchors[anchorCount][2] = GetActorZ(0);
        anchorCount++;
    }
    
    SetActivator(rightID);
    BezierAnchors[anchorCount][0] = GetActorX(0);
    BezierAnchors[anchorCount][1] = GetActorY(0);
    BezierAnchors[anchorCount][2] = GetActorZ(0);
    anchorCount++;
        
    BezierPoints[0][B_X]   = BezierAnchors[0][0];
    BezierPoints[0][B_Y]   = BezierAnchors[0][1];
    BezierPoints[0][B_Z]   = BezierAnchors[0][2];
    BezierPoints[0][B_LEN] = 0;
    
    for (int i = 1; i <= BEZIER_POINTCOUNT; i++)
    {
        int dist = itofDiv(i, BEZIER_POINTCOUNT);
        
        for (int step = 0; step < anchorCount; step++)
        {
            for (int point = 0; point < anchorCount - step; point++)
            {
                if (step == 0)
                {
                    BezierMidPoints[point][0] = BezierAnchors[point][0];
                    BezierMidPoints[point][1] = BezierAnchors[point][1];
                    BezierMidPoints[point][2] = BezierAnchors[point][2];
                }
                else
                {
                    int pointX = BezierMidPoints[point][0];
                    int pointY = BezierMidPoints[point][1];
                    int pointZ = BezierMidPoints[point][2];
                    
                    int diffX = BezierMidPoints[point+1][0] - pointX;
                    int diffY = BezierMidPoints[point+1][1] - pointY;
                    int diffZ = BezierMidPoints[point+1][2] - pointZ;
                    
                    BezierMidPoints[point][0] = pointX + FixedMul(diffX, dist);
                    BezierMidPoints[point][1] = pointY + FixedMul(diffY, dist);
                    BezierMidPoints[point][2] = pointZ + FixedMul(diffZ, dist);
                }
            }
        }
        
        int endX = BezierMidPoints[0][0];
        int endY = BezierMidPoints[0][1];
        int endZ = BezierMidPoints[0][2];
        
        BezierPoints[i][B_X] = endX;
        BezierPoints[i][B_Y] = endY;
        BezierPoints[i][B_Z] = endZ;
        
        int newDist = VectorLength(VectorLength(BezierPoints[i-1][B_X] - endX,
                                                BezierPoints[i-1][B_Y] - endY),
                                                BezierPoints[i-1][B_Z] - endZ);
        
        BezierPoints[i][B_LEN] = BezierPoints[i-1][B_LEN] + newDist;
    }
    
    int length    = BezierPoints[BEZIER_POINTCOUNT][B_LEN];
    int steps     = oldRound(FixedDiv(length, BEZIER_POINTDENSITY));
        
    if (ConsolePlayerNumber() == -1)
    {
        int stepCap = GetCVar("golf_sv_paintinterpcap") - 1;
        if (stepCap >= 0) { steps = min(steps, max(0, stepCap)); }
    }
    
    ActivatorToPlayer(pln);
    
    if (steps > 0)
    {
        int stepDist  = length / steps;
        int curStep   = 1;
        int drawPoint = stepDist;
        
        for (i = 0; i < BEZIER_POINTCOUNT; i++)
        {
           int startX = BezierPoints[i][B_X];
           int startY = BezierPoints[i][B_Y];
           int startZ = BezierPoints[i][B_Z];
           
           endX = BezierPoints[i+1][B_X];
           endY = BezierPoints[i+1][B_Y];
           endZ = BezierPoints[i+1][B_Z];
           
           int startLength = BezierPoints[i][B_LEN];
           int endLength   = BezierPoints[i+1][B_LEN];
           
           while (drawPoint < endLength)
           {
               int betweenDist = FixedDiv(drawPoint - startLength, endLength - startLength);
               int betweenX    = startX + FixedMul(endX - startX, betweenDist);
               int betweenY    = startY + FixedMul(endY - startY, betweenDist);
               int betweenZ    = startZ + FixedMul(endZ - startZ, betweenDist);
               
               curStep++;
               drawPoint = fractionMult(length, curStep, steps);
               
               ACS_NamedExecuteWithResult("Painter_InterpSpray", betweenX, betweenY, betweenZ);
           }
        }
    }
    
    ACS_NamedExecuteWithResult("Painter_InterpSpray",
                                BezierPoints[BEZIER_POINTCOUNT][B_X],
                                BezierPoints[BEZIER_POINTCOUNT][B_Y],
                                BezierPoints[BEZIER_POINTCOUNT][B_Z]);
}


script "Painter_InterpSpray" (int spotX, int spotY, int spotZ)
{
    int startX = GetActorX(0);
    int startY = GetActorY(0);
    int startZ = GetActorZ(0);
    
    int diffX, diffY, diffZ;
    int colorIndex = CheckInventory("ColorType");
    
    if (ConsolePlayerNumber() == -1)
    {
        startZ += GetActorViewHeight(0) - Golf_GetFloorClip(0);
        
        diffX = spotX - startX;
        diffY = spotY - startY;
        diffZ = spotZ - startZ;
        int diffLen = VectorLength(VectorLength(diffX, diffY), diffZ);
        
        int vx = 320 * FixedDiv(diffX, diffLen);
        int vy = 320 * FixedDiv(diffY, diffLen);
        int vz = 320 * FixedDiv(diffZ, diffLen);
    
        int myTID_old = ActivatorTID();
        int myTID     = UniqueTID();
        Thing_ChangeTID(0, myTID);
        
        int projTID = UniqueTID();
        SpawnForced(PaintColorActors[colorIndex], startX, startY, startZ, projTID);
        SetActivator(projTID);
        
        SetPointer(AAPTR_TARGET, myTID);
        SetActorVelocity(0, vx, vy, vz, false, false);
        
        Thing_ChangeTID(myTID, myTID_old);
    }
    else
    {
        int attackHeight = (GetActorProperty(0, APROP_Height) / 2) + GetActorProperty(0, APROP_AttackZOffset);
        int heightScale  = FixedDiv(GetActorProperty(0, APROP_ViewHeight), GetActorViewHeight(0));
        
        startZ += FixedMul(attackHeight, heightScale);
        
        diffX = spotX - startX;
        diffY = spotY - startY;
        diffZ = spotZ - startZ;
        
        int angle =  VectorAngle(diffX, diffY);
        int pitch = -VectorAngle(VectorLength(diffX, diffY), diffZ);
        
        LineAttack(0, angle, pitch, 0, PaintColorPuffs[colorIndex], "None", 640.0, FHF_NORANDOMPUFFZ);
    }
}
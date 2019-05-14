script "Golf_SelectPainter" (int wand) clientside
{
    if (wand)
    {
        LocalAmbientSound("wand/select", 127);
    }
    else if (CheckInventory("IsMaestro") == 1)
    {
        LocalAmbientSound("8bpaint/select", 127);
    }
    else
    {
        LocalAmbientSound("paint/select", 127);
    }
}



script "Golf_SelectColor" (int index) net
{
    if (index < 0 || index >= COLORCOUNT) { terminate; }
    SetInventory("ColorType", index);
}



script "Golf_SprayPaint" (int aimX, int aimY, int aimZ) net
{
    // normalize this in case someone's getting cheeky with puke commands and is
    //  trying to make the server choke via lots of fastprojectiles going very fast
    //
    // but don't bother if it's close enough, because fixed point numbers don't
    //  have a lot of precision and every operation introduces more imprecision
    int aimLen = VectorLength(VectorLength(aimX, aimY), aimZ);
    if (aimLen == 0) { terminate; }
    
    if (aimLen < 0.9 || aimLen > 1.1)
    {
        aimX = FixedDiv(aimX, aimLen);
        aimY = FixedDiv(aimY, aimLen);
        aimZ = FixedDiv(aimZ, aimLen);
    }
    
    int myTID_old = ActivatorTID();
    int myTID     = UniqueTID();
    Thing_ChangeTID(0, myTID);
    
    int projTID = UniqueTID();
    int projX   = GetActorX(0);
    int projY   = GetActorY(0);
    int projZ   = GetActorZ(0) + GetActorViewHeight(0) - Golf_GetFloorClip(0);
    
    int colorIndex = CheckInventory("ColorType");
    SpawnForced(PaintColorActors[colorIndex], projX, projY, projZ, projTID);
    SetActivator(projTID);
    
    int projSpeed = GetActorProperty(0, APROP_Speed);
    int projVX    = FixedMul(projSpeed, aimX);
    int projVY    = FixedMul(projSpeed, aimY);
    int projVZ    = FixedMul(projSpeed, aimZ);
    
    SetPointer(AAPTR_TARGET, myTID);
    SetActorVelocity(0, projVX, projVY, projVZ, false, false);
    
    Thing_ChangeTID(myTID, myTID_old);
}
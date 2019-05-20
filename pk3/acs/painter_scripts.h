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
    int aimLen = VectorLength(VectorLength(aimX, aimY), aimZ);
    if (aimLen == 0) { terminate; }
    
    int aimAngle =  VectorAngle(aimX, aimY);
    int aimPitch = -VectorAngle(VectorLength(aimX, aimY), aimZ);
    
    int viewHeight   = GetActorViewHeight(0) - Golf_GetFloorClip(0);
    int zOffset      = GetActorProperty(0, APROP_AttackZOffset);
    int halfHeight   = GetActorProperty(0, APROP_Height) / 2;
    int heightScale  = FixedDiv(GetActorProperty(0, APROP_ViewHeight), GetActorViewHeight(0));
    
    SetActorProperty(0, APROP_AttackZOffset, FixedDiv(viewHeight - halfHeight, heightScale));
    LineAttack(0, aimAngle, aimPitch, 0, "PaintPuff", "None", 640.0, FHF_NORANDOMPUFFZ);
    SetActorProperty(0, APROP_AttackZOffset, zOffset);
}


script "Golf_ServerOpen" open
{
    IsServer = true;
}
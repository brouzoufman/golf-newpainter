// Once in commonFuncs, now here

function int itof(int x) { return x << 16; }

// setFraction( 9.4, 0.2) =  9.2
// setFraction(-9.4, 0.2) = -9.2
function int setFraction(int num, int frac)
{
    if (num < 0) { return (num & 0xFFFF0000) - frac; }
    return (num & 0xFFFF0000) + frac;
}

// multiply integer by fraction, and avoid overflows
function int fractionMult(int num, int mult, int div)
{
    return ((num / div) * mult) + (((num % div) * mult) / div);
}

// convert to fixed point and divide, while avoiding overflows
function int itofDiv(int x, int div)
{
    return ((x / div) << 16) + ((x % div) << 16) / div;
}


function int min(int x, int y)
{
    if (x < y) { return x; }
    return y;
}

function int max(int x, int y)
{
    if (x > y) { return x; }
    return y;
}

function int middle(int x, int y, int z)
{
    if ((x < z) && (y < z)) { return max(x, y); }
    return max(min(x, y), z);
}


// none of these four functions should be necessary
function int tan(int a)
{
    return FixedDiv(sin(a), cos(a));
}

function int cot(int a)
{
    return FixedDiv(cos(a), sin(a));
}

function int atan(int x)
{
    return VectorAngle(1.0, x);
}

function int acos(int x)
{
    return atan(FixedDiv(FixedSqrt(1.0 - FixedMul(x,x)), x));
}


function int oldRound(int toround)
{
    return (toround + 0.5) >> 16;
}

function int ceilTo(int num, int interval)
{
    return ((num + interval - 1) / interval) * interval;
}


function int mod(int x, int y)
{
    int ret = x - ((x / y) * y);
    if (ret < 0) { ret = y + ret; }
    return ret;
}

function int sign(int x)
{
    if (x < 0)  { return -1; }
    if (x == 0) { return 0; }
    return 1;
}

function int abs(int x)
{
    if (x < 0) { return -x; }
    return x;
}


// Shortest distance between two angles
function int angleDifference(int ang1, int ang2)
{
    ang1 = mod(ang1, 1.0);
    ang2 = mod(ang2, 1.0);

    int angDiff = ang2 - ang1;

    if (angDiff >= 0.5) { return angDiff - 1.0; }
    if (angDiff < -0.5) { return angDiff + 1.0; }
    return angDiff;
}


// Dot product of 3D vectors
function int dot3(int x1, int y1, int z1, int x2, int y2, int z2)
{
    return FixedMul(x1, x2) + FixedMul(y1, y2) + FixedMul(z1, z2);
}


int cross3_ret[3];

// Stores the cross product of the given 3D vectors in cross3_ret
function void cross3(int x1, int y1, int z1, int x2, int y2, int z2)
{
    cross3_ret[0] = FixedMul(y1, z2) - FixedMul(z1, y2);
    cross3_ret[1] = FixedMul(z1, x2) - FixedMul(x1, z2);
    cross3_ret[2] = FixedMul(x1, y2) - FixedMul(y1, x2);
}


// Calculates the angle between two 3D unit vectors
function int angle3Unit(int x1, int y1, int z1, int x2, int y2, int z2)
{
    return acos(dot3(x1,y1,z1, x2,y2,z2));
}


function void GiveAmmo(int type, int amount)
{
    int expected = CheckInventory(type) + amount;
    GiveInventory(type, amount);
    TakeInventory(type, CheckInventory(type) - expected);
}

function void SetInventory(int item, int amount)
{
    int count = CheckInventory(item);
    if (count == amount) { return; }

    if (count > amount)
    {
        TakeInventory(item, count - amount);
        return;
    }

    GiveAmmo(item, amount - count);
    return;
}


function int ActivatorToPlayer(int i)
{
    if (i < 0 || i >= PLAYERMAX) { return false; }
    if (i < 8) { SetActivator(0, AAPTR_PLAYER1 << i); }
    return SetActivatorToPlayer(i);
}


int Rotate3D_Ret[3];

function void Rotate3D(int x, int y, int z, int yaw, int pitch)
{
    // x' =  cos(angle) cos(pitch)x + -sin(angle)y + cos(angle) sin(pitch)z
    // y' =  sin(angle) cos(pitch)x +  cos(angle)y + sin(angle) sin(pitch)z
    // z' =            -sin(pitch)x                +            cos(pitch)z

    Rotate3D_Ret[0] = FixedMul(x, FixedMul(cos(yaw), cos(pitch)))
                    - FixedMul(y,          sin(yaw))
                    + FixedMul(z, FixedMul(cos(yaw), sin(pitch)));

    Rotate3D_Ret[1] = FixedMul(x, FixedMul(sin(yaw), cos(pitch)))
                    + FixedMul(y,          cos(yaw))
                    + FixedMul(z, FixedMul(sin(yaw), sin(pitch)));

    Rotate3D_Ret[2] = FixedMul(x,                   -sin(pitch))
                    + FixedMul(z,                    cos(pitch));
}



function int HasPainterOut(void)
{
    return CheckWeapon("NewPainter") || CheckWeapon("NewPaintWand");
}

function int WantsToPaint(void)
{
    return CheckInventory("PaintingEnabled") && (GetPlayerInput(-1, MODINPUT_BUTTONS) & BT_ATTACK);
}



#define FOOTCLIPTYPES 5

str KnownFloors[FOOTCLIPTYPES] = 
{
    "FLTWAWA1",
    "FLTFLWW1",
    "FLTLAVA1",
    "FLATHUH1",
    "FLTSLUD1",
};

int FloorFootClips[FOOTCLIPTYPES] =
{
    10.0,
    10.0,
    10.0,
    10.0,
    10.0,
};


function int Golf_GetFloorClip(int tid)
{
    if (GetActorFloorZ(0) < GetActorZ(0)) { return 0; }
    
    if (ScriptCall("GolfStuff", "HasZScript"))
    {
        return ScriptCall("GolfStuff", "GetFloorClip", tid);
    }
    
    str flat = GetActorFloorTexture(tid);
    int i;
    
    for (i = 0; i < FOOTCLIPTYPES; i++)
    {
        if (!stricmp(KnownFloors[i], flat))
        {
            return FloorFootClips[i];
        }
    }
    
    return 0;
}
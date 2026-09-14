#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265

// ======================================================
// SEASONS
// ======================================================
#define sedlife 0
#define SPRING 1
#define SUMMER 2
#define RAINY 3
#define AUTUMN 4
#define WINTER 5

int currentSeason = sedlife;
int targetSeason = sedlife;

bool paused = false;
bool automaticMode = false;

// ======================================================
// JOURNEY STATE  (F starts it, 9 ends it)
// ======================================================
bool journeyStarted = false;   // true while the car is rolling
bool manInCar = false;         // true only while he is driving

// ======================================================
// TRAVEL / TUNNEL STATE
// ======================================================

bool changingSeason = false;

int transitionStage = 0;

bool tunnelVisible = false;
bool tunnelOnRight = false;
bool tunnelOnLeft = false;
bool carInsideTunnel = false;

// ======================================================
// WORLD MOVEMENT
// ======================================================

float worldMove = 0.0f;

// worldSpeed starts at 0 (car parked), climbs to worldSpeedMax
// once the man is in the car, and brakes back to 0 after 9.
float worldSpeed = 0.0f;
float worldSpeedMax = 0.5f;
float worldAcceleration = 0.02f;
float worldBrake = 0.004f;

// --- Per-layer offsets (accumulated each frame) ---
float forestMove = 0.0f;      // trees
float grassMove = 0.0f;       // foreground grass field
float bgGrassMove = 0.0f;     // forest background grass strip
float flowerMove = 0.0f;      // spring flowers
float butterflyMove = 0.0f;   // spring butterflies
float mountainMove = 0.0f;    // distant mountains
float roadGrassMove = 0.0f;   // little grass tufts along the road edge
float winterBgMove = 0.0f;    // winter background sweep

float forestSpeedMul = 1.00f;
float grassSpeedMul = 1.05f;
float bgGrassSpeedMul = 0.85f;
float flowerSpeedMul = 1.00f;
float butterflySpeedMul = 1.00f;
float mountainSpeedMul = 0.25f;
float roadGrassSpeedMul = 1.10f;
float winterBgSpeedMul = 0.35f;

float carX = 0.0f;
float carY = -48.0f;

// The whole car is drawn through this scale, so the man behind
// the glass is big enough to see.
float carScale = 1.45f;

float transitionCarX = 0.0f;
float transitionCarSpeed = 1.5f;

float wheelRotation = 0.0f;

// ======================================================
// TUNNEL
// ======================================================

float tunnelRightX = 90.0f;
float tunnelLeftX = -90.0f;
float movingTunnelX = 90.0f;
float tunnelExitSpeed = 1.0f;

// ======================================================
// SEASON TEXT SCREEN
// ======================================================

float seasonScreenTimer = 0.0f;

// ======================================================
// TREE DATA
// ======================================================

// More trees, but with larger gaps between them.
// Trees exist outside the visible screen as well.
const int TREE_COUNT = 20;

float treeX[TREE_COUNT] = {
    -148, -132, -117, -101, -86, -70, -55, -39, -24, -8,
    7, 23, 38, 54, 69, 85, 100, 116, 131, 147
};

// Tree sizes
float treeScale[TREE_COUNT] = {
    0.65, 0.90, 0.75, 1.00, 0.80, 1.10, 0.70, 0.95, 0.75, 1.05,
    0.80, 0.90, 1.05, 0.70, 0.85, 0.75, 1.00, 0.80, 0.95, 0.65
};

// Y positions of trees
float treeY[TREE_COUNT] = {
    -23, -21, -25, -22, -24, -26, -22, -23, -21, -24,
    -22, -25, -21, -23, -22, -24, -20, -23, -22, -24
};

int treeType[TREE_COUNT] = {
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0
};

// ======================================================
// FOREST BACKGROUND GRASS
// ======================================================

const int FOREST_BG_COUNT = 32;

float forestBgX[FOREST_BG_COUNT] = {
    -145, -132, -119, -106, -93, -80, -67, -54, -41, -28,
    -15, -2, 11, 24, 37, 50, 63, 76, 89, 102,
    115, 128, 141, -125, -75, -25, 25, 75, 125, -100,
    100, -150
};

float forestBgScale[FOREST_BG_COUNT] = {
    0.8, 1.0, 0.7, 0.9, 1.1, 0.8, 0.6, 1.0, 0.9, 0.7,
    1.0, 0.8, 0.9, 1.1, 0.7, 0.8, 1.0, 0.6, 0.9, 0.8,
    1.0, 0.7, 0.9, 0.8, 1.1, 0.7, 0.9, 0.8, 1.0, 0.6,
    0.6, 0.7
};

// ======================================================
// GRASS DATA
// ======================================================

// Grass is also spread farther apart.
const int GRASS_COUNT = 40;

float grassX[GRASS_COUNT] = {
    -148, -135, -122, -109, -96, -83, -70, -57, -44, -31,
    -18, -5, 8, 21, 34, 47, 60, 73, 86, 99,
    112, 125, 138, -128, -102, -76, -50, -24, 4, 30,
    56, 82, 108, 134, -140, -65, -5, 45, 95, 145
};

float grassY[GRASS_COUNT] = {
    -22, -30, -25, -35, -28, -40, -24, -33, -45, -27,
    -38, -23, -42, -31, -26, -48, -29, -36, -22, -44,
    -34, -25, -50, -30, -39, -23, -46, -28, -33, -55,
    -37, -52, -41, -22, -47, -31, -24, -43, -53, -35
};

float grassScale[GRASS_COUNT] = {
    0.8, 1.0, 0.7, 0.9, 1.1, 0.8, 0.6, 1.0, 0.9, 0.7,
    1.0, 0.8, 0.9, 1.1, 0.7, 0.8, 1.0, 0.6, 0.9, 0.8,
    1.0, 0.7, 0.9, 0.8, 1.1, 0.7, 0.9, 0.8, 1.0, 0.6,
    0.85, 0.95, 0.65, 1.05, 0.75, 0.9, 0.7, 1.0, 0.6, 0.95
};

// ======================================================
// SPRING FLOWERS
// ======================================================

const int FLOWER_COUNT = 34;

float flowerX[FLOWER_COUNT] = {
    -145, -132, -119, -106, -93, -80, -67, -54, -41, -28,
    -15, -2, 11, 24, 37, 50, 63, 76, 89, 102,
    115, 128, 141, -125, -75, -25, 25, 75, 125, -100,
    100, -140, -50, 50
};

float flowerY[FLOWER_COUNT] = {
    -24, -30, -27, -34, -23, -31, -29, -25, -32, -30,
    -22, -33, -26, -36, -30, -24, -32, -28, -34, -30,
    -21, -35, -27, -32, -25, -36, -29, -23, -31, -35,
    -26, -33, -28, -30
};

float flowerScale[FLOWER_COUNT] = {
    0.65, 0.85, 0.70, 0.80, 0.95, 0.75, 0.90, 0.70, 0.85, 0.75,
    0.95, 0.80, 0.65, 0.90, 0.75, 0.85, 0.70, 0.95, 0.80, 0.70,
    0.90, 0.75, 0.85, 0.70, 0.80, 0.65, 0.90, 0.75, 0.85, 0.70,
    0.95, 0.75, 0.80, 0.70
};

int flowerColor[FLOWER_COUNT] = {
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1,
    2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1
};

// ======================================================
// BUTTERFLIES
// ======================================================

const int BUTTERFLY_COUNT = 8;

float butterflyBaseX[BUTTERFLY_COUNT] = {
    -78, -45, -18, 8, 30, 52, 72, 92
};

float butterflyBaseY[BUTTERFLY_COUNT] = {
    8, -4, 6, -2, -6, 7, -1, 5
};

float butterflyPhase[BUTTERFLY_COUNT] = {
    0.0f, 1.6f, 3.1f, 4.7f, 0.8f, 2.2f, 3.8f, 5.3f
};

float butterflyScale[BUTTERFLY_COUNT] = {
    0.75f, 1.0f, 0.8f, 0.9f, 1.1f, 0.75f, 0.95f, 0.8f
};

float butterflyTime = 0.0f;

// ======================================================
// BEE HIVE  (ambient, no button - always present in spring)
// ======================================================

const int BEE_HIVE_TREE_INDEX = 1;   // must be a round tree (type 0)

const int BEE_COUNT = 4;
float beePhase[BEE_COUNT] = { 0.0f, 1.7f, 3.4f, 5.1f };
float beeOrbitTime = 0.0f;

// ======================================================
// WINTER SNOW DATA
// ======================================================

const int SNOW_COUNT = 140;

float snowX[SNOW_COUNT];
float snowY[SNOW_COUNT];
float snowSpeed[SNOW_COUNT];

// ======================================================
// ROAD GRASS DATA
// ======================================================

const int ROAD_GRASS_COUNT = 6;

float roadGrassX[ROAD_GRASS_COUNT] = { -110, -70, -30, 15, 60, 105 };
float roadGrassY[ROAD_GRASS_COUNT] = { -54, -46, -52, -44, -55, -48 };
float roadGrassScale[ROAD_GRASS_COUNT] = { 0.28f, 0.22f, 0.30f, 0.24f, 0.27f, 0.23f };

// ======================================================
// AUTUMN LEAVES DATA
// ======================================================

const int AUTUMN_LEAF_COUNT = 26;

float autumnLeafBaseX[AUTUMN_LEAF_COUNT];
float autumnLeafY[AUTUMN_LEAF_COUNT];
float autumnLeafLandY[AUTUMN_LEAF_COUNT];   // this leaf's target landing height
float autumnLeafSpeed[AUTUMN_LEAF_COUNT];   // downward fall speed
float autumnLeafSway[AUTUMN_LEAF_COUNT];    // how wide it swings side to side
float autumnLeafPhase[AUTUMN_LEAF_COUNT];   // per-leaf phase offset
float autumnLeafRot[AUTUMN_LEAF_COUNT];     // current rotation (degrees)
float autumnLeafRotSpeed[AUTUMN_LEAF_COUNT];
int   autumnLeafColor[AUTUMN_LEAF_COUNT];   // 0=orange 1=red 2=yellow

// timer used for the side-to-side sway motion
float autumnTime = 0.0f;

const int MAX_LANDED_LEAVES = 90;

float landedLeafBaseX[MAX_LANDED_LEAVES];
float landedLeafY[MAX_LANDED_LEAVES];
float landedLeafRot[MAX_LANDED_LEAVES];
int   landedLeafColor[MAX_LANDED_LEAVES];
int   landedLeafCount = 0;

// ======================================================
// RAINY SEASON DATA
// ======================================================

const int RAIN_COUNT = 220;
float rainX[RAIN_COUNT];
float rainY[RAIN_COUNT];
float rainSpeed[RAIN_COUNT];

// Each puddle is a small irregular polygon, so they read as
// real splashes of water rather than perfect ellipses.
const int PUDDLE_COUNT = 4;
const int PUDDLE_VERTS = 6;

float puddleVertX[PUDDLE_COUNT][PUDDLE_VERTS] = {
    { -85, -65, -60, -68, -83, -88 },
    { -38, -15, -10, -18, -35, -42 },
    {  20,  43,  48,  38,  18,  15 },
    {  60,  82,  88,  78,  58,  55 }
};

float puddleVertY[PUDDLE_COUNT][PUDDLE_VERTS] = {
    { -36, -36, -38, -40, -39, -37 },
    { -49, -49, -51, -53, -52, -50 },
    { -34, -34, -36, -38, -37, -35 },
    { -53, -53, -55, -57, -56, -54 }
};

float puddleCenterX[PUDDLE_COUNT] = { -75, -26, 30, 70 };

// Lightning is a single bright flash that decays away; T fires it.
float lightningAlpha = 0.0f;
float lightningBoltX = 0.0f;

// ======================================================
// CLOUD VARIABLES
// ======================================================

float cloud1X = -80;
float cloud2X = -10;
float cloud3X = 60;
float cloud4X = -50;
float cloud5X = 40;

// ======================================================
// EXTRA EFFECTS STATE  (J / K / N)
// ======================================================

// --- Duck / animal crossing ---
bool duckActive = false;
float duckX = 0.0f;
float duckY = -38.0f;
float duckTimer = 0.0f;

bool duckStopActive = false;
bool duckWaitingResume = false;
float duckBrake = 0.02f;   // how hard the car brakes for them

const int DUCKLING_COUNT = 3;

#define DUCK_TRAIL_LEN 150
float duckTrailX[DUCK_TRAIL_LEN];
float duckTrailY[DUCK_TRAIL_LEN];
int duckTrailHead = 0;     // index the NEXT sample will be written to
int duckTrailCount = 0;    // how many valid samples exist so far

float ducklingLateral[DUCKLING_COUNT] = { -1.2f, 1.0f, -0.6f };   // small single-file stagger
int ducklingDelayFrames[DUCKLING_COUNT] = { 9, 18, 28 };           // how far back in the trail each one reads

// --- Bird flock flying across the sky ---
const int BIRD_COUNT = 6;
bool birdActive = false;
float birdTimer = 0.0f;
float flockX = -140.0f;
float flockY = 40.0f;

// Offsets behind/around the lead bird, forming a loose V. Colors
// cycle blue/yellow/red/white across the six birds.
float birdOffsetX[BIRD_COUNT] = { 0.0f, -4.5f, -4.5f, -9.0f, -9.0f, -13.5f };
float birdOffsetY[BIRD_COUNT] = { 0.0f,  2.2f, -2.2f,  4.4f, -4.4f,  6.2f };
float birdFlapPhase[BIRD_COUNT] = { 0.0f, 1.1f, 2.3f, 3.4f, 4.6f, 5.7f };

bool povActive = false;

// 0 = not blinking, 1 = blinking INTO the pov, 2 = blinking back
// OUT to the normal third-person view.
int povBlinkStage = 0;
float povBlinkTimer = 0.0f;
float povBlinkDuration = 1.0f;   // ~0.3s at the 0.05/frame pace used elsewhere

float povWheelTime = 0.0f;       // drives the road-dash scroll and the wheel's idle sway

// ======================================================
// STEERING  (A / D)
// ======================================================
bool steerLeftDown = false;
bool steerRightDown = false;
float carSteerSpeed = 0.9f;
float carSteerMin = -70.0f;
float carSteerMax = 70.0f;

// ======================================================
// BASIC SHAPES
// ======================================================

// (Talha) filled quad
void rectangle(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

// (Talha) filled circle (100 segments)
void circle(float x, float y, float radius)
{
    glBegin(GL_POLYGON);
    for(int i = 0; i < 100; i++)
    {
        float angle = 2.0f * PI * i / 100.0f;
        float px = x + radius * cos(angle);
        float py = y + radius * sin(angle);
        glVertex2f(px, py);
    }
    glEnd();
}

// (Talha) filled triangle
void triangleShape(float x1, float y1, float x2, float y2, float x3, float y3)
{
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}

// (Sanzida) oval (stretched circle)
void ellipse(float x, float y, float rx, float ry)
{
    glBegin(GL_POLYGON);
    for(int i = 0; i < 60; i++)
    {
        float angle = 2.0f * PI * i / 60.0f;
        glVertex2f(x + rx * cos(angle), y + ry * sin(angle));
    }
    glEnd();
}

// (Sanzida) quad rotated along a direction vector
void orientedQuad(float cx, float cy, float ux, float uy, float halfLen, float halfWidth)
{
    float vx = -uy;
    float vy = ux;

    glBegin(GL_QUADS);
    glVertex2f(cx + ux * halfLen + vx * halfWidth, cy + uy * halfLen + vy * halfWidth);
    glVertex2f(cx - ux * halfLen + vx * halfWidth, cy - uy * halfLen + vy * halfWidth);
    glVertex2f(cx - ux * halfLen - vx * halfWidth, cy - uy * halfLen - vy * halfWidth);
    glVertex2f(cx + ux * halfLen - vx * halfWidth, cy + uy * halfLen - vy * halfWidth);
    glEnd();
}

// (Talha) bitmap text
void drawText(float x, float y, const char *text)
{
    glRasterPos2f(x, y);
    for(int i = 0; text[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}

// (Talha) lighten / darken a base colour
void shadedColor(int r, int g, int b, float factor)
{
    int nr = (int)(r * factor);
    int ng = (int)(g * factor);
    int nb = (int)(b * factor);
    if(nr < 0) nr = 0;
    if(nr > 255) nr = 255;
    if(ng < 0) ng = 0;
    if(ng > 255) ng = 255;
    if(nb < 0) nb = 0;
    if(nb > 255) nb = 255;
    glColor3ub((GLubyte)nr, (GLubyte)ng, (GLubyte)nb);
}

// ======================================================
// WINTER SNOW
// ======================================================

// ======================================================
// AUTUMN LEAVES  -  SPAWN / INIT / RESET
// ======================================================

// (Sanzida) send one leaf back up to a tree canopy
void respawnAutumnLeaf(int i)
{
    int treeIndex = rand() % TREE_COUNT;

    autumnLeafBaseX[i] = treeX[treeIndex] + (float)((rand() % 25) - 12);
    autumnLeafY[i] = treeY[treeIndex] + (55.0f + (float)(rand() % 20)) * treeScale[treeIndex];

    autumnLeafSpeed[i] = 0.18f + (float)(rand() % 25) / 100.0f;
    autumnLeafSway[i] = 3.0f + (float)(rand() % 5);
    autumnLeafPhase[i] = (float)(rand() % 628) / 100.0f;
    autumnLeafRot[i] = (float)(rand() % 360);
    autumnLeafRotSpeed[i] = -3.0f + (float)(rand() % 60) / 10.0f;
    autumnLeafColor[i] = rand() % 3;

    // ~72% land on the green grass strip right under the trees,
    // ~28% get blown further down onto the road.
    if(rand() % 100 < 72)
        autumnLeafLandY[i] = -22.0f - (float)(rand() % 16); // grass: -22 .. -37
    else
        autumnLeafLandY[i] = -43.0f - (float)(rand() % 13); // road:  -43 .. -55
}

// (Sanzida) seed the falling leaves
void initAutumnLeaves()
{
    for(int i = 0; i < AUTUMN_LEAF_COUNT; i++)
    {
        respawnAutumnLeaf(i);

        // Scatter their starting heights on init so they don't
        // all begin mid-canopy in one synchronized wave.
        autumnLeafY[i] -= (float)(rand() % 120);
    }

    landedLeafCount = 0;
}

// (Sanzida) clear the ground leaves
void resetAutumnLeaves()
{
    landedLeafCount = 0;

    for(int i = 0; i < AUTUMN_LEAF_COUNT; i++)
        respawnAutumnLeaf(i);
}

// (Susmita) seed the snowflakes
void initSnow()
{
    for(int i = 0; i < SNOW_COUNT; i++)
    {
        snowX[i] = -100.0f + (float)(rand() % 201);
        snowY[i] = -60.0f + (float)(rand() % 161);
        snowSpeed[i] = 0.20f + (float)(rand() % 30) / 100.0f;
    }
}

// (Susmita) snowfall
void drawSnow()
{
    if(currentSeason != WINTER)
        return;
    glColor3ub(255, 255, 255);
    for(int i = 0; i < SNOW_COUNT; i++)
    {
        circle(snowX[i], snowY[i], 0.35f + (i % 4) * 0.16f);
    }
}

// (Susmita) snowflake fall and reset
void updateSnow()
{
    if(currentSeason != WINTER)
        return;
    for(int i = 0; i < SNOW_COUNT; i++)
    {
        snowY[i] -= snowSpeed[i];
        snowX[i] -= 0.03f;
        if(snowY[i] < -60)
        {
            snowY[i] = 100;
            snowX[i] = -100.0f + (float)(rand() % 201);
        }
        if(snowX[i] < -105)
            snowX[i] = 105;
    }
}

// ======================================================
// AUTUMN LEAVES  -  DRAW / UPDATE
// ======================================================

// (Sanzida) one rotated leaf
void drawLeafShape(float cx, float cy, float size, float rotDeg, int colorType)
{
    switch(colorType)
    {
        case 0:  glColor3ub(224, 122, 25); break; // orange
        case 1:  glColor3ub(178, 48, 32);  break; // red
        default: glColor3ub(230, 178, 40); break; // yellow
    }

    float rad = rotDeg * PI / 180.0f;
    float c = cos(rad);
    float s = sin(rad);

    // Local-space kite shape (tip up, wide middle, short base)
    float lx[4] = { 0.0f,        size * 0.55f, 0.0f,         -size * 0.55f };
    float ly[4] = { size * 1.0f, size * 0.15f, -size * 0.65f, size * 0.15f };

    glBegin(GL_QUADS);
    for(int i = 0; i < 4; i++)
    {
        float rx = lx[i] * c - ly[i] * s;
        float ry = lx[i] * s + ly[i] * c;
        glVertex2f(cx + rx, cy + ry);
    }
    glEnd();

    // Center vein/stem line, rotated along with the leaf
    glColor3ub(110, 55, 20);
    glLineWidth(1.0f);

    float vx1 = -size * 0.65f * s;
    float vy1 =  size * 0.65f * c;
    float vx2 =  size * 1.0f * s;
    float vy2 = -size * 1.0f * c;

    glBegin(GL_LINES);
    glVertex2f(cx + vx1, cy + vy1);
    glVertex2f(cx + vx2, cy + vy2);
    glEnd();
}

// (Sanzida) leaves still falling
void drawFallingAutumnLeaves()
{
    if(currentSeason != AUTUMN)
        return;

    for(int i = 0; i < AUTUMN_LEAF_COUNT; i++)
    {
        float bx = autumnLeafBaseX[i] + forestMove;
        while(bx > 150) bx -= 300;
        while(bx < -150) bx += 300;

        float sway = sin(autumnTime * 1.4f + autumnLeafPhase[i]) * autumnLeafSway[i];

        drawLeafShape(bx + sway, autumnLeafY[i], 2.2f, autumnLeafRot[i], autumnLeafColor[i]);
    }
}

// (Sanzida) leaves resting on the ground
void drawLandedAutumnLeaves()
{
    if(currentSeason != AUTUMN)
        return;

    for(int i = 0; i < landedLeafCount; i++)
    {
        float bx = landedLeafBaseX[i] + forestMove;
        while(bx > 150) bx -= 300;
        while(bx < -150) bx += 300;

        drawLeafShape(bx, landedLeafY[i], 1.8f, landedLeafRot[i], landedLeafColor[i]);
    }
}

// (Sanzida) fall, sway and land
void updateAutumnLeaves()
{
    if(currentSeason != AUTUMN)
        return;

    for(int i = 0; i < AUTUMN_LEAF_COUNT; i++)
    {
        autumnLeafY[i] -= autumnLeafSpeed[i];
        autumnLeafRot[i] += autumnLeafRotSpeed[i];

        if(autumnLeafY[i] <= autumnLeafLandY[i])
        {
            if(landedLeafCount < MAX_LANDED_LEAVES)
            {
                float sway = sin(autumnTime * 1.4f + autumnLeafPhase[i]) * autumnLeafSway[i];

                landedLeafBaseX[landedLeafCount] = autumnLeafBaseX[i] + sway;
                landedLeafY[landedLeafCount] = autumnLeafLandY[i];
                landedLeafRot[landedLeafCount] = (float)(rand() % 360);
                landedLeafColor[landedLeafCount] = autumnLeafColor[i];

                landedLeafCount++;
            }

            respawnAutumnLeaf(i);
        }
    }
}

// ======================================================
// SKY / SUN / GROUND
// ======================================================

// (Talha) sky, colour per season
void drawSky()
{
    if(currentSeason == SPRING)
        glColor3ub(150, 217, 255);
    else if(currentSeason == SUMMER)
        glColor3ub(105, 195, 255);
    else if(currentSeason == RAINY)
        glColor3ub(70, 78, 92);
    else if(currentSeason == WINTER)
        glColor3ub(180, 210, 235);
    else if(currentSeason == AUTUMN)
        glColor3ub(190, 210, 230);
    else
        glColor3ub(128, 204, 255);
    rectangle(-100, -20, 100, 100);
}

// (Talha) sun disc
void drawSun()
{
    // No sun disc in winter, and none behind the rain clouds either.
    if(currentSeason == WINTER || currentSeason == RAINY)
        return;
    if(currentSeason == SUMMER)
        glColor3ub(255, 180, 20);
    else
        glColor3ub(255, 217, 26);
    circle(75, 78, 9);
}

// (Talha) ground band
void drawGround()
{
    if(currentSeason == SPRING)
        glColor3ub(72, 168, 58);
    else if(currentSeason == SUMMER)
        glColor3ub(190, 170, 75);
    else if(currentSeason == RAINY)
        glColor3ub(30, 68, 34);
    else if(currentSeason == WINTER)
        glColor3ub(235, 240, 245);
    else if(currentSeason == AUTUMN)
        glColor3ub(150, 120, 55);
    else
        glColor3ub(51, 140, 46);
    rectangle(-100, -60, 100, -20);
}

// ======================================================
// CLOUDS
// ======================================================

// (Talha) one cloud
void drawCloud(float x, float y, float size)
{
    // Heavy, near-black storm clouds in the rainy season.
    if(currentSeason == RAINY)
        glColor3ub(40, 42, 52);
    else
        glColor3ub(242, 242, 242);
    circle(x - size * 0.8f, y, size * 0.7f);
    circle(x, y + 2, size);
    circle(x + size * 0.8f, y, size * 0.8f);
    rectangle(x - size * 0.8f, y - size * 0.5f, x + size * 0.8f, y + size * 0.35f);
}

// (Talha) all clouds
void drawClouds()
{
    drawCloud(cloud1X, 80, 7);
    drawCloud(cloud2X, 85, 8);
    drawCloud(cloud3X, 75, 6);
    drawCloud(cloud2X, 60, 9);
    drawCloud(cloud3X, 60, 5);
}

// (Talha) cloud drift
void updateClouds()
{
    if(currentSeason != RAINY)
    {
        cloud1X += 0.08f;
        cloud2X += 0.05f;
        cloud3X += 0.06f;
        if(cloud1X > 115) cloud1X = -115;
        if(cloud2X > 115) cloud2X = -115;
        if(cloud3X > 115) cloud3X = -115;
    }
}

// ======================================================
// RAIN, PUDDLES AND LIGHTNING  (rainy season)
// ======================================================

// (Rakib) seed the raindrops
void initRain()
{
    for(int i = 0; i < RAIN_COUNT; i++)
    {
        rainX[i] = -100 + (i * 37) % 200;
        rainY[i] = -60 + (i * 53) % 160;
        rainSpeed[i] = 0.8f + (i % 5) * 0.1f;
    }
}

// (Rakib) one slanted drop
void drawRainDrop(float x, float y)
{
    glColor3ub(180, 220, 255);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x - 1.2f, y - 5.0f);
    glEnd();
}

// (Rakib) rainfall
void drawRain()
{
    if(currentSeason != RAINY)
        return;

    // Drops below the road line are skipped, so the rain looks
    // like it lands rather than falling through the ground.
    for(int i = 0; i < RAIN_COUNT; i++)
        if(rainY[i] >= -40)
            drawRainDrop(rainX[i], rainY[i]);
}

// (Rakib) raindrop fall and reset
void updateRain()
{
    if(currentSeason != RAINY)
        return;

    for(int i = 0; i < RAIN_COUNT; i++)
    {
        rainY[i] -= rainSpeed[i];
        if(rainY[i] < -60)
        {
            rainY[i] = 100;
            rainX[i] = -100 + (i * 37) % 200;
        }
    }
}

// (Rakib) puddles on the ground
void drawPuddles()
{
    if(currentSeason != RAINY)
        return;

    for(int p = 0; p < PUDDLE_COUNT; p++)
    {
        float shiftedCenter = puddleCenterX[p] + worldMove;
        while(shiftedCenter > 110) shiftedCenter -= 220;
        while(shiftedCenter < -110) shiftedCenter += 220;
        float delta = shiftedCenter - puddleCenterX[p];

        glColor3ub(70, 100, 120);
        glBegin(GL_POLYGON);
        for(int v = 0; v < PUDDLE_VERTS; v++)
            glVertex2f(puddleVertX[p][v] + delta, puddleVertY[p][v]);
        glEnd();

        // A pale streak across the top of each puddle, so it
        // reads as a wet reflective surface.
        float hx = puddleVertX[p][1] + delta - 4.0f;
        float hy = puddleVertY[p][1] - 1.0f;
        glColor3ub(150, 180, 195);
        glBegin(GL_POLYGON);
        glVertex2f(hx - 5.0f, hy);
        glVertex2f(hx + 3.0f, hy);
        glVertex2f(hx + 1.0f, hy - 1.2f);
        glVertex2f(hx - 6.0f, hy - 1.0f);
        glEnd();
    }
}

// (Rakib) dark overcast sheet
void drawStormOverlay()
{
    if(currentSeason != RAINY)
        return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4ub(15, 18, 30, 110);
    rectangle(-100, -60, 100, 100);
    glDisable(GL_BLEND);
}

// (Rakib) zig-zag bolt
void drawLightningBolt(float x)
{
    if(lightningAlpha <= 40.0f)
        return;

    glColor3ub(150, 190, 255);
    glLineWidth(6.0f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, 95); glVertex2f(x - 4, 78);
    glVertex2f(x + 2, 78); glVertex2f(x - 6, 58);
    glVertex2f(x + 3, 58); glVertex2f(x - 3, 38);
    glVertex2f(x + 5, 38); glVertex2f(x, 18);
    glEnd();

    glColor3ub(235, 240, 255);
    glLineWidth(2.5f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, 95); glVertex2f(x - 4, 78);
    glVertex2f(x + 2, 78); glVertex2f(x - 6, 58);
    glVertex2f(x + 3, 58); glVertex2f(x - 3, 38);
    glVertex2f(x + 5, 38); glVertex2f(x, 18);
    glEnd();
}

// (Rakib) lightning flash
void drawLightning()
{
    if(currentSeason != RAINY || lightningAlpha <= 1.0f)
        return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4ub(210, 220, 255, (GLubyte)lightningAlpha);
    rectangle(-100, -60, 100, 100);
    drawLightningBolt(lightningBoltX);
    glDisable(GL_BLEND);
}

// (Rakib) flash fade-out
void updateLightning()
{
    if(currentSeason != RAINY)
    {
        lightningAlpha = 0.0f;
        return;
    }

    if(lightningAlpha > 1.0f)
        lightningAlpha *= 0.80f;
}

// (Rakib) fire a flash (key T)
void triggerThunder()
{
    if(currentSeason == RAINY)
    {
        lightningBoltX = -55.0f + (float)(rand() % 111);
        lightningAlpha = 255.0f;
    }
}

// ======================================================
// MOUNTAINS
// ======================================================

// (Susmita) winter mountain range
void drawWinterMountains()
{
    if(currentSeason != WINTER)
        return;
    glColor3ub(205, 220, 238);
    triangleShape(-100, -20, -58, -20, -80, 28);
    triangleShape(-70, -20, -22, -20, -46, 36);
    triangleShape(-35, -20, 15, -20, -10, 25);
    triangleShape(5, -20, 55, -20, 30, 34);
    triangleShape(45, -20, 100, -20, 74, 27);
    glColor3ub(242, 247, 252);
    triangleShape(-58, 5, -46, 36, -34, 5);
    triangleShape(-22, 8, -10, 25, 2, 8);
    triangleShape(18, 8, 30, 34, 42, 8);
    triangleShape(64, 4, 74, 27, 84, 4);
}
// ======================================================
// MOUNTAIN DATA(got some issues need to be fixed)
// ======================================================
const int MOUNTAIN_COUNT = 6;

float mountainX[MOUNTAIN_COUNT] = {
    -120, -80, -40, 10, 55, 110
};

float mountainWidth[MOUNTAIN_COUNT] = {
    50, 55, 45, 60, 50, 75
};

float mountainHeight[MOUNTAIN_COUNT] = {
    46, 42, 60, 45, 70, 40
};

// (Talha) 6 mountains with snow caps
void drawMountains()
{
    if(currentSeason == WINTER)
        return;

    for(int i = 0; i < MOUNTAIN_COUNT; i++)
    {
        // Mountains now use their own independent offset
        // (mountainMove) instead of piggy-backing on worldMove.
        float x = mountainX[i] + mountainMove;
        while(x > 150) x -= 300;
        while(x < -150) x += 300;
        float w = mountainWidth[i];
        float h = mountainHeight[i] * 1.35f;
        float baseY = -20.0f;

        if(currentSeason == SPRING)
            glColor3ub(140, 190, 160);
        else if(currentSeason == SUMMER)
            glColor3ub(160, 150, 110);
        else if(currentSeason == RAINY)
            glColor3ub(110, 120, 130);
        else if(currentSeason == AUTUMN)
            glColor3ub(170, 130, 90);
        else
            glColor3ub(120, 150, 170);

        triangleShape(x - w * 0.5f, baseY, x + w * 0.5f, baseY, x, baseY + h);

        glColor3ub(255, 255, 255);
        triangleShape(x - w * 0.15f, baseY + h * 0.75f, x + w * 0.15f, baseY + h * 0.75f, x, baseY + h);
    }
}

// ======================================================
// WINTER BACKGROUND
// ======================================================

// (Susmita) winter background trees
void drawWinterBackground()
{
    if(currentSeason != WINTER)
        return;
    for(int i = -90; i <= 95; i += 18)
    {
        // Independent winter-background offset.
        float x = i + winterBgMove;
        while(x > 105) x -= 300;
        while(x < -105) x += 300;
    }
}

// ======================================================
// TREE / LEAF / BRANCH / FOREST
// ======================================================

// (Susmita) bare winter tree
void drawWinterBareTree(float x, float y, float scale, int type)
{
    glColor3ub(78, 49, 30);
    float h = (type == 1 ? 46.0f : 52.0f) * scale;
    float trunkW = (type == 1 ? 2.0f : 2.8f) * scale;
    rectangle(x - trunkW, y, x + trunkW, y + h);
    glLineWidth(3.5f * scale + 0.8f);
    glBegin(GL_LINES);
    glVertex2f(x, y + h*0.28f);       glVertex2f(x - 18*scale, y + h*0.52f);
    glVertex2f(x, y + h*0.34f);       glVertex2f(x + 20*scale, y + h*0.60f);
    glVertex2f(x, y + h*0.50f);       glVertex2f(x - 15*scale, y + h*0.73f);
    glVertex2f(x, y + h*0.60f);       glVertex2f(x + 14*scale, y + h*0.82f);
    glVertex2f(x, y + h*0.74f);       glVertex2f(x - 10*scale, y + h*0.94f);
    glVertex2f(x, y + h*0.84f);       glVertex2f(x + 9*scale, y + h*1.02f);
    glVertex2f(x - 18*scale, y + h*0.52f); glVertex2f(x - 25*scale, y + h*0.63f);
    glVertex2f(x - 18*scale, y + h*0.52f); glVertex2f(x - 22*scale, y + h*0.45f);
    glVertex2f(x + 20*scale, y + h*0.60f); glVertex2f(x + 28*scale, y + h*0.72f);
    glVertex2f(x + 20*scale, y + h*0.60f); glVertex2f(x + 25*scale, y + h*0.52f);
    glVertex2f(x - 15*scale, y + h*0.73f); glVertex2f(x - 21*scale, y + h*0.84f);
    glVertex2f(x + 14*scale, y + h*0.82f); glVertex2f(x + 19*scale, y + h*0.92f);
    glVertex2f(x - 10*scale, y + h*0.94f); glVertex2f(x - 15*scale, y + h*1.03f);
    glVertex2f(x + 9*scale, y + h*1.02f);  glVertex2f(x + 13*scale, y + h*1.10f);
    glEnd();
    glColor3ub(248, 250, 253);
    glLineWidth(2.2f * scale + 0.6f);
    glBegin(GL_LINES);
    glVertex2f(x - 17*scale, y + h*0.55f); glVertex2f(x - 3*scale, y + h*0.38f);
    glVertex2f(x + 2*scale, y + h*0.40f);  glVertex2f(x + 19*scale, y + h*0.62f);
    glVertex2f(x - 14*scale, y + h*0.76f); glVertex2f(x - 2*scale, y + h*0.54f);
    glVertex2f(x + 2*scale, y + h*0.65f);  glVertex2f(x + 13*scale, y + h*0.84f);
    glEnd();
}

// (Talha) round tree trunk
void drawRoundTreeTrunk(float x, float y, float scale)
{
    glColor3ub(89, 46, 18);
    glBegin(GL_QUADS);
    glVertex2f(x - 3 * scale, y);
    glVertex2f(x + 3 * scale, y);
    glVertex2f(x + 2 * scale, y + 50 * scale);
    glVertex2f(x - 2 * scale, y + 50 * scale);
    glEnd();
}

// (Talha) pine trunk
void drawPineTreeTrunk(float x, float y, float scale)
{
    glColor3ub(89, 46, 18);
    glBegin(GL_QUADS);
    glVertex2f(x - 5 * scale, y);
    glVertex2f(x + 5 * scale, y);
    glVertex2f(x + 3.5f * scale, y + 62 * scale);
    glVertex2f(x - 3.5f * scale, y + 62 * scale);
    glEnd();
}

// (Talha) bushy tree trunk
void drawBushyTreeTrunk(float x, float y, float scale)
{
    glColor3ub(89, 46, 18);
    glBegin(GL_QUADS);
    glVertex2f(x - 2 * scale, y);
    glVertex2f(x + 2 * scale, y);
    glVertex2f(x + 1.5f * scale, y + 50 * scale);
    glVertex2f(x - 1.5f * scale, y + 50 * scale);
    glEnd();
}

// (Talha) branches
void drawBranches(float x, float y, float scale)
{
    glColor3ub(77, 38, 13);
    glLineWidth(5.0f);
    glBegin(GL_LINES);
    glVertex2f(x, y + 15 * scale);        glVertex2f(x - 20 * scale, y + 40 * scale);
    glVertex2f(x, y + 18 * scale);        glVertex2f(x + 20 * scale, y + 42 * scale);
    glVertex2f(x, y + 20 * scale);        glVertex2f(x, y + 50 * scale);
    glVertex2f(x - 7 * scale, y + 27 * scale);  glVertex2f(x - 25 * scale, y + 35 * scale);
    glVertex2f(x + 7 * scale, y + 30 * scale);  glVertex2f(x + 25 * scale, y + 38 * scale);
    glEnd();
}

// (Talha) round tree foliage
void drawTreeLeaves(float x, float y, float scale)
{
    int r, g, b;

    if(currentSeason == SPRING)
    {
        r = 72; g = 160; b = 77;
    }
    else if(currentSeason == SUMMER)
    {
        r = 8; g = 105; b = 15;
    }
    else if(currentSeason == AUTUMN)
    {
        // Warm orange autumn foliage
        r = 224; g = 122; b = 25;
    }
    else
    {
        r = 10; g = 122; b = 18;
    }
    shadedColor(r, g, b, 0.88f);
    circle(x - 15 * scale, y + 48 * scale, 14 * scale);
    shadedColor(r, g, b, 1.12f);
    circle(x, y + 60 * scale, 17 * scale);
    shadedColor(r, g, b, 1.05f);
    circle(x + 15 * scale, y + 48 * scale, 14 * scale);
    shadedColor(r, g, b, 0.92f);
    circle(x - 7 * scale, y + 40 * scale, 12 * scale);
    shadedColor(r, g, b, 1.00f);
    circle(x + 8 * scale, y + 40 * scale, 12 * scale);
}

// (Talha) round tree
void drawTreeRound(float x, float y, float scale)
{
    drawRoundTreeTrunk(x, y, scale);
    drawBranches(x, y, scale);
    drawTreeLeaves(x, y, scale);
}

// (Talha) pine foliage
void drawPineLeaves(float x, float y, float scale)
{
    if(currentSeason == SPRING)
        glColor3ub(55, 130, 45);
    else if(currentSeason == SUMMER)
        glColor3ub(10, 60, 24);
    else if(currentSeason == AUTUMN)
        // Pines stay evergreen but dull/darken a little in autumn
        glColor3ub(55, 78, 30);
    else
        glColor3ub(12, 70, 28);
    triangleShape(x - 16 * scale, y + 28 * scale, x + 16 * scale, y + 28 * scale, x, y + 48 * scale);
    triangleShape(x - 13 * scale, y + 41 * scale, x + 13 * scale, y + 41 * scale, x, y + 60 * scale);
    triangleShape(x - 9 * scale, y + 54 * scale, x + 9 * scale, y + 54 * scale, x, y + 72 * scale);
}

// (Talha) pine tree
void drawTreePine(float x, float y, float scale)
{
    drawPineTreeTrunk(x, y, scale);
    drawPineLeaves(x, y, scale);
}

// (Talha) one triangular leaf
void leafTriangle(float cx, float cy, float size)
{
    triangleShape(cx - size, cy - size * 0.6f, cx + size, cy - size * 0.6f, cx, cy + size * 0.9f);
}

// (Talha) bushy foliage
void drawBushyTriangleLeaves(float x, float y, float scale)
{
    int r, g, b;
    if(currentSeason == SPRING)
    {
        r = 173; g = 209; b = 60;
    }
    else if(currentSeason == SUMMER)
    {
        r = 95; g = 125; b = 30;
    }
    else if(currentSeason == AUTUMN)
    {
        // Deep red autumn foliage
        r = 178; g = 48; b = 32;
    }
    else
    {
        r = 107; g = 142; b = 35;
    }
    shadedColor(r, g, b, 0.90f);
    leafTriangle(x - 9 * scale, y + 42 * scale, 15 * scale);
    shadedColor(r, g, b, 1.05f);
    leafTriangle(x + 9 * scale, y + 42 * scale, 15 * scale);
    shadedColor(r, g, b, 1.00f);
    leafTriangle(x, y + 48 * scale, 16 * scale);
    shadedColor(r, g, b, 0.85f);
    leafTriangle(x - 15 * scale, y + 50 * scale, 15 * scale);
    shadedColor(r, g, b, 1.02f);
    leafTriangle(x + 15 * scale, y + 50 * scale, 15 * scale);
    shadedColor(r, g, b, 1.12f);
    leafTriangle(x, y + 62 * scale, 18 * scale);
}

// (Talha) bushy tree
void drawTreeBushy(float x, float y, float scale)
{
    drawBushyTreeTrunk(x, y, scale);
    drawBranches(x, y, scale);
    drawBushyTriangleLeaves(x, y, scale);
}

// (Talha) pick tree by type
void drawTree(float x, float y, float scale, int type)
{
    if(currentSeason == WINTER)
    {
        drawWinterBareTree(x, y, scale, type);
        return;
    }
    if(type == 0)
        drawTreeRound(x, y, scale);
    else if(type == 1)
        drawTreePine(x, y, scale);
    else
        drawTreeBushy(x, y, scale);
}

// (Talha) the tree row
void drawForest()
{
    for(int i = 0; i < TREE_COUNT; i++)
    {
        float x = treeX[i] + forestMove;
        while(x > 150) x -= 300;
        while(x < -150) x += 300;
        // Only draw when inside the extended area.
        if(x >= -155 && x <= 155)
            drawTree(x, treeY[i], treeScale[i], treeType[i]);
    }
}

// ======================================================
// FOREST BACKGROUND
// ======================================================

// (Sanzida) one background tuft
void drawForestBgGrass(float x, float y, float scale)
{
    if(currentSeason == WINTER)
        glColor3ub(125, 140, 155);
    else if(currentSeason == SUMMER)
        glColor3ub(120, 125, 35);
    else if(currentSeason == AUTUMN)
        glColor3ub(150, 90, 40);
    else
        glColor3ub(34, 120, 34);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 6 * scale, y); glVertex2f(x - 2 * scale, y); glVertex2f(x - 5 * scale, y + 10 * scale);
    glVertex2f(x - 3 * scale, y); glVertex2f(x + 1 * scale, y); glVertex2f(x - 1 * scale, y + 14 * scale);
    glVertex2f(x, y);             glVertex2f(x + 4 * scale, y); glVertex2f(x + 2 * scale, y + 13 * scale);
    glVertex2f(x + 3 * scale, y); glVertex2f(x + 7 * scale, y); glVertex2f(x + 6 * scale, y + 9 * scale);
    glEnd();
}

// (Sanzida) background grass strip
void drawForestBg()
{
    for(int i = 0; i < FOREST_BG_COUNT; i++)
    {
        float x = forestBgX[i] + bgGrassMove;
        while(x > 150) x -= 300;
        while(x < -150) x += 300;
        drawForestBgGrass(x, -20, forestBgScale[i]);
    }
}

// ======================================================
// GRASS
// ======================================================

// (Sanzida) one grass tuft
void drawGrass(float x, float y, float scale)
{
    if(currentSeason == WINTER)
        return;
    if(currentSeason == SUMMER)
        glColor3ub(170, 150, 45);
    else if(currentSeason == AUTUMN)
        // Golden-brown grass in autumn
        glColor3ub(196, 140, 55);
    else
        glColor3ub(40, 158, 45);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 6 * scale, y); glVertex2f(x - 2 * scale, y); glVertex2f(x - 5 * scale, y + 10 * scale);
    glVertex2f(x - 3 * scale, y); glVertex2f(x + 1 * scale, y); glVertex2f(x - 1 * scale, y + 14 * scale);
    glVertex2f(x, y);             glVertex2f(x + 4 * scale, y); glVertex2f(x + 2 * scale, y + 13 * scale);
    glVertex2f(x + 3 * scale, y); glVertex2f(x + 7 * scale, y); glVertex2f(x + 6 * scale, y + 9 * scale);
    glEnd();
}

// (Sanzida) grass field
void drawGrassField()
{
    if(currentSeason == WINTER)
        return;
    for(int i = 0; i < GRASS_COUNT; i++)
    {
        float x = grassX[i] + grassMove;
        while(x > 150) x -= 300;
        while(x < -150) x += 300;
        if(x >= -155 && x <= 155)
            drawGrass(x, grassY[i], grassScale[i]);
    }
}

// (Sanzida) grass at the road edge
void drawRoadGrass()
{
    if(currentSeason == WINTER)
        return;
    for(int i = 0; i < ROAD_GRASS_COUNT; i++)
    {
        float x = roadGrassX[i] + roadGrassMove;
        while(x > 150) x -= 300;
        while(x < -150) x += 300;
        drawGrass(x, roadGrassY[i], roadGrassScale[i]);
    }
}

// ======================================================
// FLOWERS
// ======================================================

// (Sanzida) one flower
void drawFlower(float x, float y, float scale, int colorType)
{
    glColor3ub(46, 125, 50);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x, y + 8 * scale);
    glEnd();
    switch(colorType)
    {
        case 0: glColor3ub(255, 128, 171); break;
        case 1: glColor3ub(186, 104, 200); break;
        case 2: glColor3ub(255, 255, 255); break;
        default: glColor3ub(255, 111, 74); break;
    }
    float petalDist = 2.6f * scale;
    float petalRadius = 1.5f * scale;
    float centerY = y + 8 * scale;
    for(int p = 0; p < 5; p++)
    {
        float angle = 2.0f * PI * p / 5.0f;
        float px = x + petalDist * cos(angle);
        float py = centerY + petalDist * sin(angle);
        circle(px, py, petalRadius);
    }
    glColor3ub(255, 235, 59);
    circle(x, centerY, 1.3f * scale);
}

// (Sanzida) all spring flowers
void drawFlowers()
{
    if(currentSeason != SPRING)
        return;
    for(int i = 0; i < FLOWER_COUNT; i++)
    {
        float x = flowerX[i] + flowerMove;
        while(x > 150) x -= 300;
        while(x < -150) x += 300;
        drawFlower(x, flowerY[i], flowerScale[i], flowerColor[i]);
    }
}

// ======================================================
// BUTTERFLIES
// ======================================================

// (Sanzida) one butterfly
void drawButterfly(float x, float y, float scale, float flap)
{
    float wingSpread = (0.5f + 0.5f * flap) * scale;
    glColor3ub(255, 179, 71);
    circle(x - 2.0f * scale, y + 0.5f * scale, wingSpread);
    glColor3ub(255, 202, 58);
    circle(x + 2.0f * scale, y + 0.5f * scale, wingSpread);
    glColor3ub(66, 40, 14);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(x, y - 1.5f * scale);
    glVertex2f(x, y + 2.5f * scale);
    glEnd();
}

// (Sanzida) all butterflies
void drawButterflies()
{
    if(currentSeason != SPRING)
        return;
    for(int i = 0; i < BUTTERFLY_COUNT; i++)
    {
        float baseX = butterflyBaseX[i] + butterflyMove;
        while(baseX > 150) baseX -= 300;
        while(baseX < -150) baseX += 300;
        float x = baseX + 14.0f * sin(butterflyTime * 0.6f + butterflyPhase[i]);
        float y = butterflyBaseY[i] + 5.0f * sin(butterflyTime * 1.3f + butterflyPhase[i]);
        float flap = 0.5f + 0.5f * fabs((float)sin(butterflyTime * 6.0f + butterflyPhase[i]));
        drawButterfly(x, y, butterflyScale[i], flap);
    }
}

// ======================================================
// SPRING ENVIRONMENT
// ======================================================

// (Sanzida) bee hive and bees
void drawBeeHive()
{
    if(currentSeason != SPRING)
        return;

    float x = treeX[BEE_HIVE_TREE_INDEX] + forestMove;
    while(x > 150) x -= 300;
    while(x < -150) x += 300;

    // If the host tree itself isn't in the drawn range, skip the
    // hive too, so it never appears floating with no tree under it.
    if(x < -155 || x > 155)
        return;

    float scale = treeScale[BEE_HIVE_TREE_INDEX];
    float y = treeY[BEE_HIVE_TREE_INDEX];

    // Tucked into the branch fork, off to one side of the trunk.
    float hx = x + 14.0f * scale;
    float hy = y + 32.0f * scale;

    // Hive body: three stacked, narrowing ovals - a simple
    // paper-lantern silhouette.
    glColor3ub(214, 165, 92);
    circle(hx, hy, 4.6f * scale);
    glColor3ub(224, 178, 108);
    circle(hx, hy + 3.4f * scale, 3.6f * scale);
    glColor3ub(232, 190, 120);
    circle(hx, hy + 6.0f * scale, 2.3f * scale);

    // Horizontal banding, like a real paper hive.
    glColor3ub(150, 108, 55);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(hx - 4.2f * scale, hy - 1.0f * scale);
    glVertex2f(hx + 4.2f * scale, hy - 1.0f * scale);
    glVertex2f(hx - 3.5f * scale, hy + 2.0f * scale);
    glVertex2f(hx + 3.5f * scale, hy + 2.0f * scale);
    glVertex2f(hx - 2.5f * scale, hy + 4.6f * scale);
    glVertex2f(hx + 2.5f * scale, hy + 4.6f * scale);
    glEnd();

    // Entrance hole near the bottom.
    glColor3ub(60, 40, 20);
    circle(hx, hy - 2.8f * scale, 0.85f * scale);

    // A few bees looping around the entrance.
    for(int i = 0; i < BEE_COUNT; i++)
    {
        float angle = beeOrbitTime * 2.4f + beePhase[i];
        float radiusX = 3.4f * scale;
        float radiusY = 1.8f * scale;

        float bx = hx + radiusX * cos(angle);
        float by = (hy - 2.8f * scale) + radiusY * sin(angle);

        glColor3ub(35, 28, 12);
        circle(bx, by, 0.32f * scale);

        glColor3ub(238, 202, 40);
        rectangle(
            bx - 0.32f * scale, by - 0.10f * scale,
            bx + 0.32f * scale, by + 0.10f * scale
        );

        // Tiny flickering wings.
        glColor3ub(255, 255, 255);
        circle(bx, by + 0.30f * scale, 0.18f * scale);
    }
}

// (Sanzida) flowers + butterflies + hive
void drawSpringEnvironment()
{
    drawFlowers();
    drawButterflies();
    drawBeeHive();
}

// ======================================================
// EXTRA EFFECTS  (J duck / K birds)
// ======================================================

// (Sanzida) spawn the duck family and stop the car
void startDuckCrossing()
{
    if(duckActive)
        return;

    duckActive = true;
    duckTimer = 0.0f;
    duckY = -37.0f;
    duckTrailHead = 0;
    duckTrailCount = 0;

    duckX = carX + 24.0f;

    // Seeing them, the driver hits the brakes: the car stops on
    // its own and won't move again until S is pressed.
    duckStopActive = true;
    duckWaitingResume = false;
}

// (Sanzida) ducks cross, then release the car
void updateDuckCrossing()
{
    if(!duckActive)
        return;

    duckTimer += 0.02f;
    duckY -= 0.22f;

    float wobble = 0.9f * (float)sin(duckTimer * 9.0f);
    duckTrailX[duckTrailHead] = duckX + wobble;
    duckTrailY[duckTrailHead] = duckY;
    duckTrailHead = (duckTrailHead + 1) % DUCK_TRAIL_LEN;
    if(duckTrailCount < DUCK_TRAIL_LEN)
        duckTrailCount++;

    if(duckY < -60.0f)
    {
        duckActive = false;

        // They're off the road now, so the car is free to go -
        // but only once the player actually presses S.
        if(duckStopActive)
            duckWaitingResume = true;
    }
}

// (Sanzida) one duck (parent or duckling)
void drawDuck(float x, float y, float scale, bool isParent, float legPhase)
{
    float legKick = 0.5f * (float)sin(legPhase);

    // --- feet, drawn first so the body overlaps them ---
    glColor3ub(235, 140, 30);
    triangleShape(x - 0.9f * scale, y - 1.5f * scale - legKick * scale,
                  x - 0.2f * scale, y - 1.5f * scale - legKick * scale,
                  x - 0.55f * scale, y - 2.3f * scale - legKick * scale);
    triangleShape(x + 0.2f * scale, y - 1.5f * scale + legKick * scale,
                  x + 0.9f * scale, y - 1.5f * scale + legKick * scale,
                  x + 0.55f * scale, y - 2.3f * scale + legKick * scale);

    if(isParent)
    {
        // tail curl
        glColor3ub(35, 30, 25);
        triangleShape(x - 2.0f * scale, y + 0.4f * scale,
                      x - 2.9f * scale, y + 1.1f * scale,
                      x - 2.2f * scale, y - 0.3f * scale);

        // body
        glColor3ub(150, 128, 96);
        ellipse(x, y, 2.2f * scale, 1.35f * scale);

        // folded wing, with a hint of the blue speculum feathers
        glColor3ub(120, 100, 74);
        ellipse(x - 0.2f * scale, y + 0.1f * scale, 1.3f * scale, 0.75f * scale);
        glColor3ub(70, 110, 165);
        ellipse(x - 0.3f * scale, y + 0.05f * scale, 0.55f * scale, 0.28f * scale);

        // white neck ring
        glColor3ub(250, 250, 245);
        ellipse(x + 1.55f * scale, y + 0.75f * scale, 0.5f * scale, 0.65f * scale);

        // head - glossy mallard green
        glColor3ub(30, 90, 55);
        circle(x + 1.85f * scale, y + 1.35f * scale, 0.95f * scale);
        glColor3ub(45, 120, 72);
        circle(x + 1.65f * scale, y + 1.55f * scale, 0.45f * scale);

        // bill
        glColor3ub(230, 170, 30);
        triangleShape(x + 2.55f * scale, y + 1.30f * scale,
                      x + 3.35f * scale, y + 1.18f * scale,
                      x + 2.55f * scale, y + 0.95f * scale);

        // eye
        glColor3ub(15, 15, 15);
        circle(x + 2.0f * scale, y + 1.5f * scale, 0.12f * scale);
    }
    else
    {
        // duckling - round, fluffy, big-headed
        glColor3ub(255, 221, 90);
        ellipse(x, y, 1.55f * scale, 1.15f * scale);

        // soft brownish cap/back marking, typical of real ducklings
        glColor3ub(214, 178, 60);
        ellipse(x - 0.2f * scale, y + 0.35f * scale, 0.85f * scale, 0.4f * scale);

        // tiny wing stub
        glColor3ub(230, 195, 70);
        ellipse(x - 0.3f * scale, y - 0.1f * scale, 0.55f * scale, 0.32f * scale);

        // head - proportionally larger than the parent's, baby-like
        glColor3ub(255, 230, 110);
        circle(x + 1.25f * scale, y + 0.85f * scale, 0.85f * scale);
        glColor3ub(224, 190, 66);
        circle(x + 1.05f * scale, y + 1.15f * scale, 0.35f * scale);

        // bill
        glColor3ub(235, 150, 40);
        triangleShape(x + 1.85f * scale, y + 0.82f * scale,
                      x + 2.45f * scale, y + 0.74f * scale,
                      x + 1.85f * scale, y + 0.58f * scale);

        // eye
        glColor3ub(20, 15, 10);
        circle(x + 1.35f * scale, y + 0.95f * scale, 0.11f * scale);
    }
}

// (Sanzida) parent + 3 ducklings
void drawDuckFamily()
{
    if(!duckActive)
        return;

    // Gentle waddle side to side as the parent crosses.
    float wobble = 0.9f * (float)sin(duckTimer * 9.0f);
    float legPhaseParent = duckTimer * 14.0f;

    drawDuck(duckX + wobble, duckY, 1.0f, true, legPhaseParent);

    for(int i = 0; i < DUCKLING_COUNT; i++)
    {
        int delay = ducklingDelayFrames[i];
        float px, py;

        if(delay < duckTrailCount)
        {
            int idx = duckTrailHead - 1 - delay;
            while(idx < 0) idx += DUCK_TRAIL_LEN;
            px = duckTrailX[idx];
            py = duckTrailY[idx];
        }
        else
        {
            // Not enough history yet right at the very start -
            // sit just behind the parent instead of popping in.
            px = duckX + wobble;
            py = duckY + 1.5f * (i + 1);
        }

        float legPhase = duckTimer * 14.0f + i * 1.3f;
        drawDuck(px + ducklingLateral[i], py, 0.55f, false, legPhase);
    }
}

// (Sanzida) launch the flock
void startBirdFlock()
{
    if(birdActive)
        return;

    birdActive = true;
    birdTimer = 0.0f;
    flockX = -145.0f;
    flockY = 32.0f + (float)(rand() % 25);
}

// (Sanzida) fly the flock across
void updateBirdFlock()
{
    if(!birdActive)
        return;

    birdTimer += 0.02f;
    flockX += 1.15f;

    if(flockX > 145.0f)
        birdActive = false;
}

// (Sanzida) one bird
void drawBird(float x, float y, float scale, int colorIdx, float flap)
{
    int r, g, b;

    switch(colorIdx % 4)
    {
        case 0: r = 70;  g = 130; b = 225; break; // blue
        case 1: r = 240; g = 200; b = 45;  break; // yellow
        case 2: r = 215; g = 60;  b = 55;  break; // red
        default: r = 248; g = 248; b = 248; break; // white
    }

    float wingLift = (1.3f + flap) * scale;

    shadedColor(r, g, b, 1.00f);
    triangleShape(
        x - 0.6f * scale, y + 0.25f * scale,
        x - 1.6f * scale, y - 0.55f * scale,
        x - 4.4f * scale, y + wingLift
    );
    shadedColor(r, g, b, 0.92f);
    triangleShape(
        x + 0.6f * scale, y + 0.25f * scale,
        x + 1.6f * scale, y - 0.55f * scale,
        x + 4.4f * scale, y + wingLift
    );

    // Small tail fan at the back.
    shadedColor(r, g, b, 0.80f);
    triangleShape(
        x - 0.8f * scale, y + 0.05f * scale,
        x - 2.1f * scale, y + 0.55f * scale,
        x - 2.1f * scale, y - 0.45f * scale
    );

    // Rounded body, a touch darker than the wings so it reads as
    // a separate, shaded form rather than a flat cutout.
    shadedColor(r, g, b, 0.88f);
    circle(x, y, 0.95f * scale);

    // Head, angled slightly up and forward.
    shadedColor(r, g, b, 0.98f);
    circle(x + 0.95f * scale, y + 0.55f * scale, 0.55f * scale);

    // Tiny beak.
    glColor3ub(235, 150, 40);
    triangleShape(
        x + 1.35f * scale, y + 0.62f * scale,
        x + 2.05f * scale, y + 0.52f * scale,
        x + 1.35f * scale, y + 0.38f * scale
    );

    // A single dark eye dot, so the head doesn't read as a blank
    // circle at this small scale.
    glColor3ub(25, 25, 25);
    circle(x + 1.05f * scale, y + 0.62f * scale, 0.11f * scale);
}

// (Sanzida) the V formation
void drawBirdFlock()
{
    if(!birdActive)
        return;

    for(int i = 0; i < BIRD_COUNT; i++)
    {
        float bx = flockX + birdOffsetX[i];
        float by = flockY + birdOffsetY[i];
        float flap = (float)sin(birdTimer * 10.0f + birdFlapPhase[i]);

        drawBird(bx, by, 1.0f, i, flap);
    }
}

// ======================================================
// ROAD
// ======================================================

// (Talha) road strip and lane stripes
void drawRoad()
{
    glColor3ub(55, 55, 55);
    rectangle(-100, -58, 100, -40);
    glColor3ub(255, 255, 255);
    for(float x = -150; x < 150; x += 30)
    {
        float lineX = x + worldMove;
        while(lineX > 150) lineX -= 300;
        while(lineX < -150) lineX += 300;
        if(lineX >= -110 && lineX <= 110)
            rectangle(lineX, -50, lineX + 10, -48);
    }
}

// ======================================================
// THE ENDING  (bench on key 9, last words on key 8)
// ======================================================

bool endingStarted = false;   // 9 has been pressed
bool benchVisible = false;
bool showEndMessage = false;  // 8 has been pressed

float benchX = 120.0f;        // comes in from the right
float benchY = -40.0f;        // stands on the grass edge of the road

// (Talha) the bench
void drawBench()
{
    if(!benchVisible)
        return;

    float x = benchX;
    float y = benchY;

    // legs
    glColor3ub(110, 70, 38);
    rectangle(x - 8.5f, y, x - 6.5f, y + 6);
    rectangle(x + 6.5f, y, x + 8.5f, y + 6);

    // seat
    glColor3ub(140, 92, 50);
    rectangle(x - 11, y + 6, x + 11, y + 8);

    // back posts
    glColor3ub(110, 70, 38);
    rectangle(x - 8.5f, y + 8, x - 6.8f, y + 16);
    rectangle(x + 6.8f, y + 8, x + 8.5f, y + 16);

    // back planks
    glColor3ub(140, 92, 50);
    rectangle(x - 11, y + 11, x + 11, y + 12.6f);
    rectangle(x - 11, y + 14, x + 11, y + 15.6f);
}

// (Talha) final message screen
void drawEndMessage()
{
    if(!showEndMessage)
        return;

    glColor3ub(0, 0, 0);
    rectangle(-100, -60, 100, 100);

    glColor3ub(255, 255, 255);
    drawText(-21, 12, "LIFE IS LIKE THE SEASON,");
    drawText(-25, 0, "YOU WONT NOTICE WHEN ITS GONE");
}

// ======================================================
// THE MAN  -  ONE LIFE, SIX AGES
// ======================================================

#define MAN_ON_GRASS      0   // sitting on the grass, waiting
#define MAN_WALK_TO_CAR   1   // walking over to the car (key F)
#define MAN_IN_CAR        2   // driving through the seasons
#define MAN_WALK_TO_BENCH 3   // walking to the bench (key 9)
#define MAN_ON_BENCH      4   // sitting on the bench - the end

int manState = MAN_ON_GRASS;

float manX = -58.0f;          // feet position while he is outside
float manY = -30.0f;          // he starts on the grass
float manWalkSpeed = 0.42f;

// (Talha) age stage = season number
int manAge()
{
    // sedlife = 0 ... WINTER = 5
    return currentSeason;
}

// (Talha) skin colour by age
void manSkinColor(int stage)
{
    if(stage <= 1)      glColor3ub(245, 205, 170);
    else if(stage <= 3) glColor3ub(238, 196, 160);
    else                glColor3ub(228, 195, 172);
}

// (Talha) hair colour by age
void manHairColor(int stage)
{
    if(stage <= 2)      glColor3ub(35, 25, 20);
    else if(stage == 3) glColor3ub(65, 52, 45);
    else if(stage == 4) glColor3ub(150, 150, 150);
    else                glColor3ub(242, 242, 242);
}

// (Talha) shirt colour by age
void manShirtColor(int stage)
{
    if(stage == 0)      glColor3ub(70, 150, 220);
    else if(stage == 1) glColor3ub(60, 130, 190);
    else if(stage == 2) glColor3ub(50, 105, 160);
    else if(stage == 3) glColor3ub(70, 90, 110);
    else if(stage == 4) glColor3ub(85, 85, 95);
    else                glColor3ub(110, 110, 120);
}

// (Talha) trouser colour by age
void manTrouserColor(int stage)
{
    if(stage <= 2)      glColor3ub(45, 55, 85);
    else                glColor3ub(60, 60, 70);
}

// (Talha) hair behind the face
void drawManHairBack(int stage, float hx, float hy, float hr)
{
    manHairColor(stage);

    if(stage <= 2)
    {
        // Full head of hair.
        circle(hx, hy + 0.28f * hr, hr * 1.16f);
    }
    else if(stage == 3)
    {
        // Pushed up and back, so the forehead starts to show.
        circle(hx - 0.32f * hr, hy + 0.32f * hr, hr * 1.06f);
    }
    else if(stage == 4)
    {
        // Balding: only two side tufts are left.
        circle(hx - 0.85f * hr, hy + 0.10f * hr, 0.42f * hr);
        circle(hx + 0.80f * hr, hy + 0.05f * hr, 0.34f * hr);
    }
    else
    {
        // Bald, with one thin white tuft at the back.
        circle(hx - 0.85f * hr, hy + 0.05f * hr, 0.36f * hr);
    }
}

// (Talha) forehead fringe
void drawManFringe(int stage, float hx, float hy, float hr)
{
    if(stage > 2)
        return;
    manHairColor(stage);
    rectangle(hx + 0.35f * hr, hy + 0.40f * hr, hx + 0.95f * hr, hy + 0.88f * hr);
}

// (Talha) beard by age
void drawManBeard(int stage, float hx, float hy, float hr)
{
    if(stage == 0)
        return;

    manHairColor(stage);

    if(stage == 1)
    {
        // Light stubble: three small dots along the jaw.
        circle(hx - 0.55f * hr, hy - 0.62f * hr, 0.30f * hr);
        circle(hx + 0.02f * hr, hy - 0.76f * hr, 0.32f * hr);
        circle(hx + 0.55f * hr, hy - 0.60f * hr, 0.30f * hr);
        return;
    }

    // The beard circles get bigger with age.
    float b = 0.42f;
    if(stage == 3) b = 0.48f;
    if(stage >= 4) b = 0.52f;

    circle(hx - 0.85f * hr, hy - 0.32f * hr, b * hr);
    circle(hx - 0.45f * hr, hy - 0.70f * hr, b * hr);
    circle(hx,              hy - 0.85f * hr, b * hr);
    circle(hx + 0.45f * hr, hy - 0.70f * hr, b * hr);
    circle(hx + 0.80f * hr, hy - 0.30f * hr, b * hr);

    // The last two beards hang below the chin.
    if(stage == 4)
    {
        rectangle(hx - 0.45f * hr, hy - 1.35f * hr, hx + 0.45f * hr, hy - 0.60f * hr);
        circle(hx, hy - 1.35f * hr, 0.45f * hr);
    }
    else if(stage == 5)
    {
        rectangle(hx - 0.50f * hr, hy - 1.85f * hr, hx + 0.50f * hr, hy - 0.60f * hr);
        circle(hx, hy - 1.85f * hr, 0.50f * hr);
    }

    // Moustache.
    rectangle(hx + 0.05f * hr, hy - 0.52f * hr, hx + 0.85f * hr, hy - 0.18f * hr);
}

// (Talha) face and head
void drawManHead(int stage, float hx, float hy, float hr)
{
    // Hair first, so the face covers all but its rim.
    drawManHairBack(stage, hx, hy, hr);

    // Face.
    manSkinColor(stage);
    circle(hx, hy, hr);

    drawManBeard(stage, hx, hy, hr);
    drawManFringe(stage, hx, hy, hr);

    // Ear (the man faces right, so the ear is on the left side).
    manSkinColor(stage);
    circle(hx - hr * 0.92f, hy - hr * 0.10f, hr * 0.26f);

    // Eye.
    glColor3ub(30, 30, 30);
    circle(hx + hr * 0.42f, hy + hr * 0.12f, hr * 0.15f);

    // Eyebrow - greys along with the hair.
    manHairColor(stage);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(hx + hr * 0.16f, hy + hr * 0.50f);
    glVertex2f(hx + hr * 0.74f, hy + hr * 0.44f);
    glEnd();

    // Age lines on the last two stages.
    if(stage >= 4)
    {
        glColor3ub(196, 165, 145);
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glVertex2f(hx + hr * 0.18f, hy - hr * 0.12f);
        glVertex2f(hx + hr * 0.70f, hy - hr * 0.12f);
        glVertex2f(hx + hr * 0.14f, hy + hr * 0.74f);
        glVertex2f(hx + hr * 0.76f, hy + hr * 0.72f);
        glEnd();
    }
}

// (Talha) man behind the windscreen
void drawManInCar(float x, float y)
{
    if(!manInCar)
        return;

    int stage = manAge();

    float hx = x - 0.8f;      // head centre, inside the window
    float hy = y + 9.4f;
    float hr = 2.0f;

    // Shoulders behind the glass.
    manShirtColor(stage);
    rectangle(hx - 2.6f, y + 7.2f, hx + 2.6f, hy - hr * 0.55f);

    // Neck.
    manSkinColor(stage);
    rectangle(hx - 0.7f, hy - hr * 1.25f, hx + 0.7f, hy - hr * 0.40f);

    drawManHead(stage, hx, hy, hr);
}

// (Talha) standing pose
void drawManStanding(float fx, float fy)
{
    int stage = manAge();

    float hipY  = fy + 6.5f;
    float shY   = fy + 12.0f;      // shoulder height
    float headY = fy + 15.0f;
    float hr    = 2.6f;

    // Legs, a little apart.
    manTrouserColor(stage);
    glLineWidth(5.0f);
    glBegin(GL_LINES);
    glVertex2f(fx, hipY); glVertex2f(fx + 1.4f, fy);
    glVertex2f(fx, hipY); glVertex2f(fx - 1.4f, fy);
    glEnd();

    // Shoes.
    glColor3ub(40, 40, 45);
    circle(fx + 1.4f, fy, 0.9f);
    circle(fx - 1.4f, fy, 0.9f);

    // Body.
    manShirtColor(stage);
    rectangle(fx - 2.2f, hipY, fx + 2.2f, shY);

    // Arm hanging down at his side.
    manShirtColor(stage);
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glVertex2f(fx, shY - 0.5f);
    glVertex2f(fx - 2.0f, hipY + 0.5f);
    glEnd();
    manSkinColor(stage);
    circle(fx - 2.0f, hipY + 0.4f, 0.8f);

    // Neck.
    manSkinColor(stage);
    rectangle(fx - 0.9f, shY - 0.4f, fx + 0.9f, headY - hr * 0.55f);

    drawManHead(stage, fx, headY, hr);
}

// (Talha) sitting on the grass
void drawManSitOnGrass(float x, float y)
{
    int stage = manAge();

    float hipY  = y + 2.2f;
    float shY   = hipY + 5.5f;
    float headY = shY + 3.0f;
    float hr    = 2.6f;

    // Legs stretched forward.
    manTrouserColor(stage);
    glLineWidth(5.0f);
    glBegin(GL_LINES);
    glVertex2f(x, hipY);        glVertex2f(x + 7.5f, y + 1.0f);
    glVertex2f(x, hipY - 0.8f); glVertex2f(x + 7.0f, y + 0.4f);
    glEnd();
    glColor3ub(40, 40, 45);
    circle(x + 7.8f, y + 1.0f, 0.9f);

    // Body.
    manShirtColor(stage);
    rectangle(x - 2.0f, hipY, x + 2.2f, shY);

    // Arm resting back on the grass.
    manShirtColor(stage);
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glVertex2f(x, shY - 0.8f);
    glVertex2f(x - 3.4f, hipY - 1.2f);
    glEnd();
    manSkinColor(stage);
    circle(x - 3.6f, hipY - 1.4f, 0.8f);

    // Neck.
    manSkinColor(stage);
    rectangle(x - 0.9f, shY - 0.4f, x + 0.9f, headY - hr * 0.55f);

    drawManHead(stage, x, headY, hr);
}

// (Talha) sitting on the bench
void drawManSitOnBench(float x, float seatY, float groundY)
{
    int stage = manAge();

    float hipY  = seatY + 1.6f;
    float shY   = hipY + 5.5f;
    float headY = shY + 3.0f;
    float hr    = 2.6f;

    // Thigh forward, then the shin down to the ground.
    manTrouserColor(stage);
    glLineWidth(5.0f);
    glBegin(GL_LINES);
    glVertex2f(x, hipY);        glVertex2f(x + 5.0f, hipY - 0.4f);
    glVertex2f(x + 5.0f, hipY); glVertex2f(x + 5.6f, groundY);
    glEnd();
    glColor3ub(40, 40, 45);
    circle(x + 5.8f, groundY + 0.4f, 0.9f);

    // Body.
    manShirtColor(stage);
    rectangle(x - 2.0f, hipY, x + 2.2f, shY);

    // Arm resting on his knee.
    manShirtColor(stage);
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glVertex2f(x, shY - 0.8f);
    glVertex2f(x + 3.6f, hipY + 0.4f);
    glEnd();
    manSkinColor(stage);
    circle(x + 3.8f, hipY + 0.3f, 0.8f);

    // Neck.
    manSkinColor(stage);
    rectangle(x - 0.9f, shY - 0.4f, x + 0.9f, headY - hr * 0.55f);

    drawManHead(stage, x, headY, hr);
}

// (Talha) pick the pose
void drawManOutside()
{
    if(manState == MAN_ON_GRASS)
        drawManSitOnGrass(manX, manY);
    else if(manState == MAN_WALK_TO_CAR || manState == MAN_WALK_TO_BENCH)
        drawManStanding(manX, manY);
    else if(manState == MAN_ON_BENCH)
        drawManSitOnBench(benchX - 1.0f, benchY + 8.0f, benchY);
}

// (Talha) one step toward a target
void manWalkTowards(float tx, float ty, int nextState)
{
    float dx = tx - manX;
    float dy = ty - manY;
    float d = (float)sqrt(dx * dx + dy * dy);

    if(d <= manWalkSpeed)
    {
        manX = tx;
        manY = ty;
        manState = nextState;
        return;
    }
    manX += manWalkSpeed * dx / d;
    manY += manWalkSpeed * dy / d;
}

// (Talha) walk to the car / bench
void updateMan()
{
    if(manState == MAN_WALK_TO_CAR)
    {
        // He comes down the grass and reaches the car from its
        // upper side, so he is always drawn behind it.
        manWalkTowards(carX - 5.0f, carY + 6.0f, MAN_IN_CAR);
        if(manState == MAN_IN_CAR)
        {
            // He is inside now: the engine starts and the car rolls.
            manInCar = true;
            journeyStarted = true;
            worldSpeed = 0.0f;
        }
    }
    else if(manState == MAN_WALK_TO_BENCH)
    {
        manWalkTowards(benchX - 5.0f, benchY, MAN_ON_BENCH);
    }
}

// (Talha) brake, bench in, man out
void updateEnding()
{
    if(!endingStarted)
        return;

    if(worldSpeed > 0.0f)
    {
        benchX -= worldSpeed;
        worldSpeed -= worldBrake;
        if(worldSpeed < 0.0f)
            worldSpeed = 0.0f;
    }
    else if(manState == MAN_IN_CAR)
    {
        manInCar = false;
        manState = MAN_WALK_TO_BENCH;
        manX = carX - 5.0f;
        manY = carY + 6.0f;
    }
}

// ======================================================
// CAR
// ======================================================

// (Talha) car body, headlight, wheels
void drawCarShape(float x, float y)
{
    glColor3ub(200, 30, 30);
    rectangle(x - 11, y, x + 11, y + 7);
    glColor3ub(180, 20, 20);
    glBegin(GL_QUADS);
    glVertex2f(x - 6, y + 7);
    glVertex2f(x - 5, y + 13);
    glVertex2f(x + 2, y + 13);
    glVertex2f(x + 8, y + 7);
    glEnd();
    glColor3ub(200, 30, 30);
    glBegin(GL_QUADS);
    glVertex2f(x + 8, y + 7);
    glVertex2f(x + 11, y + 7);
    glVertex2f(x + 13, y + 3);
    glVertex2f(x + 11, y + 2);
    glEnd();
    glColor3ub(120, 200, 230);
    rectangle(x - 4.5f, y + 7.5f, x + 4.5f, y + 11.0f);
    glColor3ub(180, 20, 20);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(x, y + 7.5f);
    glVertex2f(x, y + 11.0f);
    glEnd();

    // The man sits behind the glass, so he is drawn right after
    // the window and before the lights and wheels.
    drawManInCar(x, y);

    // ---- HEADLIGHT ----
    // Only on while the man is actually driving.
    if(manInCar)
    {
        glEnable(GL_BLEND);
        glColor4ub(255, 245, 160, 80);
        triangleShape(x + 12.5f, y + 2.5f, x + 34.0f, y + 9.0f, x + 34.0f, y - 4.0f);
        glDisable(GL_BLEND);
        glColor3ub(255, 250, 200);
        circle(x + 12.0f, y + 2.5f, 1.4f);
    }
    else
    {
        glColor3ub(150, 150, 140);
        circle(x + 12.0f, y + 2.5f, 1.1f);
    }

    glColor3ub(255, 70, 40);
    rectangle(x - 12.5f, y + 1.5f, x - 10.5f, y + 4.5f);
    glColor3ub(20, 20, 20);
    circle(x - 7, y, 3.2f);
    circle(x + 7, y, 3.2f);
    glColor3ub(220, 220, 220);
    glLineWidth(2.0f);
    float angle = wheelRotation * PI / 180.0f;
    for(int i = 0; i < 4; i++)
    {
        float a = angle + i * PI / 2.0f;
        glBegin(GL_LINES);
        glVertex2f(x - 7, y);
        glVertex2f(x - 7 + 2.3f * cos(a), y + 2.3f * sin(a));
        glVertex2f(x + 7, y);
        glVertex2f(x + 7 + 2.3f * cos(a), y + 2.3f * sin(a));
        glEnd();
    }
}

// (Talha) place and scale the car
void drawCar(float x, float y)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(carScale, carScale, 1.0f);
    drawCarShape(0.0f, 0.0f);
    glPopMatrix();
}

// ======================================================
// CAVE / TUNNEL
// ======================================================

// (Talha) cave mouth
void drawCave()
{
    if(!tunnelVisible)
        return;
    float x = movingTunnelX;
    glColor3ub(75, 75, 75);
    glBegin(GL_POLYGON);
    glVertex2f(x - 27, -40); glVertex2f(x - 23, -22); glVertex2f(x - 16, -10); glVertex2f(x - 7, -3);
    glVertex2f(x + 7, -3);   glVertex2f(x + 17, -10); glVertex2f(x + 24, -22); glVertex2f(x + 27, -40);
    glEnd();
    glColor3ub(5, 5, 5);
    glBegin(GL_POLYGON);
    glVertex2f(x - 13, -40); glVertex2f(x - 13, -24); glVertex2f(x - 8, -16);
    glVertex2f(x, -12);      glVertex2f(x + 8, -16); glVertex2f(x + 13, -24); glVertex2f(x + 13, -40);
    glEnd();
}

// (Talha) dark season screen
void drawTunnelDarkness()
{
    if(transitionStage != 3)
        return;
    glColor3ub(0, 0, 0);
    rectangle(-100, -60, 100, 100);
    glColor3ub(255, 255, 255);
    if(targetSeason == SPRING)
        drawText(-18, 5, "SPRING");
    else if(targetSeason == SUMMER)
        drawText(-19, 5, "SUMMER");
    else if(targetSeason == RAINY)
        drawText(-16, 5, "RAINY");
    else if(targetSeason == AUTUMN)
        drawText(-20, 5, "AUTUMN");
    else if(targetSeason == WINTER)
        drawText(-18, 5, "WINTER");
    else
        drawText(-22, 5, "DEFAULT");
}

// ======================================================
// HINT BOX  (top left, only before the journey starts)
// ======================================================

// (Talha) key list
void drawHintBox()
{
    if(manState != MAN_ON_GRASS)
        return;

    // box
    glColor3ub(255, 255, 240);
    rectangle(-96, -18, -40, 96);
    glColor3ub(40, 40, 40);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-96, -18);
    glVertex2f(-40, -18);
    glVertex2f(-40, 96);
    glVertex2f(-96, 96);
    glEnd();

    // text
    glColor3ub(30, 30, 30);
    drawText(-93, 88, "PRESS  F  TO START");
    drawText(-93, 80, "0 - 5   SEASONS");
    drawText(-93, 72, "9   END OF JOURNEY");
    drawText(-93, 64, "8   LAST WORDS");
    drawText(-93, 52, "WHILE DRIVING (SPRING):");
    drawText(-93, 44, "J   DUCKS CROSS ROAD");
    drawText(-93, 36, "S   DRIVE ON AFTER DUCKS");
    drawText(-93, 28, "K   BIRDS FLYING");
    drawText(-93, 20, "V   FIRST-PERSON VIEW");
    drawText(-93, 8,  "RAINY:  T  THUNDER");
    drawText(-93, -4, "ANY SEASON:");
    drawText(-93, -12, "A/D STEER LEFT/RIGHT");
}

// ======================================================
// WORLD & TRANSITION UPDATE
// ======================================================

// (Talha) scroll every background layer
void updateWorld()
{
    // Nothing moves until the man is in the car.
    if(!journeyStarted)
        return;

    if(transitionStage == 0)
    {
        worldMove     -= worldSpeed;
        forestMove    -= worldSpeed * forestSpeedMul;
        grassMove     -= worldSpeed * grassSpeedMul;
        bgGrassMove   -= worldSpeed * bgGrassSpeedMul;
        flowerMove    -= worldSpeed * flowerSpeedMul;
        butterflyMove -= worldSpeed * butterflySpeedMul;
        mountainMove  -= worldSpeed * mountainSpeedMul;
        roadGrassMove -= worldSpeed * roadGrassSpeedMul;
        winterBgMove  -= worldSpeed * winterBgSpeedMul;

        // 300-unit scrolling cycle, wrapped independently per layer
        if(worldMove < -300)     worldMove     += 300;
        if(forestMove < -300)    forestMove    += 300;
        if(grassMove < -300)     grassMove     += 300;
        if(bgGrassMove < -300)   bgGrassMove   += 300;
        if(flowerMove < -300)    flowerMove    += 300;
        if(butterflyMove < -300) butterflyMove += 300;
        if(mountainMove < -300)  mountainMove  += 300;
        if(roadGrassMove < -300) roadGrassMove += 300;
        if(winterBgMove < -300)  winterBgMove  += 300;
    }
}

// (Talha) the 6-stage season change
void updateSeasonTransition()
{
    if(!changingSeason)
        return;
    if(transitionStage == 1)
    {
        transitionCarX += transitionCarSpeed;
        if(transitionCarX >= movingTunnelX - 16)
            transitionStage = 2;
    }
    else if(transitionStage == 2)
    {
        carInsideTunnel = true;
        transitionStage = 3;
        seasonScreenTimer = 0.0f;
    }
    else if(transitionStage == 3)
    {
        seasonScreenTimer += 0.05f;
        if(seasonScreenTimer >= 1.0f)
        {
            currentSeason = targetSeason;
            if(currentSeason == AUTUMN)
                resetAutumnLeaves();
            carInsideTunnel = false;
            tunnelOnRight = false;
            tunnelOnLeft = true;
            movingTunnelX = -90.0f;
            transitionCarX = movingTunnelX + 12.0f;
            transitionStage = 4;
        }
    }
    else if(transitionStage == 4)
    {
        transitionCarX += transitionCarSpeed;
        if(transitionCarX >= 0.0f)
        {
            transitionCarX = carX;
            transitionStage = 5;
            // Reset every layer's offset together so the new
            // season's background starts clean and in sync.
            worldMove = 0.0f;
            forestMove = 0.0f;
            grassMove = 0.0f;
            bgGrassMove = 0.0f;
            flowerMove = 0.0f;
            butterflyMove = 0.0f;
            mountainMove = 0.0f;
            roadGrassMove = 0.0f;
            winterBgMove = 0.0f;
        }
    }
    else if(transitionStage == 5)
    {
        worldMove     -= worldSpeed;
        forestMove    -= worldSpeed * forestSpeedMul;
        grassMove     -= worldSpeed * grassSpeedMul;
        bgGrassMove   -= worldSpeed * bgGrassSpeedMul;
        flowerMove    -= worldSpeed * flowerSpeedMul;
        butterflyMove -= worldSpeed * butterflySpeedMul;
        mountainMove  -= worldSpeed * mountainSpeedMul;
        roadGrassMove -= worldSpeed * roadGrassSpeedMul;
        winterBgMove  -= worldSpeed * winterBgSpeedMul;

        if(worldMove < -300)     worldMove     += 300;
        if(forestMove < -300)    forestMove    += 300;
        if(grassMove < -300)     grassMove     += 300;
        if(bgGrassMove < -300)   bgGrassMove   += 300;
        if(flowerMove < -300)    flowerMove    += 300;
        if(butterflyMove < -300) butterflyMove += 300;
        if(mountainMove < -300)  mountainMove  += 300;
        if(roadGrassMove < -300) roadGrassMove += 300;
        if(winterBgMove < -300)  winterBgMove  += 300;

        movingTunnelX -= tunnelExitSpeed;
        transitionCarX = carX;
        if(movingTunnelX < -120.0f)
            transitionStage = 6;
    }
    else if(transitionStage == 6)
    {
        tunnelVisible = false;
        tunnelOnLeft = false;
        tunnelOnRight = false;
        changingSeason = false;
        transitionStage = 0;
        transitionCarX = carX;
    }
}

// ======================================================
// FIRST-PERSON POV  (V key)
// ======================================================

// (Sanzida) start the blink into / out of POV
void startPovToggle()
{
    if(povBlinkStage != 0)
        return;

    if(!povActive && currentSeason != SPRING)
        return;

    povBlinkStage = povActive ? 2 : 1;
    povBlinkTimer = 0.0f;
}

// (Sanzida) fixed pseudo-random scatter
float povHash(float n)
{
    float x = (float)sin((double)n * 12.9898) * 43758.5453f;
    return x - (float)floor((double)x);
}

// (Sanzida) blink timer
void updatePovBlink()
{
    if(povBlinkStage == 0)
        return;

    povBlinkTimer += 0.05f;
    if(povBlinkTimer >= povBlinkDuration)
    {
        povActive = (povBlinkStage == 1);
        povBlinkStage = 0;
        povBlinkTimer = 0.0f;
    }
}

// (Sanzida) sun rays from the sun disc
void drawPovSunRays()
{
    if(currentSeason == WINTER)
        return;

    float sunX = 75.0f, sunY = 78.0f, sunR = 9.0f;

    float rayAngleDeg[4] = { 222.0f, 242.0f, 258.0f, 274.0f };
    float rayLength[4]   = { 190.0f, 158.0f, 138.0f, 128.0f };

    // Three layers per ray: wide+faint outer glow, a medium band,
    // and a narrower, slightly brighter core.
    float spreadDeg[3] = { 9.0f, 5.5f, 2.6f };
    GLubyte layerAlpha[3] = { 12, 18, 26 };

    glEnable(GL_BLEND);
    for(int i = 0; i < 4; i++)
    {
        float baseAngle = rayAngleDeg[i] * (float)PI / 180.0f;

        for(int layer = 0; layer < 3; layer++)
        {
            float halfSpread = spreadDeg[layer] * (float)PI / 180.0f;
            float baseR = sunR * 0.7f;

            float ax = sunX + baseR * (float)cos(baseAngle - halfSpread);
            float ay = sunY + baseR * (float)sin(baseAngle - halfSpread);
            float bx = sunX + baseR * (float)cos(baseAngle + halfSpread);
            float by = sunY + baseR * (float)sin(baseAngle + halfSpread);
            float tx = sunX + rayLength[i] * (float)cos(baseAngle);
            float ty = sunY + rayLength[i] * (float)sin(baseAngle);

            glColor4ub(255, 255, 250, layerAlpha[layer]);
            triangleShape(ax, ay, bx, by, tx, ty);
        }
    }
    glDisable(GL_BLEND);
}

// (Sanzida) haze along the horizon
void drawPovHorizonGlow()
{
    float horizonY = -20.0f;
    glEnable(GL_BLEND);
    for(int i = 0; i < 6; i++)
    {
        float f = (float)i / 6.0f;
        GLubyte a = (GLubyte)(75.0f * (1.0f - f));
        glColor4ub(255, 255, 245, a);
        float y0 = horizonY + f * 11.0f;
        rectangle(-100, y0, 100, y0 + 2.4f);
    }
    glDisable(GL_BLEND);
}

// (Sanzida) POV clouds
void drawPovClouds()
{
    drawCloud(-82, 58, 8);
    drawCloud(-68, 74, 12);
    drawCloud(-28, 84, 14);
    drawCloud(6, 70, 9);
    drawCloud(42, 80, 13);
    drawCloud(76, 62, 9);
    drawCloud(94, 76, 7);
}

// (Sanzida) POV road and lane dashes
void drawPovRoad()
{
    float horizonY = -20.0f;

    glColor3ub(58, 96, 46);
    rectangle(-100, -60, 100, horizonY);

    if(currentSeason == WINTER)
        glColor3ub(225, 230, 235);
    else if(currentSeason == SUMMER)
        glColor3ub(150, 132, 45);
    else
        glColor3ub(66, 128, 52);
    rectangle(-100, -60, 100, -33);

    // The road itself: light dirt/gravel, not dark asphalt, as a
    // long straight wedge.
    glColor3ub(160, 150, 126);
    glBegin(GL_QUADS);
    glVertex2f(-58, -60);
    glVertex2f(58, -60);
    glVertex2f(4, horizonY);
    glVertex2f(-4, horizonY);
    glEnd();

    // A brighter, sunlit strip of grass right at the road's edge.
    glColor3ub(122, 170, 68);
    glBegin(GL_QUADS);
    glVertex2f(-64, -60); glVertex2f(-58, -60);
    glVertex2f(-4, horizonY); glVertex2f(-4.6f, horizonY);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(58, -60); glVertex2f(64, -60);
    glVertex2f(4.6f, horizonY); glVertex2f(4, horizonY);
    glEnd();

    glColor3ub(210, 200, 175);
    float t = povWheelTime * 0.6f;
    t -= (float)((int)t);
    for(int i = 0; i < 8; i++)
    {
        float f = (i + t) / 8.0f;
        if(f > 1.0f) f -= 1.0f;
        float y = -55.0f + f * (horizonY - (-55.0f));
        float halfW = (1.0f - f) * 1.3f + 0.04f;
        float barH = (1.0f - f) * 3.0f + 0.10f;
        rectangle(-halfW, y, halfW, y + barH);
    }
}

// (Sanzida) gapless undergrowth clump
void drawBushClump(float x, float y, float scale)
{
    if(currentSeason == WINTER)
        return;

    int r, g, b;
    if(currentSeason == SUMMER) { r = 150; g = 132; b = 40; }
    else                        { r = 44;  g = 140; b = 42; }

    shadedColor(r, g, b, 0.92f);
    ellipse(x - 1.6f * scale, y + 0.9f * scale, 3.0f * scale, 2.1f * scale);
    shadedColor(r, g, b, 1.08f);
    ellipse(x + 1.6f * scale, y + 0.8f * scale, 2.9f * scale, 2.0f * scale);
    shadedColor(r, g, b, 1.0f);
    ellipse(x, y + 1.7f * scale, 2.7f * scale, 2.0f * scale);

    drawGrass(x - 1.0f * scale, y + 0.3f * scale, 1.1f * scale);
    drawGrass(x + 1.1f * scale, y + 0.2f * scale, 1.0f * scale);
}

// (Sanzida) roadside forest in perspective
void drawPovForestSide(float side)
{
    float horizonY = -20.0f;
    const int SLOTS = 9;
    const int ROWS = 2;
    const int N = SLOTS * ROWS;
    float scrollSpeed = 0.045f;   // same 0.05-ish/frame pace used elsewhere

    float zFar = 50.0f;
    float zNear = 4.0f;
    float groundDropK = 25.0f;    // only a mild sink as trees approach

    int n = 0;
    float fArr[N], xArr[N], yArr[N], scaleArr[N];
    int typeArr[N];
    float g1x[N], g1y[N], g1s[N];
    float g2x[N], g2y[N], g2s[N];

    for(int row = 0; row < ROWS; row++)
    {
        float lateralK = (row == 0) ? 560.0f : 780.0f;
        float scaleK   = (row == 0) ? 7.5f   : 6.0f;

        for(int i = 0; i < SLOTS; i++)
        {
            float raw = (float)i / SLOTS + povWheelTime * scrollSpeed + row * (0.5f / SLOTS);
            float loopIndex = (float)floor((double)raw);
            float f = raw - loopIndex;

            // Distance ahead of the car, shrinking smoothly from
            // far to near.
            float z = zNear + (1.0f - f) * (zFar - zNear);

            float seed = side * 61.0f + i * 7.3f + row * 133.0f + loopIndex * 271.0f;
            float lateralJitter = 1.0f + (povHash(seed) - 0.5f) * 0.3f;

            float screenX = side * (lateralK * lateralJitter) / z;
            float screenY = horizonY - groundDropK * (1.0f / z - 1.0f / zFar);
            float scale = scaleK / z;

            float jx = (povHash(seed + 0.37f) - 0.5f) * 2.0f;
            float jy = (povHash(seed + 0.53f) - 0.5f) * 1.2f;

            float typeRoll = povHash(seed + 0.71f);
            int type = (typeRoll < 0.85f) ? 1 : (typeRoll < 0.94f ? 0 : 2);

            fArr[n] = f;
            xArr[n] = screenX + jx;
            yArr[n] = screenY + jy;
            scaleArr[n] = scale;
            typeArr[n] = type;

            float gx = screenX + (povHash(seed + 0.19f) - 0.5f) * (1.5f + scale * 1.5f);
            float gy = screenY - (1.5f + scale * 1.5f);
            g1x[n] = gx;  g1y[n] = gy;  g1s[n] = 0.55f + scale * 0.75f;

            float gx2 = screenX + (povHash(seed + 0.61f) - 0.5f) * (2.5f + scale * 2.2f);
            float gy2 = screenY - (1.2f + scale * 1.2f);
            g2x[n] = gx2; g2y[n] = gy2; g2s[n] = 0.45f + scale * 0.65f;

            n++;
        }
    }

    for(int a = 1; a < n; a++)
    {
        float kf = fArr[a], kx = xArr[a], ky = yArr[a], ks = scaleArr[a];
        int kt = typeArr[a];
        float kg1x = g1x[a], kg1y = g1y[a], kg1s = g1s[a];
        float kg2x = g2x[a], kg2y = g2y[a], kg2s = g2s[a];
        int b = a - 1;
        while(b >= 0 && fArr[b] > kf)
        {
            fArr[b + 1] = fArr[b]; xArr[b + 1] = xArr[b]; yArr[b + 1] = yArr[b];
            scaleArr[b + 1] = scaleArr[b]; typeArr[b + 1] = typeArr[b];
            g1x[b + 1] = g1x[b]; g1y[b + 1] = g1y[b]; g1s[b + 1] = g1s[b];
            g2x[b + 1] = g2x[b]; g2y[b + 1] = g2y[b]; g2s[b + 1] = g2s[b];
            b--;
        }
        fArr[b + 1] = kf; xArr[b + 1] = kx; yArr[b + 1] = ky; scaleArr[b + 1] = ks; typeArr[b + 1] = kt;
        g1x[b + 1] = kg1x; g1y[b + 1] = kg1y; g1s[b + 1] = kg1s;
        g2x[b + 1] = kg2x; g2y[b + 1] = kg2y; g2s[b + 1] = kg2s;
    }

    for(int k = 0; k < n; k++)
    {
        // Bush clumps first, so they sit behind/at the base of
        // their tree rather than floating in front of the canopy.
        drawBushClump(g1x[k], g1y[k], g1s[k]);
        drawBushClump(g2x[k], g2y[k], g2s[k]);
        drawTree(xArr[k], yArr[k], scaleArr[k], typeArr[k]);
    }
}

// (Sanzida) roadside flowers
void drawPovFlowers()
{
    if(currentSeason != SPRING)
        return;

    float horizonY = -20.0f;
    const int SLOTS = 6;
    const int N = SLOTS * 2;
    float scrollSpeed = 0.045f;

    float zFar = 45.0f;
    float zNear = 5.0f;
    float lateralK = 720.0f;
    float scaleK = 5.5f;
    float groundDropK = 15.0f;

    float fArr[N], xArr[N], yArr[N], scaleArr[N];
    int colorArr[N];
    int n = 0;

    for(int side = -1; side <= 1; side += 2)
    {
        for(int i = 0; i < SLOTS; i++)
        {
            float raw = (float)i / SLOTS + povWheelTime * scrollSpeed;
            float loopIndex = (float)floor((double)raw);
            float f = raw - loopIndex;

            float z = zNear + (1.0f - f) * (zFar - zNear);

            float seed = side * 87.0f + i * 5.9f + loopIndex * 191.0f;
            float lateralJitter = 1.0f + (povHash(seed) - 0.5f) * 0.3f;

            float screenX = side * (lateralK * lateralJitter) / z;
            float screenY = horizonY - groundDropK * (1.0f / z - 1.0f / zFar);
            float scale = scaleK / z;

            int colorType = (povHash(seed + 0.4f) < 0.5f) ? 0 : 2; // pink / white only

            fArr[n] = f; xArr[n] = screenX; yArr[n] = screenY; scaleArr[n] = scale; colorArr[n] = colorType;
            n++;
        }
    }

    for(int a = 1; a < n; a++)
    {
        float kf = fArr[a], kx = xArr[a], ky = yArr[a], ks = scaleArr[a];
        int kc = colorArr[a];
        int b = a - 1;
        while(b >= 0 && fArr[b] > kf)
        {
            fArr[b + 1] = fArr[b]; xArr[b + 1] = xArr[b]; yArr[b + 1] = yArr[b];
            scaleArr[b + 1] = scaleArr[b]; colorArr[b + 1] = colorArr[b];
            b--;
        }
        fArr[b + 1] = kf; xArr[b + 1] = kx; yArr[b + 1] = ky; scaleArr[b + 1] = ks; colorArr[b + 1] = kc;
    }

    for(int k = 0; k < n; k++)
        drawFlower(xArr[k], yArr[k], scaleArr[k], colorArr[k]);
}

// (Sanzida) windscreen, mirror, dashboard
void drawPovCarInterior()
{
    glColor3ub(28, 26, 24);
    rectangle(-100, 94, 100, 100);

    // Rear-view mirror, hanging from the header, with a small
    // hint of road reflected inside it.
    glColor3ub(20, 18, 16);
    rectangle(-11, 80, 11, 88);
    glColor3ub(150, 178, 190);
    rectangle(-9.3f, 81.3f, 9.3f, 86.7f);
    glColor3ub(120, 130, 100);
    triangleShape(-9.3f, 81.3f, 9.3f, 81.3f, 0, 86.7f);
    glColor3ub(215, 215, 210);
    rectangle(-0.6f, 81.3f, 0.6f, 86.7f);

    // Dashboard along the bottom - dark and flat, with a soft
    // sheen catching the daylight along its top edge.
    glColor3ub(32, 30, 28);
    rectangle(-100, -60, 100, -30);
    glEnable(GL_BLEND);
    glColor4ub(255, 255, 255, 18);
    rectangle(-100, -32, 100, -30);
    glDisable(GL_BLEND);

    // A pair of air vents set into the dash.
    for(int side = -1; side <= 1; side += 2)
    {
        float vx = side * 30.0f;
        glColor3ub(20, 19, 18);
        rectangle(vx - 7.0f, -46.0f, vx + 7.0f, -38.0f);
        glColor3ub(48, 46, 44);
        for(int s = 0; s < 4; s++)
        {
            float sy = -45.0f + s * 1.8f;
            rectangle(vx - 6.0f, sy, vx + 6.0f, sy + 0.7f);
        }
    }

    // A slim centre console strip.
    glColor3ub(24, 22, 20);
    rectangle(-6.0f, -60.0f, 6.0f, -40.0f);
}

// (Sanzida) wheel and hands
void drawPovSteeringWheel()
{
    float cx = -34.0f;
    float cy = -58.0f;
    float outerR = 24.0f;
    float innerR = 18.5f;

    float swayDeg = 5.0f * (float)sin(povWheelTime * 0.6f);
    if(steerLeftDown)  swayDeg -= 22.0f;
    if(steerRightDown) swayDeg += 22.0f;
    float swayRad = swayDeg * (float)PI / 180.0f;

    glPushMatrix();
    glTranslatef(cx, cy, 0.0f);
    glRotatef(swayDeg, 0.0f, 0.0f, 1.0f);

    glColor3ub(16, 16, 16);
    circle(0, 0, outerR);
    glColor3ub(40, 40, 44);
    circle(0, 0, innerR);

    glColor3ub(16, 16, 16);
    rectangle(-1.8f, 0.0f, 1.8f, innerR);
    glBegin(GL_QUADS);
    glVertex2f(-1.6f, -1.0f);
    glVertex2f(-innerR * 0.85f, -innerR * 0.5f);
    glVertex2f(-innerR * 0.72f, -innerR * 0.22f);
    glVertex2f(-1.6f, 1.0f);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(1.6f, -1.0f);
    glVertex2f(innerR * 0.85f, -innerR * 0.5f);
    glVertex2f(innerR * 0.72f, -innerR * 0.22f);
    glVertex2f(1.6f, 1.0f);
    glEnd();

    glColor3ub(24, 24, 24);
    circle(0, 0, 4.0f);
    glColor3ub(60, 60, 60);
    circle(0, 0, 1.4f);

    glPopMatrix();

    int stage = manAge();
    // Roughly 10 and 2 o'clock on the visible upper arc.
    float gripAngle[2] = { (float)PI * 0.62f, (float)PI * 0.18f };

    for(int side = 0; side < 2; side++)
    {
        float a = gripAngle[side] + swayRad;
        float radialX = (float)cos(a);
        float radialY = (float)sin(a);
        float tangX = -radialY;
        float tangY = radialX;
        // Left hand fans one way along the tangent, right hand
        // the other, like a mirrored pair of real hands.
        float fan = (side == 0) ? -1.0f : 1.0f;

        float hx = cx + (outerR - 1.0f) * radialX;
        float hy = cy + (outerR - 1.0f) * radialY;

        manShirtColor(stage);
        orientedQuad(hx - radialX * 5.5f, hy - radialY * 5.5f, radialX, radialY, 3.6f, 2.6f);
        orientedQuad(hx - radialX * 2.6f, hy - radialY * 2.6f, radialX, radialY, 2.2f, 2.9f);

        // Wrist - a short transition quad between sleeve and hand.
        manSkinColor(stage);
        orientedQuad(hx - radialX * 0.8f, hy - radialY * 0.8f, radialX, radialY, 1.1f, 2.5f);

        orientedQuad(hx + radialX * 0.9f, hy + radialY * 0.9f, tangX * fan, tangY * fan, 3.4f, 2.3f);
    }
}

// (Sanzida) the whole first-person view
void drawPovScene()
{
    float bobY = 0.35f * (float)sin(povWheelTime * 3.1f);
    float bobX = 0.15f * (float)sin(povWheelTime * 2.3f + 1.0f);

    glPushMatrix();
    glTranslatef(bobX, bobY, 0.0f);

    drawSky();
    drawSun();
    drawPovClouds();
    drawPovHorizonGlow();
    drawPovSunRays();

    float savedMountainMove = mountainMove;
    mountainMove = 0.0f;
    drawMountains();
    mountainMove = savedMountainMove;

    float shiftScale = 0.35f;
    glPushMatrix();
    glTranslatef(-carX * shiftScale, 0.0f, 0.0f);

    drawPovRoad();
    drawPovForestSide(-1.0f);
    drawPovForestSide(1.0f);
    drawPovFlowers();

    glPopMatrix();

    drawPovCarInterior();
    drawPovSteeringWheel();

    glColor3ub(220, 220, 215);
    drawText(4, -55, "V - EXIT");

    glPopMatrix();
}

// (Sanzida) on-screen wait / press-S message
void drawDuckStopPrompt()
{
    if(!duckStopActive)
        return;

    glColor3ub(255, 255, 255);
    if(duckWaitingResume)
        drawText(-26, 74, "PRESS  S  TO DRIVE ON");
    else
        drawText(-30, 74, "DUCKS CROSSING - WAIT...");
}

// ======================================================
// DISPLAY
// ======================================================

// (Talha) draw order for the whole scene
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Mid-blink: a plain black flash, nothing else drawn this
    // frame - the world keeps updating underneath regardless.
    if(povBlinkStage != 0)
    {
        glColor3ub(0, 0, 0);
        rectangle(-100, -60, 100, 100);
        glutSwapBuffers();
        return;
    }

    // Showing the first-person pov instead of the normal scene.
    if(povActive)
    {
        drawPovScene();
        drawDuckStopPrompt();
        glutSwapBuffers();
        return;
    }

    drawSky();
    drawSun();
    drawClouds();
    if(currentSeason == WINTER)
    {
        drawWinterMountains();
        drawWinterBackground();
    }
        drawMountains();
    drawGround();
    drawPuddles();
    drawRoad();
    drawRoadGrass();

    drawForestBg();
   // drawMountains();
    drawForest();

        if(currentSeason == sedlife)
        {drawGrassField();}
    if(currentSeason == SPRING)
        {drawSpringEnvironment();}
    if(currentSeason == AUTUMN)
    {
        drawLandedAutumnLeaves();
    }
    drawBirdFlock();
    if(currentSeason == WINTER)
        {drawSnow();}
    if(currentSeason == AUTUMN)
        {drawFallingAutumnLeaves();}
    // Darken the rainy world first, then keep the rain and the
    // lightning bright on top of it.
    drawStormOverlay();
    drawRain();
    drawLightning();

    drawBench();
    drawDuckFamily();
    if(tunnelVisible && transitionStage != 3)
        {drawCave();}
    // The man walks on the far side, so he is drawn before the car.
    drawManOutside();
    if(changingSeason)
        {drawCar(transitionCarX, carY);}
    else
        drawCar(carX, carY);
    drawHintBox();
    drawDuckStopPrompt();
    drawTunnelDarkness();
    drawEndMessage();
    glutSwapBuffers();
}

// ======================================================
// UPDATE / ANIMATION
// ======================================================

// (Talha) animation timer
void update(int value)
{
    if(!paused)
    {
        // The car speeds up smoothly once the man is in it -
        // unless it's currently stopped for a duck crossing.
        if(journeyStarted && !endingStarted && !duckStopActive
           && worldSpeed < worldSpeedMax)
        {
            worldSpeed += worldAcceleration;
            if(worldSpeed > worldSpeedMax)
                worldSpeed = worldSpeedMax;
        }

        if(duckStopActive && !endingStarted && worldSpeed > 0.0f)
        {
            worldSpeed -= duckBrake;
            if(worldSpeed < 0.0f)
                worldSpeed = 0.0f;
        }

        updateEnding();
        updateWorld();
        updateMan();

        // A / D steering: only while actually driving, same
        // restriction as the effect keys and season keys.
        if(manState == MAN_IN_CAR && !changingSeason && !endingStarted)
        {
            if(steerLeftDown)  carX -= carSteerSpeed;
            if(steerRightDown) carX += carSteerSpeed;
            if(carX < carSteerMin) carX = carSteerMin;
            if(carX > carSteerMax) carX = carSteerMax;
        }

        // The wheels turn only as fast as the car really moves,
        // so they stand still before and after the journey.
        wheelRotation -= 30.0f * worldSpeed;
        if(wheelRotation < 0)
            wheelRotation += 360.0f;

        if(changingSeason)
            updateSeasonTransition();
        updateClouds();
        updateLightning();
        if(currentSeason == RAINY)
            updateRain();
        if(currentSeason == SPRING)
        {
            butterflyTime += 0.05f;
            beeOrbitTime += 0.05f;
        }
        if(currentSeason == WINTER)
            updateSnow();
        if(currentSeason == AUTUMN)
        {
            autumnTime += 0.05f;
            updateAutumnLeaves();
        }

        updateDuckCrossing();
        updateBirdFlock();
        updatePovBlink();
        if(povActive)
            povWheelTime += 0.05f;
    }
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ======================================================
// KEYBOARD
// ======================================================

// (Sanzida) gate for J / K / V (spring, driving only)
bool canTriggerEffect()
{
    return (manState == MAN_IN_CAR && !endingStarted && !changingSeason
            && currentSeason == SPRING);
}

// (Talha) begin a season change
void startSeasonChange(int season)
{
    if(manState != MAN_IN_CAR || endingStarted)
        return;
    if(currentSeason == season || changingSeason)
        return;

    targetSeason = season;
    changingSeason = true;
    transitionStage = 1;
    tunnelVisible = true;
    tunnelOnRight = true;
    tunnelOnLeft = false;
    movingTunnelX = tunnelRightX;
    carInsideTunnel = false;
    transitionCarX = carX;

    povActive = false;
    povBlinkStage = 0;
    povBlinkTimer = 0.0f;

    duckActive = false;
    duckStopActive = false;
    duckWaitingResume = false;
    birdActive = false;
}

// (Talha) key handling
void keyboard(unsigned char key, int x, int y)
{
    if(key == 'f' || key == 'F')
    {
        // He gets up from the grass and walks to the car.
        if(manState == MAN_ON_GRASS)
            manState = MAN_WALK_TO_CAR;
    }
    else if(key == '9')
    {
        // End of the journey: the bench comes in from the right.
        if(manState == MAN_IN_CAR && !changingSeason && !endingStarted)
        {
            endingStarted = true;
            benchVisible = true;
            benchX = 72.0f;

            // Hand braking over to updateEnding() - clear any duck
            // hold so the two don't fight over worldSpeed.
            duckActive = false;
            duckStopActive = false;
            duckWaitingResume = false;
        }
    }
    else if(key == '8')
    {
        // The last words, after the journey has ended.
        if(endingStarted)
            showEndMessage = true;
    }
    else if(key == '0')
        startSeasonChange(sedlife);
    else if(key == '1')
        startSeasonChange(SPRING);
    else if(key == '2')
        startSeasonChange(SUMMER);
    else if(key == '3')
        startSeasonChange(RAINY);
    else if(key == '4')
        startSeasonChange(AUTUMN);
    else if(key == '5')
        startSeasonChange(WINTER);
    else if(key == 'j' || key == 'J')
    {
        if(canTriggerEffect())
            startDuckCrossing();
    }
    else if(key == 'k' || key == 'K')
    {
        if(canTriggerEffect())
            startBirdFlock();
    }
    else if(key == 'v' || key == 'V')
    {
        if(canTriggerEffect())
            startPovToggle();
    }
    else if(key == 's' || key == 'S')
    {
        if(duckWaitingResume)
        {
            duckStopActive = false;
            duckWaitingResume = false;
        }
    }
    else if(key == 't' || key == 'T')
    {
        // Thunder clap and lightning flash - rainy season only.
        triggerThunder();
    }
    else if(key == 'a' || key == 'A')
        steerLeftDown = true;
    else if(key == 'd' || key == 'D')
        steerRightDown = true;
    else if(key == ' ')
        paused = !paused;
    else if(key == 27)
        exit(0);

    glutPostRedisplay();
}

// (Sanzida) release A / D steering
void keyboardUp(unsigned char key, int x, int y)
{
    if(key == 'a' || key == 'A')
        steerLeftDown = false;
    else if(key == 'd' || key == 'D')
        steerRightDown = false;
}

// ======================================================
// INITIALIZATION
// ======================================================

// (Talha) projection and start-up
void init()
{
    glClearColor(0.50f, 0.80f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-100, 100, -60, 100);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    initSnow();
    initRain();
    initAutumnLeaves();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ======================================================
// MAIN
// ======================================================

// (Talha) window and GLUT setup
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1100, 700);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("Forest Through The Cycle Of A Year");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}

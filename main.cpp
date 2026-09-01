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
// TRAVEL / TUNNEL STATE
// ======================================================

bool changingSeason = false;

// 0 = normal running
// 1 = tunnel on right, car approaching
// 2 = car entering right tunnel
// 3 = black season screen
// 4 = new season, tunnel on left, car coming out
// 5 = car reached the middle; tunnel continues moving left
// 6 = tunnel has left the window; transition finished
int transitionStage = 0;

bool tunnelVisible = false;
bool tunnelOnRight = false;
bool tunnelOnLeft = false;
bool carInsideTunnel = false;

// ======================================================
// WORLD MOVEMENT
// ======================================================

// Forest and ground objects now use a 300-unit cycle.
// The visible screen is only -100 to +100.
// Therefore objects can exist from about -150 to +150.
//
// worldMove is kept as the "reference" scroll speed and is
// still used to drive the road stripes (they must always
// match the car's actual travel speed 1:1).
//
// Every other background layer now has its OWN offset and its
// OWN speed multiplier, so each layer scrolls independently.
// This gives a simple parallax effect: layers meant to feel
// closer to the camera move a bit faster than worldSpeed,
// layers meant to feel farther away move slower.
float worldMove = 0.0f;
float worldSpeed = 0.5f;

// --- Per-layer offsets (accumulated each frame) ---
float forestMove = 0.0f;      // trees
float grassMove = 0.0f;       // foreground grass field
float bgGrassMove = 0.0f;     // forest background grass strip
float flowerMove = 0.0f;      // spring flowers
float butterflyMove = 0.0f;   // spring butterflies
float mountainMove = 0.0f;    // distant mountains
float roadGrassMove = 0.0f;   // little grass tufts along the road edge
float winterBgMove = 0.0f;    // winter background sweep

// --- Per-layer speed multipliers, relative to worldSpeed ---
// < 1.0 = feels farther away (moves slower than the road)
// = 1.0 = moves with the road
// > 1.0 = feels closer to the camera (moves faster than the road)
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

// 0 = Round
// 1 = Pine
// 2 = Bushy Triangle
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
// CLOUD VARIABLES
// ======================================================

float cloud1X = -80;
float cloud2X = -10;
float cloud3X = 60;
float cloud4X = -50;
float cloud5X = 40;

// ======================================================
// BASIC SHAPES
// ======================================================

void rectangle(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

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

void triangleShape(float x1, float y1, float x2, float y2, float x3, float y3)
{
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}

// Takes a base color and a shade factor (e.g. 0.7 = darker,
// 1.3 = lighter) and applies it, clamped to [0,255].
// Used to give each leaf circle/triangle on a tree its own
// distinct shade of the season's base foliage color.
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

void initSnow()
{
    for(int i = 0; i < SNOW_COUNT; i++)
    {
        snowX[i] = -100.0f + (float)(rand() % 201);
        snowY[i] = -60.0f + (float)(rand() % 161);
        snowSpeed[i] = 0.20f + (float)(rand() % 30) / 100.0f;
    }
}

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
// SKY / SUN / GROUND
// ======================================================

void drawSky()
{
    if(currentSeason == SPRING)
        glColor3ub(150, 217, 255);
    else if(currentSeason == SUMMER)
        glColor3ub(105, 195, 255);
    else if(currentSeason == WINTER)
        glColor3ub(180, 210, 235);
    else
        glColor3ub(128, 204, 255);
    rectangle(-100, -20, 100, 100);
}

void drawSun()
{
    if(currentSeason == WINTER)
        return;
    if(currentSeason == SUMMER)
        glColor3ub(255, 180, 20);
    else
        glColor3ub(255, 217, 26);
    circle(75, 78, 9);
}

void drawGround()
{
    if(currentSeason == SPRING)
        glColor3ub(72, 168, 58);
    else if(currentSeason == SUMMER)
        glColor3ub(190, 170, 75);
    else if(currentSeason == WINTER)
        glColor3ub(235, 240, 245);
    else
        glColor3ub(51, 140, 46);
    rectangle(-100, -60, 100, -20);
}

// ======================================================
// CLOUDS
// ======================================================

void drawCloud(float x, float y, float size)
{
    glColor3ub(242, 242, 242);
    circle(x - size * 0.8f, y, size * 0.7f);
    circle(x, y + 2, size);
    circle(x + size * 0.8f, y, size * 0.8f);
    rectangle(x - size * 0.8f, y - size * 0.5f, x + size * 0.8f, y + size * 0.35f);
}

void drawClouds()
{
    drawCloud(cloud1X, 80, 7);
    drawCloud(cloud2X, 85, 8);
    drawCloud(cloud3X, 75, 6);
    drawCloud(cloud2X, 60, 9);
    drawCloud(cloud3X, 60, 5);
}

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
// MOUNTAINS
// ======================================================

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

// Each of the 5 leaf circles gets its own shade of the
// season's base foliage color instead of one flat color.
// Light is treated as coming from the upper right (where the
// sun is drawn), so blobs up and to the right are a touch
// lighter, blobs down and to the left a touch darker.
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

void drawTreeRound(float x, float y, float scale)
{
    drawRoundTreeTrunk(x, y, scale);
    drawBranches(x, y, scale);
    drawTreeLeaves(x, y, scale);
}

// Left unchanged (flat color) per request - pine excluded.
void drawPineLeaves(float x, float y, float scale)
{
    if(currentSeason == SPRING)
        glColor3ub(55, 130, 45);
    else if(currentSeason == SUMMER)
        glColor3ub(10, 60, 24);
    else
        glColor3ub(12, 70, 28);
    triangleShape(x - 16 * scale, y + 28 * scale, x + 16 * scale, y + 28 * scale, x, y + 48 * scale);
    triangleShape(x - 13 * scale, y + 41 * scale, x + 13 * scale, y + 41 * scale, x, y + 60 * scale);
    triangleShape(x - 9 * scale, y + 54 * scale, x + 9 * scale, y + 54 * scale, x, y + 72 * scale);
}

void drawTreePine(float x, float y, float scale)
{
    drawPineTreeTrunk(x, y, scale);
    drawPineLeaves(x, y, scale);
}

void leafTriangle(float cx, float cy, float size)
{
    triangleShape(cx - size, cy - size * 0.6f, cx + size, cy - size * 0.6f, cx, cy + size * 0.9f);
}

// Each of the 6 leaf triangles gets its own shade of the
// season's base foliage color instead of one flat color, using
// the same soft, upper-right light logic as the round tree.
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

void drawTreeBushy(float x, float y, float scale)
{
    drawBushyTreeTrunk(x, y, scale);
    drawBranches(x, y, scale);
    drawBushyTriangleLeaves(x, y, scale);
}

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

// IMPORTANT:
// Trees disappear at approximately -150.
// Because the world moves LEFT, they are recycled to +150.
//
// This gives extra forest outside the visible screen.
//
// Trees now scroll on their own independent offset
// (forestMove), separate from the road / other layers.
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

void drawForestBgGrass(float x, float y, float scale)
{
    if(currentSeason == WINTER)
        glColor3ub(125, 140, 155);
    else if(currentSeason == SUMMER)
        glColor3ub(120, 125, 35);
    else
        glColor3ub(34, 120, 34);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 6 * scale, y); glVertex2f(x - 2 * scale, y); glVertex2f(x - 5 * scale, y + 10 * scale);
    glVertex2f(x - 3 * scale, y); glVertex2f(x + 1 * scale, y); glVertex2f(x - 1 * scale, y + 14 * scale);
    glVertex2f(x, y);             glVertex2f(x + 4 * scale, y); glVertex2f(x + 2 * scale, y + 13 * scale);
    glVertex2f(x + 3 * scale, y); glVertex2f(x + 7 * scale, y); glVertex2f(x + 6 * scale, y + 9 * scale);
    glEnd();
}

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

void drawGrass(float x, float y, float scale)
{
    if(currentSeason == WINTER)
        return;
    if(currentSeason == SUMMER)
        glColor3ub(170, 150, 45);
    else
        glColor3ub(40, 158, 45);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 6 * scale, y); glVertex2f(x - 2 * scale, y); glVertex2f(x - 5 * scale, y + 10 * scale);
    glVertex2f(x - 3 * scale, y); glVertex2f(x + 1 * scale, y); glVertex2f(x - 1 * scale, y + 14 * scale);
    glVertex2f(x, y);             glVertex2f(x + 4 * scale, y); glVertex2f(x + 2 * scale, y + 13 * scale);
    glVertex2f(x + 3 * scale, y); glVertex2f(x + 7 * scale, y); glVertex2f(x + 6 * scale, y + 9 * scale);
    glEnd();
}

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

void drawSpringEnvironment()
{
    drawFlowers();
    drawButterflies();
}

// ======================================================
// ROAD
// ======================================================

void drawRoad()
{
    glColor3ub(55, 55, 55);
    rectangle(-100, -58, 100, -40);
    glColor3ub(255, 255, 255);
    for(float x = -150; x < 150; x += 30)
    {
        // The road stripes are the "reference" layer and still
        // use worldMove directly so they always match the car's
        // true travel distance.
        float lineX = x + worldMove;
        while(lineX > 150) lineX -= 300;
        while(lineX < -150) lineX += 300;
        if(lineX >= -110 && lineX <= 110)
            rectangle(lineX, -50, lineX + 10, -48);
    }
}

// ======================================================
// CAR
// ======================================================

void drawCar(float x, float y)
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
    glColor3ub(255, 250, 200);
    circle(x + 12.0f, y + 2.5f, 1.1f);
    glColor3ub(255, 70, 40);
    rectangle(x - 12.5f, y + 1.5f, x - 10.5f, y + 4.5f);
    glColor3ub(20, 20, 20);
    circle(x - 7, y, 3.2f);
    circle(x + 7, y, 3.2f);
    glColor3ub(220, 220, 220);
    glLineWidth(1.5f);
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

// ======================================================
// CAVE / TUNNEL
// ======================================================

void drawCave()
{
    if(!tunnelVisible)
        return;
    float x = movingTunnelX;
    glColor3ub(75, 75, 75);
    glBegin(GL_POLYGON);
    glVertex2f(x - 20, -40); glVertex2f(x - 17, -25); glVertex2f(x - 12, -13); glVertex2f(x - 5, -7);
    glVertex2f(x + 5, -7);   glVertex2f(x + 13, -13); glVertex2f(x + 18, -25); glVertex2f(x + 20, -40);
    glEnd();
    glColor3ub(5, 5, 5);
    glBegin(GL_POLYGON);
    glVertex2f(x - 9, -40); glVertex2f(x - 9, -25); glVertex2f(x - 6, -19);
    glVertex2f(x, -16);     glVertex2f(x + 6, -19); glVertex2f(x + 9, -25); glVertex2f(x + 9, -40);
    glEnd();
}

void drawTunnelDarkness()
{
    if(transitionStage != 3)
        return;
    glColor3ub(0, 0, 0);
    rectangle(-100, -60, 100, 100);
    glColor3ub(255, 255, 255);
    if(targetSeason == SPRING)
    {
        glRasterPos2f(-18, 5);
        char text[] = "SPRING";
        for(int i = 0; text[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
    else if(targetSeason == SUMMER)
    {
        glRasterPos2f(-18, 5);
        char text[] = "SUMMER";
        for(int i = 0; text[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
    else if(targetSeason == WINTER)
    {
        glRasterPos2f(-18, 5);
        char text[] = "WINTER";
        for(int i = 0; text[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
    else
    {
        glRasterPos2f(-22, 5);
        char text[] = "DEFAULT";
        for(int i = 0; text[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

// ======================================================
// WORLD & TRANSITION UPDATE
// ======================================================

// Advances every layer's own offset by worldSpeed times that
// layer's speed multiplier. Layers with a multiplier below 1.0
// scroll slower (read as "farther away"); above 1.0 scroll
// faster (read as "closer to the camera").
void updateWorld()
{
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

void updateSeasonTransition()
{
    if(!changingSeason)
        return;
    if(transitionStage == 1)
    {
        transitionCarX += transitionCarSpeed;
        if(transitionCarX >= movingTunnelX - 12)
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
// DISPLAY
// ======================================================

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
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
    drawRoad();
    drawRoadGrass();
    if(currentSeason == sedlife)
        {drawGrassField();}

    drawForestBg();
   // drawMountains();
    drawForest();
    if(currentSeason == SPRING)
        {drawSpringEnvironment();}
    if(currentSeason == WINTER)
        {drawSnow();}
    if(tunnelVisible && transitionStage != 3)
        {drawCave();}
    if(changingSeason)
        {drawCar(transitionCarX, carY);}
    else
        drawCar(carX, carY);
    drawTunnelDarkness();
    glutSwapBuffers();
}

// ======================================================
// UPDATE / ANIMATION
// ======================================================

void update(int value)
{
    if(!paused)
    {
        updateWorld();
        wheelRotation -= 15.0f;
        if(wheelRotation < 0)
            wheelRotation += 360.0f;
        if(changingSeason)
            updateSeasonTransition();
        updateClouds();
        if(currentSeason == SPRING)
            butterflyTime += 0.05f;
        if(currentSeason == WINTER)
            updateSnow();
    }
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ======================================================
// KEYBOARD
// ======================================================

void keyboard(unsigned char key, int x, int y)
{
    if(key == '0')
    {
        if(currentSeason != sedlife && !changingSeason)
        {
            targetSeason = sedlife;
            changingSeason = true;
            transitionStage = 1;
            tunnelVisible = true;
            tunnelOnRight = true;
            tunnelOnLeft = false;
            movingTunnelX = tunnelRightX;
            carInsideTunnel = false;
            transitionCarX = carX;
        }
    }
    else if(key == '1')
    {
        if(currentSeason != SPRING && !changingSeason)
        {
            targetSeason = SPRING;
            changingSeason = true;
            transitionStage = 1;
            tunnelVisible = true;
            tunnelOnRight = true;
            tunnelOnLeft = false;
            movingTunnelX = tunnelRightX;
            carInsideTunnel = false;
            transitionCarX = carX;
        }
    }
    else if(key == '2')
    {
        if(currentSeason != SUMMER && !changingSeason)
        {
            targetSeason = SUMMER;
            changingSeason = true;
            transitionStage = 1;
            tunnelVisible = true;
            tunnelOnRight = true;
            tunnelOnLeft = false;
            movingTunnelX = tunnelRightX;
            carInsideTunnel = false;
            transitionCarX = carX;
        }
    }
    else if(key == '5')
    {
        if(currentSeason != WINTER && !changingSeason)
        {
            targetSeason = WINTER;
            changingSeason = true;
            transitionStage = 1;
            tunnelVisible = true;
            tunnelOnRight = true;
            tunnelOnLeft = false;
            movingTunnelX = tunnelRightX;
            carInsideTunnel = false;
            transitionCarX = carX;
        }
    }
    else if(key == ' ')
    {
        paused = !paused;
    }
    else if(key == 27)
    {
        exit(0);
    }
    glutPostRedisplay();
}

// ======================================================
// INITIALIZATION
// ======================================================

void init()
{
    glClearColor(0.50f, 0.80f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-100, 100, -60, 100);
    initSnow();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ======================================================
// MAIN
// ======================================================

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
    glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}

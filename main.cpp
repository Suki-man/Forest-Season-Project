#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265

// ======================================================
// NEW CHANGES IN THIS VERSION
// ======================================================
// 1. More butterflies: 4 -> 8.
// 2. More spring flowers: 22 -> 34.
// 3. Flowers are kept off the road by keeping their bases above -40.
// 4. Only six small grass tufts are allowed on the road.
// 5. Road grass uses much smaller scale values.
// 6. Road markings and road grass follow worldMove, so they move left.
// 7. Car/cave/black-screen transition comments were expanded to make
//    the movement sequence explicit.
// 8. Trees now come in 3 different styles (Round, Pine, Bushy
//    Triangle) instead of one repeated shape, so the forest line
//    doesn't look identical tree after tree.
// 9. FIXED: Bushy Triangle tree canopy no longer has a hollow gap
//    in the middle - see the comment above drawBushyTriangleLeaves().
// 10. NEW: Winter mode added with snowfall, snowy mountains, bare branches,
//     snow-covered ground, no sun, and no green roadside grass.
// 11. Each tree type now has its OWN trunk function instead of
//     sharing one drawTreeTrunk(). Round tree ("apple tree" base,
//     no apples yet) uses an average trunk width, Pine tree uses a
//     wider trunk, and Bushy tree uses a thinner trunk. Pine leaf
//     colors were also darkened so pine reads as visually distinct
//     from the round and bushy trees.
// ======================================================

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

// ------------------------------------------------------
// TRAVEL / TUNNEL STATE
// ------------------------------------------------------

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

// Everything that belongs to the scrolling background
// (forest, road markings, grass tufts, spring flowers,
// butterflies) shares this single offset. The tunnel is
// deliberately kept independent of worldMove so it can
// travel on its own timeline during a season transition.
float worldMove = 0.0f;
float worldSpeed = 0.5f;

// Fixed car position during normal driving
float carX = 0.0f;
float carY = -48.0f;

// During the transition the car temporarily moves
float transitionCarX = 0.0f;
float transitionCarSpeed = 1.5f;

// Wheel rotation
float wheelRotation = 0.0f;

// Tunnel position
float tunnelRightX = 90.0f;
float tunnelLeftX = -90.0f;
float movingTunnelX = 90.0f;
float tunnelExitSpeed = 1.0f;

// Season text screen
float seasonScreenTimer = 0.0f;

// ======================================================
// TREE DATA
// ======================================================

const int TREE_COUNT = 28;
//x positon of trees
float treeX[TREE_COUNT] = {
    113,-98, -92, -78, -64, -50, -35, -18, 0,
    18, 35, 52, 72, 80, 92
    //-98, -92, -78, -64, -50, -35, -18, 0,//more trees
    //18, 35, 52, 72, 90, 92
};
// basically size of trees
float treeScale[TREE_COUNT] = {
    0.6,0.65, 0.85, 0.70, 1.00, 0.75, 0.90, 1.10,
    0.75, 0.95, 0.70, 1.05, 0.80, 1.05, 0.80

    //0.4,0.3,0.3,0.2,0.3,0.4,0.4,0.6,0.4,0.3,0.2,0.4,0.5,0.3//more small size trees
};


//y position of trees
float treeY[TREE_COUNT] = {
    -24,-20, -25, -20, -23, -21, -26, -22,
    -22, -23, -24, -21, -20, -24, -22

    //,-35, -45, -50, -63, -41, -46, -62,.//more tress y position
   // -62, -63, -64, -61, -50, -24, -52,
};

// which of the 3 tree styles each tree uses.
// 0 = Round Tree (circle leaves)
// 1 = Pine Tree (stacked triangle cone)
// 2 = Bushy Triangle Tree (triangle leaf clusters)
// Cycled 0,1,2,0,1,2... across the array so the tree line
// alternates styles instead of repeating the same shape.
int treeType[TREE_COUNT] = {
    2,0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1

    // pattern keeps repeating 0,1,2 for any additional
    // trees added later in treeX/treeY/treeScale
};

// ======================================================
// FOREST BG DATA (small triangle tufts placed along the
// tree line, used as background filler near the trees)
// ======================================================

const int FOREST_BG_COUNT = 32;

// x position of forest bg tufts
float forestBgX[FOREST_BG_COUNT] = {
    -95, -88, -80, -72, -64, -56, -48, -40,
    -32, -24, -16, -8, 0, 8, 16, 24,
    32, 40, 48, 56, 64, 72, 80, 88,
    95, -60, -20, 20, 60, 90,100,120
};

// scale (size) of forest bg tufts
float forestBgScale[FOREST_BG_COUNT] = {
    0.8, 1.0, 0.7, 0.9, 1.1, 0.8, 0.6, 1.0,
    0.9, 0.7, 1.0, 0.8, 0.9, 1.1, 0.7, 0.8,
    1.0, 0.6, 0.9, 0.8, 1.0, 0.7, 0.9, 0.8,
    1.1, 0.7, 0.9, 0.8, 1.0, 0.6,0.6,0.7
};

// ======================================================
// GRASS DATA (scattered across the plain green ground)
// ======================================================

const int GRASS_COUNT = 40;

// x position of grass tufts
float grassX[GRASS_COUNT] = {
    -97, -90, -83, -76, -69, -62, -55, -48,
    -41, -34, -27, -20, -13, -6, 1, 8,
    15, 22, 29, 36, 43, 50, 57, 64,
    71, 78, 85, 92, 99, -95,
    -70, -45, -15, 10, 35, 65, 88, -85,
    -30, 50
};

// y position of grass tufts (spread across the ground,
// between the sky/ground line and the bottom of the screen)
float grassY[GRASS_COUNT] = {
    -22, -30, -25, -35, -28, -40, -24, -33,
    -45, -27, -38, -23, -42, -31, -26, -48,
    -29, -36, -22, -44, -34, -25, -50, -30,
    -39, -23, -46, -28, -33, -55,
    -37, -52, -41, -22, -47, -31, -24, -43,
    -53, -35
};

// scale (size) of grass tufts
float grassScale[GRASS_COUNT] = {
    0.8, 1.0, 0.7, 0.9, 1.1, 0.8, 0.6, 1.0,
    0.9, 0.7, 1.0, 0.8, 0.9, 1.1, 0.7, 0.8,
    1.0, 0.6, 0.9, 0.8, 1.0, 0.7, 0.9, 0.8,
    1.1, 0.7, 0.9, 0.8, 1.0, 0.6,
    0.85, 0.95, 0.65, 1.05, 0.75, 0.9, 0.7, 1.0,
    0.6, 0.95
};

// ======================================================
// SPRING - FLOWER DATA (blooming flowers on the forest
// floor, only drawn during the SPRING season)
// ======================================================

// All flower Y positions stay above the road
// so flowers can never be drawn on the road surface.
const int FLOWER_COUNT = 34;

// x position of flowers
float flowerX[FLOWER_COUNT] = {
    -96, -90, -84, -77, -70, -63, -56, -49,
    -42, -35, -28, -21, -14, -7, 0, 7,
    14, 21, 28, 35, 42, 49, 56, 63,
    70, 77, 84, 91, -67, -31, 3, 38,
    67, 95
};

// y position of flowers (kept low, near the ground line
// and scattered a little deeper toward the front)
// Every flower base is >= -38. Since the road begins at -40,
// the flowers remain entirely on the green ground and never on the road.
float flowerY[FLOWER_COUNT] = {
    -24, -30, -27, -34, -23, -31, -29, -25,
    -32, -30, -22, -33, -26, -36, -30, -24,
    -32, -28, -34, -30, -21, -35, -27, -32,
    -25, -36, -29, -23, -31, -35, -26, -33,
    -28, -30
};

// scale (size) of flowers
float flowerScale[FLOWER_COUNT] = {
    0.65, 0.85, 0.70, 0.80, 0.95, 0.75, 0.90, 0.70,
    0.85, 0.75, 0.95, 0.80, 0.65, 0.90, 0.75, 0.85,
    0.70, 0.95, 0.80, 0.70, 0.90, 0.75, 0.85, 0.70,
    0.80, 0.65, 0.90, 0.75, 0.85, 0.70, 0.95, 0.75,
    0.80, 0.70
};

// petal color of each flower (0 = pink, 1 = purple,
// 2 = white, 3 = red-orange) - cycles through a few
// classic spring colors
int flowerColor[FLOWER_COUNT] = {
    0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3,
    0, 1
};

// ======================================================
// SPRING - BUTTERFLY DATA (small animated butterflies
// that drift and flap above the ground, SPRING only)
// ======================================================

const int BUTTERFLY_COUNT = 8;

// base (center) position each butterfly drifts around
float butterflyBaseX[BUTTERFLY_COUNT] = {
    -78, -45, -18, 8, 30, 52, 72, 92
};
float butterflyBaseY[BUTTERFLY_COUNT] = {
    8, -4, 6, -2, -6, 7, -1, 5
};

// phase offset so each butterfly flaps/drifts out of sync
float butterflyPhase[BUTTERFLY_COUNT] = {
    0.0f, 1.6f, 3.1f, 4.7f, 0.8f, 2.2f, 3.8f, 5.3f
};

// size of each butterfly
float butterflyScale[BUTTERFLY_COUNT] = {
    0.75f, 1.0f, 0.8f, 0.9f, 1.1f, 0.75f, 0.95f, 0.8f
};

// timer that drives butterfly flying + wing-flap animation
float butterflyTime = 0.0f;

// ======================================================
// CLOUD VARIABLES
// ======================================================

float cloud1X = -80;
float cloud2X = -10;
float cloud3X = 60;


// ======================================================
// WINTER SNOW DATA
// ======================================================
const int SNOW_COUNT = 140;
float snowX[SNOW_COUNT];
float snowY[SNOW_COUNT];
float snowSpeed[SNOW_COUNT];

void initSnow()
{
    for(int i = 0; i < SNOW_COUNT; i++)
    {
        snowX[i] = -100.0f + (float)(rand() % 201);
        snowY[i] = -60.0f + (float)(rand() % 161);
        snowSpeed[i] = 0.20f + (float)(rand() % 30) / 100.0f;
    }
}

// ======================================================
// BASIC RECTANGLE FUNCTION
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

// ======================================================
// BASIC CIRCLE FUNCTION
// ======================================================

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

// ======================================================
// BASIC TRIANGLE FUNCTION
// ======================================================

void triangleShape(float x1, float y1, float x2, float y2, float x3, float y3)
{
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}

// ======================================================
// SKY
// ======================================================


// ======================================================
// WINTER SNOWFALL
// ======================================================
void drawSnow()
{
    if(currentSeason != WINTER) return;

    glColor3ub(255, 255, 255);
    for(int i = 0; i < SNOW_COUNT; i++)
    {
        circle(snowX[i], snowY[i], 0.35f + (i % 4) * 0.16f);
    }
}

void updateSnow()
{
    if(currentSeason != WINTER) return;

    for(int i = 0; i < SNOW_COUNT; i++)
    {
        snowY[i] -= snowSpeed[i];
        snowX[i] -= 0.03f;

        if(snowY[i] < -60)
        {
            snowY[i] = 100;
            snowX[i] = -100.0f + (float)(rand() % 201);
        }

        if(snowX[i] < -105) snowX[i] = 105;
    }
}

// ======================================================
// WINTER SCENERY
// ======================================================
// ================================================================================================================================================================th
void drawWinterMountains()
{
    if(currentSeason != WINTER) return;

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
//so groupmade made a new set of pine trees for winter in the bg=======================================================================================================================
/*
void drawWinterPine(float x, float y, float scale)
{
    glColor3ub(70, 50, 35);
    rectangle(x - 1.4f * scale, y, x + 1.4f * scale, y + 25 * scale);

    glColor3ub(82, 98, 112);
    triangleShape(x - 13*scale, y + 8*scale, x + 13*scale, y + 8*scale, x, y + 28*scale);
    glColor3ub(94, 110, 125);
    triangleShape(x - 10*scale, y + 17*scale, x + 10*scale, y + 17*scale, x, y + 35*scale);
    glColor3ub(110, 125, 140);
    triangleShape(x - 7*scale, y + 25*scale, x + 7*scale, y + 25*scale, x, y + 41*scale);

    glColor3ub(245, 248, 252);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
        glVertex2f(x - 10*scale, y + 12*scale); glVertex2f(x + 10*scale, y + 12*scale);
        glVertex2f(x - 7.5f*scale, y + 21*scale); glVertex2f(x + 7.5f*scale, y + 21*scale);
        glVertex2f(x - 5*scale, y + 29*scale); glVertex2f(x + 5*scale, y + 29*scale);
    glEnd();
}
*/
void drawWinterBackground()
{
    if(currentSeason != WINTER) return;

    for(int i = -90; i <= 95; i += 18)
    {
        float x = i + worldMove * 0.35f;
        while(x > 105) x -= 220;
        while(x < -105) x += 220;
//        drawWinterPine(x, -20, 0.48f + (abs(i) % 3) * 0.05f);
    }
}
// so in winter groupmane made trees from the scratch===============================================================================================================================================================
void drawWinterBareTree(float x, float y, float scale, int type)
{
    glColor3ub(78, 49, 30);

    float h = (type == 1 ? 46.0f : 52.0f) * scale;
    float trunkW = (type == 1 ? 2.0f : 2.8f) * scale;

    rectangle(x - trunkW, y, x + trunkW, y + h);

    glLineWidth(3.5f * scale + 0.8f);
    glBegin(GL_LINES);

    glVertex2f(x, y + h*0.28f); glVertex2f(x - 18*scale, y + h*0.52f);
    glVertex2f(x, y + h*0.34f); glVertex2f(x + 20*scale, y + h*0.60f);
    glVertex2f(x, y + h*0.50f); glVertex2f(x - 15*scale, y + h*0.73f);
    glVertex2f(x, y + h*0.60f); glVertex2f(x + 14*scale, y + h*0.82f);
    glVertex2f(x, y + h*0.74f); glVertex2f(x - 10*scale, y + h*0.94f);
    glVertex2f(x, y + h*0.84f); glVertex2f(x + 9*scale, y + h*1.02f);

    glVertex2f(x - 18*scale, y + h*0.52f); glVertex2f(x - 25*scale, y + h*0.63f);
    glVertex2f(x - 18*scale, y + h*0.52f); glVertex2f(x - 22*scale, y + h*0.45f);
    glVertex2f(x + 20*scale, y + h*0.60f); glVertex2f(x + 28*scale, y + h*0.72f);
    glVertex2f(x + 20*scale, y + h*0.60f); glVertex2f(x + 25*scale, y + h*0.52f);
    glVertex2f(x - 15*scale, y + h*0.73f); glVertex2f(x - 21*scale, y + h*0.84f);
    glVertex2f(x + 14*scale, y + h*0.82f); glVertex2f(x + 19*scale, y + h*0.92f);
    glVertex2f(x - 10*scale, y + h*0.94f); glVertex2f(x - 15*scale, y + h*1.03f);
    glVertex2f(x + 9*scale, y + h*1.02f); glVertex2f(x + 13*scale, y + h*1.10f);
    glEnd();

    glColor3ub(248, 250, 253);
    glLineWidth(2.2f * scale + 0.6f);
    glBegin(GL_LINES);
        glVertex2f(x - 17*scale, y + h*0.55f); glVertex2f(x - 3*scale, y + h*0.38f);
        glVertex2f(x + 2*scale, y + h*0.40f); glVertex2f(x + 19*scale, y + h*0.62f);
        glVertex2f(x - 14*scale, y + h*0.76f); glVertex2f(x - 2*scale, y + h*0.54f);
        glVertex2f(x + 2*scale, y + h*0.65f); glVertex2f(x + 13*scale, y + h*0.84f);
    glEnd();
}

// ======================================================
// SKY
// ======================================================

void drawSky()
{
    if(currentSeason == SPRING)
    {
        glColor3ub(150, 217, 255);
    }
    else if(currentSeason == SUMMER)
    {
        glColor3ub(105, 195, 255);
    }
    else if(currentSeason == WINTER)
    {
        glColor3ub(180, 210, 235);
    }
    else
    {
        glColor3ub(128, 204, 255);
    }

    rectangle(-100, -20, 100, 100);
}

// ======================================================
// GROUND
// ======================================================

void drawGround()
{
    if(currentSeason == SPRING)
    {
        glColor3ub(72, 168, 58);
    }
    else if(currentSeason == SUMMER)
    {
        glColor3ub(190, 170, 75);
    }
    else if(currentSeason == WINTER)
    {
        glColor3ub(235, 240, 245);
    }
    else
    {
        glColor3ub(51, 140, 46);
    }

    rectangle(-100, -60, 100, -20);
}


// ======================================================
// SUN
// ======================================================

void drawSun()
{
    // No bright sun in winter
    if(currentSeason == WINTER) return;
    if(currentSeason == SUMMER)
    {
        // Hot orange-yellow sun for summer
        glColor3ub(255, 180, 20);
    }
    else
    {
        glColor3ub(255, 217, 26);
    }

    circle(75, 78, 9);
}

// ======================================================
// CLOUD
// ======================================================

void drawCloud(float x, float y, float size)
{
    // White cloud

    glColor3ub(242, 242, 242);

    // Left part

    circle(x - size * 0.8f, y, size * 0.7f);

    // Center part

    circle(x, y + 2, size);

    // Right part

    circle(x + size * 0.8f, y, size * 0.8f);

    // Bottom part

    rectangle(
        x - size * 0.8f,
        y - size * 0.5f,
        x + size * 0.8f,
        y + size * 0.35f
    );
}

// ======================================================
// ALL CLOUDS
// ======================================================

void drawClouds()
{
    drawCloud(cloud1X, 80, 7);
    drawCloud(cloud2X, 85, 8);
    drawCloud(cloud3X, 75, 6);
}

// ======================================================
// TREE TRUNKS
// Each tree type now has its own trunk function instead
// of sharing one drawTreeTrunk(), so trunk width can
// differ per style:
//   - Round tree ("apple tree" base): average trunk width
//   - Pine tree: wider trunk
//   - Bushy tree: thinner trunk
// ======================================================

// ROUND TREE TRUNK � average width (same proportions as
// the original shared trunk)
void drawRoundTreeTrunk(float x, float y, float scale)
{
    // Brown trunk

    glColor3ub(89, 46, 18);

    glBegin(GL_QUADS);

    // Bottom-left
    glVertex2f(x - 3 * scale, y);

    // Bottom-right
    glVertex2f(x + 3 * scale, y);

    // Top-right
    glVertex2f(x + 2 * scale, y + 50 * scale);

    // Top-left
    glVertex2f(x - 2 * scale, y + 50 * scale);

    glEnd();
}

// PINE TREE TRUNK � wider than the round tree's trunk,
// and taller so the whole pine tree stands higher
void drawPineTreeTrunk(float x, float y, float scale)
{
    // Brown trunk

    glColor3ub(89, 46, 18);

    glBegin(GL_QUADS);

    // Bottom-left
    glVertex2f(x - 5 * scale, y);

    // Bottom-right
    glVertex2f(x + 5 * scale, y);

    // Top-right
    glVertex2f(x + 3.5f * scale, y + 62 * scale);

    // Top-left
    glVertex2f(x - 3.5f * scale, y + 62 * scale);

    glEnd();
}

// BUSHY TREE TRUNK � thinner than the round tree's trunk
void drawBushyTreeTrunk(float x, float y, float scale)
{
    // Brown trunk

    glColor3ub(89, 46, 18);

    glBegin(GL_QUADS);

    // Bottom-left
    glVertex2f(x - 2 * scale, y);

    // Bottom-right
    glVertex2f(x + 2 * scale, y);

    // Top-right
    glVertex2f(x + 1.5f * scale, y + 50 * scale);

    // Top-left
    glVertex2f(x - 1.5f * scale, y + 50 * scale);

    glEnd();
}

// ======================================================
// TREE BRANCHES (used by the Round Tree and Bushy
// Triangle Tree; the Pine Tree does not use branches)
// ======================================================

void drawBranches(float x, float y, float scale)
{
    // Dark brown branches

    glColor3ub(77, 38, 13);

    glLineWidth(5.0f);

    glBegin(GL_LINES);

    // --------------------------------------------------
    // LEFT MAIN BRANCH
    // --------------------------------------------------

    glVertex2f(x, y + 15 * scale);
    glVertex2f(x - 20 * scale, y + 40 * scale);

    // --------------------------------------------------
    // RIGHT MAIN BRANCH
    // --------------------------------------------------

    glVertex2f(x, y + 18 * scale);
    glVertex2f(x + 20 * scale, y + 42 * scale);

    // --------------------------------------------------
    // CENTER BRANCH
    // --------------------------------------------------

    glVertex2f(x, y + 20 * scale);
    glVertex2f(x, y + 50 * scale);

    // --------------------------------------------------
    // SMALL LEFT BRANCH
    // --------------------------------------------------

    glVertex2f(x - 7 * scale, y + 27 * scale);
    glVertex2f(x - 25 * scale, y + 35 * scale);

    // --------------------------------------------------
    // SMALL RIGHT BRANCH
    // --------------------------------------------------

    glVertex2f(x + 7 * scale, y + 30 * scale);
    glVertex2f(x + 25 * scale, y + 38 * scale);

    glEnd();
}

// ======================================================
// TREE TYPE 1: ROUND TREE LEAVES (circle clusters)
// This is the "apple tree" base shape - no apples added
// yet, just the round leaf canopy for now.
// Color changes slightly for spring to give a fresh,
// bright-green blooming look.
// ======================================================

void drawTreeLeaves(float x, float y, float scale)
{
    // Bright fresh green in spring, normal forest green otherwise

if(currentSeason == SPRING)
{
    glColor3ub(76, 187, 23);
}
else if(currentSeason == SUMMER)
{
    // Slightly darker foliage in summer
    glColor3ub(8, 105, 15);
}
else
{
    glColor3ub(10, 122, 18);
}

    // --------------------------------------------------
    // LEFT FOLIAGE
    // --------------------------------------------------

    circle(x - 15 * scale, y + 48 * scale, 14 * scale);

    // --------------------------------------------------
    // TOP FOLIAGE
    // --------------------------------------------------

    circle(x, y + 60 * scale, 17 * scale);

    // --------------------------------------------------
    // RIGHT FOLIAGE
    // --------------------------------------------------

    circle(x + 15 * scale, y + 48 * scale, 14 * scale);

    // --------------------------------------------------
    // LOWER LEFT FOLIAGE
    // --------------------------------------------------

    circle(x - 7 * scale, y + 40 * scale, 12 * scale);

    // --------------------------------------------------
    // LOWER RIGHT FOLIAGE
    // --------------------------------------------------

    circle(x + 8 * scale, y + 40 * scale, 12 * scale);
}

// ======================================================
// COMPLETE TREE TYPE 1: ROUND TREE ("apple tree" base)
// average-width trunk + branches + circle leaves
// ======================================================

void drawTreeRound(float x, float y, float scale)
{
    drawRoundTreeTrunk(x, y, scale);
    drawBranches(x, y, scale);
    drawTreeLeaves(x, y, scale);
}

// ======================================================
// TREE TYPE 2: PINE TREE LEAVES (3 stacked triangles,
// classic conical fir/pine look, no branch lines)
// Colors pulled darker than the original version so the
// pine tree reads as visually distinct/deeper green
// compared to the round and bushy trees.
// ======================================================

void drawPineLeaves(float x, float y, float scale)
{
    // Deeper, darker green for pine, still a bit brighter in spring

    if(currentSeason == SPRING)
{
    glColor3ub(55, 130, 45);
}
else if(currentSeason == SUMMER)
{
    glColor3ub(10, 60, 24);
}
else
{
    glColor3ub(12, 70, 28);
}

    // --------------------------------------------------
    // BOTTOM LAYER (widest, lowest)
    // --------------------------------------------------

    triangleShape(
        x - 16 * scale, y + 28 * scale,
        x + 16 * scale, y + 28 * scale,
        x,              y + 48 * scale
    );

    // --------------------------------------------------
    // MIDDLE LAYER
    // --------------------------------------------------

    triangleShape(
        x - 13 * scale, y + 41 * scale,
        x + 13 * scale, y + 41 * scale,
        x,               y + 60 * scale
    );

    // --------------------------------------------------
    // TOP LAYER (narrowest, highest)
    // --------------------------------------------------

    triangleShape(
        x - 9 * scale, y + 54 * scale,
        x + 9 * scale, y + 54 * scale,
        x,              y + 72 * scale
    );
}

// ======================================================
// COMPLETE TREE TYPE 2: PINE TREE
// wider trunk + stacked triangle cone, no branches
// ======================================================

void drawTreePine(float x, float y, float scale)
{
    drawPineTreeTrunk(x, y, scale);
    drawPineLeaves(x, y, scale);
}

// ======================================================
// SMALL HELPER: one upward-pointing "leaf" triangle
// centered at (cx, cy) with the given half-size - used to
// build the Bushy Triangle Tree's angular leaf clusters.
// ======================================================

void leafTriangle(float cx, float cy, float size)
{
    triangleShape(
        cx - size, cy - size * 0.6f,
        cx + size, cy - size * 0.6f,
        cx,        cy + size * 0.9f
    );
}

// ======================================================
// TREE TYPE 3: BUSHY TRIANGLE TREE LEAVES
//
// FIX: the previous version placed 5 tall, narrow-apex
// triangles far apart (left/top/right/lower-left/lower-
// right). Because a triangle tapers to a single POINT at
// its apex, two clusters whose y-ranges overlapped often
// still had almost no actual overlapping AREA near the
// top - that's what showed up as a hollow gap in the
// middle of the canopy. The clusters also sat noticeably
// higher above the trunk than the other two tree types,
// which read as "floating"/oddly positioned foliage.
//
// FIX APPLIED:
//  - Pulled every cluster down so the canopy starts right
//    at the trunk top (~y+42-50*scale), matching the Round
//    Tree's proportions.
//  - Enlarged each triangle so neighboring clusters overlap
//    at their wide BASE instead of their narrow tip.
//  - Added a CENTER cluster between left/right/top that
//    plugs the hole that used to sit in the middle.
// ======================================================

void drawBushyTriangleLeaves(float x, float y, float scale)
{
    // Olive / yellow-green hue keeps this tree visually distinct
    // from the Round Tree's forest green.

   if(currentSeason == SPRING)
{
    glColor3ub(173, 209, 60);
}
else if(currentSeason == SUMMER)
{
    glColor3ub(95, 125, 30);
}
else
{
    glColor3ub(107, 142, 35);
}
    // --------------------------------------------------
    // LOWER LEFT CLUSTER (anchors the canopy to the trunk)
    // --------------------------------------------------

    leafTriangle(x - 9 * scale, y + 42 * scale, 15 * scale);

    // --------------------------------------------------
    // LOWER RIGHT CLUSTER
    // --------------------------------------------------

    leafTriangle(x + 9 * scale, y + 42 * scale, 15 * scale);

    // --------------------------------------------------
    // CENTER CLUSTER - bridges left/right/top and removes
    // the hollow that used to appear in the middle.
    // --------------------------------------------------

    leafTriangle(x, y + 48 * scale, 16 * scale);

    // --------------------------------------------------
    // LEFT CLUSTER
    // --------------------------------------------------

    leafTriangle(x - 15 * scale, y + 50 * scale, 15 * scale);

    // --------------------------------------------------
    // RIGHT CLUSTER
    // --------------------------------------------------

    leafTriangle(x + 15 * scale, y + 50 * scale, 15 * scale);

    // --------------------------------------------------
    // TOP CLUSTER
    // --------------------------------------------------

    leafTriangle(x, y + 62 * scale, 18 * scale);
}

// ======================================================
// COMPLETE TREE TYPE 3: BUSHY TRIANGLE TREE
// thinner trunk + branches + triangle leaf clusters
// ======================================================

void drawTreeBushy(float x, float y, float scale)
{
    drawBushyTreeTrunk(x, y, scale);
    drawBranches(x, y, scale);
    drawBushyTriangleLeaves(x, y, scale);
}

// ======================================================
// TREE DISPATCHER
// Picks which of the 3 tree styles to draw based on
// treeType[i]. This is what drawForest() calls now.
// ======================================================

void drawTree(float x, float y, float scale, int type)
{
    if(currentSeason == WINTER)
    {
        drawWinterBareTree(x, y, scale, type);
        return;
    }

    if(type == 0)
    {
        drawTreeRound(x, y, scale);
    }
    else if(type == 1)
    {
        drawTreePine(x, y, scale);
    }
    else
    {
        drawTreeBushy(x, y, scale);
    }
}

// ======================================================
// COMPLETE FOREST (now passes each tree's type so the
// line of trees alternates between the 3 styles)
// ======================================================

void drawForest()
{
    for(int i = 0; i < TREE_COUNT; i++)
    {
        float x = treeX[i] + worldMove;

        // Wrap the tree around the screen
        if(x > 110)
            x -= 220;

        if(x < -110)
            x += 220;

        drawTree(x, treeY[i], treeScale[i], treeType[i]);
    }
}

// ======================================================
// SINGLE FOREST BG TUFT (4 TRIANGLES, STRAIGHT BOTTOM)
// ======================================================

void drawForestBgGrass(float x, float y, float scale)
{
    // Winter background shrubs are cold gray-blue, never green.
    if(currentSeason == WINTER)
    {
        glColor3ub(125, 140, 155);
    }
    else    if(currentSeason == SUMMER)
{
    glColor3ub(120, 125, 35);
}
else
{
    glColor3ub(34, 120, 34);
}

    glBegin(GL_TRIANGLES);

    // --------------------------------------------------
    // BLADE 1 (far left, leaning left)
    // --------------------------------------------------

    glVertex2f(x - 6 * scale, y);
    glVertex2f(x - 2 * scale, y);
    glVertex2f(x - 5 * scale, y + 10 * scale);

    // --------------------------------------------------
    // BLADE 2 (left-center, leaning slightly left)
    // --------------------------------------------------

    glVertex2f(x - 3 * scale, y);
    glVertex2f(x + 1 * scale, y);
    glVertex2f(x - 1 * scale, y + 14 * scale);

    // --------------------------------------------------
    // BLADE 3 (right-center, leaning slightly right)
    // --------------------------------------------------

    glVertex2f(x, y);
    glVertex2f(x + 4 * scale, y);
    glVertex2f(x + 2 * scale, y + 13 * scale);

    // --------------------------------------------------
    // BLADE 4 (far right, leaning right)
    // --------------------------------------------------

    glVertex2f(x + 3 * scale, y);
    glVertex2f(x + 7 * scale, y);
    glVertex2f(x + 6 * scale, y + 9 * scale);

    glEnd();
}

// ======================================================
// COMPLETE FOREST BG
// ======================================================

void drawForestBg()
{
    for(int i = 0; i < FOREST_BG_COUNT; i++)
    {
        float x = forestBgX[i] + worldMove;

        if(x > 110)
            x -= 220;

        if(x < -110)
            x += 220;

        drawForestBgGrass(
            x,
            -20,
            forestBgScale[i]
        );
    }
}

// ======================================================
// SINGLE GRASS TUFT ON THE GROUND (4 TRIANGLES,
// STRAIGHT BOTTOM) - scattered on the plain green ground
// ======================================================

void drawGrass(float x, float y, float scale)
{
    // Grass green (slightly different shade from forest bg)

   if(currentSeason == SUMMER)
{
    // Dry yellow-green grass in summer
    glColor3ub(170, 150, 45);
}
else
{
    glColor3ub(40, 158, 45);
}

    glBegin(GL_TRIANGLES);

    // --------------------------------------------------
    // BLADE 1 (far left, leaning left)
    // --------------------------------------------------

    glVertex2f(x - 6 * scale, y);
    glVertex2f(x - 2 * scale, y);
    glVertex2f(x - 5 * scale, y + 10 * scale);

    // --------------------------------------------------
    // BLADE 2 (left-center, leaning slightly left)
    // --------------------------------------------------

    glVertex2f(x - 3 * scale, y);
    glVertex2f(x + 1 * scale, y);
    glVertex2f(x - 1 * scale, y + 14 * scale);

    // --------------------------------------------------
    // BLADE 3 (right-center, leaning slightly right)
    // --------------------------------------------------

    glVertex2f(x, y);
    glVertex2f(x + 4 * scale, y);
    glVertex2f(x + 2 * scale, y + 13 * scale);

    // --------------------------------------------------
    // BLADE 4 (far right, leaning right)
    // --------------------------------------------------

    glVertex2f(x + 3 * scale, y);
    glVertex2f(x + 7 * scale, y);
    glVertex2f(x + 6 * scale, y + 9 * scale);

    glEnd();
}

// ======================================================
// COMPLETE GRASS FIELD (scattered over the ground)
// ======================================================

void drawGrassField()
{
    for(int i = 0; i < GRASS_COUNT; i++)
    {
        float x = grassX[i] + worldMove;

        if(x > 110)
            x -= 220;

        if(x < -110)
            x += 220;

        drawGrass(x, grassY[i], grassScale[i]);
    }
}

// ======================================================
// SINGLE FLOWER (stem + center + 5 petals)
// ======================================================

void drawFlower(float x, float y, float scale, int colorType)
{
    // --------------------------------------------------
    // STEM
    // --------------------------------------------------

    glColor3ub(46, 125, 50);

    glLineWidth(2.0f);

    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x, y + 8 * scale);
    glEnd();

    // --------------------------------------------------
    // PETALS (5 small circles arranged around the center)
    // --------------------------------------------------

    switch(colorType)
    {
        case 0: glColor3ub(255, 128, 171); break; // pink
        case 1: glColor3ub(186, 104, 200); break; // purple
        case 2: glColor3ub(255, 255, 255); break; // white
        default: glColor3ub(255, 111, 74); break; // red-orange
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

    // --------------------------------------------------
    // FLOWER CENTER
    // --------------------------------------------------

    glColor3ub(255, 235, 59);

    circle(x, centerY, 1.3f * scale);
}

// ======================================================
// COMPLETE FLOWER BED (spring only)
// ======================================================

void drawFlowers()
{
    for(int i = 0; i < FLOWER_COUNT; i++)
    {
        float x = flowerX[i] + worldMove;

        if(x > 110)
            x -= 220;

        if(x < -110)
            x += 220;

        drawFlower(x, flowerY[i], flowerScale[i], flowerColor[i]);
    }
}

// ======================================================
// SINGLE BUTTERFLY (two flapping wings + small body)
// ======================================================

void drawButterfly(float x, float y, float scale, float flap)
{
    float wingSpread = (0.5f + 0.5f * flap) * scale;

    // --------------------------------------------------
    // LEFT WING
    // --------------------------------------------------

    glColor3ub(255, 179, 71);

    circle(x - 2.0f * scale, y + 0.5f * scale, wingSpread);

    // --------------------------------------------------
    // RIGHT WING
    // --------------------------------------------------

    glColor3ub(255, 202, 58);

    circle(x + 2.0f * scale, y + 0.5f * scale, wingSpread);

    // --------------------------------------------------
    // BODY
    // --------------------------------------------------

    glColor3ub(66, 40, 14);

    glLineWidth(2.0f);

    glBegin(GL_LINES);
    glVertex2f(x, y - 1.5f * scale);
    glVertex2f(x, y + 2.5f * scale);
    glEnd();
}

// ======================================================
// COMPLETE BUTTERFLIES (spring only, animated)
// ======================================================

void drawButterflies()
{
    for(int i = 0; i < BUTTERFLY_COUNT; i++)
    {
        float baseX = butterflyBaseX[i] + worldMove;

        if(baseX > 110)
            baseX -= 220;

        if(baseX < -110)
            baseX += 220;

        // Gentle drifting flight path around the base position

        float x = baseX + 14.0f * sin(butterflyTime * 0.6f + butterflyPhase[i]);
        float y = butterflyBaseY[i] + 5.0f * sin(butterflyTime * 1.3f + butterflyPhase[i]);

        // Wings flap faster than the body drifts

        float flap = 0.5f + 0.5f * fabs((float)sin(butterflyTime * 6.0f + butterflyPhase[i]));

        drawButterfly(x, y, butterflyScale[i], flap);
    }
}

// ======================================================
// COMPLETE SPRING ENVIRONMENT (flowers + butterflies)
// ======================================================

void drawSpringEnvironment()
{
    drawFlowers();
    drawButterflies();
}

// ======================================================
// CLOUD ANIMATION
// ======================================================

void updateClouds()
{
    // Clouds move normally except
    // during the rainy season.

    if(currentSeason != RAINY)
    {
        cloud1X += 0.08f;
        cloud2X += 0.05f;
        cloud3X += 0.06f;

        // Reset cloud when it leaves
        // the right side.

        if(cloud1X > 115)
        {
            cloud1X = -115;
        }

        if(cloud2X > 115)
        {
            cloud2X = -115;
        }

        if(cloud3X > 115)
        {
            cloud3X = -115;
        }
    }
}


// ======================================================
// SMALL ROAD-SIDE GRASS
// ======================================================

const int ROAD_GRASS_COUNT = 6;

float roadGrassX[ROAD_GRASS_COUNT] = {
    -78, -42, -8, 24, 58, 88
};

float roadGrassY[ROAD_GRASS_COUNT] = {
    -54, -46, -52, -44, -55, -48
};

float roadGrassScale[ROAD_GRASS_COUNT] = {
    0.28f, 0.22f, 0.30f, 0.24f, 0.27f, 0.23f
};

void drawRoadGrass()
{
    if(currentSeason == WINTER) return;

    // Draw only a few small tufts on the road.
    for(int i = 0; i < ROAD_GRASS_COUNT; i++)
    {
        float x = roadGrassX[i] + worldMove;

        if(x > 110)
            x -= 220;

        if(x < -110)
            x += 220;

        drawGrass(x, roadGrassY[i], roadGrassScale[i]);
    }
}

// ======================================================
// ROAD
// ======================================================

void drawRoad()
{
    // ROAD: the asphalt itself stays fixed; its markings and the
    // small road grass move left using worldMove to create forward travel.
    glColor3ub(55, 55, 55);
    rectangle(-100, -58, 100, -40);

    // Road divider marks move from right to left
    glColor3ub(255, 255, 255);

    for(float x = -100; x < 120; x += 20)
    {
        float lineX = x + worldMove;

        if(lineX > 110)
            lineX -= 220;

        if(lineX < -110)
            lineX += 220;

        rectangle(lineX, -50, lineX + 10, -48);
    }
}

// ======================================================
// CAR
// ======================================================

void drawCar(float x, float y)
{
    // Main body (chassis)
    glColor3ub(200, 30, 30);
    rectangle(x - 11, y, x + 11, y + 7);

    // --------------------------------------------------
    // CABIN / ROOF
    // --------------------------------------------------

    glColor3ub(180, 20, 20);

    glBegin(GL_QUADS);

    glVertex2f(x - 6, y + 7);   // rear-bottom  (back of car)
    glVertex2f(x - 5, y + 13);  // rear-top     (upright rear window)
    glVertex2f(x + 2, y + 13);  // front-top    (roof ends before windshield)
    glVertex2f(x + 8, y + 7);   // front-bottom (windshield meets the hood)

    glEnd();

    // --------------------------------------------------
    // HOOD (front, right side) - slopes down to the bumper
    // --------------------------------------------------

    glColor3ub(200, 30, 30);

    glBegin(GL_QUADS);

    glVertex2f(x + 8, y + 7);
    glVertex2f(x + 11, y + 7);
    glVertex2f(x + 13, y + 3);
    glVertex2f(x + 11, y + 2);

    glEnd();

    // --------------------------------------------------
    // WINDOWS
    // --------------------------------------------------

    glColor3ub(120, 200, 230);

    rectangle(x - 4.5f, y + 7.5f,
              x + 4.5f, y + 11.0f);

    glColor3ub(180, 20, 20);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    glVertex2f(x, y + 7.5f);
    glVertex2f(x, y + 11.0f);
    glEnd();

    // --------------------------------------------------
    // HEADLIGHT (front / right) and TAILLIGHT (back / left)
    // --------------------------------------------------

    glColor3ub(255, 250, 200);
    circle(x + 12.0f, y + 2.5f, 1.1f);

    glColor3ub(255, 70, 40);
    rectangle(x - 12.5f, y + 1.5f, x - 10.5f, y + 4.5f);

    // Wheels
    glColor3ub(20, 20, 20);

    circle(x - 7, y, 3.2f);
    circle(x + 7, y, 3.2f);

    // Rotating wheel spokes
    glColor3ub(220, 220, 220);
    glLineWidth(1.5f);

    float angle = wheelRotation * PI / 180.0f;

    for(int i = 0; i < 4; i++)
    {
        float a = angle + i * PI / 2.0f;

        glBegin(GL_LINES);

        glVertex2f(x - 7, y);
        glVertex2f(x - 7 + 2.3f * cos(a),
                   y + 2.3f * sin(a));

        glVertex2f(x + 7, y);
        glVertex2f(x + 7 + 2.3f * cos(a),
                   y + 2.3f * sin(a));

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

    float x;

    x = movingTunnelX;

    // --------------------------------------------------
    // SIDE VIEW OF THE TUNNEL / ROCK
    // --------------------------------------------------

    glColor3ub(75, 75, 75);

    glBegin(GL_POLYGON);

    glVertex2f(x - 20, -40);
    glVertex2f(x - 17, -25);
    glVertex2f(x - 12, -13);
    glVertex2f(x - 5, -7);
    glVertex2f(x + 5, -7);
    glVertex2f(x + 13, -13);
    glVertex2f(x + 18, -25);
    glVertex2f(x + 20, -40);

    glEnd();

    // Black opening
    glColor3ub(5, 5, 5);

    glBegin(GL_POLYGON);

    glVertex2f(x - 9, -40);
    glVertex2f(x - 9, -25);
    glVertex2f(x - 6, -19);
    glVertex2f(x, -16);
    glVertex2f(x + 6, -19);
    glVertex2f(x + 9, -25);
    glVertex2f(x + 9, -40);

    glEnd();
}

// ======================================================
// TUNNEL DARKNESS
// ======================================================

void drawTunnelDarkness()
{
    if(transitionStage == 3)
    {
        // Completely black screen
        glColor3ub(0, 0, 0);
        rectangle(-100, -60, 100, 100);

        // Season name
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
}

// ======================================================
// MOVING WORLD
// ======================================================

void updateWorld()
{
    if(transitionStage == 0)
    {
        worldMove -= worldSpeed;

        if(worldMove < -220)
            worldMove += 220;
    }
}

// ======================================================
// SEASON TRANSITION
// ======================================================

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

            worldMove = 0.0f;
        }
    }

    else if(transitionStage == 5)
    {
        worldMove -= worldSpeed;

        if(worldMove < -220)
            worldMove += 220;

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
// MAIN DISPLAY
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

    drawGround();
    drawRoad();
    drawRoadGrass();

    if(currentSeason == sedlife)
    {
        drawGrassField();
    }

    drawForestBg();
    drawForest();

    if(currentSeason == SPRING)
    {
        drawSpringEnvironment();
    }

    if(currentSeason == WINTER)
    {
        drawSnow();
    }

    if(tunnelVisible && transitionStage != 3)
    {
        drawCave();
    }

    if(changingSeason)
    {
        drawCar(transitionCarX, carY);
    }
    else
    {
        drawCar(carX, carY);
    }

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
        {
            updateSeasonTransition();
        }

        updateClouds();

        if(currentSeason == SPRING)
        {
            butterflyTime += 0.05f;
        }

        if(currentSeason == WINTER)
        {
            updateSnow();
        }
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
//tesdtiojngj famdfkakdmamdmakdm kam d

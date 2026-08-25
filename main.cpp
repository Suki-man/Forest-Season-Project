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
// 8. NEW: trees now come in 3 different styles (Round, Pine, Bushy
//    Triangle) instead of one repeated shape, so the forest line
//    doesn't look identical tree after tree.
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
    -98, -92, -78, -64, -50, -35, -18, 0,
    18, 35, 52, 72, 80, 92,
    //-98, -92, -78, -64, -50, -35, -18, 0,//more trees
    //18, 35, 52, 72, 90, 92
};
// basically size of trees
float treeScale[TREE_COUNT] = {
    0.65, 0.85, 0.70, 1.00, 0.75, 0.90, 1.10,
    0.75, 0.95, 0.70, 1.05, 0.80, 1.05, 0.80,

    //0.4,0.3,0.3,0.2,0.3,0.4,0.4,0.6,0.4,0.3,0.2,0.4,0.5,0.3//more small size trees
};


//y position of trees
float treeY[TREE_COUNT] = {
    -20, -25, -20, -23, -21, -26, -22,
    -22, -23, -24, -21, -20, -24, -22

    //,-35, -45, -50, -63, -41, -46, -62,.//more tress y position
   // -62, -63, -64, -61, -50, -24, -52,
};

// NEW: which of the 3 tree styles each tree uses.
// 0 = Round Tree (circle leaves)
// 1 = Pine Tree (stacked triangle cone)
// 2 = Bushy Triangle Tree (triangle leaf clusters)
// Cycled 0,1,2,0,1,2... across the array so the tree line
// alternates styles instead of repeating the same shape.
int treeType[TREE_COUNT] = {
    0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1

    // pattern keeps repeating 0,1,2 for any additional
    // trees added later in treeX/treeY/treeScale
};

// ======================================================
// FOREST BG DATA (small triangle tufts placed along the
// tree line, used as background filler near the trees)
// ======================================================

const int FOREST_BG_COUNT = 30;

// x position of forest bg tufts
float forestBgX[FOREST_BG_COUNT] = {
    -95, -88, -80, -72, -64, -56, -48, -40,
    -32, -24, -16, -8, 0, 8, 16, 24,
    32, 40, 48, 56, 64, 72, 80, 88,
    95, -60, -20, 20, 60, 90
};

// scale (size) of forest bg tufts
float forestBgScale[FOREST_BG_COUNT] = {
    0.8, 1.0, 0.7, 0.9, 1.1, 0.8, 0.6, 1.0,
    0.9, 0.7, 1.0, 0.8, 0.9, 1.1, 0.7, 0.8,
    1.0, 0.6, 0.9, 0.8, 1.0, 0.7, 0.9, 0.8,
    1.1, 0.7, 0.9, 0.8, 1.0, 0.6
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

// NEW: increased spring flower count. All flower Y positions stay above the road
// so flowers can never be drawn on the road surface.
const int FLOWER_COUNT = 34;

// x position of flowers
// NEW: more flower positions, distributed across the grass/forest floor.
float flowerX[FLOWER_COUNT] = {
    -96, -90, -84, -77, -70, -63, -56, -49,
    -42, -35, -28, -21, -14, -7, 0, 7,
    14, 21, 28, 35, 42, 49, 56, 63,
    70, 77, 84, 91, -67, -31, 3, 38,
    67, 95
};

// y position of flowers (kept low, near the ground line
// and scattered a little deeper toward the front)
// NEW: every flower base is >= -38. Since the road begins at -40,
// the flowers remain entirely on the green ground and never on the road.
float flowerY[FLOWER_COUNT] = {
    -24, -30, -27, -34, -23, -31, -29, -25,
    -32, -30, -22, -33, -26, -36, -30, -24,
    -32, -28, -34, -30, -21, -35, -27, -32,
    -25, -36, -29, -23, -31, -35, -26, -33,
    -28, -30
};

// scale (size) of flowers
// NEW: varied sizes keep the larger flower bed from looking repetitive.
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
// NEW: continue cycling through the existing spring flower colors.
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

// NEW: doubled the number of butterflies for a fuller spring scene.
const int BUTTERFLY_COUNT = 8;

// base (center) position each butterfly drifts around
// NEW: four additional butterflies with different starting positions.
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
// NEW: simple 3-point triangle helper, same style as
// rectangle()/circle() above, used by the new tree types.

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

void drawSky()
{
    // Sky blue (slightly fresher/brighter for spring)

    if(currentSeason == SPRING)
    {
        glColor3ub(150, 217, 255);
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
    // Green grass (brighter fresh green for spring)

    if(currentSeason == SPRING)
    {
        glColor3ub(72, 168, 58);
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
    // Yellow sun

    glColor3ub(255, 217, 26);

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
// TREE TRUNK (shared by all 3 tree types)
// ======================================================
//basically were setting a default tree structure , pore icha moto place e amra tree ke positioning koraitesi
//tree er position amra line 28 thika korsi, eikhane just tree fucntion ta add korsi, reason is (different season e specific element edit korbo just)

void drawTreeTrunk(float x, float y, float scale)
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
// COMPLETE TREE TYPE 1: ROUND TREE
// (trunk + branches + circle leaves - the original tree)
// ======================================================

void drawTreeRound(float x, float y, float scale)
{
    drawTreeTrunk(x, y, scale);
    drawBranches(x, y, scale);
    drawTreeLeaves(x, y, scale);
}

// ======================================================
// TREE TYPE 2: PINE TREE LEAVES (3 stacked triangles,
// classic conical fir/pine look, no branch lines)
// ======================================================

void drawPineLeaves(float x, float y, float scale)
{
    // Deeper, slightly cooler green for pine, brighter in spring

    if(currentSeason == SPRING)
    {
        glColor3ub(85, 170, 60);
    }
    else
    {
        glColor3ub(20, 100, 40);
    }

    // --------------------------------------------------
    // BOTTOM LAYER (widest, lowest)
    // --------------------------------------------------

    triangleShape(
        x - 16 * scale, y + 20 * scale,
        x + 16 * scale, y + 20 * scale,
        x,              y + 38 * scale
    );

    // --------------------------------------------------
    // MIDDLE LAYER
    // --------------------------------------------------

    triangleShape(
        x - 13 * scale, y + 32 * scale,
        x + 13 * scale, y + 32 * scale,
        x,               y + 48 * scale
    );

    // --------------------------------------------------
    // TOP LAYER (narrowest, highest)
    // --------------------------------------------------

    triangleShape(
        x - 9 * scale, y + 43 * scale,
        x + 9 * scale, y + 43 * scale,
        x,              y + 58 * scale
    );
}

// ======================================================
// COMPLETE TREE TYPE 2: PINE TREE
// (trunk + stacked triangle cone, no branches)
// ======================================================

void drawTreePine(float x, float y, float scale)
{
    drawTreeTrunk(x, y, scale);
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
// Same 5-cluster layout idea as the Round Tree, but built
// from angular triangles instead of circles.
// ======================================================

void drawBushyTriangleLeaves(float x, float y, float scale)
{
    if(currentSeason == SPRING)
    {
        glColor3ub(96, 200, 40);
    }
    else
    {
        glColor3ub(15, 130, 20);
    }

    // --------------------------------------------------
    // LEFT CLUSTER
    // --------------------------------------------------

    leafTriangle(x - 15 * scale, y + 48 * scale, 14 * scale);

    // --------------------------------------------------
    // TOP CLUSTER
    // --------------------------------------------------

    leafTriangle(x, y + 60 * scale, 17 * scale);

    // --------------------------------------------------
    // RIGHT CLUSTER
    // --------------------------------------------------

    leafTriangle(x + 15 * scale, y + 48 * scale, 14 * scale);

    // --------------------------------------------------
    // LOWER LEFT CLUSTER
    // --------------------------------------------------

    leafTriangle(x - 7 * scale, y + 40 * scale, 12 * scale);

    // --------------------------------------------------
    // LOWER RIGHT CLUSTER
    // --------------------------------------------------

    leafTriangle(x + 8 * scale, y + 40 * scale, 12 * scale);
}

// ======================================================
// COMPLETE TREE TYPE 3: BUSHY TRIANGLE TREE
// (trunk + branches + triangle leaf clusters)
// ======================================================

void drawTreeBushy(float x, float y, float scale)
{
    drawTreeTrunk(x, y, scale);
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
// same idea as the tree, scale controls size, base (bottom)
// of every triangle is a flat horizontal line sitting on the ground

void drawForestBgGrass(float x, float y, float scale)
{
    // Grass green

    glColor3ub(34, 120, 34);

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

    glColor3ub(40, 158, 45);

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
// This is the DEFAULT-season ground detail. It now scrolls
// together with the forest/road using the shared worldMove
// offset instead of its own separate movement variable, and
// wraps around the screen the same way trees do.
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
// scale controls overall size, colorType picks the petal
// color so the field doesn't look like one repeated flower

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
// Now scrolls with worldMove, exactly like the forest and
// the default-season grass field, and wraps the same way.
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
// x, y = current position (already animated),
// scale = size, flap = 0..1 wing-openness value

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
// Base position now scrolls with worldMove too, so the
// butterflies stay anchored to the moving landscape instead
// of drifting independently of the trees/flowers, and wraps
// around the screen just like everything else.
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
// Both pieces share the same worldMove offset as the
// forest, so the whole spring scene scrolls together.
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
// NEW: SMALL ROAD-SIDE GRASS
// ======================================================
// A few tiny grass tufts are allowed on the road, but they are
// intentionally much smaller than the normal field grass.
// They use worldMove so they travel with the road markings.

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
    // NEW: draw only a few small tufts on the road.
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
    // NEW: the car now has a clear FRONT (right side - hood,
    // bumper, headlight) and BACK (left side - flat trunk,
    // taillight). The world scrolls left, which means the car
    // is effectively travelling to the RIGHT, so the nose must
    // point right - that's what was missing before and made the
    // car look backward/direction-less.

    // Main body (chassis)
    glColor3ub(200, 30, 30);
    rectangle(x - 11, y, x + 11, y + 7);

    // --------------------------------------------------
    // CABIN / ROOF
    // Rear window (left) stands nearly upright - that's the
    // back of the car. Windshield (right) leans forward into
    // the hood - that's the front, in the direction of travel.
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
    // WINDOWS (with a center divider so front/back windows
    // read separately, matching the front/back roofline)
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
    // These make the direction unmistakable at a glance.
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
        // The car stays fixed while the world moves left, creating
        // the visual effect that the car is driving forward.
        worldMove -= worldSpeed;

        if(worldMove < -220)
            worldMove += 220;

        // Forest, road, grass field and spring environment
        // all read worldMove directly inside their own draw
        // functions now, so nothing extra needs to be synced
        // here anymore.
    }
}

// ======================================================
// SEASON TRANSITION
// ======================================================

void updateSeasonTransition()
{
    if(!changingSeason)
        return;

    // CAR + CAVE MOVEMENT:
    // 1. Tunnel is on the right, background is stopped, and the car
    //    moves right toward the cave entrance.
    if(transitionStage == 1)
    {
        transitionCarX += transitionCarSpeed;

        if(transitionCarX >= movingTunnelX - 12)
            transitionStage = 2;
    }

    // 2. Car reaches the cave entrance and enters it.
    //    The cave disappears behind the black transition screen.
    else if(transitionStage == 2)
    {
        carInsideTunnel = true;
        transitionStage = 3;
        seasonScreenTimer = 0.0f;
    }

    // 3. BLACK SCREEN:
    //    The entire window is intentionally covered in black while
    //    the season changes. The season name is drawn above it.
    else if(transitionStage == 3)
    {
        // NEW: this timer controls the short black season card.
        // At a 30 ms update interval, 1.0f is roughly 0.6 seconds.
        seasonScreenTimer += 0.05f;

        if(seasonScreenTimer >= 1.0f)
        {
            currentSeason = targetSeason;

            carInsideTunnel = false;

            // Put the tunnel at the far left.
            tunnelOnRight = false;
            tunnelOnLeft = true;
            movingTunnelX = -90.0f;

            // Car begins inside it.
            transitionCarX = movingTunnelX + 12.0f;

            transitionStage = 4;
        }
    }

    // 4. Car comes out of the LEFT cave after the black screen.
    //    The background is still stopped until the car reaches center.
    else if(transitionStage == 4)
    {
        transitionCarX += transitionCarSpeed;

        if(transitionCarX >= 0.0f)
        {
            transitionCarX = carX;

            // Car reached center:
            // NOW the whole forest/background starts moving.
            transitionStage = 5;

            worldMove = 0.0f;
        }
    }

    // 5. NORMAL TRAVEL RESUMES:
    //    Forest + road markings + grass + flowers + butterflies
    //    move together through worldMove. The car stays centered.
    //    The cave independently continues moving left and exits.
    else if(transitionStage == 5)
    {
        worldMove -= worldSpeed;

        if(worldMove < -220)
            worldMove += 220;

        movingTunnelX -= tunnelExitSpeed;

        // Car stays fixed at center.
        transitionCarX = carX;

        // Tunnel leaves the window and does not wrap.
        if(movingTunnelX < -120.0f)
            transitionStage = 6;
    }

    // 6. Tunnel is completely outside.
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

    // --------------------------------------------------
    // SKY
    // --------------------------------------------------

    drawSky();

    // --------------------------------------------------
    // SUN
    // --------------------------------------------------

    drawSun();

    // --------------------------------------------------
    // CLOUDS
    // --------------------------------------------------

    drawClouds();

    // --------------------------------------------------
    // GROUND
    // --------------------------------------------------

    drawGround();

    // --------------------------------------------------
    // ROAD
    // --------------------------------------------------

    drawRoad();

    // NEW: only tiny grass is drawn on the road.
    // Flowers are deliberately NOT drawn here.
    drawRoadGrass();

    // --------------------------------------------------
    // GROUND DETAILS (season-specific, using the existing
    // grass/flower functions, all scrolling via worldMove)
    // --------------------------------------------------

    if(currentSeason == sedlife)
    {
        drawGrassField();
    }

    // --------------------------------------------------
    // FOREST BACKGROUND
    // --------------------------------------------------

    drawForestBg();

    // --------------------------------------------------
    // FOREST
    // --------------------------------------------------

    drawForest();

    // --------------------------------------------------
    // SPRING ELEMENTS
    // --------------------------------------------------

    if(currentSeason == SPRING)
    {
        drawSpringEnvironment();
    }

    // --------------------------------------------------
    // TUNNEL
    // Only appears during a season transition
    // --------------------------------------------------

    if(tunnelVisible && transitionStage != 3)
    {
        drawCave();
    }

    // --------------------------------------------------
    // CAR
    // Fixed during normal mode.
    // Moves only during tunnel transition.
    // --------------------------------------------------

    if(changingSeason)
    {
        if(transitionStage == 1 ||
           transitionStage == 2 ||
           transitionStage == 3)
        {
            drawCar(transitionCarX, carY);
        }
        else
        {
            drawCar(transitionCarX, carY);
        }
    }
    else
    {
        drawCar(carX, carY);
    }

    // --------------------------------------------------
    // BLACK SEASON SCREEN
    // --------------------------------------------------

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
        // ----------------------------------------------
        // BACKGROUND MOVEMENT
        // Stops automatically during tunnel transition.
        // ----------------------------------------------

        updateWorld();

        // ----------------------------------------------
        // WHEEL ROTATION
        // ----------------------------------------------

        wheelRotation -= 15.0f;

        if(wheelRotation < 0)
            wheelRotation += 360.0f;

        // ----------------------------------------------
        // TUNNEL / SEASON TRANSITION
        // ----------------------------------------------

        if(changingSeason)
        {
            updateSeasonTransition();
        }

        // ----------------------------------------------
        // CLOUDS
        // ----------------------------------------------

        updateClouds();

        // ----------------------------------------------
        // SPRING BUTTERFLIES
        // ----------------------------------------------

        if(currentSeason == SPRING)
        {
            butterflyTime += 0.05f;
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
    // --------------------------------------------------
    // DEFAULT SEASON
    // --------------------------------------------------

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

            // Car begins at the center and moves toward
            // the tunnel during the transition.
            transitionCarX = carX;
        }
    }

    // --------------------------------------------------
    // SPRING
    // --------------------------------------------------

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

    // --------------------------------------------------
    // PAUSE
    // --------------------------------------------------

    else if(key == ' ')
    {
        paused = !paused;
    }

    // --------------------------------------------------
    // ESCAPE
    // --------------------------------------------------

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
    // NOTE:
    // glClearColor uses FLOAT values,
    // so it remains glClearColor.

    glClearColor(0.50f, 0.80f, 1.0f, 1.0f);

    // Projection matrix

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // 2D coordinate system

    gluOrtho2D(-100, 100, -60, 100);

    // Model-view matrix

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ======================================================
// MAIN
// ======================================================

int main(int argc, char** argv)
{
    // Initialize GLUT

    glutInit(&argc, argv);

    // Double buffering + RGB

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // Window size

    glutInitWindowSize(1100, 700);

    // Window position

    glutInitWindowPosition(100, 50);

    // Window title

    glutCreateWindow("Forest Through The Cycle Of A Year");

    // Initialize OpenGL

    init();

    // Display callback

    glutDisplayFunc(display);

    // Keyboard callback

    glutKeyboardFunc(keyboard);

    // Animation timer

    glutTimerFunc(16, update, 0);

    // Start GLUT

    glutMainLoop();

    return 0;
}

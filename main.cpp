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
bool paused = false;
bool automaticMode = false;

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

const int FLOWER_COUNT = 22;

// x position of flowers
float flowerX[FLOWER_COUNT] = {
    -93, -84, -73, -58, -46, -37, -25, -12,
    -3, 10, 21, 33, 44, 55, 66, 77,
    86, 94, -66, -20, 5, 60
};

// y position of flowers (kept low, near the ground line
// and scattered a little deeper toward the front)
float flowerY[FLOWER_COUNT] = {
    -24, -32, -27, -37, -23, -41, -29, -25,
    -44, -31, -22, -38, -26, -46, -30, -24,
    -40, -28, -49, -33, -21, -43
};

// scale (size) of flowers
float flowerScale[FLOWER_COUNT] = {
    0.8, 1.0, 0.7, 0.9, 1.1, 0.8, 1.0, 0.9,
    0.7, 1.0, 0.85, 0.95, 0.75, 1.05, 0.8, 0.9,
    1.0, 0.7, 0.9, 1.0, 0.8, 0.95
};

// petal color of each flower (0 = pink, 1 = purple,
// 2 = white, 3 = red-orange) - cycles through a few
// classic spring colors
int flowerColor[FLOWER_COUNT] = {
    0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1
};

// ======================================================
// SPRING - BUTTERFLY DATA (small animated butterflies
// that drift and flap above the ground, SPRING only)
// ======================================================

const int BUTTERFLY_COUNT = 4;

// base (center) position each butterfly drifts around
float butterflyBaseX[BUTTERFLY_COUNT] = { -45, -8, 30, 68 };
float butterflyBaseY[BUTTERFLY_COUNT] = { -4, 6, -6, 3 };

// phase offset so each butterfly flaps/drifts out of sync
float butterflyPhase[BUTTERFLY_COUNT] = { 0.0f, 1.6f, 3.1f, 4.7f };

// size of each butterfly
float butterflyScale[BUTTERFLY_COUNT] = { 1.0f, 0.8f, 1.1f, 0.9f };

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
// TREE TRUNK
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
// TREE BRANCHES
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
// TREE LEAVES (color changes slightly for spring to give
// a fresh, bright-green blooming look)
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
// COMPLETE TREE( here were adding all the different tree element that we made before from the line 174
// ======================================================

void drawTree(float x, float y, float scale)
{
    // Draw trunk first

    drawTreeTrunk(x, y, scale);

    // Draw branches

    drawBranches(x, y, scale);

    // Draw leaves last

    drawTreeLeaves(x, y, scale);
}

// ======================================================
// COMPLETE FOREST(here we use the forest making
// ======================================================

void drawForest()
{
    for(int i = 0; i < TREE_COUNT; i++)
    {
        drawTree(treeX[i], treeY[i], treeScale[i]);// these are the positions and scales we added at the 1st of the code hehe
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
        drawForestBgGrass(forestBgX[i], -20, forestBgScale[i]); // -20 is the ground top (see drawGround)
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
// ======================================================

void drawGrassField()
{
    for(int i = 0; i < GRASS_COUNT; i++)
    {
        drawGrass(grassX[i], grassY[i], grassScale[i]); // each tuft uses its own y position this time
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
// ======================================================

void drawFlowers()
{
    for(int i = 0; i < FLOWER_COUNT; i++)
    {
        drawFlower(flowerX[i], flowerY[i], flowerScale[i], flowerColor[i]);
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
// ======================================================

void drawButterflies()
{
    for(int i = 0; i < BUTTERFLY_COUNT; i++)
    {
        // Gentle drifting flight path around the base position

        float x = butterflyBaseX[i] + 14.0f * sin(butterflyTime * 0.6f + butterflyPhase[i]);
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
    // GRASS (scattered on the plain green ground)
    // --------------------------------------------------

    drawGrassField();

    // --------------------------------------------------
    // FOREST BG
    // --------------------------------------------------

    drawForestBg();

    // --------------------------------------------------
    // FOREST
    // --------------------------------------------------

    drawForest();// we called the forest func here in display

    // --------------------------------------------------
    // OTHER MEMBERS WILL ADD THEIR FUNCTIONS HERE
    // --------------------------------------------------

    if(currentSeason == SPRING)
    {
        drawSpringEnvironment();
    }
// draw te ekta season add korlam,
    /*
        Example:

        drawSummerEnvironment();
        drawRain();
        drawAutumnLeaves();
        drawWinterEnvironment();
    */

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
        // CLOUD ANIMATION
        // ----------------------------------------------

        updateClouds();

        // ----------------------------------------------
        // SPRING BUTTERFLY ANIMATION
        // ----------------------------------------------

        if(currentSeason == SPRING)
        {
            butterflyTime += 0.05f;
        }

        // ----------------------------------------------
        // OTHER MEMBERS WILL ADD THEIR ANIMATION HERE
        // ----------------------------------------------

        /*
            Example:

            updateRain();
            updateAutumnLeaves();
            updateSnow();
        */
    }

    // Tell OpenGL to redraw

    glutPostRedisplay();

    // Call update again after 30 milliseconds

    glutTimerFunc(30, update, 0);
}

// ======================================================
// KEYBOARD
// ======================================================

void keyboard(unsigned char key, int x, int y)
{
      if(key == '0')
    {
        currentSeason = sedlife;
        automaticMode = false;
    }
    // --------------------------------------------------
    // SPRING
    // --------------------------------------------------

    else if(key == '1')
    {
        currentSeason = SPRING;
        automaticMode = false;
    }

    // --------------------------------------------------
    // SUMMER
    // --------------------------------------------------

    else if(key == '2')
    {
        currentSeason = SUMMER;
        automaticMode = false;
    }

    // --------------------------------------------------
    // RAINY
    // --------------------------------------------------

    else if(key == '3')
    {
        currentSeason = RAINY;
        automaticMode = false;
    }

    // --------------------------------------------------
    // AUTUMN
    // --------------------------------------------------

    else if(key == '4')
    {
        currentSeason = AUTUMN;
        automaticMode = false;
    }

    // --------------------------------------------------
    // WINTER
    // --------------------------------------------------

    else if(key == '5')
    {
        currentSeason = WINTER;
        automaticMode = false;
    }

    // --------------------------------------------------
    // AUTOMATIC MODE
    // --------------------------------------------------

    else if(key == 'a' || key == 'A')
    {
        automaticMode = true;
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

    glutTimerFunc(30, update, 0);

    // Start GLUT

    glutMainLoop();

    return 0;
}
// amar nam jani na

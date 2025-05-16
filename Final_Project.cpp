#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <string>

using namespace std;

const int WIDTH = 800;
const int HEIGHT = 600;

struct Rect {
    float left, right, bottom, top;
};

float arrowX = 100;
float arrowY = HEIGHT / 2;
float arrowLength = 50;
float arrowAngle = 0; // degrees
int highScore = 0;

float arrowSpeed = 20;
bool arrowMoving = false;

int currentBackground = 0;
int attemptsLeft = 5;
int score = 0;

float targetWidth = 40;
float targetY = HEIGHT / 2;

float currentTargetStartX;

Rect targetRects[4];

// Colors for target rectangles
float targetColors[4][3] = {
    {1.0f, 1.0f, 0.0f}, // Yellow
    {1.0f, 0.0f, 0.0f}, // Red
    {0.0f, 0.0f, 1.0f}, // Blue
    {0.1f, 0.1f, 0.1f}  // Black
};

// Button rectangles
Rect restartBtn = { WIDTH - 180, WIDTH - 100, HEIGHT - 50, HEIGHT - 20 };
Rect exitBtn = { WIDTH - 90, WIDTH - 10, HEIGHT - 50, HEIGHT - 20 };

void drawRectangle(const Rect& r, float rC, float gC, float bC) {
    glColor3f(rC, gC, bC);
    glBegin(GL_QUADS);
    glVertex2f(r.left, r.bottom);
    glVertex2f(r.right, r.bottom);
    glVertex2f(r.right, r.top);
    glVertex2f(r.left, r.top);
    glEnd();
}

void drawText(float x, float y, const string& text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

void randomizeTargetPosition() {
    currentTargetStartX = WIDTH - 100; // fixed horizontal position

    float minY = 150; // 100 + 50
    float maxY = HEIGHT - 150; // HEIGHT - 100 - 50
    targetY = minY + rand() % (int)(maxY - minY + 1);

    float reducedWidth = targetWidth / 5.0f;  // one-fifth width
    float startX = currentTargetStartX;

    targetRects[0] = { startX, startX + reducedWidth, targetY - 10, targetY + 10 };
    startX += reducedWidth;

    targetRects[1] = { startX, startX + reducedWidth, targetY - 20, targetY + 20 };
    startX += reducedWidth;

    targetRects[2] = { startX, startX + reducedWidth, targetY - 35, targetY + 35 };
    startX += reducedWidth;

    targetRects[3] = { startX, startX + reducedWidth, targetY - 50, targetY + 50 };
}

void drawTarget() {
    for (int i = 0; i < 4; i++) {
        drawRectangle(targetRects[i], targetColors[i][0], targetColors[i][1], targetColors[i][2]);
    }
}


void drawBow() {
    const float bowCenterX = 100.0f;  // Fixed X position
    const float bowCenterY = HEIGHT / 2.0f;  // Vertical center
    const float bowRadius = 60.0f;

    float bowRotation = 0.6f * arrowAngle;  // 60% of arrow rotation

    // Draw bow limbs (with reduced rotation)
    glPushMatrix();
    glTranslatef(bowCenterX, bowCenterY, 0);
    glRotatef(bowRotation, 0, 0, 1);  // 0.8 * arrowAngle

    glColor3f(0.6f, 0.3f, 0.0f);  // Brown limbs
    glBegin(GL_LINE_STRIP);
    for (float angle = -0.8f; angle <= 0.8f; angle += 0.05f) {
        glVertex2f(bowRadius * cos(angle), bowRadius * sin(angle));
    }
    glEnd();
    glPopMatrix();

    // Draw bowstring (connects to arrow or fixed)
    glPushMatrix();
    glTranslatef(bowCenterX, bowCenterY, 0);
    glRotatef(bowRotation, 0, 0, 1);  // Same reduced rotation

    glColor3f(0.9f, 0.9f, 0.9f);  // White string
    glBegin(GL_LINES);
    float leftX = bowRadius * cos(-0.8f);
    float leftY = bowRadius * sin(-0.8f);
    float rightX = bowRadius * cos(0.8f);
    float rightY = bowRadius * sin(0.8f);

    if (arrowMoving) {
        glVertex2f(leftX, leftY);
        glVertex2f(rightX, rightY);  
    } else {
        glVertex2f(leftX, leftY);
        glVertex2f(rightX, rightY);  // Normal bowstring
    }
    glEnd();
    glPopMatrix();

    // Reset arrow when not moving
    if (!arrowMoving) {
        arrowX = bowCenterX + 30;
        arrowY = bowCenterY;
    }
}

void drawArrow() {
    glPushMatrix();
    glTranslatef(arrowX, arrowY, 0);
    glRotatef(arrowAngle, 0, 0, 1);  // Full arrow rotation

    glColor3f(0.8f, 0.3f, 0.0f);  // Arrow shaft
    glBegin(GL_LINES);
    glVertex2f(0, 0);
    glVertex2f(arrowLength, 0);
    glEnd();

    glBegin(GL_TRIANGLES);  // Arrowhead
    glVertex2f(arrowLength, 0);
    glVertex2f(arrowLength - 10, 5);
    glVertex2f(arrowLength - 10, -5);
    glEnd();

    glPopMatrix();
}

void resetArrow() {
    arrowX = 100.0f;  // Same as bowCenterX
    arrowY = HEIGHT / 2.0f;  // Same as bowCenterY
    arrowAngle = 0;
    arrowMoving = false;
}

bool checkHit(const Rect& r) {
    float rad = arrowAngle * M_PI / 180.0f;
    float tipX = arrowX + arrowLength * cos(rad);
    float tipY = arrowY + arrowLength * sin(rad);

    return (tipX >= r.left && tipX <= r.right && tipY >= r.bottom && tipY <= r.top);
}

void updateArrow() {

    if (!arrowMoving) return;

    float rad = arrowAngle * M_PI / 180.0f;

    arrowX += arrowSpeed * cos(rad);
    arrowY += arrowSpeed * sin(rad);

    if (arrowX > WIDTH || arrowY < 0 || arrowY > HEIGHT) {
        attemptsLeft--;
        currentBackground = (currentBackground + 1) % 5; // Cycle background
        resetArrow();
        randomizeTargetPosition();
    } else {
        for (int i = 0; i < 4; i++) {
            if (checkHit(targetRects[i])) {
                int pts = 0;
                switch (i) {
                    case 0: pts = 100; break;
                    case 1: pts = 50; break;
                    case 2: pts = 20; break;
                    case 3: pts = 10; break;
                }
                score += pts;
                cout << "Hit! +" << pts << " points\n";
                resetArrow();
                randomizeTargetPosition();
                attemptsLeft--;
                currentBackground = (currentBackground + 1) % 5; // Cycle background
                break;
            }
        }
    }

    if (attemptsLeft <= 0) {
        arrowMoving = false;
        cout << "Game Over! Final Score: " << score << endl;
    }
}

void drawButton(const Rect& btnRect, const string& text, float r, float g, float b) {
    drawRectangle(btnRect, r, g, b);  // Use passed-in color
    glColor3f(1, 1, 1);  // Text color: white
    float textX = (btnRect.left + btnRect.right) / 2 - (text.size() * 4.5f);
    float textY = (btnRect.bottom + btnRect.top) / 2 - 6;
    drawText(textX, textY, text);
}

// Enhanced Background 1: Realistic Forest Range
void drawForestBackground() {
    // Sky gradient (dawn effect)
    glBegin(GL_QUADS);
    glColor3f(0.2f, 0.3f, 0.5f);  // Dark blue (top)
    glVertex2f(0, HEIGHT);
    glVertex2f(WIDTH, HEIGHT);
    glColor3f(0.6f, 0.8f, 1.0f);  // Light blue (middle)
    glVertex2f(WIDTH, HEIGHT/2);
    glVertex2f(0, HEIGHT/2);
    glEnd();

    // Distant mountains
    glColor3f(0.3f, 0.4f, 0.3f);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(0, HEIGHT/3);
    glVertex2f(WIDTH/4, HEIGHT/2.5);
    glVertex2f(WIDTH/2, HEIGHT/3);
    glVertex2f(3*WIDTH/4, HEIGHT/2.2);
    glVertex2f(WIDTH, HEIGHT/3);
    glEnd();

    // Tree line
    glColor3f(0.0f, 0.4f, 0.0f);
    for (int i = 0; i < WIDTH; i += 20) {
        glBegin(GL_TRIANGLES);
        glVertex2f(i, HEIGHT / 3);
        glVertex2f(i + 10, HEIGHT / 2.8);
        glVertex2f(i + 20, HEIGHT / 3);
        glEnd();
    }

    // Ground
    glColor3f(0.1f, 0.6f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glVertex2f(WIDTH, HEIGHT / 3);
    glVertex2f(0, HEIGHT / 3);
    glEnd();
}
// Enhanced Background 2: Desert Canyon
void drawDesertBackground() {
    // Sky gradient
    glBegin(GL_QUADS);
    glColor3f(0.8f, 0.5f, 0.2f);  // Sandy
    glVertex2f(0, HEIGHT);
    glColor3f(1.0f, 0.8f, 0.4f);  // Bright
    glVertex2f(WIDTH, HEIGHT);
    glColor3f(0.9f, 0.6f, 0.3f);  // Darker
    glVertex2f(WIDTH, 0);
    glVertex2f(0, 0);
    glEnd();

    // Canyon walls
    glColor3f(0.7f, 0.5f, 0.3f);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(0, HEIGHT/4);
    glVertex2f(WIDTH/6, HEIGHT/3);
    glVertex2f(WIDTH/3, HEIGHT/5);
    glVertex2f(WIDTH/2, HEIGHT/3.5);
    glVertex2f(2*WIDTH/3, HEIGHT/6);
    glVertex2f(5*WIDTH/6, HEIGHT/3.2);
    glVertex2f(WIDTH, HEIGHT/4);
    glEnd();

    // Sand dunes
    glColor3f(0.9f, 0.8f, 0.5f);
    glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i <= 10; i++) {
        float x = i * (WIDTH/10.0f);
        float y = HEIGHT/4 - 20 + 30 * sin(i * 0.5f);
        glVertex2f(x, HEIGHT/4);
        glVertex2f(x, y);
    }
    glEnd();

    // Sun
    glColor3f(1.0f, 0.9f, 0.4f);
    glBegin(GL_TRIANGLE_FAN);
    for(int i = 0; i <= 360; i += 10) {
        float angle = i * M_PI / 180;
        glVertex2f(WIDTH-100 + cos(angle)*40, HEIGHT-100 + sin(angle)*40);
    }
    glEnd();
}

// Enhanced Background 3: Medieval Castle
void drawCastleBackground() {
    // Night sky with gradient
    glBegin(GL_QUADS);
    glColor3f(0.05f, 0.05f, 0.1f);  // Deep blue
    glVertex2f(0, HEIGHT);
    glColor3f(0.1f, 0.1f, 0.2f);    // Slightly lighter at bottom
    glVertex2f(WIDTH, HEIGHT);
    glColor3f(0.0f, 0.0f, 0.05f);
    glVertex2f(WIDTH, 0);
    glVertex2f(0, 0);
    glEnd();

    // Castle silhouette (dark)
    glColor3f(0.3f, 0.1f, 0.19f);
    glBegin(GL_QUADS);
    glVertex2f(WIDTH - 400, HEIGHT/3);
    glVertex2f(WIDTH - 150, HEIGHT/3);
    glVertex2f(WIDTH - 150, HEIGHT/3 + 200);
    glVertex2f(WIDTH - 400, HEIGHT/3 + 200);
    glEnd();

    // Towers
    for (int i = 0; i < 3; i++) {
        float x = WIDTH - 400 + i * 125;
        glBegin(GL_QUADS);
        glVertex2f(x - 15, HEIGHT/3 + 200);
        glVertex2f(x + 15, HEIGHT/3 + 200);
        glVertex2f(x + 15, HEIGHT/3 + 300);
        glVertex2f(x - 15, HEIGHT/3 + 300);
        glEnd();

        glBegin(GL_TRIANGLES);
        glVertex2f(x - 20, HEIGHT/3 + 300);
        glVertex2f(x + 20, HEIGHT/3 + 300);
        glVertex2f(x, HEIGHT/3 + 340);
        glEnd();
    }

    // Lit windows
    glColor3f(1.0f, 1.0f, 0.3f);
    for (int i = 0; i < 4; i++) {
        float x = WIDTH - 350 + i * 60;
        glBegin(GL_QUADS);
        glVertex2f(x - 5, HEIGHT/3 + 120);
        glVertex2f(x + 5, HEIGHT/3 + 120);
        glVertex2f(x + 5, HEIGHT/3 + 140);
        glVertex2f(x - 5, HEIGHT/3 + 140);
        glEnd();
    }

    // Stars
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 200; i++) {
        float x = rand() % WIDTH;
        float y = rand() % HEIGHT;
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(x, y);
    }
    glEnd();
}


// Enhanced Background 4: Snowy Mountain
void drawSnowBackground() {
    // Sky gradient
    glBegin(GL_QUADS);
    glColor3f(0.4f, 0.6f, 0.8f);  // Light blue
    glVertex2f(0, HEIGHT);
    glColor3f(0.7f, 0.8f, 0.9f);  // Very light blue
    glVertex2f(WIDTH, HEIGHT);
    glColor3f(0.9f, 0.95f, 1.0f); // Off-white
    glVertex2f(WIDTH, 0);
    glVertex2f(0, 0);
    glEnd();

    // Mountains
    glColor3f(0.8f, 0.85f, 0.9f);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(0, HEIGHT/4);
    glVertex2f(WIDTH/5, HEIGHT/2);
    glVertex2f(WIDTH/3, HEIGHT/3);
    glVertex2f(WIDTH/2, HEIGHT/1.8);
    glVertex2f(2*WIDTH/3, HEIGHT/3.5);
    glVertex2f(4*WIDTH/5, HEIGHT/2.2);
    glVertex2f(WIDTH, HEIGHT/4);
    glEnd();

    // Snow layer
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glVertex2f(WIDTH, HEIGHT/4);
    glVertex2f(0, HEIGHT/4);
    glEnd();

    // Snowflakes
    glPointSize(2.5f);
    glBegin(GL_POINTS);
    for(int i = 0; i < 300; i++) {
        float x = rand() % WIDTH;
        float y = rand() % HEIGHT;
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(x, y);
    }
    glEnd();
}

void drawJapaneseBackground() {
    // Sunset sky gradient
    glBegin(GL_QUADS);
    glColor3f(0.9f, 0.5f, 0.3f);  // Reddish orange
    glVertex2f(0, HEIGHT);
    glColor3f(1.0f, 0.8f, 0.5f);  // Peach
    glVertex2f(WIDTH, HEIGHT);
    glColor3f(0.95f, 0.7f, 0.3f); // Light orange
    glVertex2f(WIDTH, 0);
    glVertex2f(0, 0);
    glEnd();

    // Silhouetted mountains
    glColor3f(0.3f, 0.25f, 0.2f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0, HEIGHT/4);
    glVertex2f(WIDTH/3, HEIGHT/2);
    glVertex2f(2*WIDTH/3, HEIGHT/4);

    glVertex2f(WIDTH/3, HEIGHT/4);
    glVertex2f(2*WIDTH/3, HEIGHT/2.5);
    glVertex2f(WIDTH, HEIGHT/4);
    glEnd();

    // Reflective pond
    glColor4f(0.4f, 0.6f, 0.8f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(WIDTH - 300, 0);
    glVertex2f(WIDTH, 0);
    glVertex2f(WIDTH, HEIGHT/5);
    glVertex2f(WIDTH - 250, HEIGHT/5);
    glEnd();
}


void drawBackground() {
    switch(currentBackground) {
        case 0: drawForestBackground(); break;
        case 1: drawDesertBackground(); break;
        case 2: drawCastleBackground(); break;
        case 3: drawSnowBackground(); break;
        case 4: drawJapaneseBackground(); break;
    }
}
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawBackground();
    drawTarget();
    drawBow();
    drawArrow();

    // Draw score and attempts
    glColor3f(1, 1, 1);
    if (currentBackground == 3 || currentBackground == 4) glColor3f(1, 0, 0); // Dark backgrounds
    drawText(20, HEIGHT - 30, "Score: " + to_string(score));
    drawText(20, HEIGHT - 60, "Attempts Left: " + to_string(attemptsLeft));
    drawText(20, HEIGHT - 90, "Range: " + to_string(currentBackground + 1) + "/5");
     
   if (attemptsLeft == 5) {
    float startY = HEIGHT - 100;

    // Set text color to bright yellow
    glColor3f(1.0f, 1.0f, 0.0f);  // Yellow color

    // Center X position for the text block
    float centerX = WIDTH / 2;

    // Draw text with shadow for better visibility
    auto drawShadowedText = [&](float x, float y, const std::string& text) {
        // Shadow (black, offset by 1 pixel)
        glColor3f(0.0f, 0.0f, 0.0f);
        drawText(x + 1, y - 1, text);

        // Main text (yellow)
        glColor3f(1.0f, 1.0f, 0.0f);
        drawText(x, y, text);
    };

    // Assuming drawText draws left aligned,
    // calculate text width offset for approximate centering
    // (Adjust 150 or do better text width calc if available)
    float offsetX = 150;

    drawShadowedText(centerX - offsetX, startY, "Welcome to Archery Challenge!");
    drawShadowedText(centerX - offsetX, startY - 25, "Use UP / DOWN arrow keys to set angle.");
    drawShadowedText(centerX - offsetX, startY - 50, "Press SPACE to shoot the arrow.");
    drawShadowedText(centerX - offsetX, startY - 75, "You have 5 attempts. Good luck!");
}


    // Draw buttons
   drawButton(restartBtn, "Restart", 0.0f, 0.7f, 0.0f); // Green
   drawButton(exitBtn, "Exit", 0.8f, 0.0f, 0.0f);       // Red
    

    if (attemptsLeft <= 0) {
        glColor3f(1, 0, 0);
        drawText(WIDTH / 2 - 70, HEIGHT / 2, "Game Over!");
    }

    glutSwapBuffers();
}

void timer(int) {
    if (attemptsLeft > 0) {
        updateArrow();
        glutPostRedisplay();
        glutTimerFunc(30, timer, 0);
    }
}

void keyPressed(int key, int x, int y) {
    if (attemptsLeft <= 0) return;

    switch (key) {
        case GLUT_KEY_UP:
            arrowAngle += 5;
            if (arrowAngle > 55) arrowAngle = 55;  // Max +55° (upward)
            break;
        case GLUT_KEY_DOWN:
            arrowAngle -= 5;
            if (arrowAngle < -55) arrowAngle = -55;  // Min -55° (downward)
            break;
    }
}

void keyboard(unsigned char key, int x, int y) {
    if (attemptsLeft <= 0) return;

    if (key == 27) {
        exit(0);
    } else if (key == ' ') {
        if (!arrowMoving && attemptsLeft > 0) {
            arrowMoving = true;
        }
    } else if (key == 'r' || key == 'R') {
        score = 0;
        attemptsLeft = 5;
        currentBackground = 0;
        resetArrow();
        randomizeTargetPosition();
        glutTimerFunc(30, timer, 0);
    }
}

// Check if mouse click inside a rectangle
bool isInside(float x, float y, const Rect& r) {
    return x >= r.left && x <= r.right && y >= r.bottom && y <= r.top;
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float oglY = HEIGHT - y;
        if (isInside(x, oglY, restartBtn)) {
            score = 0;
            attemptsLeft = 5;
            currentBackground = 0;
            resetArrow();
            randomizeTargetPosition();
            glutTimerFunc(30, timer, 0);
        } else if (isInside(x, oglY, exitBtn)) {
            exit(0);
        }
    }
}

void init() {
    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    
    // Enable smooth shading and blending
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    
    randomizeTargetPosition();
    resetArrow();
    srand((unsigned)time(NULL));
}

int main(int argc, char** argv) {
    srand((unsigned)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Archary_Game_2D");

    init();

    glutDisplayFunc(display);
    glutTimerFunc(30, timer, 0);
    glutSpecialFunc(keyPressed);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);

    glutMainLoop();

    return 0;
}

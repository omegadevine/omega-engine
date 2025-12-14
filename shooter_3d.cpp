#include <iostream>
#include <windows.h>
#include <conio.h>
#include <string>
#include <vector>
#include <cmath>
#include <ctime>
#include <thread>
#include <atomic>
#include <mutex>
#include <algorithm>

using namespace std;

// Console colors
#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define YELLOW 6
#define GRAY 7
#define DARK_GRAY 8
#define BRIGHT_BLUE 9
#define BRIGHT_GREEN 10
#define BRIGHT_CYAN 11
#define BRIGHT_RED 12
#define BRIGHT_MAGENTA 13
#define BRIGHT_YELLOW 14
#define BRIGHT_WHITE 15

const double PI = 3.14159265359;

// ============================================================
// ULTRA-FAST CONSOLE RENDERER
// ============================================================

class Console {
private:
    static HANDLE hConsole;
    static char screenBuffer[30][81];
    static int colorBuffer[30][81];
    static char lastScreenBuffer[30][81];
    static int lastColorBuffer[30][81];
    static atomic<bool> dirty;
    static mutex bufferMutex;
    static bool initialized;
    static CHAR_INFO writeBuffer[30 * 80];
    
public:
    static void init() {
        if (initialized) return;
        
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        
        CONSOLE_CURSOR_INFO cursorInfo;
        cursorInfo.dwSize = 1;
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
        SMALL_RECT windowSize = {0, 0, 79, 29};
        SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
        
        clearBuffer();
        for (int y = 0; y < 30; y++) {
            for (int x = 0; x < 80; x++) {
                lastScreenBuffer[y][x] = ' ';
                lastColorBuffer[y][x] = GRAY;
            }
        }
        
        initialized = true;
    }
    
    static void clearBuffer() {
        lock_guard<mutex> lock(bufferMutex);
        for (int y = 0; y < 30; y++) {
            for (int x = 0; x < 80; x++) {
                screenBuffer[y][x] = ' ';
                colorBuffer[y][x] = GRAY;
            }
        }
        dirty = true;
    }
    
    static void setChar(int x, int y, char c, int color) {
        if (x < 0 || x >= 80 || y < 0 || y >= 30) return;
        lock_guard<mutex> lock(bufferMutex);
        screenBuffer[y][x] = c;
        colorBuffer[y][x] = color;
        dirty = true;
    }
    
    static void drawString(int x, int y, const string& str, int color) {
        lock_guard<mutex> lock(bufferMutex);
        for (size_t i = 0; i < str.length() && x + i < 80; i++) {
            if (y >= 0 && y < 30) {
                screenBuffer[y][x + i] = str[i];
                colorBuffer[y][x + i] = color;
            }
        }
        dirty = true;
    }
    
    static void render() {
        if (!dirty) return;
        
        lock_guard<mutex> lock(bufferMutex);
        
        int bufferIndex = 0;
        for (int y = 0; y < 30; y++) {
            for (int x = 0; x < 80; x++) {
                writeBuffer[bufferIndex].Char.AsciiChar = screenBuffer[y][x];
                writeBuffer[bufferIndex].Attributes = colorBuffer[y][x];
                bufferIndex++;
            }
        }
        
        COORD bufferSize = {80, 30};
        COORD bufferCoord = {0, 0};
        SMALL_RECT writeRegion = {0, 0, 79, 29};
        
        WriteConsoleOutput(hConsole, writeBuffer, bufferSize, bufferCoord, &writeRegion);
        
        dirty = false;
    }
};

HANDLE Console::hConsole = nullptr;
char Console::screenBuffer[30][81] = {};
int Console::colorBuffer[30][81] = {};
char Console::lastScreenBuffer[30][81] = {};
int Console::lastColorBuffer[30][81] = {};
atomic<bool> Console::dirty(false);
mutex Console::bufferMutex;
bool Console::initialized = false;
CHAR_INFO Console::writeBuffer[30 * 80] = {};

// ============================================================
// PLAYER
// ============================================================

struct Player {
    double x, y;           // Position
    double angle;          // Viewing angle
    int health;
    int ammo;
    int score;
    
    Player() : x(3.5), y(3.5), angle(0), health(100), ammo(50), score(0) {}
    
    void move(double dx, double dy, const vector<vector<int>>& map) {
        double newX = x + dx;
        double newY = y + dy;
        
        if (map[(int)newY][(int)newX] == 0) {
            x = newX;
            y = newY;
        }
    }
    
    void rotate(double da) {
        angle += da;
        while (angle < 0) angle += 2 * PI;
        while (angle >= 2 * PI) angle -= 2 * PI;
    }
};

// ============================================================
// ENEMY
// ============================================================

struct Enemy {
    double x, y;
    int health;
    bool active;
    double lastMoveTime;
    
    Enemy(double px, double py) : x(px), y(py), health(30), active(true), lastMoveTime(0) {}
    
    void update(const Player& player, const vector<vector<int>>& map, double currentTime) {
        if (!active) return;
        
        // Simple AI: move towards player
        if (currentTime - lastMoveTime > 0.5) {
            double dx = player.x - x;
            double dy = player.y - y;
            double dist = sqrt(dx * dx + dy * dy);
            
            if (dist > 0.5) {
                dx /= dist;
                dy /= dist;
                
                double newX = x + dx * 0.1;
                double newY = y + dy * 0.1;
                
                if (map[(int)newY][(int)newX] == 0) {
                    x = newX;
                    y = newY;
                }
            }
            
            lastMoveTime = currentTime;
        }
    }
    
    double distanceToPlayer(const Player& player) {
        double dx = player.x - x;
        double dy = player.y - y;
        return sqrt(dx * dx + dy * dy);
    }
};

// ============================================================
// RAYCASTING ENGINE
// ============================================================

class RaycasterEngine {
private:
    const int screenWidth = 80;
    const int screenHeight = 25;
    const double FOV = PI / 3.0;  // 60 degrees
    
    vector<vector<int>> map;
    Player& player;
    vector<Enemy>& enemies;
    
    char getWallShade(double distance) {
        if (distance < 2.0) return '#';
        if (distance < 4.0) return '%';
        if (distance < 6.0) return '+';
        if (distance < 8.0) return '-';
        if (distance < 10.0) return '.';
        return ' ';
    }
    
    int getWallColor(double distance) {
        if (distance < 3.0) return BRIGHT_WHITE;
        if (distance < 5.0) return GRAY;
        if (distance < 7.0) return DARK_GRAY;
        return BLACK;
    }
    
public:
    RaycasterEngine(vector<vector<int>>& m, Player& p, vector<Enemy>& e) 
        : map(m), player(p), enemies(e) {}
    
    void render() {
        // Render 3D view using raycasting
        for (int x = 0; x < screenWidth; x++) {
            double rayAngle = player.angle - FOV / 2.0 + (x * FOV / screenWidth);
            double rayDirX = cos(rayAngle);
            double rayDirY = sin(rayAngle);
            
            // Cast ray
            double distance = 0;
            bool hitWall = false;
            int wallType = 1;
            
            while (!hitWall && distance < 20.0) {
                distance += 0.1;
                int testX = (int)(player.x + rayDirX * distance);
                int testY = (int)(player.y + rayDirY * distance);
                
                if (testX < 0 || testX >= map[0].size() || testY < 0 || testY >= map.size()) {
                    hitWall = true;
                    distance = 20.0;
                } else if (map[testY][testX] != 0) {
                    hitWall = true;
                    wallType = map[testY][testX];
                }
            }
            
            // Calculate wall height
            int ceiling = (int)((screenHeight / 2.0) - screenHeight / distance);
            int floor = screenHeight - ceiling;
            
            // Draw column
            for (int y = 0; y < screenHeight; y++) {
                if (y < ceiling) {
                    // Ceiling
                    Console::setChar(x, y, ' ', BLUE);
                } else if (y > floor) {
                    // Floor
                    double floorDist = screenHeight / (2.0 * y - screenHeight);
                    if (floorDist < 5.0) Console::setChar(x, y, '.', DARK_GRAY);
                    else Console::setChar(x, y, ' ', BLACK);
                } else {
                    // Wall
                    char shade = getWallShade(distance);
                    int color = getWallColor(distance);
                    if (wallType == 2) color = RED;  // Different wall type
                    Console::setChar(x, y, shade, color);
                }
            }
        }
        
        // Render enemies (sprites)
        for (auto& enemy : enemies) {
            if (!enemy.active) continue;
            
            double dx = enemy.x - player.x;
            double dy = enemy.y - player.y;
            double distance = sqrt(dx * dx + dy * dy);
            
            // Calculate angle to enemy
            double angleToEnemy = atan2(dy, dx) - player.angle;
            while (angleToEnemy < -PI) angleToEnemy += 2 * PI;
            while (angleToEnemy > PI) angleToEnemy -= 2 * PI;
            
            // Check if enemy is in FOV
            if (abs(angleToEnemy) < FOV / 2.0 + 0.5 && distance < 15.0) {
                int screenX = (int)((angleToEnemy / FOV + 0.5) * screenWidth);
                int spriteHeight = (int)(screenHeight / distance);
                int spriteY = screenHeight / 2 - spriteHeight / 2;
                
                if (screenX >= 0 && screenX < screenWidth) {
                    // Draw simple enemy sprite
                    char enemyChar = 'E';
                    int enemyColor = BRIGHT_RED;
                    
                    if (distance < 2.0) enemyChar = '@';
                    else if (distance < 5.0) enemyChar = 'E';
                    else enemyChar = 'e';
                    
                    for (int i = -1; i <= 1; i++) {
                        int drawX = screenX + i;
                        if (drawX >= 0 && drawX < screenWidth) {
                            for (int j = 0; j < spriteHeight && spriteY + j < screenHeight; j++) {
                                if (spriteY + j >= 0) {
                                    Console::setChar(drawX, spriteY + j, enemyChar, enemyColor);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
};

// ============================================================
// GAME
// ============================================================

class Game {
private:
    Player player;
    vector<Enemy> enemies;
    vector<vector<int>> map;
    RaycasterEngine* engine;
    bool running;
    double gameTime;
    
    void initMap() {
        map = {
            {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,2,2,2,0,0,0,0,2,2,2,0,0,1},
            {1,0,0,2,0,0,0,0,0,0,0,0,2,0,0,1},
            {1,0,0,2,0,0,0,0,0,0,0,0,2,0,0,1},
            {1,0,0,0,0,0,0,2,2,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,2,2,0,0,0,0,0,0,1},
            {1,0,0,2,0,0,0,0,0,0,0,0,2,0,0,1},
            {1,0,0,2,0,0,0,0,0,0,0,0,2,0,0,1},
            {1,0,0,2,2,2,0,0,0,0,2,2,2,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
        };
    }
    
    void spawnEnemies() {
        enemies.push_back(Enemy(10.5, 5.5));
        enemies.push_back(Enemy(5.5, 10.5));
        enemies.push_back(Enemy(12.5, 8.5));
        enemies.push_back(Enemy(7.5, 3.5));
    }
    
public:
    Game() : running(true), gameTime(0) {
        Console::init();
        initMap();
        spawnEnemies();
        engine = new RaycasterEngine(map, player, enemies);
    }
    
    ~Game() {
        delete engine;
    }
    
    void handleInput() {
        if (_kbhit()) {
            char key = _getch();
            
            const double moveSpeed = 0.1;
            const double rotSpeed = 0.1;
            
            // WASD movement
            if (key == 'w' || key == 'W') {
                player.move(cos(player.angle) * moveSpeed, sin(player.angle) * moveSpeed, map);
            }
            if (key == 's' || key == 'S') {
                player.move(-cos(player.angle) * moveSpeed, -sin(player.angle) * moveSpeed, map);
            }
            if (key == 'a' || key == 'A') {
                player.move(sin(player.angle) * moveSpeed, -cos(player.angle) * moveSpeed, map);
            }
            if (key == 'd' || key == 'D') {
                player.move(-sin(player.angle) * moveSpeed, cos(player.angle) * moveSpeed, map);
            }
            
            // Arrow keys for rotation
            if (key == 75) { // Left arrow
                player.rotate(-rotSpeed);
            }
            if (key == 77) { // Right arrow
                player.rotate(rotSpeed);
            }
            
            // Shooting
            if (key == ' ' && player.ammo > 0) {
                shoot();
            }
            
            // Quit
            if (key == 'q' || key == 'Q' || key == 27) {
                running = false;
            }
        }
    }
    
    void shoot() {
        player.ammo--;
        
        // Cast ray in player direction to find enemy
        double rayDirX = cos(player.angle);
        double rayDirY = sin(player.angle);
        
        double minDist = 20.0;
        Enemy* hitEnemy = nullptr;
        
        for (auto& enemy : enemies) {
            if (!enemy.active) continue;
            
            double dx = enemy.x - player.x;
            double dy = enemy.y - player.y;
            double dist = sqrt(dx * dx + dy * dy);
            
            // Check if enemy is in line of sight
            double angleToEnemy = atan2(dy, dx);
            double angleDiff = angleToEnemy - player.angle;
            while (angleDiff < -PI) angleDiff += 2 * PI;
            while (angleDiff > PI) angleDiff -= 2 * PI;
            
            if (abs(angleDiff) < 0.2 && dist < minDist) {
                minDist = dist;
                hitEnemy = &enemy;
            }
        }
        
        if (hitEnemy) {
            hitEnemy->health -= 20;
            if (hitEnemy->health <= 0) {
                hitEnemy->active = false;
                player.score += 100;
            }
        }
    }
    
    void update() {
        gameTime += 0.016; // Approximately 60 FPS
        
        // Update enemies
        for (auto& enemy : enemies) {
            enemy.update(player, map, gameTime);
            
            // Check if enemy hits player
            if (enemy.active && enemy.distanceToPlayer(player) < 0.5) {
                player.health -= 1;
            }
        }
        
        // Check win condition
        bool allEnemiesDead = true;
        for (auto& enemy : enemies) {
            if (enemy.active) {
                allEnemiesDead = false;
                break;
            }
        }
        
        if (allEnemiesDead) {
            running = false;
        }
        
        if (player.health <= 0) {
            running = false;
        }
    }
    
    void render() {
        Console::clearBuffer();
        
        // Render 3D view
        engine->render();
        
        // Render HUD
        Console::drawString(1, 26, "OMEGA SHOOTER 3D", BRIGHT_YELLOW);
        Console::drawString(1, 27, "Health: " + to_string(player.health) + " | Ammo: " + to_string(player.ammo) + " | Score: " + to_string(player.score), BRIGHT_WHITE);
        Console::drawString(1, 28, "WASD: Move | Arrows: Look | Space: Shoot | Q: Quit", GRAY);
        
        Console::render();
    }
    
    void run() {
        Console::drawString(25, 12, "OMEGA SHOOTER 3D", BRIGHT_YELLOW);
        Console::drawString(20, 14, "Wolfenstein-style Raycasting Demo", BRIGHT_CYAN);
        Console::drawString(28, 16, "Press any key to start", BRIGHT_WHITE);
        Console::render();
        _getch();
        
        while (running) {
            handleInput();
            update();
            render();
            Sleep(16); // ~60 FPS
        }
        
        // Game over screen
        Console::clearBuffer();
        if (player.health <= 0) {
            Console::drawString(32, 12, "GAME OVER", BRIGHT_RED);
            Console::drawString(28, 14, "You were defeated!", RED);
        } else {
            Console::drawString(32, 12, "VICTORY!", BRIGHT_GREEN);
            Console::drawString(25, 14, "All enemies eliminated!", GREEN);
        }
        Console::drawString(30, 16, "Final Score: " + to_string(player.score), BRIGHT_YELLOW);
        Console::drawString(27, 18, "Press any key to exit", GRAY);
        Console::render();
        _getch();
    }
};

// ============================================================
// MAIN
// ============================================================

int main() {
    try {
        Game game;
        game.run();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}

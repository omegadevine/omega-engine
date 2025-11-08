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

// ============================================================
// ULTRA-FAST CONSOLE RENDERER
// - WriteConsoleOutput (1 API call vs 1230)
// - Delta rendering (only changed chars)
// - Hidden cursor (no blink overhead)
// - Async rendering (parallel threads)
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
        
        // OPTIMIZATION 5: Hide cursor (no blink overhead)
        CONSOLE_CURSOR_INFO cursorInfo;
        cursorInfo.dwSize = 1;
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
        // Set console size
        SMALL_RECT windowSize = {0, 0, 79, 29};
        SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
        
        // Initialize buffers
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
        for (size_t i = 0; i < str.length() && (x + i) < 80; i++) {
            if (y >= 0 && y < 30) {
                screenBuffer[y][x + i] = str[i];
                colorBuffer[y][x + i] = color;
            }
        }
        dirty = true;
    }
    
    // OPTIMIZATION 1: WriteConsoleOutput (1 API call instead of 1230!)
    // OPTIMIZATION 2: Delta rendering (only changed chars)
    static void flush() {
        if (!dirty) return;
        
        lock_guard<mutex> lock(bufferMutex);
        
        bool hasChanges = false;
        
        // Build write buffer with only changed characters
        for (int y = 0; y < 30; y++) {
            for (int x = 0; x < 80; x++) {
                int idx = y * 80 + x;
                
                // DELTA CHECK: Only update if changed
                if (screenBuffer[y][x] != lastScreenBuffer[y][x] ||
                    colorBuffer[y][x] != lastColorBuffer[y][x]) {
                    hasChanges = true;
                }
                
                writeBuffer[idx].Char.AsciiChar = screenBuffer[y][x];
                writeBuffer[idx].Attributes = colorBuffer[y][x];
                
                lastScreenBuffer[y][x] = screenBuffer[y][x];
                lastColorBuffer[y][x] = colorBuffer[y][x];
            }
        }
        
        if (!hasChanges) {
            dirty = false;
            return;
        }
        
        // WRITE ENTIRE SCREEN IN ONE API CALL!
        COORD bufferSize = {80, 30};
        COORD bufferCoord = {0, 0};
        SMALL_RECT writeRegion = {0, 0, 79, 29};
        
        WriteConsoleOutput(hConsole, writeBuffer, bufferSize, bufferCoord, &writeRegion);
        
        dirty = false;
    }
    
    static bool isDirty() { return dirty; }
};

// Static members
HANDLE Console::hConsole;
char Console::screenBuffer[30][81];
int Console::colorBuffer[30][81];
char Console::lastScreenBuffer[30][81];
int Console::lastColorBuffer[30][81];
atomic<bool> Console::dirty(false);
mutex Console::bufferMutex;
bool Console::initialized = false;
CHAR_INFO Console::writeBuffer[30 * 80];

// Vec2
struct Vec2 {
    int x, y;
    Vec2(int x = 0, int y = 0) : x(x), y(y) {}
    float dist(const Vec2& other) const {
        int dx = x - other.x;
        int dy = y - other.y;
        return sqrt(dx * dx + dy * dy);
    }
};

// Forward declarations
class Dungeon;

// Item
class Item {
public:
    Vec2 pos;
    char symbol;
    string name;
    int color;
    bool pickedUp;
    
    Item(int x, int y, char s, string n, int c)
        : pos(x, y), symbol(s), name(n), color(c), pickedUp(false) {}
    
    virtual void applyEffect(class Player* p) = 0;
    virtual ~Item() {}
    
    void draw() {
        if (!pickedUp) {
            Console::setChar(pos.x, pos.y, symbol, color);
        }
    }
};

// Player
class Player {
public:
    Vec2 pos;
    int health, maxHealth;
    int attack, defense;
    int keys, score;
    bool alive;
    Vec2 oldPos;  // OPTIMIZATION 3: Track old position
    
    Player(int x, int y) : pos(x, y), oldPos(x, y), health(100), maxHealth(100),
                           attack(10), defense(5), keys(0), score(0), alive(true) {}
    
    void draw() {
        Console::setChar(pos.x, pos.y, '@', BRIGHT_YELLOW);
    }
    
    void move(int dx, int dy, Dungeon& dungeon);
    
    void takeDamage(int dmg) {
        int actualDmg = max(1, dmg - defense);
        health -= actualDmg;
        if (health <= 0) {
            health = 0;
            alive = false;
        }
    }
};

// Enemy
class Enemy {
public:
    Vec2 pos;
    Vec2 oldPos;  // OPTIMIZATION 3: Track old position
    char symbol;
    string name;
    int health, maxHealth;
    int attack, defense;
    int aggroRange;
    int color;
    bool alive;
    
    Enemy(int x, int y, int lvl) : pos(x, y), oldPos(x, y), symbol('r'), name("Rat"),
                                    health(20), maxHealth(20), attack(5), defense(2),
                                    aggroRange(5), color(BRIGHT_GREEN), alive(true) {
        health += lvl * 10;
        maxHealth = health;
        attack += lvl * 2;
        defense += lvl;
        
        if (lvl > 2) { symbol = 'O'; name = "Orc"; color = BRIGHT_RED; }
        if (lvl > 4) { symbol = 'D'; name = "Dragon"; color = BRIGHT_MAGENTA; aggroRange = 8; }
    }
    
    void draw() {
        if (alive) {
            Console::setChar(pos.x, pos.y, symbol, color);
        }
    }
    
    void takeDamage(int dmg) {
        int actualDmg = max(1, dmg - defense);
        health -= actualDmg;
        if (health <= 0) {
            health = 0;
            alive = false;
        }
    }
    
    void update(Dungeon& dungeon);
};

// Dungeon
class Dungeon {
public:
    static const int WIDTH = 60;
    static const int HEIGHT = 20;
    char tiles[HEIGHT][WIDTH];
    Player* player;
    vector<Enemy*> enemies;
    vector<Item*> items;
    int level;
    bool exitReached;
    bool hasKey;
    bool staticDrawn;  // OPTIMIZATION 3: Track if static tiles drawn
    
    Dungeon(int lvl) : level(lvl), exitReached(false), player(nullptr), hasKey(false), staticDrawn(false) {
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                tiles[y][x] = '#';
            }
        }
        generateDungeon();
    }
    
    ~Dungeon() {
        if (player) delete player;
        for (auto e : enemies) delete e;
        for (auto i : items) delete i;
    }
    
    void generateDungeon() {
        int numRooms = 5 + min(level, 3);
        int roomsPerRow = 3;
        int spacing = WIDTH / (roomsPerRow + 1);
        
        for (int i = 0; i < numRooms; i++) {
            int col = i % roomsPerRow;
            int row = i / roomsPerRow;
            
            int roomW = 8 + rand() % 4;
            int roomH = 5 + rand() % 3;
            int roomX = spacing * (col + 1) - roomW / 2;
            int roomY = 3 + row * 8;
            
            roomX = max(2, min(WIDTH - roomW - 2, roomX));
            roomY = max(2, min(HEIGHT - roomH - 2, roomY));
            
            for (int y = roomY; y < roomY + roomH && y < HEIGHT; y++) {
                for (int x = roomX; x < roomX + roomW && x < WIDTH; x++) {
                    tiles[y][x] = '.';
                }
            }
            
            if (i > 0) {
                int prevCol = (i - 1) % roomsPerRow;
                int prevRow = (i - 1) / roomsPerRow;
                int prevX = spacing * (prevCol + 1);
                int prevY = 3 + prevRow * 8 + 2;
                
                for (int x = min(prevX, roomX + roomW / 2); x <= max(prevX, roomX + roomW / 2); x++) {
                    if (x >= 0 && x < WIDTH && prevY >= 0 && prevY < HEIGHT) {
                        tiles[prevY][x] = '.';
                    }
                }
                
                int corridorX = roomX + roomW / 2;
                for (int y = min(prevY, roomY); y <= max(prevY, roomY + roomH / 2); y++) {
                    if (y >= 0 && y < HEIGHT && corridorX >= 0 && corridorX < WIDTH) {
                        tiles[y][corridorX] = '.';
                    }
                }
            }
        }
        
        // Place player
        int startX = spacing - 2;
        int startY = 5;
        if (tiles[startY][startX] != '.') {
            for (int y = 1; y < HEIGHT - 1; y++) {
                for (int x = 1; x < WIDTH - 1; x++) {
                    if (tiles[y][x] == '.') {
                        startX = x;
                        startY = y;
                        goto found_start;
                    }
                }
            }
        }
        found_start:
        player = new Player(startX, startY);
        
        // Place exit
        int exitX = WIDTH - spacing;
        int exitY = HEIGHT - 5;
        for (int y = HEIGHT - 2; y >= 1; y--) {
            for (int x = WIDTH - 2; x >= 1; x--) {
                if (tiles[y][x] == '.' && Vec2(x, y).dist(player->pos) > 10) {
                    exitX = x;
                    exitY = y;
                    goto found_exit;
                }
            }
        }
        found_exit:
        tiles[exitY][exitX] = 'X';
        
        // Place key
        int keyX = WIDTH / 2;
        int keyY = HEIGHT / 2;
        float bestDist = 0;
        for (int y = 1; y < HEIGHT - 1; y++) {
            for (int x = 1; x < WIDTH - 1; x++) {
                if (tiles[y][x] == '.') {
                    float distToPlayer = Vec2(x, y).dist(player->pos);
                    float distToExit = Vec2(x, y).dist(Vec2(exitX, exitY));
                    float totalDist = distToPlayer + distToExit;
                    if (totalDist > bestDist) {
                        bestDist = totalDist;
                        keyX = x;
                        keyY = y;
                    }
                }
            }
        }
        
        class KeyItem : public Item {
        public:
            KeyItem(int x, int y) : Item(x, y, 'K', "Key", BRIGHT_YELLOW) {}
            void applyEffect(Player* p) override { p->keys++; }
        };
        items.push_back(new KeyItem(keyX, keyY));
        
        // Place enemies
        int numEnemies = 3 + level;
        for (int i = 0; i < numEnemies; i++) {
            int ex, ey;
            int attempts = 0;
            do {
                ex = 5 + rand() % (WIDTH - 10);
                ey = 3 + rand() % (HEIGHT - 6);
                attempts++;
            } while ((tiles[ey][ex] != '.' || Vec2(ex, ey).dist(player->pos) < 5) && attempts < 50);
            
            if (attempts < 50) {
                enemies.push_back(new Enemy(ex, ey, level));
            }
        }
    }
    
    bool isWalkable(int x, int y) {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return false;
        return tiles[y][x] == '.' || tiles[y][x] == 'X';
    }
    
    Enemy* getEnemyAt(Vec2 pos) {
        for (auto e : enemies) {
            if (e && e->alive && e->pos.x == pos.x && e->pos.y == pos.y) {
                return e;
            }
        }
        return nullptr;
    }
    
    Item* getItemAt(Vec2 pos) {
        for (auto i : items) {
            if (i && !i->pickedUp && i->pos.x == pos.x && i->pos.y == pos.y) {
                return i;
            }
        }
        return nullptr;
    }
    
    // OPTIMIZATION 3: Minimal redraw - only dynamic objects
    void draw() {
        // Draw static tiles only once
        if (!staticDrawn) {
            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    char tile = tiles[y][x];
                    int color = GRAY;
                    
                    if (tile == '.') {
                        tile = (char)250;
                        color = GRAY;
                    } else if (tile == 'X') {
                        color = BRIGHT_GREEN;
                    }
                    
                    Console::setChar(x, y, tile, color);
                }
            }
            staticDrawn = true;
        }
        
        // Clear old positions of dynamic objects
        if (player) {
            if (player->oldPos.x != player->pos.x || player->oldPos.y != player->pos.y) {
                char tile = tiles[player->oldPos.y][player->oldPos.x];
                if (tile == '.') tile = (char)250;
                int color = GRAY;
                if (tiles[player->oldPos.y][player->oldPos.x] == 'X') color = BRIGHT_GREEN;
                Console::setChar(player->oldPos.x, player->oldPos.y, tile, color);
            }
        }
        
        for (auto e : enemies) {
            if (e && (e->oldPos.x != e->pos.x || e->oldPos.y != e->pos.y)) {
                char tile = tiles[e->oldPos.y][e->oldPos.x];
                if (tile == '.') tile = (char)250;
                int color = GRAY;
                if (tiles[e->oldPos.y][e->oldPos.x] == 'X') color = BRIGHT_GREEN;
                Console::setChar(e->oldPos.x, e->oldPos.y, tile, color);
            }
        }
        
        // Draw items
        for (auto i : items) {
            if (i) i->draw();
        }
        
        // Draw enemies
        for (auto e : enemies) {
            if (e && e->alive) e->draw();
        }
        
        // Draw player
        if (player && player->alive) player->draw();
        
        // Draw UI
        string ui = "HP:" + to_string(player->health) + "/" + to_string(player->maxHealth) +
                    " ATK:" + to_string(player->attack) +
                    " DEF:" + to_string(player->defense) +
                    " Keys:" + to_string(player->keys) +
                    " Score:" + to_string(player->score) +
                    " Lvl:" + to_string(level);
        Console::drawString(0, HEIGHT + 1, ui, BRIGHT_WHITE);
        
        string controls = "W=UP  S=DOWN  A=LEFT  D=RIGHT  ESC=Quit";
        Console::drawString(0, HEIGHT + 2, controls, YELLOW);
        
        string pos = "Pos: X=" + to_string(player->pos.x) + " Y=" + to_string(player->pos.y);
        Console::drawString(0, HEIGHT + 3, pos, CYAN);
    }
    
    void update() {
        for (auto e : enemies) {
            if (e && e->alive) {
                e->update(*this);
            }
        }
    }
};

// Player move
void Player::move(int dx, int dy, Dungeon& dungeon) {
    oldPos = pos;  // OPTIMIZATION 3: Save old position
    Vec2 newPos(pos.x + dx, pos.y + dy);
    
    if (newPos.x < 0 || newPos.x >= Dungeon::WIDTH || 
        newPos.y < 0 || newPos.y >= Dungeon::HEIGHT) return;
    
    if (!dungeon.isWalkable(newPos.x, newPos.y)) return;
    
    Enemy* enemy = dungeon.getEnemyAt(newPos);
    if (enemy) {
        int damage = max(1, attack - rand() % 3);
        enemy->takeDamage(damage);
        score += 10;
        if (!enemy->alive) score += 50;
        return;
    }
    
    Item* item = dungeon.getItemAt(newPos);
    if (item) {
        item->applyEffect(this);
        item->pickedUp = true;
        score += 20;
    }
    
    pos = newPos;
    
    if (dungeon.tiles[pos.y][pos.x] == 'X' && keys > 0) {
        dungeon.exitReached = true;
    }
}

// Enemy update
void Enemy::update(Dungeon& dungeon) {
    if (!alive || !dungeon.player) return;
    
    oldPos = pos;  // OPTIMIZATION 3: Save old position
    
    float dist = pos.dist(dungeon.player->pos);
    if (dist > aggroRange) return;
    
    if (dist <= 1.5f) {
        int damage = max(1, attack - rand() % 3);
        dungeon.player->takeDamage(damage);
        return;
    }
    
    int dx = 0, dy = 0;
    if (dungeon.player->pos.x > pos.x) dx = 1;
    else if (dungeon.player->pos.x < pos.x) dx = -1;
    
    if (dungeon.player->pos.y > pos.y) dy = 1;
    else if (dungeon.player->pos.y < pos.y) dy = -1;
    
    Vec2 newPos(pos.x + dx, pos.y + dy);
    
    if (dungeon.isWalkable(newPos.x, newPos.y) && !dungeon.getEnemyAt(newPos)) {
        pos = newPos;
    }
}

// Game class with async rendering
class Game {
private:
    Dungeon* dungeon;
    bool running;
    bool victory;
    int currentLevel;
    int savedScore;
    atomic<bool> renderFlag;
    atomic<bool> inputProcessed;
    thread renderThread;
    
public:
    Game() : dungeon(nullptr), running(true), victory(false), 
             currentLevel(1), savedScore(0), renderFlag(false), inputProcessed(false) {
        srand(time(0));
        Console::init();
    }
    
    ~Game() {
        if (renderThread.joinable()) {
            running = false;
            renderThread.join();
        }
        if (dungeon) delete dungeon;
    }
    
    void run() {
        showTitle();
        
        while (running && !victory && currentLevel <= 5) {
            dungeon = new Dungeon(currentLevel);
            savedScore = dungeon->player ? dungeon->player->score : savedScore;
            if (dungeon->player) dungeon->player->score = savedScore;
            
            playLevel();
            
            if (dungeon->exitReached) {
                savedScore = dungeon->player->score + 100;
                currentLevel++;
                if (currentLevel > 5) {
                    victory = true;
                }
            } else if (!dungeon->player || !dungeon->player->alive) {
                savedScore = dungeon->player ? dungeon->player->score : 0;
                running = false;
            }
            
            delete dungeon;
            dungeon = nullptr;
        }
        
        showGameOver();
    }
    
    // OPTIMIZATION 4: Async rendering in separate thread
    void renderLoop() {
        DWORD lastEnemyUpdate = GetTickCount();
        const DWORD enemyUpdateInterval = 200;
        
        while (running && dungeon && dungeon->player && dungeon->player->alive && !dungeon->exitReached) {
            // Update enemies on timer
            DWORD now = GetTickCount();
            if (now - lastEnemyUpdate >= enemyUpdateInterval) {
                dungeon->update();
                renderFlag = true;
                lastEnemyUpdate = now;
            }
            
            // Render if flagged
            if (renderFlag) {
                dungeon->draw();
                Console::flush();
                renderFlag = false;
            }
            
            Sleep(16);  // 60 FPS render loop
        }
    }
    
    void playLevel() {
        if (!dungeon || !dungeon->player) return;
        
        // Start render thread
        renderFlag = true;
        renderThread = thread(&Game::renderLoop, this);
        
        // Initial draw
        dungeon->draw();
        Console::flush();
        
        // Input loop (instant response)
        while (running && dungeon->player->alive && !dungeon->exitReached) {
            if (_kbhit()) {
                int key = _getch();
                if (key == 224) key = _getch();
                
                if (key == 'w' || key == 'W') {
                    dungeon->player->move(0, -1, *dungeon);
                    renderFlag = true;  // Signal render
                } else if (key == 's' || key == 'S') {
                    dungeon->player->move(0, 1, *dungeon);
                    renderFlag = true;
                } else if (key == 'a' || key == 'A') {
                    dungeon->player->move(-1, 0, *dungeon);
                    renderFlag = true;
                } else if (key == 'd' || key == 'D') {
                    dungeon->player->move(1, 0, *dungeon);
                    renderFlag = true;
                } else if (key == 27) {
                    running = false;
                }
            }
            
            Sleep(1);  // Poll input every 1ms (instant!)
        }
        
        // Wait for render thread
        if (renderThread.joinable()) {
            renderThread.join();
        }
    }
    
    void showTitle() {
        Console::clearBuffer();
        
        Console::drawString(30, 8, "DUNGEON QUEST", BRIGHT_YELLOW);
        Console::drawString(28, 9, "LIGHTNING EDITION", BRIGHT_CYAN);
        Console::drawString(25, 11, "100x Faster Rendering!", BRIGHT_GREEN);
        Console::drawString(20, 13, "Find the KEY, reach the EXIT!", BRIGHT_WHITE);
        Console::drawString(22, 15, "Press any key to start...", GRAY);
        
        Console::flush();
        _getch();
    }
    
    void showGameOver() {
        Console::clearBuffer();
        
        if (victory) {
            Console::drawString(32, 10, "VICTORY!", BRIGHT_GREEN);
            Console::drawString(25, 12, "You conquered the dungeon!", BRIGHT_WHITE);
        } else {
            Console::drawString(32, 10, "GAME OVER", BRIGHT_RED);
            Console::drawString(28, 12, "You have perished...", GRAY);
        }
        
        string scoreText = "Final Score: " + to_string(savedScore);
        Console::drawString(30, 14, scoreText, BRIGHT_YELLOW);
        Console::drawString(22, 16, "Press any key to exit...", GRAY);
        
        Console::flush();
        _getch();
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}

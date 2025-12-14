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
        for (size_t i = 0; i < str.length() && (x + i) < 80; i++) {
            if (y >= 0 && y < 30) {
                screenBuffer[y][x + i] = str[i];
                colorBuffer[y][x + i] = color;
            }
        }
        dirty = true;
    }
    
    static void flush() {
        if (!dirty) return;
        
        lock_guard<mutex> lock(bufferMutex);
        
        bool hasChanges = false;
        
        for (int y = 0; y < 30; y++) {
            for (int x = 0; x < 80; x++) {
                int idx = y * 80 + x;
                
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
        return sqrt((float)(dx * dx + dy * dy));
    }
};

// Forward declarations
class Dungeon;
class OpenWorld;

// ============================================================
// MAGIC BOLT SYSTEM (Ranged spell casting)
// ============================================================

class MagicBolt {
public:
    Vec2 pos;
    Vec2 oldPos;
    int dx, dy;
    int power;
    bool active;
    char symbol;
    int color;
    int maxDist;
    int traveled;
    int speed;
    bool hasSplash;
    
    MagicBolt(int x, int y, int dirX, int dirY, int pwr = 15, int spd = 1, bool splash = false) 
        : pos(x, y), oldPos(x, y), dx(dirX), dy(dirY), power(pwr), 
          active(true), maxDist(20), traveled(0), speed(spd), hasSplash(splash) {
        if (dx != 0 && dy == 0) symbol = '-';
        else if (dx == 0 && dy != 0) symbol = '|';
        else symbol = '*';
        color = BRIGHT_YELLOW;
    }
    
    void update() {
        if (!active) return;
        oldPos = pos;
        // Clear previous path positions
        pathPositions.clear();
        
        // Move multiple times based on speed, recording each position
        for (int i = 0; i < speed; i++) {
            pos.x += dx;
            pos.y += dy;
            pathPositions.push_back(pos);  // Record this position for collision checking
            traveled++;
            if (traveled >= maxDist) {
                active = false;
                break;
            }
        }
    }
    
    // Get all positions the spell passed through this update
    vector<Vec2>& getPathPositions() {
        return pathPositions;
    }
    
    void draw(int offsetX = 0, int offsetY = 0) {
        if (!active) return;
        int screenX = pos.x - offsetX;
        int screenY = pos.y - offsetY;
        if (screenX >= 0 && screenX < 60 && screenY >= 0 && screenY < 20) {
            Console::setChar(screenX, screenY, symbol, color);
        }
    }
    
private:
    vector<Vec2> pathPositions;
};

// ============================================================
// BARRIER OBJECTS (can be broken with spells)
// ============================================================

class BarrierObject {
public:
    Vec2 pos;
    char symbol;
    int durability;
    int maxDurability;
    int color;
    bool broken;
    string name;
    
    BarrierObject(int x, int y, char s, int hp, string n, int c = BRIGHT_RED) 
        : pos(x, y), symbol(s), durability(hp), maxDurability(hp), color(c), broken(false), name(n) {}
    
    void weaken(int amount) {
        durability -= amount;
        if (durability <= 0) {
            broken = true;
        }
    }
    
    void draw(int offsetX = 0, int offsetY = 0) {
        if (broken) return;
        int screenX = pos.x - offsetX;
        int screenY = pos.y - offsetY;
        if (screenX >= 0 && screenX < 60 && screenY >= 0 && screenY < 20) {
            Console::setChar(screenX, screenY, symbol, color);
        }
    }
};

// ============================================================
// GAME ENTITIES
// ============================================================

class Player {
public:
    Vec2 pos;
    Vec2 oldPos;
    int health, maxHealth;
    int attack, defense;
    int keys, score;
    int gold;
    int mana;
    bool alive;
    bool hasWand;
    
    // Spell upgrades
    int spellDamage;
    int spellSpeed;
    bool tripleShot;
    bool splashDamage;
    
    Player(int x, int y) : pos(x, y), oldPos(x, y), health(100), maxHealth(100),
                           attack(10), defense(5), keys(0), score(0), gold(0), 
                           mana(0), alive(true), hasWand(false),
                           spellDamage(15), spellSpeed(1), tripleShot(false), splashDamage(false) {}
    
    void draw(int offsetX = 0, int offsetY = 0) {
        int screenX = pos.x - offsetX;
        int screenY = pos.y - offsetY;
        if (screenX >= 0 && screenX < 60 && screenY >= 0 && screenY < 20) {
            Console::setChar(screenX, screenY, '@', BRIGHT_YELLOW);
        }
    }
    
    void reduceHealth(int dmg) {
        int actualDmg = max(1, dmg - defense);
        health -= actualDmg;
        if (health <= 0) {
            health = 0;
            alive = false;
        }
    }
    
    void heal(int amount) {
        health = min(maxHealth, health + amount);
    }
};

class Enemy {
public:
    Vec2 pos;
    Vec2 oldPos;
    char symbol;
    string name;
    int health, maxHealth;
    int attack, defense;
    int aggroRange;
    int color;
    bool alive;
    int goldDrop;
    
    Enemy(int x, int y, int lvl, bool isOverworld = false) 
        : pos(x, y), oldPos(x, y), symbol('r'), name("Rat"),
          health(15), maxHealth(15), attack(3), defense(1),
          aggroRange(5), color(BRIGHT_GREEN), alive(true), goldDrop(5) {
        
        if (isOverworld) {
            // Overworld enemies
            int type = rand() % 5;
            switch(type) {
                case 0: symbol = 'W'; name = "Wolf"; color = GRAY; 
                        health = 25; attack = 6; defense = 2; goldDrop = 10; aggroRange = 6; break;
                case 1: symbol = 'B'; name = "Bandit"; color = BRIGHT_RED;
                        health = 30; attack = 8; defense = 3; goldDrop = 20; aggroRange = 5; break;
                case 2: symbol = 'S'; name = "Snake"; color = BRIGHT_GREEN;
                        health = 15; attack = 10; defense = 1; goldDrop = 8; aggroRange = 4; break;
                case 3: symbol = 'G'; name = "Goblin"; color = GREEN;
                        health = 20; attack = 5; defense = 2; goldDrop = 12; aggroRange = 7; break;
                case 4: symbol = 'T'; name = "Troll"; color = DARK_GRAY;
                        health = 50; attack = 12; defense = 5; goldDrop = 35; aggroRange = 4; break;
            }
        } else {
            // Dungeon enemies (original)
            health += lvl * 5;
            maxHealth = health;
            attack += lvl * 1;
            defense += lvl / 2;
            goldDrop = 5 + lvl * 3;
            
            if (lvl > 2) { symbol = 'O'; name = "Orc"; color = BRIGHT_RED; goldDrop = 15; }
            if (lvl > 4) { symbol = 'D'; name = "Dragon"; color = BRIGHT_MAGENTA; aggroRange = 8; goldDrop = 50; }
        }
        maxHealth = health;
    }
    
    void draw(int offsetX = 0, int offsetY = 0) {
        if (alive) {
            int screenX = pos.x - offsetX;
            int screenY = pos.y - offsetY;
            if (screenX >= 0 && screenX < 60 && screenY >= 0 && screenY < 20) {
                Console::setChar(screenX, screenY, symbol, color);
            }
        }
    }
    
    void reduceHealth(int dmg) {
        int actualDmg = max(1, dmg - defense);
        health -= actualDmg;
        if (health <= 0) {
            health = 0;
            alive = false;
        }
    }
};

class Item {
public:
    Vec2 pos;
    char symbol;
    string name;
    int color;
    bool pickedUp;
    
    Item(int x, int y, char s, string n, int c)
        : pos(x, y), symbol(s), name(n), color(c), pickedUp(false) {}
    
    virtual void applyEffect(Player* p) = 0;
    virtual ~Item() {}
    
    void draw(int offsetX = 0, int offsetY = 0) {
        if (!pickedUp) {
            int screenX = pos.x - offsetX;
            int screenY = pos.y - offsetY;
            if (screenX >= 0 && screenX < 60 && screenY >= 0 && screenY < 20) {
                Console::setChar(screenX, screenY, symbol, color);
            }
        }
    }
};

class HealthPotion : public Item {
public:
    HealthPotion(int x, int y) : Item(x, y, 'H', "Health Potion", BRIGHT_RED) {}
    void applyEffect(Player* p) override { p->heal(50); }
};

class KeyItem : public Item {
public:
    KeyItem(int x, int y) : Item(x, y, 'K', "Key", BRIGHT_YELLOW) {}
    void applyEffect(Player* p) override { p->keys++; }
};

class GoldPile : public Item {
public:
    int amount;
    GoldPile(int x, int y, int amt = 10) : Item(x, y, '$', "Gold", BRIGHT_YELLOW), amount(amt) {}
    void applyEffect(Player* p) override { p->gold += amount; }
};

class ManaCrystal : public Item {
public:
    int amount;
    ManaCrystal(int x, int y, int amt = 6) : Item(x, y, 'a', "Mana Crystal", BRIGHT_CYAN), amount(amt) {}
    void applyEffect(Player* p) override { p->mana += amount; }
};

class WandItem : public Item {
public:
    WandItem(int x, int y) : Item(x, y, 'W', "Ancient Wand", BRIGHT_MAGENTA) {}
    void applyEffect(Player* p) override { 
        p->hasWand = true; 
        p->mana += 12;
    }
};

// NPC for towns
class NPC {
public:
    Vec2 pos;
    char symbol;
    string name;
    string dialogue;
    int color;
    bool isShop;
    
    NPC(int x, int y, string n, string d, bool shop = false) 
        : pos(x, y), symbol('P'), name(n), dialogue(d), color(BRIGHT_CYAN), isShop(shop) {
        if (shop) { symbol = 'M'; color = BRIGHT_MAGENTA; }
    }
    
    void draw(int offsetX = 0, int offsetY = 0) {
        int screenX = pos.x - offsetX;
        int screenY = pos.y - offsetY;
        if (screenX >= 0 && screenX < 60 && screenY >= 0 && screenY < 20) {
            Console::setChar(screenX, screenY, symbol, color);
        }
    }
};

// ============================================================
// DUNGEON (Original 5 levels)
// ============================================================

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
    bool staticDrawn;
    
    Dungeon(int lvl) : level(lvl), exitReached(false), player(nullptr), hasKey(false), staticDrawn(false) {
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                tiles[y][x] = '#';
            }
        }
        generateDungeon();
    }
    
    ~Dungeon() {
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
        
        // Place player start
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
        items.push_back(new KeyItem(keyX, keyY));
        
        // Place healing potions
        int numPotions = 3 + level;
        for (int i = 0; i < numPotions; i++) {
            int px, py;
            int attempts = 0;
            do {
                px = 5 + rand() % (WIDTH - 10);
                py = 3 + rand() % (HEIGHT - 6);
                attempts++;
            } while ((tiles[py][px] != '.' || Vec2(px, py).dist(player->pos) < 3) && attempts < 50);
            
            if (attempts < 50) {
                items.push_back(new HealthPotion(px, py));
            }
        }
        
        // Place gold
        int numGold = 2 + level;
        for (int i = 0; i < numGold; i++) {
            int gx, gy;
            int attempts = 0;
            do {
                gx = 5 + rand() % (WIDTH - 10);
                gy = 3 + rand() % (HEIGHT - 6);
                attempts++;
            } while ((tiles[gy][gx] != '.' || Vec2(gx, gy).dist(player->pos) < 3) && attempts < 50);
            
            if (attempts < 50) {
                items.push_back(new GoldPile(gx, gy, 5 + rand() % 15));
            }
        }
        
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
                enemies.push_back(new Enemy(ex, ey, level, false));
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
    
    void draw() {
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
        
        // Clear old positions
        if (player) {
            char tile = tiles[player->oldPos.y][player->oldPos.x];
            if (tile == '.') tile = (char)250;
            int color = GRAY;
            if (tiles[player->oldPos.y][player->oldPos.x] == 'X') color = BRIGHT_GREEN;
            Console::setChar(player->oldPos.x, player->oldPos.y, tile, color);
        }
        
        for (auto e : enemies) {
            if (e) {
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
                    " Gold:" + to_string(player->gold) +
                    " Lvl:" + to_string(level);
        Console::drawString(0, HEIGHT + 1, ui, BRIGHT_WHITE);
        
        string controls = "WASD=Move  ESC=Quit  [DUNGEON " + to_string(level) + "/5]";
        Console::drawString(0, HEIGHT + 2, controls, YELLOW);
    }
    
    void update() {
        for (auto e : enemies) {
            if (e && e->alive && player) {
                e->oldPos = e->pos;
                
                float dist = e->pos.dist(player->pos);
                if (dist > e->aggroRange) continue;
                
                if (dist <= 1.5f) {
                    int damage = max(1, e->attack - rand() % 3);
                    player->reduceHealth(damage);
                    continue;
                }
                
                int dx = 0, dy = 0;
                if (player->pos.x > e->pos.x) dx = 1;
                else if (player->pos.x < e->pos.x) dx = -1;
                
                if (player->pos.y > e->pos.y) dy = 1;
                else if (player->pos.y < e->pos.y) dy = -1;
                
                Vec2 newPos(e->pos.x + dx, e->pos.y + dy);
                
                if (isWalkable(newPos.x, newPos.y) && !getEnemyAt(newPos)) {
                    e->pos = newPos;
                }
            }
        }
    }
    
    void playerMove(int dx, int dy) {
        if (!player) return;
        player->oldPos = player->pos;
        Vec2 newPos(player->pos.x + dx, player->pos.y + dy);
        
        if (!isWalkable(newPos.x, newPos.y)) return;
        
        Enemy* enemy = getEnemyAt(newPos);
        if (enemy) {
            int damage = max(1, player->attack - rand() % 3);
            enemy->reduceHealth(damage);
            player->score += 10;
            if (!enemy->alive) {
                player->score += 50;
                player->gold += enemy->goldDrop;
            }
            return;
        }
        
        Item* item = getItemAt(newPos);
        if (item) {
            item->applyEffect(player);
            item->pickedUp = true;
            player->score += 20;
        }
        
        player->pos = newPos;
        
        if (tiles[player->pos.y][player->pos.x] == 'X' && player->keys > 0) {
            exitReached = true;
        }
    }
};

// ============================================================
// OPEN WORLD (After dungeon escape)
// ============================================================

class OpenWorld {
public:
    static const int WIDTH = 200;
    static const int HEIGHT = 100;
    static const int VIEW_WIDTH = 60;
    static const int VIEW_HEIGHT = 20;
    
    char tiles[HEIGHT][WIDTH];
    Player* player;
    vector<Enemy*> enemies;
    vector<Item*> items;
    vector<NPC*> npcs;
    vector<MagicBolt*> magicbolts;
    vector<BarrierObject*> barriers;
    
    int cameraX, cameraY;
    string currentMessage;
    int messageTimer;
    bool questComplete;
    bool forgeUnlocked;
    Vec2 forgeLocation;
    bool inUpgradeMenu;
    
    OpenWorld() : player(nullptr), cameraX(0), cameraY(0), messageTimer(0), 
                  questComplete(false), forgeUnlocked(false), forgeLocation(0,0), inUpgradeMenu(false) {
        // Pre-allocate vectors to avoid suspicious reallocation patterns
        enemies.reserve(64);
        items.reserve(64);
        npcs.reserve(16);
        magicbolts.reserve(32);
        barriers.reserve(8);
        generateWorld();
    }
    
    ~OpenWorld() {
        for (auto e : enemies) delete e;
        for (auto i : items) delete i;
        for (auto n : npcs) delete n;
        for (auto p : magicbolts) delete p;
        for (auto d : barriers) delete d;
    }
    
    void generateWorld() {
        // Fill with grass
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                tiles[y][x] = '.';
            }
        }
        
        // Generate forests (trees)
        for (int i = 0; i < 15; i++) {
            int fx = 10 + rand() % (WIDTH - 20);
            int fy = 10 + rand() % (HEIGHT - 20);
            int fw = 8 + rand() % 12;
            int fh = 6 + rand() % 8;
            
            for (int y = fy; y < fy + fh && y < HEIGHT; y++) {
                for (int x = fx; x < fx + fw && x < WIDTH; x++) {
                    if (rand() % 3 != 0) {
                        tiles[y][x] = 'T';  // Tree
                    }
                }
            }
        }
        
        // Generate mountains
        for (int i = 0; i < 8; i++) {
            int mx = 20 + rand() % (WIDTH - 40);
            int my = 20 + rand() % (HEIGHT - 40);
            int mw = 10 + rand() % 15;
            int mh = 6 + rand() % 10;
            
            for (int y = my; y < my + mh && y < HEIGHT; y++) {
                for (int x = mx; x < mx + mw && x < WIDTH; x++) {
                    if (rand() % 4 != 0) {
                        tiles[y][x] = '^';  // Mountain
                    }
                }
            }
        }
        
        // Generate water/lakes
        for (int i = 0; i < 5; i++) {
            int wx = 15 + rand() % (WIDTH - 30);
            int wy = 15 + rand() % (HEIGHT - 30);
            int ww = 6 + rand() % 10;
            int wh = 4 + rand() % 6;
            
            for (int y = wy; y < wy + wh && y < HEIGHT; y++) {
                for (int x = wx; x < wx + ww && x < WIDTH; x++) {
                    tiles[y][x] = '~';  // Water
                }
            }
        }
        
        // Generate paths/roads
        // Main road from dungeon exit to town
        int roadY = HEIGHT / 2;
        for (int x = 0; x < WIDTH; x++) {
            tiles[roadY][x] = '_';
            if (roadY > 1) tiles[roadY-1][x] = '_';
        }
        
        // Vertical road to second town
        int roadX = WIDTH * 3 / 4;
        for (int y = 10; y < HEIGHT - 10; y++) {
            tiles[y][roadX] = '_';
        }
        
        // Create starting village (near dungeon exit)
        createVillage(15, HEIGHT/2 - 5, "Dungeon's End");
        
        // Create main town
        createVillage(WIDTH/2, HEIGHT/2 - 8, "Oakvale");
        
        // Create second town
        createVillage(WIDTH * 3/4, 20, "Northwatch");
        
        // Place dungeon entrance marker (where player emerges)
        tiles[HEIGHT/2][5] = 'E';  // Exit from dungeon
        
        // Place player at dungeon exit
        player = new Player(8, HEIGHT/2);
        
        // Spawn enemies throughout the world
        for (int i = 0; i < 40; i++) {
            int ex, ey;
            int attempts = 0;
            do {
                ex = 10 + rand() % (WIDTH - 20);
                ey = 10 + rand() % (HEIGHT - 20);
                attempts++;
            } while (!isWalkable(ex, ey) && attempts < 50);
            
            if (attempts < 50 && Vec2(ex, ey).dist(player->pos) > 10) {
                enemies.push_back(new Enemy(ex, ey, 1, true));
            }
        }
        
        // Scatter gold and items
        for (int i = 0; i < 30; i++) {
            int ix, iy;
            int attempts = 0;
            do {
                ix = 10 + rand() % (WIDTH - 20);
                iy = 10 + rand() % (HEIGHT - 20);
                attempts++;
            } while (!isWalkable(ix, iy) && attempts < 50);
            
            if (attempts < 50) {
                if (rand() % 3 == 0) {
                    items.push_back(new HealthPotion(ix, iy));
                } else {
                    items.push_back(new GoldPile(ix, iy, 10 + rand() % 30));
                }
            }
        }
        
        // ============================================
        // ANCIENT PISTOL - In the eastern ruins
        // ============================================
        int artifactX = WIDTH - 30;
        int artifactY = HEIGHT - 30;
        // Clear area for ruins
        for (int y = artifactY; y < artifactY + 7 && y < HEIGHT; y++) {
            for (int x = artifactX; x < artifactX + 7 && x < WIDTH; x++) {
                tiles[y][x] = '.';
            }
        }
        // Ruins walls
        for (int x = artifactX; x < artifactX + 7; x++) {
            tiles[artifactY][x] = '#';
            tiles[artifactY + 6][x] = '#';
        }
        for (int y = artifactY; y < artifactY + 7; y++) {
            tiles[y][artifactX] = '#';
            tiles[y][artifactX + 6] = '#';
        }
        tiles[artifactY + 3][artifactX] = '+';  // Entrance
        
        // Clear approach path to ruins entrance to prevent blocking by terrain
        // Clear a corridor leading to the entrance from the west
        for (int x = artifactX - 10; x < artifactX; x++) {
            if (x >= 0) {
                for (int y = artifactY + 2; y <= artifactY + 4; y++) {
                    if (y >= 0 && y < HEIGHT) {
                        tiles[y][x] = '.';  // Clear grass path to entrance
                    }
                }
            }
        }
        // Also clear a small area around the entrance itself
        for (int dy = -1; dy <= 1; dy++) {
            int clearY = artifactY + 3 + dy;
            if (clearY >= 0 && clearY < HEIGHT && artifactX - 1 >= 0) {
                if (tiles[clearY][artifactX - 1] == 'T' || 
                    tiles[clearY][artifactX - 1] == '^' || 
                    tiles[clearY][artifactX - 1] == '~') {
                    tiles[clearY][artifactX - 1] = '.';
                }
            }
        }
        
        // Place the WAND instead of generic artifact
        items.push_back(new WandItem(artifactX + 3, artifactY + 3));
        
        // ============================================
        // MAGIC FORGE - Far east, requires wand to enter
        // ============================================
        forgeLocation = Vec2(WIDTH - 15, HEIGHT / 2);
        int forgeX = forgeLocation.x;
        int forgeY = forgeLocation.y;
        
        // Create path to forge with mana scattered along
        for (int x = artifactX + 10; x < forgeX; x++) {
            tiles[forgeY][x] = '_';  // Road to forge
            tiles[forgeY - 1][x] = '_';
        }
        
        // Scatter mana crystals along the path to forge
        int pathLength = forgeX - artifactX - 15;
        if (pathLength > 1) {
            for (int i = 0; i < 15; i++) {
                int ax = artifactX + 12 + rand() % pathLength;
                int ay = forgeY - 3 + rand() % 7;
                if (ay >= 0 && ay < HEIGHT && ax >= 0 && ax < WIDTH) {
                    if (tiles[ay][ax] == '.' || tiles[ay][ax] == '_') {
                        items.push_back(new ManaCrystal(ax, ay, 4 + rand() % 5));
                    }
                }
            }
        }
        
        // More enemies in the forge approach area
        int enemySpawnRange = forgeX - artifactX - 12;
        if (enemySpawnRange > 1) {
            for (int i = 0; i < 15; i++) {
                int ex = artifactX + 10 + rand() % enemySpawnRange;
                int ey = forgeY - 8 + rand() % 17;
                if (ey >= 0 && ey < HEIGHT && ex >= 0 && ex < WIDTH) {
                    if (isWalkable(ex, ey)) {
                        enemies.push_back(new Enemy(ex, ey, 1, true));
                    }
                }
            }
        }
        
        // Build the Magic Forge structure
        for (int y = forgeY - 4; y <= forgeY + 4; y++) {
            for (int x = forgeX - 2; x <= forgeX + 6; x++) {
                if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                    tiles[y][x] = '.';
                }
            }
        }
        // Forge walls
        for (int x = forgeX - 2; x <= forgeX + 6; x++) {
            tiles[forgeY - 4][x] = '#';
            tiles[forgeY + 4][x] = '#';
        }
        for (int y = forgeY - 4; y <= forgeY + 4; y++) {
            tiles[y][forgeX - 2] = '#';
            tiles[y][forgeX + 6] = '#';
        }
        // Forge door (destructible) - marked with 'D'
        tiles[forgeY][forgeX - 2] = 'D';
        barriers.push_back(new BarrierObject(forgeX - 2, forgeY, 'D', 50, "Forge Door", BRIGHT_RED));
        
        // Inside forge - quest completion marker
        tiles[forgeY][forgeX + 3] = 'F';  // Forge itself
        
        // Add Forge NPC inside (but player can't reach until door destroyed)
        npcs.push_back(new NPC(forgeX + 2, forgeY, "Wizard", "Welcome! The Magic Forge is yours!", false));
        
        currentMessage = "You've escaped the dungeon! Explore the world and find the Ancient Wand!";
        messageTimer = 200;
    }
    
    void createVillage(int cx, int cy, const string& name) {
        // Clear area for village
        for (int y = cy - 3; y <= cy + 3; y++) {
            for (int x = cx - 5; x <= cx + 5; x++) {
                if (y >= 0 && y < HEIGHT && x >= 0 && x < WIDTH) {
                    tiles[y][x] = '.';
                }
            }
        }
        
        // Place buildings (walls)
        // Inn
        for (int y = cy - 2; y <= cy; y++) {
            tiles[y][cx - 4] = '#';
            tiles[y][cx - 1] = '#';
        }
        tiles[cy - 2][cx - 3] = '#';
        tiles[cy - 2][cx - 2] = '#';
        tiles[cy][cx - 2] = '+';  // Door
        
        // Shop
        for (int y = cy - 2; y <= cy; y++) {
            tiles[y][cx + 1] = '#';
            tiles[y][cx + 4] = '#';
        }
        tiles[cy - 2][cx + 2] = '#';
        tiles[cy - 2][cx + 3] = '#';
        tiles[cy][cx + 2] = '+';  // Door
        
        // Add NPCs
        npcs.push_back(new NPC(cx - 3, cy + 1, "Innkeeper", "Welcome, traveler! Rest here to restore health.", false));
        npcs.push_back(new NPC(cx + 3, cy + 1, "Merchant", "Buy supplies! Mana Crystal: 20g", true));
        
        if (name == "Oakvale") {
            npcs.push_back(new NPC(cx, cy + 2, "Elder", "Seek the Ancient Wand in the eastern ruins!", false));
        }
    }
    
    bool isWalkable(int x, int y) {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return false;
        char tile = tiles[y][x];
        // Check if there's a barrier blocking
        for (auto d : barriers) {
            if (d && !d->broken && d->pos.x == x && d->pos.y == y) {
                return false;
            }
        }
        return tile == '.' || tile == '_' || tile == '+' || tile == 'E' || tile == 'F';
    }
    
    BarrierObject* getBarrierAt(Vec2 pos) {
        for (auto d : barriers) {
            if (d && !d->broken && d->pos.x == pos.x && d->pos.y == pos.y) {
                return d;
            }
        }
        return nullptr;
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
    
    NPC* getNPCAt(Vec2 pos) {
        for (auto n : npcs) {
            if (n && n->pos.x == pos.x && n->pos.y == pos.y) {
                return n;
            }
        }
        return nullptr;
    }
    
    NPC* getNPCAdjacent(Vec2 pos) {
        for (auto n : npcs) {
            if (n && abs(n->pos.x - pos.x) <= 1 && abs(n->pos.y - pos.y) <= 1) {
                return n;
            }
        }
        return nullptr;
    }
    
    void updateCamera() {
        // Center camera on player
        cameraX = player->pos.x - VIEW_WIDTH / 2;
        cameraY = player->pos.y - VIEW_HEIGHT / 2;
        
        // Clamp to world bounds
        cameraX = max(0, min(WIDTH - VIEW_WIDTH, cameraX));
        cameraY = max(0, min(HEIGHT - VIEW_HEIGHT, cameraY));
    }
    
    void draw() {
        updateCamera();
        
        // Draw visible portion of world
        for (int sy = 0; sy < VIEW_HEIGHT; sy++) {
            for (int sx = 0; sx < VIEW_WIDTH; sx++) {
                int wx = cameraX + sx;
                int wy = cameraY + sy;
                
                if (wx >= 0 && wx < WIDTH && wy >= 0 && wy < HEIGHT) {
                    char tile = tiles[wy][wx];
                    int color = GRAY;
                    
                    switch (tile) {
                        case '.': tile = (char)250; color = GREEN; break;      // Grass
                        case 'T': color = BRIGHT_GREEN; break;                  // Tree
                        case '^': color = GRAY; break;                          // Mountain
                        case '~': color = BRIGHT_BLUE; break;                   // Water
                        case '_': tile = (char)176; color = YELLOW; break;      // Road
                        case '#': color = BRIGHT_WHITE; break;                  // Building wall
                        case '+': color = YELLOW; break;                        // Door
                        case 'E': color = BRIGHT_MAGENTA; break;                // Dungeon exit
                        case 'D': color = BRIGHT_RED; break;                    // Destructible door
                        case 'F': color = BRIGHT_YELLOW; tile = 'F'; break;     // Forge
                    }
                    
                    Console::setChar(sx, sy, tile, color);
                }
            }
        }
        
        // Draw barriers
        for (auto d : barriers) {
            if (d && !d->broken) d->draw(cameraX, cameraY);
        }
        
        // Draw items
        for (auto i : items) {
            if (i && !i->pickedUp) i->draw(cameraX, cameraY);
        }
        
        // Draw NPCs
        for (auto n : npcs) {
            if (n) n->draw(cameraX, cameraY);
        }
        
        // Draw enemies
        for (auto e : enemies) {
            if (e && e->alive) e->draw(cameraX, cameraY);
        }
        
        // Draw magic bolts
        for (auto p : magicbolts) {
            if (p && p->active) p->draw(cameraX, cameraY);
        }
        
        // Draw player
        if (player && player->alive) player->draw(cameraX, cameraY);
        
        // Draw UI
        string ui = "HP:" + to_string(player->health) + "/" + to_string(player->maxHealth) +
                    " ATK:" + to_string(player->attack) +
                    " DEF:" + to_string(player->defense) +
                    " Gold:" + to_string(player->gold);
        Console::drawString(0, VIEW_HEIGHT + 1, ui, BRIGHT_WHITE);
        
        // Second line: mana and wand status
        string ui2;
        if (player->hasWand) {
            ui2 = "WAND [Arrows=Cast]  Mana:" + to_string(player->mana) + "  Score:" + to_string(player->score);
        } else {
            ui2 = "No magic  Score:" + to_string(player->score);
        }
        Console::drawString(0, VIEW_HEIGHT + 2, ui2, player->hasWand ? BRIGHT_CYAN : GRAY);
        
        string controls = "WASD=Move  E=Interact  Arrows=Cast Spell  ESC=Quit";
        Console::drawString(0, VIEW_HEIGHT + 3, controls, YELLOW);
        
        // Draw message on line 4
        if (messageTimer > 0 && !inUpgradeMenu) {
            // Clear line first
            Console::drawString(0, VIEW_HEIGHT + 4, string(60, ' '), GRAY);
            Console::drawString(0, VIEW_HEIGHT + 4, currentMessage, BRIGHT_CYAN);
            messageTimer--;
        } else if (!inUpgradeMenu) {
            Console::drawString(0, VIEW_HEIGHT + 4, string(60, ' '), GRAY);
        }
        
        // Mini-map in corner
        drawMiniMap();
        
        // Draw upgrade menu if active
        if (inUpgradeMenu) {
            drawUpgradeMenu();
        }
    }
    
    void drawMiniMap() {
        int mapX = 62;
        int mapY = 1;
        int mapW = 16;
        int mapH = 10;
        
        Console::drawString(mapX, mapY - 1, "== MAP ==", BRIGHT_CYAN);
        
        for (int my = 0; my < mapH; my++) {
            for (int mx = 0; mx < mapW; mx++) {
                int wx = (mx * WIDTH) / mapW;
                int wy = (my * HEIGHT) / mapH;
                
                char c = ' ';
                int col = GRAY;
                
                if (wx >= 0 && wx < WIDTH && wy >= 0 && wy < HEIGHT) {
                    char tile = tiles[wy][wx];
                    if (tile == 'T') { c = (char)176; col = GREEN; }
                    else if (tile == '^') { c = '^'; col = GRAY; }
                    else if (tile == '~') { c = '~'; col = BLUE; }
                    else if (tile == '#') { c = '#'; col = BRIGHT_WHITE; }
                    else if (tile == '_') { c = '-'; col = YELLOW; }
                    else { c = '.'; col = DARK_GRAY; }
                }
                
                Console::setChar(mapX + mx, mapY + my, c, col);
            }
        }
        
        // Player position on map
        int playerMapX = mapX + (player->pos.x * mapW) / WIDTH;
        int playerMapY = mapY + (player->pos.y * mapH) / HEIGHT;
        Console::setChar(playerMapX, playerMapY, '@', BRIGHT_YELLOW);
    }
    
    void update() {
        // Update magic bolts
        for (auto p : magicbolts) {
            if (p && p->active) {
                p->update();
                
                // Check collision at each position along the path
                vector<Vec2>& path = p->getPathPositions();
                for (Vec2& checkPos : path) {
                    if (!p->active) break;  // Already hit something
                    
                    // Check collision with walls
                    if (checkPos.x < 0 || checkPos.x >= WIDTH || checkPos.y < 0 || checkPos.y >= HEIGHT) {
                        p->active = false;
                        break;
                    }
                    char tile = tiles[checkPos.y][checkPos.x];
                    if (tile == '#' || tile == 'T' || tile == '^' || tile == '~') {
                        p->active = false;
                        break;
                    }
                    
                    // Check collision with barriers
                    BarrierObject* barrier = getBarrierAt(checkPos);
                    if (barrier) {
                        barrier->weaken(p->power);
                        p->active = false;
                        if (barrier->broken) {
                            currentMessage = "The " + barrier->name + " has been broken!";
                            messageTimer = 100;
                            tiles[barrier->pos.y][barrier->pos.x] = '.';
                            if (barrier->name == "Forge Door") {
                                forgeUnlocked = true;
                            }
                        }
                        break;
                    }
                    
                    // Check collision with enemies
                    Enemy* enemy = getEnemyAt(checkPos);
                    if (enemy) {
                        enemy->reduceHealth(p->power);
                        p->active = false;
                        currentMessage = "Spell hit " + enemy->name + " for " + to_string(p->power) + "!";
                        messageTimer = 60;
                        player->score += 15;
                        
                        // Splash damage - hit nearby enemies too
                        if (p->hasSplash) {
                            for (auto e : enemies) {
                                if (e && e->alive && e != enemy) {
                                    if (e->pos.dist(checkPos) <= 2.0f) {
                                        int splashPower = p->power / 2;
                                        e->reduceHealth(splashPower);
                                        player->score += 10;
                                    }
                                }
                            }
                        }
                        
                        if (!enemy->alive) {
                            player->score += 50;
                            player->gold += enemy->goldDrop;
                            currentMessage = "Spell defeated " + enemy->name + "! +" + to_string(enemy->goldDrop) + " gold!";
                            messageTimer = 80;
                        }
                        break;
                    }
                }
            }
        }
        
        // Clean up inactive magic bolts
        magicbolts.erase(
            remove_if(magicbolts.begin(), magicbolts.end(), 
                [](MagicBolt* p) { 
                    if (p && !p->active) { delete p; return true; }
                    return false;
                }),
            magicbolts.end()
        );
        
        // Update enemies
        for (auto e : enemies) {
            if (e && e->alive && player) {
                e->oldPos = e->pos;
                
                float dist = e->pos.dist(player->pos);
                if (dist > e->aggroRange) continue;
                
                if (dist <= 1.5f) {
                    int damage = max(1, e->attack - rand() % 3);
                    player->reduceHealth(damage);
                    continue;
                }
                
                // Only move sometimes (slower than player)
                if (rand() % 3 != 0) continue;
                
                int dx = 0, dy = 0;
                if (player->pos.x > e->pos.x) dx = 1;
                else if (player->pos.x < e->pos.x) dx = -1;
                
                if (player->pos.y > e->pos.y) dy = 1;
                else if (player->pos.y < e->pos.y) dy = -1;
                
                Vec2 newPos(e->pos.x + dx, e->pos.y + dy);
                
                if (isWalkable(newPos.x, newPos.y) && !getEnemyAt(newPos)) {
                    e->pos = newPos;
                }
            }
        }
    }
    
    void castSpell(int dx, int dy) {
        if (!player || !player->hasWand) {
            currentMessage = "You don't have a wand!";
            messageTimer = 60;
            return;
        }
        if (player->mana <= 0) {
            currentMessage = "Out of mana!";
            messageTimer = 60;
            return;
        }
        
        player->mana--;
        int startX = player->pos.x + dx;
        int startY = player->pos.y + dy;
        
        // Create spell(s) based on upgrades
        if (player->tripleShot) {
            // Triple shot: one straight, two at angles
            magicbolts.push_back(new MagicBolt(startX, startY, dx, dy, player->spellDamage, player->spellSpeed, player->splashDamage));
            
            // Perpendicular shots
            if (dx != 0) {
                // Horizontal shot, add vertical spread
                magicbolts.push_back(new MagicBolt(startX, startY - 1, dx, dy, player->spellDamage, player->spellSpeed, player->splashDamage));
                magicbolts.push_back(new MagicBolt(startX, startY + 1, dx, dy, player->spellDamage, player->spellSpeed, player->splashDamage));
            } else {
                // Vertical shot, add horizontal spread
                magicbolts.push_back(new MagicBolt(startX - 1, startY, dx, dy, player->spellDamage, player->spellSpeed, player->splashDamage));
                magicbolts.push_back(new MagicBolt(startX + 1, startY, dx, dy, player->spellDamage, player->spellSpeed, player->splashDamage));
            }
        } else {
            magicbolts.push_back(new MagicBolt(startX, startY, dx, dy, player->spellDamage, player->spellSpeed, player->splashDamage));
        }
        
        currentMessage = "Spell cast! Mana: " + to_string(player->mana);
        messageTimer = 30;
    }
    
    void playerMove(int dx, int dy) {
        if (!player) return;
        player->oldPos = player->pos;
        Vec2 newPos(player->pos.x + dx, player->pos.y + dy);
        
        // Check for NPC collision
        NPC* npc = getNPCAt(newPos);
        if (npc) {
            currentMessage = npc->name + ": \"" + npc->dialogue + "\"";
            messageTimer = 150;
            return;
        }
        
        if (!isWalkable(newPos.x, newPos.y)) {
            char tile = tiles[newPos.y][newPos.x];
            if (tile == 'T') {
                currentMessage = "A dense forest blocks your path.";
            } else if (tile == '^') {
                currentMessage = "The mountain is too steep to climb here.";
            } else if (tile == '~') {
                currentMessage = "You can't swim across the water.";
            }
            messageTimer = 80;
            return;
        }
        
        Enemy* enemy = getEnemyAt(newPos);
        if (enemy) {
            int damage = max(1, player->attack - rand() % 3);
            enemy->reduceHealth(damage);
            currentMessage = "You hit the " + enemy->name + " for " + to_string(damage) + " damage!";
            messageTimer = 60;
            player->score += 10;
            if (!enemy->alive) {
                player->score += 50;
                player->gold += enemy->goldDrop;
                currentMessage = "Defeated " + enemy->name + "! +" + to_string(enemy->goldDrop) + " gold!";
                messageTimer = 80;
            }
            return;
        }
        
        Item* item = getItemAt(newPos);
        if (item) {
            bool wasWand = item->name == "Ancient Wand";
            item->applyEffect(player);
            item->pickedUp = true;
            player->score += 20;
            if (wasWand) {
                currentMessage = "FOUND THE ANCIENT WAND! Use Arrow Keys to cast spells!";
                messageTimer = 200;
            } else if (item->name == "Mana Crystal") {
                currentMessage = "Picked up Mana Crystal! Total: " + to_string(player->mana);
                messageTimer = 60;
            } else {
                currentMessage = "Picked up " + item->name + "!";
                messageTimer = 60;
            }
        }
        
        player->pos = newPos;
        
        // Check for reaching the Magic Forge interior
        if (tiles[player->pos.y][player->pos.x] == 'F') {
            questComplete = true;
            currentMessage = "You reached the Magic Forge! QUEST COMPLETE!";
            messageTimer = 300;
            player->score += 1000;
        }
    }
    
    void interact() {
        NPC* npc = getNPCAdjacent(player->pos);
        if (npc) {
            if (npc->isShop) {
                // Shop sells mana
                if (player->gold >= 20) {
                    player->gold -= 20;
                    player->mana += 8;
                    currentMessage = "Bought Mana Crystal! -20 gold, +8 mana";
                } else {
                    currentMessage = "Not enough gold! Need 20g for mana.";
                }
            } else if (npc->name == "Innkeeper") {
                player->health = player->maxHealth;
                currentMessage = "You rest at the inn. Health fully restored!";
            } else if (npc->name == "Merchant" && player->hasWand) {
                // Can buy mana if has wand
                if (player->gold >= 15) {
                    player->gold -= 15;
                    player->mana += 6;
                    currentMessage = "Bought Mana Crystal! -15 gold, +6 mana";
                } else {
                    currentMessage = "Not enough gold! Need 15g for mana.";
                }
            } else if (npc->name == "Wizard") {
                // Open upgrade menu
                inUpgradeMenu = true;
            } else {
                currentMessage = npc->name + ": \"" + npc->dialogue + "\"";
            }
            messageTimer = 120;
        } else {
            currentMessage = "Nothing to interact with here.";
            messageTimer = 60;
        }
    }
    
    void drawUpgradeMenu() {
        // Draw upgrade menu overlay
        int menuX = 10;
        int menuY = 3;
        
        // Draw menu background
        for (int y = menuY; y < menuY + 14; y++) {
            for (int x = menuX; x < menuX + 40; x++) {
                Console::setChar(x, y, ' ', BLACK);
            }
        }
        
        // Border
        for (int x = menuX; x < menuX + 40; x++) {
            Console::setChar(x, menuY, '=', BRIGHT_CYAN);
            Console::setChar(x, menuY + 13, '=', BRIGHT_CYAN);
        }
        
        Console::drawString(menuX + 10, menuY + 1, "SPELL UPGRADES", BRIGHT_YELLOW);
        Console::drawString(menuX + 2, menuY + 2, "Gold: " + to_string(player->gold), BRIGHT_YELLOW);
        
        // Option 1: Triple Shot
        string tripleStatus = player->tripleShot ? "(OWNED)" : "(100g)";
        int tripleColor = player->tripleShot ? GREEN : BRIGHT_WHITE;
        Console::drawString(menuX + 2, menuY + 4, "1. Triple Shot " + tripleStatus, tripleColor);
        
        // Option 2: Damage Upgrade (max 5 tiers: 15 -> 20 -> 25 -> 30 -> 35 -> 40)
        int dmgTier = (player->spellDamage - 15) / 5;
        string dmgStatus = dmgTier >= 5 ? "(MAX)" : "(50g)";
        int dmgColor = dmgTier >= 5 ? GREEN : BRIGHT_WHITE;
        string dmgText = "2. Spell Damage " + to_string(player->spellDamage) + " " + dmgStatus;
        Console::drawString(menuX + 2, menuY + 5, dmgText, dmgColor);
        
        // Option 3: Splash Damage
        string splashStatus = player->splashDamage ? "(OWNED)" : "(75g)";
        int splashColor = player->splashDamage ? GREEN : BRIGHT_WHITE;
        Console::drawString(menuX + 2, menuY + 6, "3. Splash Damage " + splashStatus, splashColor);
        
        // Option 4: Speed Upgrade (max 2 tiers: x1 -> x2 -> x3)
        string speedStatus = player->spellSpeed >= 3 ? "(MAX)" : "(60g)";
        int speedColor = player->spellSpeed >= 3 ? GREEN : BRIGHT_WHITE;
        string speedText = "4. Spell Speed x" + to_string(player->spellSpeed) + " " + speedStatus;
        Console::drawString(menuX + 2, menuY + 7, speedText, speedColor);
        
        Console::drawString(menuX + 2, menuY + 9, "Press 1-4 to buy, ESC to close", GRAY);
        
        if (!currentMessage.empty()) {
            Console::drawString(menuX + 2, menuY + 11, currentMessage, BRIGHT_CYAN);
        }
    }
    
    void handleUpgradeInput(int key) {
        if (key == 27) { // ESC
            inUpgradeMenu = false;
            currentMessage = "";
            return;
        }
        
        switch (key) {
            case '1':
                if (player->tripleShot) {
                    currentMessage = "Already owned!";
                } else if (player->gold >= 100) {
                    player->gold -= 100;
                    player->tripleShot = true;
                    currentMessage = "Triple Shot unlocked!";
                } else {
                    currentMessage = "Need 100 gold!";
                }
                break;
            case '2':
                // Max 5 tiers (15 -> 40)
                if (player->spellDamage >= 40) {
                    currentMessage = "Damage already at maximum!";
                } else if (player->gold >= 50) {
                    player->gold -= 50;
                    player->spellDamage += 5;
                    currentMessage = "Spell damage increased to " + to_string(player->spellDamage) + "!";
                } else {
                    currentMessage = "Need 50 gold!";
                }
                break;
            case '3':
                if (player->splashDamage) {
                    currentMessage = "Already owned!";
                } else if (player->gold >= 75) {
                    player->gold -= 75;
                    player->splashDamage = true;
                    currentMessage = "Splash Damage unlocked!";
                } else {
                    currentMessage = "Need 75 gold!";
                }
                break;
            case '4':
                // Max 2 tiers (x1 -> x2 -> x3)
                if (player->spellSpeed >= 3) {
                    currentMessage = "Speed already at maximum!";
                } else if (player->gold >= 60) {
                    player->gold -= 60;
                    player->spellSpeed++;
                    currentMessage = "Spell speed increased to x" + to_string(player->spellSpeed) + "!";
                } else {
                    currentMessage = "Need 60 gold!";
                }
                break;
        }
        messageTimer = 60;
    }
};

// ============================================================
// MAIN GAME CLASS
// ============================================================

class Game {
private:
    Dungeon* dungeon;
    OpenWorld* world;
    Player* sharedPlayer;
    bool running;
    bool victory;
    bool inOpenWorld;
    int currentLevel;
    int savedScore;
    int savedGold;
    int savedHealth;
    int savedMaxHealth;
    int savedAttack;
    int savedDefense;
    atomic<bool> renderFlag;
    thread renderThread;
    
public:
    Game() : dungeon(nullptr), world(nullptr), sharedPlayer(nullptr),
             running(true), victory(false), inOpenWorld(false),
             currentLevel(1), savedScore(0), savedGold(0),
             savedHealth(100), savedMaxHealth(100), savedAttack(10), savedDefense(5),
             renderFlag(false) {
        srand((unsigned int)time(0));
        Console::init();
    }
    
    ~Game() {
        running = false;
        if (renderThread.joinable()) {
            renderThread.join();
        }
        if (dungeon) delete dungeon;
        if (world) delete world;
    }
    
    void run() {
        showTitle();
        
        // Phase 1: Dungeon levels
        while (running && !inOpenWorld && currentLevel <= 5) {
            dungeon = new Dungeon(currentLevel);
            
            // Transfer stats
            if (dungeon->player) {
                dungeon->player->score = savedScore;
                dungeon->player->gold = savedGold;
                dungeon->player->health = savedHealth;
                dungeon->player->maxHealth = savedMaxHealth;
                dungeon->player->attack = savedAttack;
                dungeon->player->defense = savedDefense;
            }
            
            playDungeonLevel();
            
            if (dungeon->exitReached) {
                // Save stats
                savedScore = dungeon->player->score + 100;
                savedGold = dungeon->player->gold;
                savedHealth = dungeon->player->health;
                savedMaxHealth = dungeon->player->maxHealth;
                savedAttack = dungeon->player->attack;
                savedDefense = dungeon->player->defense;
                
                currentLevel++;
                if (currentLevel > 5) {
                    inOpenWorld = true;
                    showDungeonEscape();
                }
            } else if (!dungeon->player || !dungeon->player->alive) {
                running = false;
            }
            
            delete dungeon;
            dungeon = nullptr;
        }
        
        // Phase 2: Open World
        if (running && inOpenWorld) {
            // Small delay before world creation
            Sleep(100);
            
            try {
                world = new OpenWorld();
            } catch (...) {
                MessageBoxA(NULL, "Crash in OpenWorld constructor", "Debug", MB_OK);
                return;
            }
            
            // Transfer stats to open world player
            if (world && world->player) {
                world->player->score = savedScore;
                world->player->gold = savedGold;
                world->player->health = savedHealth;
                world->player->maxHealth = savedMaxHealth;
                world->player->attack = savedAttack;
                world->player->defense = savedDefense;
            }
            
            playOpenWorld();
            
            if (world->questComplete) {
                victory = true;
                savedScore = world->player->score;
            } else if (!world->player || !world->player->alive) {
                // Can continue from open world on death? For now, game over
            }
            
            delete world;
            world = nullptr;
        }
        
        showGameOver();
    }
    
    void dungeonRenderLoop() {
        DWORD lastEnemyUpdate = GetTickCount();
        const DWORD enemyUpdateInterval = 200;
        
        while (running && dungeon && dungeon->player && dungeon->player->alive && !dungeon->exitReached) {
            // Check dungeon pointer is still valid before any access
            if (!dungeon) break;
            
            DWORD now = GetTickCount();
            if (now - lastEnemyUpdate >= enemyUpdateInterval) {
                if (dungeon && dungeon->player) {
                    dungeon->update();
                    renderFlag = true;
                }
                lastEnemyUpdate = now;
            }
            
            if (renderFlag && dungeon) {
                dungeon->draw();
                Console::flush();
                renderFlag = false;
            }
            
            Sleep(16);
        }
    }
    
    void worldRenderLoop() {
        DWORD lastEnemyUpdate = GetTickCount();
        const DWORD enemyUpdateInterval = 300;  // Slightly slower in open world
        
        while (running && world && world->player && world->player->alive) {
            // Check world pointer is still valid before any access
            if (!world) break;
            DWORD now = GetTickCount();
            if (now - lastEnemyUpdate >= enemyUpdateInterval) {
                if (world && world->player) {
                    world->update();
                    renderFlag = true;
                }
                lastEnemyUpdate = now;
            }
            
            if (renderFlag && world) {
                Console::clearBuffer();
                world->draw();
                Console::flush();
                renderFlag = false;
            }
            
            Sleep(16);
        }
    }
    
    void playDungeonLevel() {
        if (!dungeon || !dungeon->player) return;
        
        renderFlag = true;
        renderThread = thread(&Game::dungeonRenderLoop, this);
        
        dungeon->draw();
        Console::flush();
        
        while (running && dungeon->player->alive && !dungeon->exitReached) {
            if (_kbhit()) {
                int key = _getch();
                if (key == 224) key = _getch();
                
                if (key == 'w' || key == 'W') {
                    dungeon->playerMove(0, -1);
                    renderFlag = true;
                } else if (key == 's' || key == 'S') {
                    dungeon->playerMove(0, 1);
                    renderFlag = true;
                } else if (key == 'a' || key == 'A') {
                    dungeon->playerMove(-1, 0);
                    renderFlag = true;
                } else if (key == 'd' || key == 'D') {
                    dungeon->playerMove(1, 0);
                    renderFlag = true;
                } else if (key == 27) {
                    running = false;
                }
            }
            
            Sleep(1);
        }
        
        // Wait for render thread to finish before returning
        if (renderThread.joinable()) {
            renderThread.join();
        }
        // Small delay to ensure thread cleanup is complete
        Sleep(50);
    }
    
    void playOpenWorld() {
        if (!world || !world->player) return;
        
        renderFlag = true;
        renderThread = thread(&Game::worldRenderLoop, this);
        
        Console::clearBuffer();
        world->draw();
        Console::flush();
        
        while (running && world->player->alive && !world->questComplete) {
            if (_kbhit()) {
                int key = _getch();
                
                // Handle upgrade menu input separately
                if (world->inUpgradeMenu) {
                    if (key == 224 || key == 0) {
                        _getch(); // consume arrow key
                    } else {
                        world->handleUpgradeInput(key);
                    }
                    renderFlag = true;
                    continue;
                }
                
                if (key == 224 || key == 0) {
                    // Arrow keys for shooting
                    int arrowKey = _getch();
                    switch (arrowKey) {
                        case 72: // Up arrow
                            world->castSpell(0, -1);
                            renderFlag = true;
                            break;
                        case 80: // Down arrow
                            world->castSpell(0, 1);
                            renderFlag = true;
                            break;
                        case 75: // Left arrow
                            world->castSpell(-1, 0);
                            renderFlag = true;
                            break;
                        case 77: // Right arrow
                            world->castSpell(1, 0);
                            renderFlag = true;
                            break;
                    }
                } else if (key == 'w' || key == 'W') {
                    world->playerMove(0, -1);
                    renderFlag = true;
                } else if (key == 's' || key == 'S') {
                    world->playerMove(0, 1);
                    renderFlag = true;
                } else if (key == 'a' || key == 'A') {
                    world->playerMove(-1, 0);
                    renderFlag = true;
                } else if (key == 'd' || key == 'D') {
                    world->playerMove(1, 0);
                    renderFlag = true;
                } else if (key == 'e' || key == 'E') {
                    world->interact();
                    renderFlag = true;
                } else if (key == 27) {
                    running = false;
                }
            }
            
            Sleep(1);
        }
        
        if (renderThread.joinable()) {
            renderThread.join();
        }
    }
    
    void showTitle() {
        Console::clearBuffer();
        
        Console::drawString(25, 5, "=============================", BRIGHT_CYAN);
        Console::drawString(25, 6, "       DUNGEON QUEST", BRIGHT_YELLOW);
        Console::drawString(25, 7, "       MAGIC EDITION", BRIGHT_RED);
        Console::drawString(25, 8, "=============================", BRIGHT_CYAN);
        
        Console::drawString(15, 10, "Escape the dungeon, find the Ancient Wand,", BRIGHT_WHITE);
        Console::drawString(15, 11, "and cast your way to the Magic Forge!", BRIGHT_WHITE);
        
        Console::drawString(18, 13, "- 5 dungeon levels to conquer", GRAY);
        Console::drawString(18, 14, "- Vast open world to explore", GRAY);
        Console::drawString(18, 15, "- Find the Ancient Wand in the ruins", BRIGHT_MAGENTA);
        Console::drawString(18, 16, "- Cast spells with Arrow Keys", BRIGHT_CYAN);
        Console::drawString(18, 17, "- Destroy the Forge Door to win!", BRIGHT_RED);
        
        Console::drawString(22, 20, "Press any key to begin...", YELLOW);
        
        Console::flush();
        _getch();
    }
    
    void showDungeonEscape() {
        // Small delay to let system stabilize after dungeon cleanup
        Sleep(100);
        
        Console::clearBuffer();
        
        Console::drawString(25, 7, "=============================", BRIGHT_GREEN);
        Console::drawString(25, 8, "    DUNGEON ESCAPED!", BRIGHT_YELLOW);
        Console::drawString(25, 9, "=============================", BRIGHT_GREEN);
        
        Console::drawString(12, 11, "You emerge from the depths into the sunlight!", BRIGHT_WHITE);
        Console::drawString(12, 12, "A vast world stretches before you...", BRIGHT_CYAN);
        
        Console::drawString(15, 14, "OBJECTIVES:", BRIGHT_YELLOW);
        Console::drawString(15, 15, "1. Find the Ancient Wand in the eastern ruins", GRAY);
        Console::drawString(15, 16, "2. Collect mana crystals along the path to the Forge", GRAY);
        Console::drawString(15, 17, "3. Cast spells to destroy the Forge Door!", GRAY);
        
        Console::drawString(18, 19, "WASD=Move  Arrows=Cast Spell  E=Interact", BRIGHT_CYAN);
        
        Console::drawString(22, 22, "Press any key to continue...", YELLOW);
        
        Console::flush();
        _getch();
    }
    
    void showGameOver() {
        Console::clearBuffer();
        
        if (victory) {
            Console::drawString(25, 8, "=============================", BRIGHT_GREEN);
            Console::drawString(25, 9, "      VICTORY!", BRIGHT_YELLOW);
            Console::drawString(25, 10, "=============================", BRIGHT_GREEN);
            Console::drawString(15, 12, "You found the Ancient Artifact!", BRIGHT_WHITE);
            Console::drawString(15, 13, "The realm is saved! You are a true hero!", BRIGHT_CYAN);
        } else if (!running) {
            Console::drawString(25, 9, "GAME OVER", BRIGHT_RED);
            Console::drawString(22, 11, "Thanks for playing!", GRAY);
        } else {
            Console::drawString(25, 9, "GAME OVER", BRIGHT_RED);
            Console::drawString(22, 11, "You have perished...", GRAY);
        }
        
        string scoreText = "Final Score: " + to_string(savedScore);
        Console::drawString(28, 15, scoreText, BRIGHT_YELLOW);
        Console::drawString(22, 18, "Press any key to exit...", GRAY);
        
        Console::flush();
        _getch();
    }
};

int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        MessageBoxA(NULL, e.what(), "Game Crash", MB_OK | MB_ICONERROR);
    } catch (...) {
        MessageBoxA(NULL, "Unknown error occurred", "Game Crash", MB_OK | MB_ICONERROR);
    }
    return 0;
}

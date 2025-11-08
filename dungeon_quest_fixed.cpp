#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <windows.h>
#include <cmath>
#include <ctime>
#include <algorithm>

using namespace std;

// Colors
enum Color { BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3, RED = 4, MAGENTA = 5, YELLOW = 6, WHITE = 7, 
             GRAY = 8, BRIGHT_BLUE = 9, BRIGHT_GREEN = 10, BRIGHT_CYAN = 11, BRIGHT_RED = 12, 
             BRIGHT_MAGENTA = 13, BRIGHT_YELLOW = 14, BRIGHT_WHITE = 15 };

// Vector2 for positions
struct Vec2 {
    int x, y;
    Vec2(int _x = 0, int _y = 0) : x(_x), y(_y) {}
    bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }
    float distance(const Vec2& other) const {
        float dx = (float)(x - other.x);
        float dy = (float)(y - other.y);
        return sqrt(dx * dx + dy * dy);
    }
};

// Console utility with buffering
class Console {
private:
    static HANDLE hConsole;
    static char screenBuffer[30][81]; // Buffer for screen
    static int colorBuffer[30][81];
    static bool bufferDirty;
    
public:
    static void init() {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
        // Initialize buffer
        for (int y = 0; y < 30; y++) {
            for (int x = 0; x < 81; x++) {
                screenBuffer[y][x] = ' ';
                colorBuffer[y][x] = WHITE;
            }
        }
        bufferDirty = true;
    }
    
    static void setChar(int x, int y, char c, int color) {
        if (x >= 0 && x < 80 && y >= 0 && y < 30) {
            screenBuffer[y][x] = c;
            colorBuffer[y][x] = color;
            bufferDirty = true;
        }
    }
    
    static void clearBuffer() {
        for (int y = 0; y < 30; y++) {
            for (int x = 0; x < 80; x++) {
                screenBuffer[y][x] = ' ';
                colorBuffer[y][x] = BLACK;
            }
        }
        bufferDirty = true;
    }
    
    static void flush() {
        if (!bufferDirty) return;
        
        COORD coord = {0, 0};
        SetConsoleCursorPosition(hConsole, coord);
        
        for (int y = 0; y < 30; y++) {
            for (int x = 0; x < 80; x++) {
                SetConsoleTextAttribute(hConsole, colorBuffer[y][x]);
                cout << screenBuffer[y][x];
            }
        }
        cout.flush();
        bufferDirty = false;
    }
};

HANDLE Console::hConsole;
char Console::screenBuffer[30][81];
int Console::colorBuffer[30][81];
bool Console::bufferDirty = true;

// Forward declare classes
class Dungeon;
class Player;
class Enemy;

// Particle effect
struct Particle {
    Vec2 pos;
    char symbol;
    int color;
    float life;
    float vx, vy;
};

// Base Entity class
class Entity {
public:
    Vec2 pos;
    char symbol;
    int color;
    int health;
    int maxHealth;
    bool alive;
    string name;
    
    Entity(Vec2 p, char s, int c, int hp, string n) 
        : pos(p), symbol(s), color(c), health(hp), maxHealth(hp), alive(true), name(n) {}
    
    virtual ~Entity() {}
    virtual void update(Dungeon& dungeon) {}
    
    virtual void takeDamage(int damage) {
        health -= damage;
        if (health <= 0) {
            health = 0;
            alive = false;
        }
    }
    
    void draw() {
        Console::setChar(pos.x, pos.y, symbol, color);
    }
};

// Item class
class Item {
public:
    Vec2 pos;
    string name;
    char symbol;
    int color;
    enum Type { HEALTH_POTION, SWORD, SHIELD, KEY } type;
    int value;
    
    Item(Vec2 p, Type t) : pos(p), type(t), value(0) {
        switch(t) {
            case HEALTH_POTION: name = "Health Potion"; symbol = '+'; color = BRIGHT_RED; value = 30; break;
            case SWORD: name = "Sword"; symbol = '/'; color = BRIGHT_CYAN; value = 5; break;
            case SHIELD: name = "Shield"; symbol = 'O'; color = BRIGHT_YELLOW; value = 3; break;
            case KEY: name = "Key"; symbol = 'k'; color = BRIGHT_YELLOW; value = 1; break;
        }
    }
    
    void draw() {
        Console::setChar(pos.x, pos.y, symbol, color);
    }
};

// Player class
class Player : public Entity {
public:
    int attack;
    int defense;
    int keys;
    int score;
    
    Player(Vec2 p) : Entity(p, '@', BRIGHT_WHITE, 100, "Hero"), 
        attack(10), defense(0), keys(0), score(0) {}
    
    void move(int dx, int dy, Dungeon& dungeon);
    
    void addItem(const Item& item) {
        if (item.type == Item::HEALTH_POTION) {
            health = min(maxHealth, health + item.value);
        } else if (item.type == Item::SWORD) {
            attack += item.value;
        } else if (item.type == Item::SHIELD) {
            defense += item.value;
        } else if (item.type == Item::KEY) {
            keys += item.value;
        }
    }
};

// Enemy class
class Enemy : public Entity {
public:
    int attack;
    int aggroRange;
    bool aggressive;
    
    Enemy(Vec2 p, int lvl) : Entity(p, 'E', RED, 20 + lvl * 10, "Goblin"), 
        attack(5 + lvl * 2), aggroRange(5), aggressive(false) {
        if (lvl > 2) { symbol = 'O'; name = "Orc"; color = BRIGHT_RED; }
        if (lvl > 4) { symbol = 'D'; name = "Dragon"; color = BRIGHT_MAGENTA; aggroRange = 8; }
    }
    
    void update(Dungeon& dungeon);
};

// Dungeon class
class Dungeon {
public:
    static const int WIDTH = 80;
    static const int HEIGHT = 24;
    char tiles[HEIGHT][WIDTH];
    Player* player;
    vector<Enemy*> enemies;
    vector<Item*> items;
    vector<Particle> particles;
    int level;
    bool exitReached;
    
    Dungeon(int lvl) : level(lvl), exitReached(false), player(nullptr) {
        // Initialize tiles
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
        // Create rooms
        int numRooms = 5 + level;
        for (int i = 0; i < numRooms; i++) {
            int roomW = 5 + rand() % 8;
            int roomH = 4 + rand() % 6;
            int roomX = 2 + rand() % (WIDTH - roomW - 4);
            int roomY = 2 + rand() % (HEIGHT - roomH - 4);
            
            // Carve room
            for (int y = roomY; y < roomY + roomH && y < HEIGHT; y++) {
                for (int x = roomX; x < roomX + roomW && x < WIDTH; x++) {
                    tiles[y][x] = '.';
                }
            }
            
            // Add enemies
            if (i > 0 && rand() % 100 < 60) {
                int ex = roomX + 1 + rand() % max(1, roomW - 2);
                int ey = roomY + 1 + rand() % max(1, roomH - 2);
                if (ex < WIDTH && ey < HEIGHT) {
                    enemies.push_back(new Enemy(Vec2(ex, ey), level));
                }
            }
            
            // Add items
            if (rand() % 100 < 40) {
                int ix = roomX + 1 + rand() % max(1, roomW - 2);
                int iy = roomY + 1 + rand() % max(1, roomH - 2);
                if (ix < WIDTH && iy < HEIGHT) {
                    Item::Type types[] = {Item::HEALTH_POTION, Item::SWORD, Item::SHIELD, Item::KEY};
                    items.push_back(new Item(Vec2(ix, iy), types[rand() % 4]));
                }
            }
        }
        
        // Create corridors
        for (int i = 0; i < numRooms * 2; i++) {
            int x = 2 + rand() % (WIDTH - 4);
            int y = 2 + rand() % (HEIGHT - 4);
            int len = 5 + rand() % 10;
            bool horizontal = rand() % 2;
            
            for (int j = 0; j < len; j++) {
                if (horizontal && x + j < WIDTH - 1) tiles[y][x + j] = '.';
                else if (!horizontal && y + j < HEIGHT - 1) tiles[y + j][x] = '.';
            }
        }
        
        // Place exit
        for (int tries = 0; tries < 100; tries++) {
            int ex = 2 + rand() % (WIDTH - 4);
            int ey = 2 + rand() % (HEIGHT - 4);
            if (tiles[ey][ex] == '.') {
                tiles[ey][ex] = 'X';
                break;
            }
        }
    }
    
    bool isWalkable(int x, int y) {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return false;
        return tiles[y][x] != '#';
    }
    
    Enemy* getEnemyAt(Vec2 pos) {
        for (auto e : enemies) {
            if (e && e->alive && e->pos == pos) return e;
        }
        return nullptr;
    }
    
    Item* getItemAt(Vec2 pos) {
        for (auto i : items) {
            if (i && i->pos == pos) return i;
        }
        return nullptr;
    }
    
    void addParticle(Vec2 pos, char symbol, int color) {
        Particle p;
        p.pos = pos;
        p.symbol = symbol;
        p.color = color;
        p.life = 1.0f;
        p.vx = (rand() % 3 - 1) * 0.5f;
        p.vy = (rand() % 3 - 1) * 0.5f;
        particles.push_back(p);
    }
    
    void update() {
        // Update enemies
        for (auto e : enemies) {
            if (e && e->alive) e->update(*this);
        }
        
        // Update particles
        for (auto it = particles.begin(); it != particles.end();) {
            it->life -= 0.1f;
            if (it->life <= 0) {
                it = particles.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void draw() {
        Console::clearBuffer();
        
        // Draw tiles
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                char tile = tiles[y][x];
                int color = GRAY;
                if (tile == '.') color = BLACK;
                else if (tile == 'X') color = BRIGHT_GREEN;
                Console::setChar(x, y, tile, color);
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
        
        // Draw particles
        for (auto& p : particles) {
            Console::setChar(p.pos.x, p.pos.y, p.symbol, p.color);
        }
        
        // Draw player
        if (player && player->alive) player->draw();
        
        Console::flush();
    }
};

// Player move implementation
void Player::move(int dx, int dy, Dungeon& dungeon) {
    Vec2 newPos(pos.x + dx, pos.y + dy);
    
    // Bounds check
    if (newPos.x < 0 || newPos.x >= Dungeon::WIDTH || 
        newPos.y < 0 || newPos.y >= Dungeon::HEIGHT) return;
    
    if (!dungeon.isWalkable(newPos.x, newPos.y)) return;
    
    // Check for enemy
    Enemy* enemy = dungeon.getEnemyAt(newPos);
    if (enemy) {
        int damage = max(1, attack - rand() % 3);
        enemy->takeDamage(damage);
        dungeon.addParticle(enemy->pos, '*', BRIGHT_RED);
        score += 10;
        if (!enemy->alive) score += 50;
        return;
    }
    
    // Check for item
    Item* item = dungeon.getItemAt(newPos);
    if (item) {
        addItem(*item);
        dungeon.items.erase(remove(dungeon.items.begin(), dungeon.items.end(), item), dungeon.items.end());
        delete item;
        dungeon.addParticle(newPos, '+', BRIGHT_YELLOW);
        score += 25;
    }
    
    // Check for exit
    if (dungeon.tiles[newPos.y][newPos.x] == 'X') {
        if (keys > 0) {
            dungeon.exitReached = true;
            keys--;
        }
    }
    
    pos = newPos;
}

// Enemy update implementation
void Enemy::update(Dungeon& dungeon) {
    if (!dungeon.player || !dungeon.player->alive) return;
    
    float dist = pos.distance(dungeon.player->pos);
    
    if (dist < aggroRange) {
        aggressive = true;
        
        // Simple pathfinding
        int dx = 0, dy = 0;
        if (dungeon.player->pos.x > pos.x) dx = 1;
        else if (dungeon.player->pos.x < pos.x) dx = -1;
        if (dungeon.player->pos.y > pos.y) dy = 1;
        else if (dungeon.player->pos.y < pos.y) dy = -1;
        
        Vec2 newPos(pos.x + dx, pos.y + dy);
        
        // Bounds check
        if (newPos.x < 0 || newPos.x >= Dungeon::WIDTH || 
            newPos.y < 0 || newPos.y >= Dungeon::HEIGHT) return;
        
        if (newPos == dungeon.player->pos) {
            // Attack player
            int damage = max(1, attack - dungeon.player->defense - rand() % 2);
            dungeon.player->takeDamage(damage);
            dungeon.addParticle(dungeon.player->pos, 'X', RED);
        } else if (dungeon.isWalkable(newPos.x, newPos.y) && !dungeon.getEnemyAt(newPos)) {
            pos = newPos;
        }
    }
}

// Game class
class Game {
public:
    Dungeon* dungeon;
    bool running;
    bool gameOver;
    bool victory;
    int totalScore;
    int savedHealth;
    int savedAttack;
    int savedDefense;
    int savedKeys;
    int savedScore;
    
    Game() : dungeon(nullptr), running(true), gameOver(false), victory(false), totalScore(0),
        savedHealth(100), savedAttack(10), savedDefense(0), savedKeys(0), savedScore(0) {}
    
    void start() {
        Console::init();
        srand((unsigned int)time(NULL));
        
        showTitle();
        
        int currentLevel = 1;
        while (running && !gameOver) {
            dungeon = new Dungeon(currentLevel);
            
            // Place player
            for (int tries = 0; tries < 100; tries++) {
                int px = 2 + rand() % (Dungeon::WIDTH - 4);
                int py = 2 + rand() % (Dungeon::HEIGHT - 4);
                if (dungeon->tiles[py][px] == '.') {
                    dungeon->player = new Player(Vec2(px, py));
                    if (currentLevel > 1) {
                        dungeon->player->health = savedHealth;
                        dungeon->player->attack = savedAttack;
                        dungeon->player->defense = savedDefense;
                        dungeon->player->keys = savedKeys;
                        dungeon->player->score = savedScore;
                    }
                    break;
                }
            }
            
            if (!dungeon->player) {
                delete dungeon;
                break;
            }
            
            playLevel();
            
            if (dungeon->player) {
                savedHealth = dungeon->player->health;
                savedAttack = dungeon->player->attack;
                savedDefense = dungeon->player->defense;
                savedKeys = dungeon->player->keys;
                savedScore = dungeon->player->score;
            }
            
            if (dungeon->exitReached) {
                totalScore = savedScore;
                currentLevel++;
                if (currentLevel > 5) {
                    victory = true;
                    gameOver = true;
                }
            } else if (!dungeon->player || !dungeon->player->alive) {
                totalScore = savedScore;
                gameOver = true;
            }
            
            delete dungeon;
            dungeon = nullptr;
        }
        
        showGameOver();
    }
    
    void playLevel() {
        if (!dungeon || !dungeon->player) return;
        
        while (running && dungeon->player->alive && !dungeon->exitReached) {
            dungeon->draw();
            drawUI();
            
            if (_kbhit()) {
                int key = _getch();
                if (key == 224) { // Arrow keys prefix
                    key = _getch();
                }
                
                if (key == 'w' || key == 'W') dungeon->player->move(0, -1, *dungeon);
                else if (key == 's' || key == 'S') dungeon->player->move(0, 1, *dungeon);
                else if (key == 'a' || key == 'A') dungeon->player->move(-1, 0, *dungeon);
                else if (key == 'd' || key == 'D') dungeon->player->move(1, 0, *dungeon);
                else if (key == 27) running = false; // ESC
            }
            
            dungeon->update();
            Sleep(150); // Slower update rate
        }
    }
    
    void drawUI() {
        if (!dungeon || !dungeon->player) return;
        
        string ui = "HP:" + to_string(dungeon->player->health) + "/" + to_string(dungeon->player->maxHealth) +
                    " ATK:" + to_string(dungeon->player->attack) +
                    " DEF:" + to_string(dungeon->player->defense) +
                    " Keys:" + to_string(dungeon->player->keys) +
                    " Score:" + to_string(dungeon->player->score);
        
        for (size_t i = 0; i < ui.length() && i < 80; i++) {
            Console::setChar((int)i, 24, ui[i], WHITE);
        }
        
        string controls = "WASD:Move  ESC:Quit  Find KEY to unlock EXIT(X)";
        for (size_t i = 0; i < controls.length() && i < 80; i++) {
            Console::setChar((int)i, 25, controls[i], GRAY);
        }
    }
    
    void showTitle() {
        Console::clearBuffer();
        
        string title[] = {
            "====================================",
            "    OMEGA ENGINE - DUNGEON QUEST   ",
            "===================================="
        };
        
        for (int i = 0; i < 3; i++) {
            for (size_t j = 0; j < title[i].length(); j++) {
                Console::setChar(20 + (int)j, 5 + i, title[i][j], BRIGHT_CYAN);
            }
        }
        
        string legend[] = {
            "@ = You (Hero)",
            "E/O/D = Enemies (Goblin/Orc/Dragon)",
            "+ = Health Potion (+30 HP)",
            "/ = Sword (+5 ATK)",
            "k = Key (Unlock Exit)",
            "X = Exit (Need Key!)",
            "",
            "Press any key to start..."
        };
        
        for (int i = 0; i < 8; i++) {
            for (size_t j = 0; j < legend[i].length(); j++) {
                Console::setChar(15 + (int)j, 10 + i, legend[i][j], WHITE);
            }
        }
        
        Console::flush();
        _getch();
    }
    
    void showGameOver() {
        Console::clearBuffer();
        
        string msg = victory ? "VICTORY! YOU ESCAPED!" : "GAME OVER - YOU DIED!";
        for (size_t i = 0; i < msg.length(); i++) {
            Console::setChar(30 + (int)i, 10, msg[i], victory ? BRIGHT_GREEN : BRIGHT_RED);
        }
        
        string score = "Final Score: " + to_string(totalScore);
        for (size_t i = 0; i < score.length(); i++) {
            Console::setChar(30 + (int)i, 12, score[i], BRIGHT_YELLOW);
        }
        
        string exit = "Press any key to exit...";
        for (size_t i = 0; i < exit.length(); i++) {
            Console::setChar(30 + (int)i, 15, exit[i], WHITE);
        }
        
        Console::flush();
        _getch();
    }
};

int main() {
    Game game;
    game.start();
    return 0;
}

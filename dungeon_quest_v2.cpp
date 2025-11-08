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

// Room structure
struct Room {
    int x, y, width, height;
    Vec2 center() const { return Vec2(x + width / 2, y + height / 2); }
};

// Console utility with buffering
class Console {
private:
    static HANDLE hConsole;
    static char screenBuffer[30][81];
    static int colorBuffer[30][81];
    static bool bufferDirty;
    
public:
    static void init() {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
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
            case SHIELD: name = "Shield"; symbol = ']'; color = BRIGHT_YELLOW; value = 3; break;
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
    
    Enemy(Vec2 p, int lvl) : Entity(p, 'E', RED, 20 + lvl * 10, "Goblin"), 
        attack(5 + lvl * 2), aggroRange(5) {
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
    vector<Room> rooms;
    int level;
    bool exitReached;
    bool hasKey;
    
    Dungeon(int lvl) : level(lvl), exitReached(false), player(nullptr), hasKey(false) {
        // Initialize with walls
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
        // Generate rooms
        int numRooms = 6 + level;
        for (int i = 0; i < numRooms; i++) {
            int roomW = 6 + rand() % 8;
            int roomH = 5 + rand() % 6;
            int roomX = 1 + rand() % (WIDTH - roomW - 2);
            int roomY = 1 + rand() % (HEIGHT - roomH - 2);
            
            Room room;
            room.x = roomX;
            room.y = roomY;
            room.width = roomW;
            room.height = roomH;
            
            // Check overlap with existing rooms
            bool overlaps = false;
            for (auto& r : rooms) {
                if (!(roomX + roomW < r.x || roomX > r.x + r.width ||
                      roomY + roomH < r.y || roomY > r.y + r.height)) {
                    overlaps = true;
                    break;
                }
            }
            
            if (!overlaps) {
                rooms.push_back(room);
                
                // Carve room
                for (int y = roomY; y < roomY + roomH && y < HEIGHT; y++) {
                    for (int x = roomX; x < roomX + roomW && x < WIDTH; x++) {
                        tiles[y][x] = '.';
                    }
                }
            }
        }
        
        // Connect rooms with corridors
        for (size_t i = 1; i < rooms.size(); i++) {
            Vec2 prev = rooms[i-1].center();
            Vec2 curr = rooms[i].center();
            
            // Horizontal corridor
            int x1 = min(prev.x, curr.x);
            int x2 = max(prev.x, curr.x);
            for (int x = x1; x <= x2; x++) {
                if (prev.y >= 0 && prev.y < HEIGHT && x >= 0 && x < WIDTH) {
                    tiles[prev.y][x] = '.';
                }
            }
            
            // Vertical corridor
            int y1 = min(prev.y, curr.y);
            int y2 = max(prev.y, curr.y);
            for (int y = y1; y <= y2; y++) {
                if (y >= 0 && y < HEIGHT && curr.x >= 0 && curr.x < WIDTH) {
                    tiles[y][curr.x] = '.';
                }
            }
        }
        
        // Place items in rooms
        for (size_t i = 0; i < rooms.size(); i++) {
            Room& room = rooms[i];
            
            // GUARANTEED KEY in first or second room
            if (i == 1 && !hasKey) {
                int kx = room.x + 2 + rand() % max(1, room.width - 4);
                int ky = room.y + 2 + rand() % max(1, room.height - 4);
                items.push_back(new Item(Vec2(kx, ky), Item::KEY));
                hasKey = true;
            }
            
            // Other items
            if (i > 0 && rand() % 100 < 50) {
                int ix = room.x + 1 + rand() % max(1, room.width - 2);
                int iy = room.y + 1 + rand() % max(1, room.height - 2);
                Item::Type types[] = {Item::HEALTH_POTION, Item::SWORD, Item::SHIELD};
                items.push_back(new Item(Vec2(ix, iy), types[rand() % 3]));
            }
            
            // Enemies in later rooms
            if (i > 1 && rand() % 100 < 60) {
                int ex = room.x + 1 + rand() % max(1, room.width - 2);
                int ey = room.y + 1 + rand() % max(1, room.height - 2);
                enemies.push_back(new Enemy(Vec2(ex, ey), level));
            }
        }
        
        // Place exit in last room
        if (!rooms.empty()) {
            Room& lastRoom = rooms[rooms.size() - 1];
            int ex = lastRoom.x + lastRoom.width / 2;
            int ey = lastRoom.y + lastRoom.height / 2;
            tiles[ey][ex] = 'X';
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
        particles.push_back(p);
    }
    
    void update() {
        for (auto e : enemies) {
            if (e && e->alive) e->update(*this);
        }
        
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
        
        // Draw tiles with VISIBLE floor
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                char tile = tiles[y][x];
                int color = GRAY;
                if (tile == '.') {
                    tile = (char)250;  // Middle dot - VISIBLE floor!
                    color = GRAY;
                } else if (tile == 'X') {
                    color = BRIGHT_GREEN;
                }
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
    
    if (newPos.x < 0 || newPos.x >= Dungeon::WIDTH || 
        newPos.y < 0 || newPos.y >= Dungeon::HEIGHT) return;
    
    if (!dungeon.isWalkable(newPos.x, newPos.y)) return;
    
    Enemy* enemy = dungeon.getEnemyAt(newPos);
    if (enemy) {
        int damage = max(1, attack - rand() % 3);
        enemy->takeDamage(damage);
        dungeon.addParticle(enemy->pos, '*', BRIGHT_RED);
        score += 10;
        if (!enemy->alive) score += 50;
        return;
    }
    
    Item* item = dungeon.getItemAt(newPos);
    if (item) {
        addItem(*item);
        dungeon.items.erase(remove(dungeon.items.begin(), dungeon.items.end(), item), dungeon.items.end());
        delete item;
        dungeon.addParticle(newPos, '+', BRIGHT_YELLOW);
        score += 25;
    }
    
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
        int dx = 0, dy = 0;
        if (dungeon.player->pos.x > pos.x) dx = 1;
        else if (dungeon.player->pos.x < pos.x) dx = -1;
        if (dungeon.player->pos.y > pos.y) dy = 1;
        else if (dungeon.player->pos.y < pos.y) dy = -1;
        
        Vec2 newPos(pos.x + dx, pos.y + dy);
        
        if (newPos.x < 0 || newPos.x >= Dungeon::WIDTH || 
            newPos.y < 0 || newPos.y >= Dungeon::HEIGHT) return;
        
        if (newPos == dungeon.player->pos) {
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
    int savedHealth, savedAttack, savedDefense, savedKeys, savedScore;
    
    Game() : dungeon(nullptr), running(true), gameOver(false), victory(false), totalScore(0),
        savedHealth(100), savedAttack(10), savedDefense(0), savedKeys(0), savedScore(0) {}
    
    void start() {
        Console::init();
        srand((unsigned int)time(NULL));
        
        showTitle();
        
        int currentLevel = 1;
        while (running && !gameOver) {
            dungeon = new Dungeon(currentLevel);
            
            // Place player in first room
            if (!dungeon->rooms.empty()) {
                Room& firstRoom = dungeon->rooms[0];
                int px = firstRoom.x + firstRoom.width / 2;
                int py = firstRoom.y + firstRoom.height / 2;
                dungeon->player = new Player(Vec2(px, py));
                
                if (currentLevel > 1) {
                    dungeon->player->health = savedHealth;
                    dungeon->player->attack = savedAttack;
                    dungeon->player->defense = savedDefense;
                    dungeon->player->keys = savedKeys;
                    dungeon->player->score = savedScore;
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
                if (key == 224) key = _getch();
                
                if (key == 'w' || key == 'W') dungeon->player->move(0, -1, *dungeon);
                else if (key == 's' || key == 'S') dungeon->player->move(0, 1, *dungeon);
                else if (key == 'a' || key == 'A') dungeon->player->move(-1, 0, *dungeon);
                else if (key == 'd' || key == 'D') dungeon->player->move(1, 0, *dungeon);
                else if (key == 27) running = false;
            }
            
            dungeon->update();
            Sleep(150);
        }
    }
    
    void drawUI() {
        if (!dungeon || !dungeon->player) return;
        
        string ui = "HP:" + to_string(dungeon->player->health) + "/" + to_string(dungeon->player->maxHealth) +
                    " | ATK:" + to_string(dungeon->player->attack) +
                    " | DEF:" + to_string(dungeon->player->defense) +
                    " | Keys:" + to_string(dungeon->player->keys) +
                    " | Score:" + to_string(dungeon->player->score) +
                    " | Level:" + to_string(dungeon->level);
        
        for (size_t i = 0; i < ui.length() && i < 80; i++) {
            Console::setChar((int)i, 24, ui[i], BRIGHT_WHITE);
        }
        
        string controls = "WASD:Move  Collect KEY(k)  Reach EXIT(X)  ESC:Quit";
        for (size_t i = 0; i < controls.length() && i < 80; i++) {
            Console::setChar((int)i, 25, controls[i], GRAY);
        }
    }
    
    void showTitle() {
        Console::clearBuffer();
        
        string title[] = {
            "====================================",
            "    OMEGA ENGINE - DUNGEON QUEST   ",
            "           V2.0 - FIXED            ",
            "===================================="
        };
        
        for (int i = 0; i < 4; i++) {
            for (size_t j = 0; j < title[i].length(); j++) {
                Console::setChar(20 + (int)j, 4 + i, title[i][j], BRIGHT_CYAN);
            }
        }
        
        string legend[] = {
            "LEGEND:",
            "  @ = You (Hero)",
            "  E/O/D = Enemies",
            "  + = Health Potion",
            "  / = Sword",
            "  ] = Shield",
            "  k = KEY (Find to unlock exit!)",
            "  X = EXIT (Need key!)",
            "  # = Wall",
            "  . = Floor (walkable)",
            "",
            "Goal: Find KEY, reach EXIT, beat 5 levels!",
            "",
            "Press any key to start..."
        };
        
        for (int i = 0; i < 14; i++) {
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

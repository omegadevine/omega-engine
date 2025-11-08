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

// Console utility with FAST double buffering AND correct coordinates
class Console {
private:
    static HANDLE hConsole;
    static char screenBuffer[30][81];
    static int colorBuffer[30][81];
    static bool dirty;
    
public:
    static void init() {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
        // Initialize buffers
        for (int y = 0; y < 30; y++) {
            for (int x = 0; x < 81; x++) {
                screenBuffer[y][x] = ' ';
                colorBuffer[y][x] = BLACK;
            }
        }
        dirty = true;
    }
    
    static void clearBuffer() {
        for (int y = 0; y < 30; y++) {
            for (int x = 0; x < 80; x++) {
                screenBuffer[y][x] = ' ';
                colorBuffer[y][x] = BLACK;
            }
        }
        dirty = true;
    }
    
    // FAST: Write to buffer, not screen
    static void setChar(int x, int y, char c, int color) {
        if (x < 0 || x >= 80 || y < 0 || y >= 30) return;
        screenBuffer[y][x] = c;
        colorBuffer[y][x] = color;
        dirty = true;
    }
    
    static void drawString(int x, int y, const string& str, int color) {
        for (size_t i = 0; i < str.length() && x + i < 80; i++) {
            setChar(x + (int)i, y, str[i], color);
        }
    }
    
    // FAST: Write entire buffer at once
    static void flush() {
        if (!dirty) return;
        
        COORD coord = {0, 0};
        SetConsoleCursorPosition(hConsole, coord);
        
        // Write entire screen in one go
        for (int y = 0; y < 30; y++) {
            for (int x = 0; x < 80; x++) {
                SetConsoleTextAttribute(hConsole, colorBuffer[y][x]);
                cout << screenBuffer[y][x];
            }
        }
        cout.flush();
        dirty = false;
    }
};

HANDLE Console::hConsole;
char Console::screenBuffer[30][81];
int Console::colorBuffer[30][81];
bool Console::dirty = true;

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

// Forward declarations
class Dungeon;

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

// Dungeon class with FIXED grid
class Dungeon {
public:
    static const int WIDTH = 60;
    static const int HEIGHT = 20;
    char tiles[HEIGHT][WIDTH];
    Player* player;
    vector<Enemy*> enemies;
    vector<Item*> items;
    vector<Room> rooms;
    int level;
    bool exitReached;
    bool hasKey;
    
    Dungeon(int lvl) : level(lvl), exitReached(false), player(nullptr), hasKey(false) {
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
            
            Room room;
            room.x = roomX;
            room.y = roomY;
            room.width = roomW;
            room.height = roomH;
            rooms.push_back(room);
            
            for (int y = roomY; y < roomY + roomH && y < HEIGHT; y++) {
                for (int x = roomX; x < roomX + roomW && x < WIDTH; x++) {
                    tiles[y][x] = '.';
                }
            }
        }
        
        for (size_t i = 1; i < rooms.size(); i++) {
            Vec2 prev = rooms[i-1].center();
            Vec2 curr = rooms[i].center();
            
            int x1 = min(prev.x, curr.x);
            int x2 = max(prev.x, curr.x);
            if (prev.y >= 0 && prev.y < HEIGHT) {
                for (int x = x1; x <= x2 && x < WIDTH; x++) {
                    tiles[prev.y][x] = '.';
                }
            }
            
            int y1 = min(prev.y, curr.y);
            int y2 = max(prev.y, curr.y);
            if (curr.x >= 0 && curr.x < WIDTH) {
                for (int y = y1; y <= y2 && y < HEIGHT; y++) {
                    tiles[y][curr.x] = '.';
                }
            }
        }
        
        if (rooms.size() > 1) {
            Room& keyRoom = rooms[1];
            int kx = keyRoom.x + keyRoom.width / 2;
            int ky = keyRoom.y + keyRoom.height / 2;
            items.push_back(new Item(Vec2(kx, ky), Item::KEY));
            hasKey = true;
        }
        
        for (size_t i = 2; i < rooms.size(); i++) {
            if (rand() % 100 < 50) {
                Room& room = rooms[i];
                int ix = room.x + 2 + rand() % max(1, room.width - 4);
                int iy = room.y + 2 + rand() % max(1, room.height - 4);
                Item::Type types[] = {Item::HEALTH_POTION, Item::SWORD, Item::SHIELD};
                items.push_back(new Item(Vec2(ix, iy), types[rand() % 3]));
            }
        }
        
        for (size_t i = 2; i < rooms.size(); i++) {
            if (rand() % 100 < 60) {
                Room& room = rooms[i];
                int ex = room.x + 2 + rand() % max(1, room.width - 4);
                int ey = room.y + 2 + rand() % max(1, room.height - 4);
                enemies.push_back(new Enemy(Vec2(ex, ey), level));
            }
        }
        
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
    
    void update() {
        for (auto e : enemies) {
            if (e && e->alive) e->update(*this);
        }
    }
    
    void draw() {
        Console::clearBuffer();
        
        // Draw grid - tiles[y][x] matches screen correctly
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
        
        // FLUSH: Write buffer to screen in one operation
        Console::flush();
    }
};

// Player move
void Player::move(int dx, int dy, Dungeon& dungeon) {
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
        addItem(*item);
        dungeon.items.erase(remove(dungeon.items.begin(), dungeon.items.end(), item), dungeon.items.end());
        delete item;
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

// Enemy update
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
            Sleep(100);  // Slightly faster update
        }
    }
    
    void showTitle() {
        Console::clearBuffer();
        
        Console::drawString(15, 5, "====================================", BRIGHT_CYAN);
        Console::drawString(15, 6, "    OMEGA ENGINE - DUNGEON QUEST   ", BRIGHT_CYAN);
        Console::drawString(15, 7, "    V3.1 - FAST RENDERING FIXED    ", BRIGHT_CYAN);
        Console::drawString(15, 8, "====================================", BRIGHT_CYAN);
        
        Console::drawString(15, 11, "LEGEND:", WHITE);
        Console::drawString(15, 12, "  @ = You", BRIGHT_WHITE);
        Console::drawString(15, 13, "  E/O/D = Enemies", RED);
        Console::drawString(15, 14, "  k = KEY (required!)", BRIGHT_YELLOW);
        Console::drawString(15, 15, "  X = EXIT (need key!)", BRIGHT_GREEN);
        Console::drawString(15, 16, "  # = Wall", GRAY);
        Console::drawString(15, 17, "  . = Floor", GRAY);
        Console::drawString(15, 19, "Controls: W=UP S=DOWN A=LEFT D=RIGHT", YELLOW);
        Console::drawString(15, 21, "Press any key to start...", WHITE);
        
        Console::flush();
        _getch();
    }
    
    void showGameOver() {
        Console::clearBuffer();
        
        string msg = victory ? "VICTORY! YOU ESCAPED!" : "GAME OVER - YOU DIED!";
        Console::drawString(30, 10, msg, victory ? BRIGHT_GREEN : BRIGHT_RED);
        
        string score = "Final Score: " + to_string(totalScore);
        Console::drawString(30, 12, score, BRIGHT_YELLOW);
        
        Console::drawString(30, 15, "Press any key to exit...", WHITE);
        
        Console::flush();
        _getch();
    }
};

int main() {
    Game game;
    game.start();
    return 0;
}

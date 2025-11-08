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

// Forward declarations
class Entity;
class Player;
class Enemy;
class Item;
class Dungeon;

// Vector2 for positions
struct Vec2 {
    int x, y;
    Vec2(int _x = 0, int _y = 0) : x(_x), y(_y) {}
    bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }
    float distance(const Vec2& other) const {
        return sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
    }
};

// Console utility
class Console {
public:
    static HANDLE hConsole;
    static void init() {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }
    
    static void setColor(int fg, int bg = BLACK) {
        SetConsoleTextAttribute(hConsole, fg + (bg << 4));
    }
    
    static void setCursor(int x, int y) {
        COORD coord = {(SHORT)x, (SHORT)y};
        SetConsoleCursorPosition(hConsole, coord);
    }
    
    static void clear() {
        system("cls");
    }
};
HANDLE Console::hConsole;

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
    
    virtual void update(Dungeon& dungeon) {}
    virtual void takeDamage(int damage) {
        health -= damage;
        if (health <= 0) {
            health = 0;
            alive = false;
        }
    }
    
    void draw() {
        Console::setCursor(pos.x, pos.y);
        Console::setColor(color);
        cout << symbol;
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
        Console::setCursor(pos.x, pos.y);
        Console::setColor(color);
        cout << symbol;
    }
};

// Player class
class Player : public Entity {
public:
    int attack;
    int defense;
    int keys;
    int score;
    vector<Item> inventory;
    
    Player(Vec2 p) : Entity(p, '@', BRIGHT_WHITE, 100, "Hero"), attack(10), defense(0), keys(0), score(0) {}
    
    void move(int dx, int dy, Dungeon& dungeon);
    
    void addItem(const Item& item) {
        inventory.push_back(item);
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
    
    Enemy(Vec2 p, int lvl) : Entity(p, 'E', RED, 20 + lvl * 10, "Goblin"), attack(5 + lvl * 2), aggroRange(5), aggressive(false) {
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
    
    Dungeon(int lvl) : level(lvl), exitReached(false) {
        player = nullptr;
        generateDungeon();
    }
    
    ~Dungeon() {
        for (auto e : enemies) delete e;
        for (auto i : items) delete i;
    }
    
    void generateDungeon() {
        // Fill with walls
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                tiles[y][x] = '#';
            }
        }
        
        // Create rooms
        int numRooms = 5 + level;
        for (int i = 0; i < numRooms; i++) {
            int roomW = 5 + rand() % 8;
            int roomH = 4 + rand() % 6;
            int roomX = 2 + rand() % (WIDTH - roomW - 4);
            int roomY = 2 + rand() % (HEIGHT - roomH - 4);
            
            // Carve room
            for (int y = roomY; y < roomY + roomH; y++) {
                for (int x = roomX; x < roomX + roomW; x++) {
                    tiles[y][x] = '.';
                }
            }
            
            // Add enemies
            if (i > 0 && rand() % 100 < 60) {
                Vec2 enemyPos(roomX + 1 + rand() % (roomW - 2), roomY + 1 + rand() % (roomH - 2));
                enemies.push_back(new Enemy(enemyPos, level));
            }
            
            // Add items
            if (rand() % 100 < 40) {
                Vec2 itemPos(roomX + 1 + rand() % (roomW - 2), roomY + 1 + rand() % (roomH - 2));
                Item::Type types[] = {Item::HEALTH_POTION, Item::SWORD, Item::SHIELD, Item::KEY};
                items.push_back(new Item(itemPos, types[rand() % 4]));
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
            if (e->alive && e->pos == pos) return e;
        }
        return nullptr;
    }
    
    Item* getItemAt(Vec2 pos) {
        for (auto i : items) {
            if (i->pos == pos) return i;
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
            if (e->alive) e->update(*this);
        }
        
        // Update particles
        for (auto it = particles.begin(); it != particles.end();) {
            it->life -= 0.1f;
            it->pos.x += (int)it->vx;
            it->pos.y += (int)it->vy;
            if (it->life <= 0) {
                it = particles.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void draw() {
        // Draw tiles
        for (int y = 0; y < HEIGHT; y++) {
            Console::setCursor(0, y);
            for (int x = 0; x < WIDTH; x++) {
                char tile = tiles[y][x];
                if (tile == '#') Console::setColor(GRAY);
                else if (tile == '.') Console::setColor(BLACK, BLACK);
                else if (tile == 'X') Console::setColor(BRIGHT_GREEN);
                cout << tile;
            }
        }
        
        // Draw items
        for (auto i : items) i->draw();
        
        // Draw enemies
        for (auto e : enemies) if (e->alive) e->draw();
        
        // Draw particles
        for (auto& p : particles) {
            Console::setCursor(p.pos.x, p.pos.y);
            Console::setColor(p.color);
            cout << p.symbol;
        }
        
        // Draw player
        if (player) player->draw();
    }
};

// Player methods
void Player::move(int dx, int dy, Dungeon& dungeon) {
    Vec2 newPos(pos.x + dx, pos.y + dy);
    
    if (!dungeon.isWalkable(newPos.x, newPos.y)) return;
    
    // Check for enemy
    Enemy* enemy = dungeon.getEnemyAt(newPos);
    if (enemy) {
        // Attack!
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

// Enemy methods
void Enemy::update(Dungeon& dungeon) {
    if (!dungeon.player) return;
    
    float dist = pos.distance(dungeon.player->pos);
    
    if (dist < aggroRange) {
        aggressive = true;
        
        // Move towards player
        int dx = 0, dy = 0;
        if (dungeon.player->pos.x > pos.x) dx = 1;
        else if (dungeon.player->pos.x < pos.x) dx = -1;
        if (dungeon.player->pos.y > pos.y) dy = 1;
        else if (dungeon.player->pos.y < pos.y) dy = -1;
        
        Vec2 newPos(pos.x + dx, pos.y + dy);
        
        if (newPos == dungeon.player->pos) {
            // Attack player!
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
    
    Game() : dungeon(nullptr), running(true), gameOver(false), victory(false), totalScore(0) {}
    
    void start() {
        Console::init();
        srand(time(NULL));
        
        showTitle();
        
        int currentLevel = 1;
        while (running && !gameOver) {
            dungeon = new Dungeon(currentLevel);
            
            // Place player
            for (int tries = 0; tries < 100; tries++) {
                int px = 2 + rand() % (Dungeon::WIDTH - 4);
                int py = 2 + rand() % (Dungeon::HEIGHT - 4);
                if (dungeon->tiles[py][px] == '.') {
                    if (dungeon->player) {
                        int oldHealth = dungeon->player->health;
                        int oldAttack = dungeon->player->attack;
                        int oldDefense = dungeon->player->defense;
                        int oldKeys = dungeon->player->keys;
                        int oldScore = dungeon->player->score;
                        delete dungeon->player;
                        dungeon->player = new Player(Vec2(px, py));
                        dungeon->player->health = oldHealth;
                        dungeon->player->attack = oldAttack;
                        dungeon->player->defense = oldDefense;
                        dungeon->player->keys = oldKeys;
                        dungeon->player->score = oldScore;
                    } else {
                        dungeon->player = new Player(Vec2(px, py));
                    }
                    break;
                }
            }
            
            playLevel();
            
            if (dungeon->exitReached) {
                totalScore += dungeon->player->score;
                currentLevel++;
                if (currentLevel > 5) {
                    victory = true;
                    gameOver = true;
                }
            } else if (!dungeon->player->alive) {
                totalScore = dungeon->player->score;
                gameOver = true;
            }
            
            delete dungeon;
        }
        
        showGameOver();
    }
    
    void playLevel() {
        while (running && dungeon->player->alive && !dungeon->exitReached) {
            Console::clear();
            dungeon->draw();
            drawUI();
            
            if (_kbhit()) {
                char key = _getch();
                if (key == 'w' || key == 'W') dungeon->player->move(0, -1, *dungeon);
                else if (key == 's' || key == 'S') dungeon->player->move(0, 1, *dungeon);
                else if (key == 'a' || key == 'A') dungeon->player->move(-1, 0, *dungeon);
                else if (key == 'd' || key == 'D') dungeon->player->move(1, 0, *dungeon);
                else if (key == 27) running = false; // ESC
            }
            
            dungeon->update();
            Sleep(100);
        }
    }
    
    void drawUI() {
        Console::setCursor(0, Dungeon::HEIGHT);
        Console::setColor(WHITE);
        cout << string(80, '-');
        
        Console::setCursor(0, Dungeon::HEIGHT + 1);
        Console::setColor(BRIGHT_WHITE);
        cout << "HP: ";
        Console::setColor(BRIGHT_RED);
        cout << dungeon->player->health << "/" << dungeon->player->maxHealth;
        
        Console::setColor(BRIGHT_WHITE);
        cout << " | ATK: ";
        Console::setColor(BRIGHT_CYAN);
        cout << dungeon->player->attack;
        
        Console::setColor(BRIGHT_WHITE);
        cout << " | DEF: ";
        Console::setColor(BRIGHT_YELLOW);
        cout << dungeon->player->defense;
        
        Console::setColor(BRIGHT_WHITE);
        cout << " | Keys: ";
        Console::setColor(BRIGHT_GREEN);
        cout << dungeon->player->keys;
        
        Console::setColor(BRIGHT_WHITE);
        cout << " | Score: ";
        Console::setColor(BRIGHT_MAGENTA);
        cout << dungeon->player->score;
        
        Console::setCursor(0, Dungeon::HEIGHT + 2);
        Console::setColor(GRAY);
        cout << "WASD: Move | Find Key to unlock Exit (X) | ESC: Quit";
    }
    
    void showTitle() {
        Console::clear();
        Console::setCursor(20, 5);
        Console::setColor(BRIGHT_CYAN);
        cout << "====================================";
        Console::setCursor(20, 6);
        cout << "    OMEGA ENGINE - DUNGEON QUEST   ";
        Console::setCursor(20, 7);
        cout << "====================================";
        
        Console::setCursor(25, 10);
        Console::setColor(BRIGHT_WHITE);
        cout << "@ = You (Hero)";
        Console::setCursor(25, 11);
        Console::setColor(RED);
        cout << "E/O/D = Enemies";
        Console::setCursor(25, 12);
        Console::setColor(BRIGHT_RED);
        cout << "+ = Health Potion";
        Console::setCursor(25, 13);
        Console::setColor(BRIGHT_CYAN);
        cout << "/ = Sword (Attack +5)";
        Console::setCursor(25, 14);
        Console::setColor(BRIGHT_YELLOW);
        cout << "O = Shield (Defense +3)";
        Console::setCursor(25, 15);
        cout << "k = Key (Unlock Exit)";
        Console::setCursor(25, 16);
        Console::setColor(BRIGHT_GREEN);
        cout << "X = Exit (Need Key!)";
        
        Console::setCursor(25, 19);
        Console::setColor(WHITE);
        cout << "Press any key to start...";
        _getch();
    }
    
    void showGameOver() {
        Console::clear();
        Console::setCursor(30, 10);
        if (victory) {
            Console::setColor(BRIGHT_GREEN);
            cout << "VICTORY! YOU ESCAPED THE DUNGEON!";
        } else {
            Console::setColor(BRIGHT_RED);
            cout << "GAME OVER - YOU DIED!";
        }
        
        Console::setCursor(30, 12);
        Console::setColor(BRIGHT_YELLOW);
        cout << "Final Score: " << totalScore;
        
        Console::setCursor(30, 15);
        Console::setColor(WHITE);
        cout << "Press any key to exit...";
        _getch();
    }
};

int main() {
    Game game;
    game.start();
    return 0;
}

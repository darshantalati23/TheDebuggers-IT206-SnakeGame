#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <vector>
#include <chrono>

using namespace std;

// ANSI Color Codes
#define COLOR_RESET   "\033[0m"
#define COLOR_BLACK   "\033[30m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BOLD    "\033[1m"

// Game dimensions
const int WIDTH = 60;
const int HEIGHT = 30;

enum Direction { STOP = 0, UP, DOWN, LEFT, RIGHT };

int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    
    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

struct Node {
    int x, y;
    Node* next;
    Node(int x, int y) : x(x), y(y), next(nullptr) {}
};

enum FruitType { NORMAL, SLOW };

struct Fruit {
    int x, y;
    FruitType type;
};

class SnakeGame {
private:
    bool gameOver;
    bool gameStarted;
    bool paused;
    Node* head;
    Node* tail;
    vector<Fruit> fruits;
    Direction dir;
    int score;
    int speed;
    vector<pair<int, int>> obstacles;
    chrono::steady_clock::time_point startTime;
    int maxScore;

    void spawnFruit() {
        Fruit newFruit;
        int attempts = 0;
        while (attempts < 100) {
            newFruit.x = rand() % WIDTH;
            newFruit.y = rand() % HEIGHT;
            if (!isObstacle(newFruit.x, newFruit.y) && !isSnakeBody(newFruit.x, newFruit.y)) {
                int chance = rand() % 100;
                newFruit.type = (chance < 20) ? SLOW : NORMAL; // 20% green, 80% red
                fruits.push_back(newFruit);
                return;
            }
            attempts++;
        }
        newFruit.x = WIDTH / 2;
        newFruit.y = HEIGHT / 2;
        newFruit.type = NORMAL;
        fruits.push_back(newFruit);
    }

    void spawnObstacles() {
        obstacles.clear();
        int numObstacles = 8 + rand() % 25;
        for (int i = 0; i < numObstacles; i++) {
            int x, y;
            do {
                x = rand() % WIDTH;
                y = rand() % HEIGHT;
            } while (isSnakeBody(x, y) || isFruit(x, y));
            obstacles.push_back({x, y});
        }
    }

    bool isObstacle(int x, int y) {
        for (auto& obs : obstacles) {
            if (obs.first == x && obs.second == y) return true;
        }
        return false;
    }

    bool isSnakeBody(int x, int y) {
        for (Node* temp = head; temp; temp = temp->next) {
            if (temp->x == x && temp->y == y) return true;
        }
        return false;
    }

    bool isFruit(int x, int y) {
        for (auto& fruit : fruits) {
            if (fruit.x == x && fruit.y == y) return true;
        }
        return false;
    }

    void clearSnake() {
        while (head) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
        tail = nullptr;
    }

public:
    SnakeGame() : maxScore(0), head(nullptr), tail(nullptr) {
        resetGame();
    }

    ~SnakeGame() {
        clearSnake();
    }

    void resetGame() {
        gameOver = false;
        gameStarted = false;
        paused = false;
        dir = STOP;
        score = 0;
        speed = 150000;

        clearSnake();

        head = new Node(WIDTH / 2, HEIGHT / 2);
        tail = head;
        tail->next = new Node(head->x - 1, head->y);
        tail = tail->next;
        tail->next = new Node(head->x - 2, head->y);
        tail = tail->next;

        fruits.clear();
        for (int i = 0; i < (rand() % 7) + 2; i++) { // Initial 3 fruits
            spawnFruit();
        }
        spawnObstacles();
    }

    void draw() {
        cout << "\033[H";

        cout << COLOR_BOLD COLOR_WHITE;
        for (int i = 0; i < WIDTH + 2; i++) cout << "■";
        cout << COLOR_RESET << endl;

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH + 2; x++) {
                if (x == 0 || x == WIDTH + 1) {
                    cout << COLOR_BOLD COLOR_WHITE << "■" COLOR_RESET;
                } else if (x == head->x && y == head->y) {
                    cout << COLOR_BOLD COLOR_YELLOW "●" COLOR_RESET;
                } else if (isFruit(x, y)) {
                    for (auto& fruit : fruits) {
                        if (fruit.x == x && fruit.y == y) {
                            if (fruit.type == NORMAL)
                                cout << COLOR_BOLD COLOR_RED "◆" COLOR_RESET;
                            else
                                cout << COLOR_BOLD COLOR_GREEN "◆" COLOR_RESET;
                            break;
                        }
                    }
                } else if (isObstacle(x, y)) {
                    cout << COLOR_BOLD COLOR_WHITE "▒" COLOR_RESET;
                } else {
                    bool isBody = false;
                    for (Node* temp = head->next; temp; temp = temp->next) {
                        if (temp->x == x && temp->y == y) {
                            cout << COLOR_BOLD COLOR_YELLOW "○" COLOR_RESET;
                            isBody = true;
                            break;
                        }
                    }
                    if (!isBody) cout << " ";
                }
            }
            cout << endl;
        }

        cout << COLOR_BOLD COLOR_WHITE;
        for (int i = 0; i < WIDTH + 2; i++) cout << "■";
        cout << COLOR_RESET << endl;

        // Game info panel
        if (gameStarted) {
            auto now = chrono::steady_clock::now();
            int elapsedTime = chrono::duration_cast<chrono::seconds>(now - startTime).count();
            cout << COLOR_BOLD COLOR_BLUE " Score: " COLOR_RESET << COLOR_GREEN << score 
                 << COLOR_BOLD COLOR_BLUE " | Time: " COLOR_RESET << COLOR_CYAN << elapsedTime << "s"
                 << COLOR_BOLD COLOR_BLUE " | Max Score: " COLOR_RESET << COLOR_YELLOW << maxScore << COLOR_RESET << endl;
        } else {
            cout << COLOR_BOLD COLOR_GREEN "\n  WELCOME TO SNAKE GAME!\n" COLOR_RESET;
            cout << COLOR_BOLD "  Use " COLOR_GREEN "W/A/S/D" COLOR_RESET COLOR_BOLD " to move\n"
                 << "  " COLOR_RED "X" COLOR_RESET COLOR_BOLD " to quit | " COLOR_MAGENTA "▒" COLOR_RESET COLOR_BOLD " are obstacles\n"
                 << "  Collect " COLOR_RED "◆" COLOR_RESET COLOR_BOLD " to grow!" COLOR_RESET << endl;
        }
    }

    void input() {
        if (kbhit()) {
            char key = getchar();
            switch (key) {
                case 'w': case 'W': 
                    if (!gameStarted) {
                        gameStarted = true;
                        startTime = chrono::steady_clock::now();
                    }
                    if (dir != DOWN) dir = UP; 
                    break;
                case 's': case 'S': 
                    if (!gameStarted) {
                        gameStarted = true;
                        startTime = chrono::steady_clock::now();
                    }
                    if (dir != UP) dir = DOWN; 
                    break;
                case 'a': case 'A': 
                    if (!gameStarted) {
                        gameStarted = true;
                        startTime = chrono::steady_clock::now();
                    }
                    if (dir != RIGHT) dir = LEFT; 
                    break;
                case 'd': case 'D': 
                    if (!gameStarted) {
                        gameStarted = true;
                        startTime = chrono::steady_clock::now();
                    }
                    if (dir != LEFT) dir = RIGHT; 
                    break;
                case 'p': case 'P': paused = !paused; break;
                case 'x': case 'X': gameOver = true; break;
            }
        }
    }

    void logic() {
        if (dir == STOP || paused) return;

        int newX = head->x, newY = head->y;
        if (dir == UP) newY--;
        else if (dir == DOWN) newY++;
        else if (dir == LEFT) newX--;
        else if (dir == RIGHT) newX++;

        if (newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT || isObstacle(newX, newY)) {
            gameOver = true;
            return;
        }

        for (Node* temp = head->next; temp; temp = temp->next) {
            if (temp->x == newX && temp->y == newY) {
                gameOver = true;
                return;
            }
        }

        Node* newHead = new Node(newX, newY);
        newHead->next = head;
        head = newHead;

        bool ateFruit = false;
        for (auto it = fruits.begin(); it != fruits.end(); ) {
            if (it->x == newX && it->y == newY) {
                if (it->type == NORMAL) {
                    score += 10;
                    speed = max(50000, speed - 5000);
                } else {
                    score += 5;
                    speed = min(300000, speed + 10000);
                }
                ateFruit = true;
                it = fruits.erase(it);
            } else {
                ++it;
            }
        }

        if (ateFruit) {
            spawnFruit();
        } else {
            Node* temp = head;
            while (temp->next->next) temp = temp->next;
            delete temp->next;
            temp->next = nullptr;
            tail = temp;
        }
    }

    void run() {
        while (true) {
            resetGame();
            while (!gameOver) {
                draw();
                input();
                if (!paused) {
                    logic();
                }
                int adjusted_speed = speed;
                if (dir == UP || dir == DOWN) {
                    adjusted_speed = (speed*5)/4;
                }
                usleep(adjusted_speed);
            }
            maxScore = max(maxScore, score);
            
            while (kbhit()) getchar();
            
            cout << "\033[H\033[J";
            cout << COLOR_BOLD COLOR_RED "\n  GAME OVER!\n\n" COLOR_RESET
                 << COLOR_BOLD "  Final Score: " COLOR_GREEN << score << COLOR_RESET "\n"
                 << COLOR_BOLD "  Max Score: " COLOR_YELLOW << maxScore << COLOR_RESET "\n\n"
                 << COLOR_BOLD "  Press " COLOR_GREEN "R" COLOR_RESET COLOR_BOLD " to restart\n"
                 << "  Press " COLOR_RED "X" COLOR_RESET COLOR_BOLD " to exit\n" COLOR_RESET;
            
            while (true) {
                if (kbhit()) {
                    char choice = getchar();
                    if (choice == 'r' || choice == 'R') {
                        break;
                    } else if (choice == 'x' || choice == 'X') {
                        return;
                    }
                }
                usleep(50000);
            }
        }
    }
};

int main() {
    srand(time(0));
    SnakeGame game;
    game.run();
    return 0;
}
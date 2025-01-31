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
#define COLOR_RED     "\033[31m"     
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

// Game dimensions
const int WIDTH = 60;  // Increased by 1.5 times
const int HEIGHT = 30; // Increased by 1.5 times

// Directions
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

class SnakeGame {
private:
    bool gameOver;
    bool gameStarted;
    Node* head;
    Node* tail;
    vector<pair<int, int>> fruits; // Multiple fruits
    Direction dir;
    int score;
    int speed;
    vector<pair<int, int>> obstacles;
    chrono::steady_clock::time_point startTime;
    int maxScore;

    void spawnFruit() {
        int attempts = 0;
        while (attempts < 100) {
            int x = rand() % WIDTH;
            int y = rand() % HEIGHT;
            if (!isObstacle(x, y) && !isSnakeBody(x, y)) {
                fruits.push_back({x, y});
                return;
            }
            attempts++;
        }
        // If no valid position found, place fruit at a default position
        fruits.push_back({WIDTH / 2, HEIGHT / 2});
    }

    void spawnObstacles() {
        obstacles.clear();
        int numObstacles = 5 + rand() % 10;
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
            if (fruit.first == x && fruit.second == y) return true;
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
        spawnFruit(); // Spawn initial fruit
        spawnObstacles();
    }

    void draw() {
        // Move cursor to top-left corner
        cout << "\033[H";

        // Draw top border
        cout << COLOR_BOLD COLOR_CYAN;
        for (int i = 0; i < WIDTH + 2; i++) cout << "■";
        cout << COLOR_RESET << endl;

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH + 2; x++) {
                if (x == 0 || x == WIDTH + 1) {
                    cout << COLOR_BOLD COLOR_CYAN << "■" COLOR_RESET;
                } else if (x == head->x && y == head->y) {
                    cout << COLOR_BOLD COLOR_YELLOW "●" COLOR_RESET; // Snake head
                } else if (isFruit(x, y)) {
                    cout << COLOR_BOLD COLOR_RED "◆" COLOR_RESET;    // Fruit
                } else if (isObstacle(x, y)) {
                    cout << COLOR_BOLD COLOR_MAGENTA "▒" COLOR_RESET; // Obstacle
                } else {
                    bool isBody = false;
                    for (Node* temp = head->next; temp; temp = temp->next) {
                        if (temp->x == x && temp->y == y) {
                            cout << COLOR_BOLD COLOR_GREEN "○" COLOR_RESET; // Snake body
                            isBody = true;
                            break;
                        }
                    }
                    if (!isBody) cout << " ";
                }
            }
            cout << endl;
        }

        // Draw bottom border
        cout << COLOR_BOLD COLOR_CYAN;
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
                case 'x': case 'X': gameOver = true; break;
            }
        }
    }

    void logic() {
        if (dir == STOP) return;

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
            if (it->first == newX && it->second == newY) {
                score += 10;
                speed = max(50000, speed - 5000);
                it = fruits.erase(it);
                ateFruit = true;
            } else {
                ++it;
            }
        }

        if (ateFruit) {
            spawnFruit(); // Spawn a new fruit when one is eaten
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
                logic();
                usleep(speed);
            }
            maxScore = max(maxScore, score);
            
            // Clear input buffer
            while (kbhit()) getchar();
            
            // Game over screen
            cout << "\033[H\033[J"; // Clear screen
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
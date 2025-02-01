#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <vector>
#include <chrono>

using namespace std;

// Game dimensions
const int WIDTH = 50;
const int HEIGHT = 25;

// Directions
enum Direction { STOP = 0, UP, DOWN, LEFT, RIGHT };

// Function to capture non-blocking keypress
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
    Node* head;
    Node* tail;
    int fruitX, fruitY;
    Direction dir;
    int score;
    int speed;
    vector<pair<int, int>> obstacles;
    chrono::steady_clock::time_point startTime;
    int maxScore;

    void spawnFruit() {
        do {
            fruitX = rand() % WIDTH;
            fruitY = rand() % HEIGHT;
        } while (isObstacle(fruitX, fruitY) || isSnakeBody(fruitX, fruitY));
    }

    void spawnObstacles() {
        obstacles.clear();
        int numObstacles = 5 + rand() % 10; // Random number of obstacles
        for (int i = 0; i < numObstacles; i++) {
            int x, y;
            do {
                x = rand() % WIDTH;
                y = rand() % HEIGHT;
            } while (isSnakeBody(x, y) || (x == fruitX && y == fruitY));
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
        dir = STOP;
        score = 0;
        speed = 150000;

        clearSnake(); // Clear existing snake before resetting

        head = new Node(WIDTH / 2, HEIGHT / 2);
        tail = head;
        tail->next = new Node(head->x - 1, head->y);
        tail = tail->next;
        tail->next = new Node(head->x - 2, head->y);
        tail = tail->next;

        spawnFruit();
        spawnObstacles();
        startTime = chrono::steady_clock::now();
    }

    void draw() {
        system("clear");

        // Top border
        for (int i = 0; i < WIDTH + 2; i++) cout << "▓";
        cout << endl;

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH + 2; x++) {
                if (x == 0 || x == WIDTH + 1) {
                    cout << "▓";
                } else if (x == head->x && y == head->y) {
                    cout << "●"; // Snake head
                } else if (x == fruitX && y == fruitY) {
                    cout << "F"; // Fruit
                } else if (isObstacle(x, y)) {
                    cout << "#"; // Obstacle
                } else {
                    bool isBody = false;
                    for (Node* temp = head->next; temp; temp = temp->next) {
                        if (temp->x == x && temp->y == y) {
                            cout << "○"; // Snake body
                            isBody = true;
                            break;
                        }
                    }
                    if (!isBody) cout << " ";
                }
            }
            cout << endl;
        }

        for (int i = 0; i < WIDTH + 2; i++) cout << "▓";
        cout << endl;

        // Display score, time, and max score
        auto now = chrono::steady_clock::now();
        int elapsedTime = chrono::duration_cast<chrono::seconds>(now - startTime).count();
        cout << "Score: " << score << " | Time: " << elapsedTime << "s | Max Score: " << maxScore << endl;
        cout << "Use W/A/S/D to move. Press X to quit." << endl;
    }

    void input() {
        if (kbhit()) {
            char key = getchar();
            switch (key) {
                case 'w': case 'W': if (dir != DOWN) dir = UP; break;
                case 's': case 'S': if (dir != UP) dir = DOWN; break;
                case 'a': case 'A': if (dir != RIGHT) dir = LEFT; break;
                case 'd': case 'D': if (dir != LEFT) dir = RIGHT; break;
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

        if (newX == fruitX && newY == fruitY) {
            score += 10;
            speed = max(50000, speed - 5000);
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
                logic();
                usleep(speed);
            }
            maxScore = max(maxScore, score);
            cout << "Game Over! Score: " << score << endl;
            cout << "Press R to restart or X to quit: ";
            char choice;
            cin >> choice;
            if (choice != 'r' && choice != 'R') break;
        }
    }
};

int main() {
    srand(time(0));
    SnakeGame game;
    game.run();
    return 0;
}
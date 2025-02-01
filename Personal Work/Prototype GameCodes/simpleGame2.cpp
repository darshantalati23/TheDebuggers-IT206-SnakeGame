#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

using namespace std;

const int WIDTH = 50;
const int HEIGHT = 25;
const int OBSTACLE_COUNT = 10;

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

class SnakeGame {
private:
    bool gameOver, paused;
    int headX, headY;
    vector<pair<int, int>> snake;
    int fruitX, fruitY;
    vector<pair<int, int>> obstacles;
    Direction dir;
    int score, maxScore;
    int speed;
    time_t lastFruitTime;
    double multiplier;

public:
    SnakeGame() : maxScore(0) {
        resetGame();
    }

    void resetGame() {
        gameOver = false;
        paused = false;
        dir = STOP;
        headX = WIDTH / 2;
        headY = HEIGHT / 2;
        snake = {{headX, headY}, {headX - 1, headY}, {headX - 2, headY}};
        score = 0;
        speed = 150000;
        multiplier = 1.0;
        lastFruitTime = time(0);
        spawnFruit();
        generateObstacles();
    }

    void spawnFruit() {
        fruitX = rand() % WIDTH;
        fruitY = rand() % HEIGHT;
    }

    void generateObstacles() {
        obstacles.clear();
        for (int i = 0; i < OBSTACLE_COUNT; i++) {
            obstacles.push_back({rand() % WIDTH, rand() % HEIGHT});
        }
    }

    void draw() {
        system("clear");

        for (int i = 0; i < WIDTH + 2; i++) cout << "□";
        cout << endl;

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH + 2; x++) {
                if (x == 0 || x == WIDTH + 1) {
                    cout << "□";
                } else if (x == headX && y == headY) {
                    cout << "0";
                } else if (x == fruitX && y == fruitY) {
                    cout << "○";
                } else {
                    bool printed = false;
                    for (auto &segment : snake) {
                        if (segment.first == x && segment.second == y) {
                            cout << "O";
                            printed = true;
                            break;
                        }
                    }
                    for (auto &obs : obstacles) {
                        if (obs.first == x && obs.second == y) {
                            cout << "□";
                            printed = true;
                            break;
                        }
                    }
                    if (!printed) cout << " ";
                }
            }
            cout << endl;
        }

        for (int i = 0; i < WIDTH + 2; i++) cout << "□";
        cout << endl;

        cout << "Score: " << score << " | Max Score: " << maxScore << " | Speed: " << (150000 - speed) / 1000 << "%" << endl;
        cout << "Multiplier: x" << multiplier << endl;
        cout << "W/A/S/D to move, X to quit, P to pause." << endl;
        if (paused) cout << "[PAUSED] Press P to resume." << endl;
    }

    void input() {
        if (kbhit()) {
            char key = getchar();
            if (key == 'p' || key == 'P') {
                paused = !paused;
            } else if (!paused) {
                switch (key) {
                    case 'w': case 'W': if (dir != DOWN) dir = UP; break;
                    case 's': case 'S': if (dir != UP) dir = DOWN; break;
                    case 'a': case 'A': if (dir != RIGHT) dir = LEFT; break;
                    case 'd': case 'D': if (dir != LEFT) dir = RIGHT; break;
                    case 'x': case 'X': gameOver = true; break;
                }
            }
        }
    }

    void logic() {
        if (paused || dir == STOP) return;

        for (int i = snake.size() - 1; i > 0; i--) {
            snake[i] = snake[i - 1];
        }

        if (dir == UP) headY--;
        else if (dir == DOWN) headY++;
        else if (dir == LEFT) headX--;
        else if (dir == RIGHT) headX++;

        snake[0] = {headX, headY};

        if (headX < 0 || headX >= WIDTH || headY < 0 || headY >= HEIGHT) gameOver = true;
        for (size_t i = 1; i < snake.size(); i++) {
            if (snake[i].first == headX && snake[i].second == headY) gameOver = true;
        }
        for (auto &obs : obstacles) {
            if (obs.first == headX && obs.second == headY) gameOver = true;
        }

        if (headX == fruitX && headY == fruitY) {
            time_t now = time(0);
            if (difftime(now, lastFruitTime) < 5) multiplier += 0.2;
            else multiplier = 1.0;
            lastFruitTime = now;
            score += 10 * multiplier;
            snake.push_back({-1, -1});
            spawnFruit();
            if (speed > 50000) speed -= 5000;
        }
    }

    void run() {
        while (!gameOver) {
            draw();
            input();
            logic();
            usleep(speed);
        }
        cout << "Game Over! Final Score: " << score << endl;
    }
};

int main() {
    srand(time(0));
    SnakeGame game;
    game.run();
    return 0;
}

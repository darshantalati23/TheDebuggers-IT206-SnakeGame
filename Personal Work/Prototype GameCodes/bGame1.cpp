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

enum Direction { STOP = 0, UP, DxOWN, LEFT, RIGHT };

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
    bool gameOver;
    int headX, headY;
    vector<pair<int, int>> snake;
    int fruitX, fruitY;
    Direction dir;
    int score;
    int maxScore;

public:
    SnakeGame() : maxScore(0) {
        resetGame();
    }

    void resetGame() {
        gameOver = false;
        dir = STOP;
        headX = WIDTH / 2;
        headY = HEIGHT / 2;
        snake = {{headX, headY}, {headX - 1, headY}, {headX - 2, headY}};
        score = 0;
        spawnFruit();
    }

    void spawnFruit() {
        fruitX = rand() % WIDTH;
        fruitY = rand() % HEIGHT;
    }

    void draw() {
        system("clear");

        for (int i = 0; i < WIDTH + 2; i++) cout << "#";
        cout << endl;

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH + 2; x++) {
                if (x == 0 || x == WIDTH + 1) {
                    cout << "#";
                } else if (x == headX && y == headY) {
                    cout << "0";
                } else if (x == fruitX && y == fruitY) {
                    cout << "F";
                } else {
                    bool isBody = false;
                    for (auto &segment : snake) {
                        if (segment.first == x && segment.second == y) {
                            cout << "O";
                            isBody = true;
                            break;
                        }
                    }
                    if (!isBody) {
                        // Draw grid lines every 5 units
                        if ((x - 1) % 5 == 0 || y % 5 == 0) {
                            cout << ".";  // Grid point
                        } else {
                            cout << " ";
                        }
                    }
                }
            }
            cout << endl;
        }

        for (int i = 0; i < WIDTH + 2; i++) cout << "#";
        cout << endl;

        cout << "Score: " << score << endl;
        cout << "Max Score: " << maxScore << endl;
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
        
        for (int i = snake.size() - 1; i > 0; i--) {
            snake[i] = snake[i - 1];
        }

        if (dir == UP) headY--;
        else if (dir == DOWN) headY++;
        else if (dir == LEFT) headX--;
        else if (dir == RIGHT) headX++;

        snake[0] = {headX, headY};

        if (headX < 0 || headX >= WIDTH || headY < 0 || headY >= HEIGHT) {
            gameOver = true;
        }

        for (size_t i = 1; i < snake.size(); i++) {
            if (snake[i].first == headX && snake[i].second == headY) {
                gameOver = true;
            }
        }

        if (headX == fruitX && headY == fruitY) {
            score += 10;
            snake.push_back({-1, -1});
            spawnFruit();
        }
    }

    void gameOverScreen() {
        if (score > maxScore) {
            maxScore = score;
        }
        cout << "Game Over!" << endl;
        cout << "Final Score: " << score << endl;
        cout << "Max Score: " << maxScore << endl;
        cout << "Press R to restart or Q to quit." << endl;
    }

    void run() {
        while (true) {
            while (!gameOver) {
                draw();
                input();
                logic();
                usleep(150000);
            }
            gameOverScreen();

            while (true) {
                if (kbhit()) {
                    char key = getchar();
                    if (key == 'r' || key == 'R') {
                        resetGame();
                        break;
                    } else if (key == 'q' || key == 'Q') {
                        return;
                    }
                }
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
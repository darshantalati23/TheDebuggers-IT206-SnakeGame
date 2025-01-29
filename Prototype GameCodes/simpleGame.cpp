#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

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

class SnakeGame {
private:
    bool gameOver;
    int headX, headY;
    vector<pair<int, int>> snake;
    int fruitX, fruitY;
    Direction dir;
    int score;

public:
    SnakeGame() {
        resetGame();
    }

    void resetGame() {
        gameOver = false;
        dir = STOP;
        headX = WIDTH / 2;
        headY = HEIGHT / 2;

        // Initialize snake with length 3
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

        // Top border
        for (int i = 0; i < WIDTH + 2; i++) cout << "#";
        cout << endl;

        // Board content
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH + 2; x++) {
                if (x == 0 || x == WIDTH + 1) {
                    cout << "#"; // Borders
                } else if (x == headX && y == headY) {
                    cout << "0"; // Snake head
                } else if (x == fruitX && y == fruitY) {
                    cout << "F"; // Fruit
                } else {
                    bool isBody = false;
                    for (auto &segment : snake) {
                        if (segment.first == x && segment.second == y) {
                            cout << "O";
                            isBody = true;
                            break;
                        }
                    }
                    if (!isBody) cout << " ";
                }
            }
            cout << endl;
        }

        // Bottom border
        for (int i = 0; i < WIDTH + 2; i++) cout << "#";
        cout << endl;

        cout << "Score: " << score << endl;
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
        if (dir == STOP) return; // Don't move if no direction is given
        
        // Move the body
        for (int i = snake.size() - 1; i > 0; i--) {
            snake[i] = snake[i - 1];
        }

        // Move the head
        if (dir == UP) headY--;
        else if (dir == DOWN) headY++;
        else if (dir == LEFT) headX--;
        else if (dir == RIGHT) headX++;

        snake[0] = {headX, headY};

        // Check collision with walls
        if (headX < 0 || headX >= WIDTH || headY < 0 || headY >= HEIGHT) {
            gameOver = true;
        }

        // Check collision with itself
        for (size_t i = 1; i < snake.size(); i++) {
            if (snake[i].first == headX && snake[i].second == headY) {
                gameOver = true;
            }
        }

        // Check if fruit is eaten
        if (headX == fruitX && headY == fruitY) {
            score += 10;
            snake.push_back({-1, -1}); // Add a new segment
            spawnFruit();
        }
    }

    void run() {
        while (!gameOver) {
            draw();
            input();
            logic();
            usleep(100000); // Slow the game loop
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

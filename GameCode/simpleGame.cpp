#include <chrono>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <bits/stdc++.h>
using namespace std;

// Function to enable non-blocking input
char getInput() {
    struct termios oldt, newt;
    char ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt); // Save old settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Disable buffering and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore old settings

    return ch;
}

// Check if a key has been pressed
bool kbhit() {
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
        return true;
    }

    return false;
}

// Class representing the game board
class GameBoard {
    int width, height;
public:
    GameBoard(int w, int h) : width(w), height(h) {}

    void draw(const vector<pair<int, int>> &snake, pair<int, int> &food) {
        system("clear");
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                if (i == 0 || i == height - 1 || j == 0 || j == width - 1) {
                    cout << "#"; // Border
                } else if (make_pair(i, j) == food) {
                    cout << "F"; // Food
                } else {
                    bool isBody = false;
                    for (auto &segment : snake) {
                        if (segment == make_pair(i, j)) {
                            isBody = true;
                            break;
                        }
                    }
                    cout << (isBody ? "O" : " "); // Snake or empty space
                }
            }
            cout << endl;
        }
    }

    int getWidth() { return width; }
    int getHeight() { return height; }
};

// Class representing the snake
class Snake {
    vector<pair<int, int>> body;
    char direction;
public:
    Snake(int x, int y) {
        body.push_back({x, y});
        direction = 'D'; // Default direction
    }

    void move(char dir) {
        direction = dir;
        pair<int, int> head = body.front();

        if (direction == 'W') head.first--;      // Up
        else if (direction == 'S') head.first++; // Down
        else if (direction == 'A') head.second--;// Left
        else if (direction == 'D') head.second++;// Right

        body.insert(body.begin(), head);
        body.pop_back(); // Remove the tail
    }

    void grow() {
        body.push_back(body.back());
    }

    bool checkCollision(int width, int height) {
        // Check wall collision
        auto head = body.front();
        if (head.first <= 0 || head.first >= height - 1 ||
            head.second <= 0 || head.second >= width - 1) {
            return true;
        }

        // Check self-collision
        for (size_t i = 1; i < body.size(); ++i) {
            if (body[i] == head) return true;
        }

        return false;
    }

    bool eatFood(pair<int, int> &food) {
        if (body.front() == food) {
            grow();
            return true;
        }
        return false;
    }

    vector<pair<int, int>> getBody() {
        return body;
    }
};

// Generate new food position
pair<int, int> generateFood(int width, int height, const vector<pair<int, int>> &snake) {
    pair<int, int> food;
    do {
        food.first = rand() % (height - 2) + 1;
        food.second = rand() % (width - 2) + 1;
    } while (find(snake.begin(), snake.end(), food) != snake.end());
    return food;
}

int main() {
    srand(time(0));
    const int width = 20, height = 10;
    GameBoard board(width, height);
    Snake snake(height / 2, width / 2);
    pair<int, int> food = generateFood(width, height, snake.getBody());
    char input = 'D'; // Initial direction

    while (true) {
        board.draw(snake.getBody(), food);

        if (kbhit()) {
            char newInput = getInput();
            if (newInput == 'W' || newInput == 'A' || newInput == 'S' || newInput == 'D') {
                input = newInput;
            }
        }

        snake.move(input);

        if (snake.eatFood(food)) {
            food = generateFood(width, height, snake.getBody());
        }

        if (snake.checkCollision(width, height)) {
            cout << "Game Over! Final Score: " << snake.getBody().size() - 1 << endl;
            break;
        }

        this_thread::sleep_for(chrono::milliseconds(200)); // Control game speed
    }

    return 0;
}
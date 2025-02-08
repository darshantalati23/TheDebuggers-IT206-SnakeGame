#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
using namespace std;

const int WIDTH = 20;
const int HEIGHT = 20;

struct SnakeNode {
    int x;
    int y;
    char symbol;
    SnakeNode* next;
    SnakeNode(int xPos, int yPos, char s, SnakeNode* n = nullptr)
        : x(xPos), y(yPos), symbol(s), next(n) {}
};

SnakeNode* head = nullptr;
SnakeNode* tail = nullptr;
int fruitX, fruitY;
int score = 0;
bool gameOver = false;

void generateFruit() {
    bool valid = false;
    do {
        fruitX = rand() % WIDTH;
        fruitY = rand() % HEIGHT;
        valid = true;
        SnakeNode* current = head;
        while (current != nullptr) {
            if (current->x == fruitX && current->y == fruitY) {
                valid = false;
                break;
            }
            current = current->next;
        }
    } while (!valid);
}

void displayGrid() {
    system("cls"); // Use "clear" for Unix-based systems
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (x == fruitX && y == fruitY) {
                cout << 'F';
            } else {
                bool isSnakePart = false;
                SnakeNode* current = head;
                while (current != nullptr) {
                    if (current->x == x && current->y == y) {
                        cout << current->symbol;
                        isSnakePart = true;
                        break;
                    }
                    current = current->next;
                }
                if (!isSnakePart) {
                    cout << ' ';
                }
            }
        }
        cout << endl;
    }
}

int main() {
    srand(time(0));

    // Initialize snake with length 3: "--O"
    head = new SnakeNode(5, 10, 'O');
    head->next = new SnakeNode(4, 10, '-');
    head->next->next = new SnakeNode(3, 10, '-');
    tail = head->next->next;

    generateFruit();

    while (!gameOver) {
        displayGrid();
        cout << "Score: " << score << endl;

        string input;
        getline(cin, input);
        if (input.empty()) {
            continue;
        }
        char dir = toupper(input[0]);

        int newX = head->x;
        int newY = head->y;
        switch (dir) {
            case 'W': newY--; break;
            case 'S': newY++; break;
            case 'A': newX--; break;
            case 'D': newX++; break;
            default: 
                cout << "Invalid direction. Use WASD." << endl;
                continue;
        }

        // Check wall collision
        if (newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT) {
            gameOver = true;
            break;
        }

        // Check self collision
        SnakeNode* current = head;
        bool collision = false;
        while (current != nullptr) {
            if (current->x == newX && current->y == newY) {
                collision = true;
                break;
            }
            current = current->next;
        }
        if (collision) {
            gameOver = true;
            break;
        }

        bool ateFruit = (newX == fruitX && newY == fruitY);

        // Create new head and update previous head to body
        SnakeNode* newHead = new SnakeNode(newX, newY, 'O');
        newHead->next = head;
        head = newHead;
        head->next->symbol = '-';

        if (ateFruit) {
            score += 10;
            generateFruit();
        } else {
            // Remove tail
            SnakeNode* prev = head;
            while (prev->next != tail) {
                prev = prev->next;
            }
            delete tail;
            tail = prev;
            tail->next = nullptr;
        }
    }

    cout << "Game Over! Final Score: " << score << endl;

    // Clean up linked list
    SnakeNode* current = head;
    while (current != nullptr) {
        SnakeNode* temp = current;
        current = current->next;
        delete temp;
    }

    return 0;
}
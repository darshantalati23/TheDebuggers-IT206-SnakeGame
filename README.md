# Classic Snake Game
## The Debuggers
### Team Members
###### - Aarohi Mehta (202401002)
###### - Alvita Thakor (202401012)
###### - Darshan Talati (202401046)
###### - Dharmesh Upadhyay (202401049)


### Table of Contents
- [Description](#-description)
- [Features](#-features)
- [Usage](#️-usage)
  - [Installation & Running](#installation--running-ubuntu)
  - [How to Play](#how-to-play)
- [Contributors](#-contributors)
- [License](#-license)
- [Credits](#-credits)


### 🚀 Description
A terminal-based implementation of the classic Snake game with modern features:
- Dynamic obstacles
- Speed-modifying fruits (red/green)
- Score tracking with time-based metrics
- Customizable gameplay (name input, obstacle toggle)

Built using C++ with ANSI escape codes for terminal graphics and real-time keyboard input handling.


### ✨ Features
- **Obstacle System**: Randomly generated barriers
- **Colored Fruits**: 
  - 🔴 Red: +10 pts, increases speed
  - 🟢 Green: +5 pts, decreases speed
- **Adaptive Difficulty**: Speed increases with score
- **Player Stats**: Name, score, time, and max score tracking
- **Pause/Restart** functionality
- **Directions/Movements** move the snake with the keys w/s/a/d

### 🕹️ Usage

##### Installation & Running (Ubuntu)
  - Install the required compiler (if not installed):
    `sudo apt install g++`
  - Compile and Run the Game:
    `theSnakeGame.cpp -o theSnakeGame`
    `./theSnakeGame`

##### How to Play
- The game starts with a snake of length 3 (--O).
- Use WASD keys to control the snake's movement.
- Eat fruits (F) to grow and score points.
- Avoid walls, obstacles, and self-collisions, or the game ends.
- The game speeds up when you eat red fruits and slows down when you eat green fruits.
- Pause anytime using the "P" key and restart using the "R" key.
- The game continuously generates fruits until you lose.


### 🤝 Contributors
The game was built and improved with contributions from:

*Aarohi & Alvita*: Implemented the snake movement and logic.
*Darshan & Dharmesh*: Implemented the fruit spawning and game board logic.
**All** worked on gameplay updates and optimization.


### 📜 License
This project is not licensed. All rights reserved.


### 👥 Credits
The game's design was inspired by the tutorial on [ ].
Thanks to Chatgpt for providing helpful resources for making the basic idea of the snake game.


### 😉 Group Icon
<img src="GroupIcon.jpg" width="250px">
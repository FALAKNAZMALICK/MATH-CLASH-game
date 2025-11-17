# DATA STRUCTURE ALGORITHMS AND APPLICATION

 Project Title: MATH CLASH

 Group Members
- Musfirah Shakil (CT-24051)
- Kinza (CT-24056)
- Falak Naz (CT-24059)

---

## Project Overview

Math Clash is an interactive educational game designed to make practicing mathematics fun and challenging.  
It combines gameplay with learning by presenting players with randomly generated math expressions of varying difficulty, which must be solved under time pressure.

Key features:
- User authentication (sign up, login, password management)
- Scoring system with point rewards and deductions
- Retry failed questions feature
- Dashboard to track progress
- Leaderboard for friendly competition

The game uses SFML (Simple and Fast Multimedia Library) for:
- Graphics and window management
- User input (mouse clicks, keyboard)
- Audio and timing
- Interactive buttons, text boxes, countdown timers
- Smooth animations and rendering

---

## DSA Concepts Used

 1. Vector (Dynamic Array)
- Stores all registered users.
- Efficient insertion and traversal.
- Used for login verification and score tracking.

 2. List (Doubly Linked List)
- Tracks incorrectly answered questions.
- Supports "Retry Failed Questions" feature.
- Maintains order and allows efficient insertion/removal.

 3. Stack (Expression Evaluation)
- Two stacks: numbers and operators.
- Ensures operator precedence (multiplication/division before addition/subtraction).
- Evaluates math expressions correctly.

 4. Sorting Algorithm (STL sort / Introsort)
- Used in leaderboard to rank users by score.
- Converts user data into vector of pairs (score, username).
- Sorts in descending order for clear ranking.

 5. Expression Parsing Algorithm
- Generates math questions and computes correct answers.
- Uses stacks to parse and evaluate expressions step‑by‑step.
- Guarantees mathematically correct evaluation.

---

## Conclusion

Math Clash is a fun and useful game that helps players practice math in an enjoyable way.  
It works smoothly because of **SFML** and uses important data structures like vectors, lists, stacks, and sorting algorithms to manage users and questions.  
Overall, the project shows how programming and learning can come together to create a helpful educational game.

---

## 📄 Full Report (PDF)

For the complete detailed report, please see the PDF version:  
[Download / View Report.pdf](Report.pdf)

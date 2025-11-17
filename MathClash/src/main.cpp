#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <stack>
#include <iomanip>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <algorithm>
#include <list>
#include <thread>

// SFML Graphics Library for game visuals
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

using namespace std;
using namespace sf;

// Our game's building blocks - how we store questions and player info
struct Question {
    string expression;
    double answer;
    bool answered_correctly = false;
    bool skipped = false;
};

struct User {
    string username;
    string password;
    int total_score = 0;
    int games_played = 0;
    int games_won = 0;
    int games_lost = 0;
    list<Question> failed_questions;

    double get_win_rate() const {
        if (games_played == 0) return 0.0;
        return (static_cast<double>(games_won) / games_played) * 100.0;
    }
};

// Game-wide variables that track everything happening
vector<User> all_users;
User current_user;

// SFML graphics components
RenderWindow* window;
Font mainFont;
Texture backgroundTexture;
Sprite backgroundSprite;
bool graphics_mode = true;
Clock gameClock;
Time remainingTime;

// All the different screens our game can show
enum GameState {
    AUTH_MENU,
    MAIN_MENU,
    PLAYING_LEVEL,
    DASHBOARD,
    LEADERBOARD,
    RETRY_FAILED,
    LEVEL_START,
    LEVEL_END
};

GameState currentState = AUTH_MENU;
int currentLevel = 0;
Question currentQuestion;
string userInputText = "";
string usernameInput = "";
string passwordInput = "";
bool inputActive = false;
bool usernameActive = true;
bool passwordActive = false;
bool gameStarted = false;
bool timeUp = false;
string levelMessage = "";
int levelScore = 0;
bool loginError = false;

// Loading game assets like fonts and images
bool loadResources() {
    // Look for font files in common locations
    const vector<string> fontPaths = {
        "arial.ttf",
        "C:/Windows/Fonts/arial.ttf", 
        "C:/Windows/Fonts/Arial.ttf",
        "assets/arial.ttf"
    };
    
    bool fontLoaded = false;
    for (const auto& path : fontPaths) {
        if (mainFont.loadFromFile(path)) {
            cout << "Font loaded successfully from: " << path << endl;
            fontLoaded = true;
            break;
        }
    }
    
    // Try different background image names and formats
    const vector<string> bgPaths = {
        "background.jpg",
        "background.png",
        "bg.jpg", 
        "math-bg.jpg",
        "image.jpg",
        "bg.png",
        "math.jpg"
    };
    
    bool bgLoaded = false;
    for (const auto& path : bgPaths) {
        if (backgroundTexture.loadFromFile(path)) {
            cout << "Background image loaded from: " << path << endl;
            bgLoaded = true;
            break;
        }
    }
    
    if (bgLoaded) {
        backgroundSprite.setTexture(backgroundTexture);
        
        // Make background fit our game window perfectly
        Vector2u textureSize = backgroundTexture.getSize();
        float scaleX = 800.0f / textureSize.x;
        float scaleY = 600.0f / textureSize.y;
        backgroundSprite.setScale(scaleX, scaleY);
        
        // Make background slightly see-through
        backgroundSprite.setColor(Color(255, 255, 255, 180));
    } else {
        cout << "No background image found - using solid color instead\n";
    }
    
    return fontLoaded;
}

// Draw text on screen with various styling options
void drawText(const string& text, float x, float y, int size, Color color = Color::White, bool center = false) {
    if (!graphics_mode) return;
    
    Text sfText(text, mainFont, size);
    sfText.setFillColor(color);
    
    if (center) {
        FloatRect bounds = sfText.getLocalBounds();
        sfText.setPosition(x - bounds.width / 2, y);
    } else {
        sfText.setPosition(x, y);
    }
    
    window->draw(sfText);
}

// Create clickable buttons with text
void drawButton(const string& text, float x, float y, float width, float height, Color bgColor = Color::Blue, Color textColor = Color::White) {
    if (!graphics_mode) return;
    
    RectangleShape button(Vector2f(width, height));
    button.setPosition(x, y);
    button.setFillColor(bgColor);
    button.setOutlineColor(Color::White);
    button.setOutlineThickness(2);
    window->draw(button);
    
    drawText(text, x + width / 2, y + height / 2 - 10, 20, textColor, true);
}

// Draw input boxes for typing text
void drawInputBox(float x, float y, float width, float height, const string& text, bool active = false) {
    if (!graphics_mode) return;
    
    RectangleShape box(Vector2f(width, height));
    box.setPosition(x, y);
    box.setFillColor(active ? Color(50, 50, 50) : Color(30, 30, 30));
    box.setOutlineColor(active ? Color::Yellow : Color::White);
    box.setOutlineThickness(2);
    window->draw(box);
    
    drawText(text, x + 10, y + 15, 24);
}

// Check if mouse is hovering over an area
bool isMouseOver(float x, float y, float width, float height) {
    if (!graphics_mode) return false;
    
    Vector2i mousePos = Mouse::getPosition(*window);
    return mousePos.x >= x && mousePos.x <= x + width &&
           mousePos.y >= y && mousePos.y <= y + height;
}

// Draw the login/signup screen
void drawAuthMenu() {
    // Show background if available
    if (backgroundTexture.getSize().x > 0) {
        window->draw(backgroundSprite);
    } else {
        RectangleShape solidBg(Vector2f(800, 600));
        solidBg.setFillColor(Color(20, 25, 45, 240));
        window->draw(solidBg);
    }

    drawText("MATH CLASH", 400, 100, 48, Color::Cyan, true);
    
    drawText("Username:", 200, 200, 24);
    drawInputBox(200, 230, 400, 40, usernameInput, usernameActive);
    
    drawText("Password:", 200, 300, 24);
    drawInputBox(200, 330, 400, 40, string(passwordInput.size(), '*'), passwordActive);
    
    // Show error if login fails
    if (loginError) {
        drawText("Login failed! Please sign up first.", 400, 380, 20, Color::Red, true);
    }
    
    drawButton("Login", 250, 400, 150, 50, Color::Green);
    drawButton("Sign Up", 450, 400, 150, 50, Color::Blue);
    drawButton("Exit", 350, 470, 100, 40, Color::Red);
    
    drawText("Click fields to type, buttons to action", 400, 530, 16, Color::Yellow, true);
}

// Draw the main navigation menu
void drawMainMenu() {
    if (backgroundTexture.getSize().x > 0) {
        window->draw(backgroundSprite);
    } else {
        RectangleShape solidBg(Vector2f(800, 600));
        solidBg.setFillColor(Color(20, 25, 45, 240));
        window->draw(solidBg);
    }

    drawText("MAIN MENU", 400, 80, 48, Color::Yellow, true);
    drawText("Welcome, " + current_user.username + "!", 400, 150, 24, Color::Green, true);
    
    drawButton("Play Levels", 300, 200, 200, 50, Color::Green);
    drawButton("Retry Failed", 300, 270, 200, 50, Color::Blue);
    drawButton("Dashboard", 300, 340, 200, 50, Color::Magenta);
    drawButton("Leaderboard", 300, 410, 200, 50, Color(139, 69, 19));
    drawButton("Logout", 300, 480, 200, 50, Color::Red);
}

// Draw the actual gameplay screen with question and timer
void drawGameLevel() {
    if (backgroundTexture.getSize().x > 0) {
        window->draw(backgroundSprite);
    } else {
        RectangleShape solidBg(Vector2f(800, 600));
        solidBg.setFillColor(Color(20, 25, 45, 240));
        window->draw(solidBg);
    }

    vector<int> levelTimes = {20, 15, 10};
    int timeLimit = levelTimes[currentLevel];
    
    drawText("Level " + to_string(currentLevel + 1), 400, 50, 36, Color::Yellow, true);
    drawText("Time per question: " + to_string(timeLimit) + " seconds", 400, 100, 24, Color::White, true);
    
    drawText("Question: " + currentQuestion.expression, 400, 200, 32, Color::Green, true);
    
    float elapsed = gameClock.getElapsedTime().asSeconds();
    float remaining = timeLimit - elapsed;
    if (remaining < 0) remaining = 0;
    
    string timeText = "Time: " + to_string(static_cast<int>(remaining)) + "s";
    drawText(timeText, 400, 250, 24, remaining < 5 ? Color::Red : Color::White, true);
    
    // Draw timer progress bar
    RectangleShape background(Vector2f(400, 20));
    background.setPosition(200, 280);
    background.setFillColor(Color(50, 50, 50));
    window->draw(background);
    
    RectangleShape progress(Vector2f(400 * (remaining / timeLimit), 20));
    progress.setPosition(200, 280);
    progress.setFillColor(remaining < 5 ? Color::Red : Color::Green);
    window->draw(progress);
    
    drawInputBox(200, 320, 400, 50, userInputText, true);
    drawText("Type answer or 's' to skip", 400, 380, 20, Color::Cyan, true);
    
    drawButton("Submit", 350, 430, 100, 40, Color::Green);
    
    if (remaining <= 0 && !timeUp) {
        timeUp = true;
        drawText("TIME'S UP!", 400, 500, 36, Color::Red, true);
    }
}

// Show player statistics and progress
void drawDashboard() {
    if (backgroundTexture.getSize().x > 0) {
        window->draw(backgroundSprite);
    } else {
        RectangleShape solidBg(Vector2f(800, 600));
        solidBg.setFillColor(Color(20, 25, 45, 240));
        window->draw(solidBg);
    }

    drawText("PLAYER DASHBOARD", 400, 50, 36, Color::Yellow, true);
    
    drawText("Username: " + current_user.username, 200, 120, 24);
    drawText("Total Score: " + to_string(current_user.total_score), 200, 160, 24);
    drawText("Games Played: " + to_string(current_user.games_played), 200, 200, 24);
    drawText("Win Rate: " + to_string(static_cast<int>(current_user.get_win_rate())) + "%", 200, 240, 24);
    drawText("Failed Questions: " + to_string(current_user.failed_questions.size()), 200, 280, 24);
    
    drawButton("Back to Menu", 300, 350, 200, 50, Color::Blue);
}

// Display top players by score
void drawLeaderboard() {
    if (backgroundTexture.getSize().x > 0) {
        window->draw(backgroundSprite);
    } else {
        RectangleShape solidBg(Vector2f(800, 600));
        solidBg.setFillColor(Color(20, 25, 45, 240));
        window->draw(solidBg);
    }

    drawText("LEADERBOARD", 400, 50, 36, Color::Yellow, true);
    
    vector<pair<int, string>> leaders;
    for (auto& u : all_users) {
        leaders.push_back({u.total_score, u.username});
    }
    sort(leaders.rbegin(), leaders.rend());
    
    int y = 120;
    for (int i = 0; i < min(5, (int)leaders.size()); i++) {
        string entry = to_string(i + 1) + ". " + leaders[i].second + " - " + to_string(leaders[i].first) + " pts";
        Color color = Color::White;
        if (i == 0) color = Color::Yellow;
        else if (i == 1) color = Color::Red;
        else if (i == 2) color = Color(205, 127, 50);
        drawText(entry, 400, y, 24, color, true);
        y += 40;
    }
    
    drawButton("Back to Menu", 300, 350, 200, 50, Color(139, 69, 19));
}

// Screen for practicing previously missed questions
void drawRetryFailed() {
    if (backgroundTexture.getSize().x > 0) {
        window->draw(backgroundSprite);
    } else {
        RectangleShape solidBg(Vector2f(800, 600));
        solidBg.setFillColor(Color(20, 25, 45, 240));
        window->draw(solidBg);
    }

    drawText("RETRY FAILED QUESTIONS", 400, 50, 36, Color::Yellow, true);
    
    if (current_user.failed_questions.empty()) {
        drawText("No failed questions to retry!", 400, 200, 24, Color::Green, true);
    } else {
        auto it = current_user.failed_questions.begin();
        drawText("Question: " + it->expression, 400, 150, 28, Color::White, true);
        drawText("Correct Answer: " + to_string(it->answer), 400, 200, 24, Color::Cyan, true);
        
        drawInputBox(200, 250, 400, 50, userInputText, true);
        drawButton("Submit Answer", 300, 320, 200, 50, Color::Green);
        drawButton("Skip", 300, 390, 200, 50, Color::Yellow);
    }
    
    drawButton("Back to Menu", 300, 460, 200, 50, Color::Blue);
}

// Level introduction screen
void drawLevelStart() {
    if (backgroundTexture.getSize().x > 0) {
        window->draw(backgroundSprite);
    } else {
        RectangleShape solidBg(Vector2f(800, 600));
        solidBg.setFillColor(Color(20, 25, 45, 240));
        window->draw(solidBg);
    }

    drawText("LEVEL " + to_string(currentLevel + 1) + " STARTED", 400, 200, 42, Color::Green, true);
    drawText("Get ready for math challenges!", 400, 280, 24, Color::White, true);
    drawText("Click anywhere to continue...", 400, 350, 20, Color::Yellow, true);
}

// Level completion results screen
void drawLevelEnd() {
    if (backgroundTexture.getSize().x > 0) {
        window->draw(backgroundSprite);
    } else {
        RectangleShape solidBg(Vector2f(800, 600));
        solidBg.setFillColor(Color(20, 25, 45, 240));
        window->draw(solidBg);
    }

    drawText("LEVEL " + to_string(currentLevel) + " COMPLETED", 400, 200, 42, Color::Yellow, true);
    drawText(levelMessage, 400, 280, 32, levelScore >= 0 ? Color::Green : Color::Red, true);
    drawText("Score gained: " + to_string(levelScore) + " points", 400, 330, 24, Color::Cyan, true);
    drawText("Click anywhere to continue...", 400, 400, 20, Color::Yellow, true);
}

// Set up random number generation for game variety
void initialize_rng() {
    srand(static_cast<unsigned>(time(0)));
}

// Generate random numbers within a range
int random_num(int min, int max) {
    if (min > max) swap(min, max);
    return min + rand() % (max - min + 1);
}

// Pick a random math operator
char random_operator() {
    const char ops[] = {'+', '-', '*', '/'};
    return ops[rand() % 4];
}

// Math expression evaluation helpers
int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

// Perform actual math operations
double applyOp(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': 
            return (abs(b) < 1e-9) ? 1e99 : a / b;
    }
    return 0;
}

// Calculate answer for math expressions
double evaluate_expression(const string& expr) {
    stack<double> values;
    stack<char> ops;

    for (int i = 0; i < expr.size(); ++i) {
        if (expr[i] == ' ') continue;

        if (isdigit(expr[i])) {
            double val = 0;
            while (i < expr.size() && isdigit(expr[i])) {
                val = val * 10 + (expr[i] - '0');
                i++;
            }
            i--;
            values.push(val);
        } else {
            char op = expr[i];
            while (!ops.empty() && precedence(ops.top()) >= precedence(op)) {
                double val2 = values.top(); values.pop();
                double val1 = values.top(); values.pop();
                char top_op = ops.top(); ops.pop();
                values.push(applyOp(val1, val2, top_op));
            }
            ops.push(op);
        }
    }

    while (!ops.empty()) {
        double val2 = values.top(); values.pop();
        double val1 = values.top(); values.pop();
        char op = ops.top(); ops.pop();
        values.push(applyOp(val1, val2, op));
    }

    return values.top();
}

// Handle fractions in user answers
double evaluate_fractional_input(const string& input) {
    size_t slash = input.find('/');
    if (slash != string::npos) {
        try {
            double num = stod(input.substr(0, slash));
            double den = stod(input.substr(slash + 1));
            if (abs(den) < 1e-9) return numeric_limits<double>::infinity();
            return num / den;
        } catch (...) {
            return numeric_limits<double>::quiet_NaN();
        }
    }
    return stod(input);
}

// Check if user's answer matches the correct one
bool is_answer_correct(const string& user_input, double correct_answer) {
    double user_ans;
    try {
        user_ans = evaluate_fractional_input(user_input);
    } catch (...) {
        return false;
    }

    const double TOLERANCE_EXACT = 0.01;
    const double TOLERANCE_DECIMAL = 0.1;

    if (fabs(user_ans - correct_answer) < TOLERANCE_DECIMAL) return true;

    if (fabs(correct_answer - floor(correct_answer)) < TOLERANCE_EXACT &&
        fabs(user_ans - floor(correct_answer)) < TOLERANCE_EXACT) return true;

    return false;
}

// Create random math questions based on level difficulty
Question generate_random_question(int level) {
    Question q;
    stringstream ss;
    int num_ops;
    int min_val, max_val;
    
    if (level == 1) {
        num_ops = 1;
        min_val = 1;
        max_val = 10;
    } else {
        num_ops = random_num(2, 3); 
        min_val = 5 * level;
        max_val = 15 * level;
    }

    char last_op = ' ';
    bool division_used = false;

    ss << random_num(min_val, max_val);
    
    for (int i = 0; i < num_ops; ++i) {
        char op;
        int next;
        
        do {
            op = random_operator();
            if (level == 3 && op == '/' && division_used) {
                op = random_operator();
            }

            if (last_op == '/' && op == '*') continue; 
            if (last_op == '*' && op == '/') continue;

            next = random_num(min_val, max_val);

            if (op == '/') {
                while (next == 0) next = random_num(min_val, max_val);
                int current_val = evaluate_expression(ss.str());
                if (current_val % next != 0 && (current_val * 4) % next != 0) {
                    vector<int> clean_divisors = {2, 4, 5, 8, 10};
                    next = current_val;
                    while (next == current_val || (current_val % next != 0 && (current_val * 2) % next != 0 && (current_val * 4) % next != 0)) {
                        if (current_val % 10 == 0 && i < num_ops - 1) {
                            next = clean_divisors[rand() % clean_divisors.size()];
                        } else {
                            next = random_num(1, 10);
                        }
                    }
                }
            }
        } while (last_op == op);

        if (level == 3 && op == '/') division_used = true;
        
        ss << " " << op << " " << next;
        last_op = op;
    }

    q.expression = ss.str();
    q.answer = evaluate_expression(q.expression);
    return q;
}

// Update current player's data in the users list
void update_user_record() {
    for (size_t i = 0; i < all_users.size(); ++i) {
        if (all_users[i].username == current_user.username) {
            all_users[i] = current_user;
            cout << "Updated player record: " << current_user.username 
                 << " Score: " << current_user.total_score 
                 << " Win Rate: " << current_user.get_win_rate() << "%" << endl;
            return;
        }
    }
    all_users.push_back(current_user);
    cout << "Added new player: " << current_user.username << endl;
}

// Save all player data to file
void save_users() {
    update_user_record();

    ofstream file("users.txt");
    if (!file.is_open()) {
        cerr << "Could not open users.txt for saving!\n";
        return;
    }

    cout << "Saving " << all_users.size() << " players to file...\n";
    
    for (auto& u : all_users) {
        file << u.username << " " << u.password << " " << u.total_score << " "
             << u.games_played << " " << u.games_won << " " << u.games_lost << " "
             << u.failed_questions.size() << "\n";
        
        cout << "Saved: " << u.username << " | Score: " << u.total_score 
             << " | Games: " << u.games_played << " | Win Rate: " << u.get_win_rate() << "%" << endl;
        
        for (auto& q : u.failed_questions)
            file << q.expression << "~" << q.answer << "\n";
    }
    
    cout << "All player data saved successfully!\n";
}

// Load player data from file
void load_users() {
    all_users.clear();
    ifstream file("users.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        User u;
        int fail_count = 0;
        
        if (!(ss >> u.username >> u.password >> u.total_score >> u.games_played
                     >> u.games_won >> u.games_lost >> fail_count)) {
            continue;
        }

        for (int i = 0; i < fail_count; ++i) {
            if (getline(file, line)) {
                size_t tilde = line.find('~');
                if (tilde != string::npos) {
                    Question q;
                    q.expression = line.substr(0, tilde);
                    try {
                        q.answer = stod(line.substr(tilde + 1));
                        u.failed_questions.push_back(q);
                    } catch (...) {
                        continue;
                    }
                }
            }
        }
        all_users.push_back(u);
    }
}

// Handle login/signup screen interactions
void handleAuthMenuInput(Event& event) {
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
        loginError = false;
        
        if (isMouseOver(200, 230, 400, 40)) {
            usernameActive = true;
            passwordActive = false;
            inputActive = true;
        } else if (isMouseOver(200, 330, 400, 40)) {
            usernameActive = false;
            passwordActive = true;
            inputActive = true;
        } else if (isMouseOver(250, 400, 150, 50)) {
            // Try to log player in
            bool loginSuccess = false;
            for (auto& u : all_users) {
                if (u.username == usernameInput && u.password == passwordInput) {
                    current_user = u;
                    currentState = MAIN_MENU;
                    usernameInput = "";
                    passwordInput = "";
                    loginSuccess = true;
                    break;
                }
            }
            if (!loginSuccess) {
                loginError = true;
            }
        } else if (isMouseOver(450, 400, 150, 50)) {
            // Create new player account
            bool userExists = false;
            for (auto& u : all_users) {
                if (u.username == usernameInput) {
                    userExists = true;
                    break;
                }
            }
            if (!userExists && !usernameInput.empty() && !passwordInput.empty()) {
                User new_user{usernameInput, passwordInput};
                all_users.push_back(new_user);
                current_user = new_user;
                currentState = MAIN_MENU;
                usernameInput = "";
                passwordInput = "";
                save_users();
            }
        } else if (isMouseOver(350, 470, 100, 40)) {
            window->close();
        }
    }
    
    if (event.type == Event::TextEntered && inputActive) {
        if (event.text.unicode == '\b') {
            if (usernameActive && !usernameInput.empty()) usernameInput.pop_back();
            if (passwordActive && !passwordInput.empty()) passwordInput.pop_back();
        } else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\t') {
            if (usernameActive) usernameInput += static_cast<char>(event.text.unicode);
            if (passwordActive) passwordInput += static_cast<char>(event.text.unicode);
        }
    }
}

// Handle main menu button clicks
void handleMainMenuInput(Event& event) {
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
        if (isMouseOver(300, 200, 200, 50)) {
            currentState = LEVEL_START;
            currentLevel = 0;
            gameStarted = true;
            timeUp = false;
            userInputText = "";
            levelScore = 0;
        } else if (isMouseOver(300, 270, 200, 50)) {
            currentState = RETRY_FAILED;
            userInputText = "";
        } else if (isMouseOver(300, 340, 200, 50)) {
            currentState = DASHBOARD;
        } else if (isMouseOver(300, 410, 200, 50)) {
            currentState = LEADERBOARD;
        } else if (isMouseOver(300, 480, 200, 50)) {
            currentState = AUTH_MENU;
            save_users();
        }
    }
}

// Handle gameplay input and answer submission
void handleGameLevelInput(Event& event) {
    if (event.type == Event::TextEntered) {
        if (event.text.unicode == '\b') {
            if (!userInputText.empty()) userInputText.pop_back();
        } else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\t') {
            userInputText += static_cast<char>(event.text.unicode);
        }
    }
    
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
        if (isMouseOver(350, 430, 100, 40)) {
            int scoreChange = 0;
            
            if (userInputText == "s" || userInputText == "S") {
                current_user.failed_questions.push_back(currentQuestion);
                scoreChange = -5;
            } else if (is_answer_correct(userInputText, currentQuestion.answer)) {
                scoreChange = 10;
            } else {
                current_user.failed_questions.push_back(currentQuestion);
                scoreChange = -5;
            }
            
            current_user.total_score += scoreChange;
            levelScore += scoreChange;
            update_user_record();
            save_users();
            
            userInputText = "";
            currentLevel++;
            
            if (currentLevel < 3) {
                currentState = LEVEL_START;
            } else {
                // Show final level results
                if (levelScore > 0) {
                    levelMessage = "LEVEL PASSED! ";
                    current_user.games_won++;
                } else {
                    levelMessage = "LEVEL FAILED! ";
                    current_user.games_lost++;
                }
                current_user.games_played++;
                currentState = LEVEL_END;
            }
        }
    }
}

// Handle dashboard navigation
void handleDashboardInput(Event& event) {
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
        if (isMouseOver(300, 350, 200, 50)) {
            currentState = MAIN_MENU;
        }
    }
}

// Handle leaderboard navigation
void handleLeaderboardInput(Event& event) {
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
        if (isMouseOver(300, 350, 200, 50)) {
            currentState = MAIN_MENU;
        }
    }
}

// Handle failed questions retry screen
void handleRetryFailedInput(Event& event) {
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
        if (isMouseOver(300, 460, 200, 50)) {
            currentState = MAIN_MENU;
            userInputText = "";
        } else if (!current_user.failed_questions.empty()) {
            if (isMouseOver(300, 320, 200, 50)) {
                auto it = current_user.failed_questions.begin();
                if (is_answer_correct(userInputText, it->answer)) {
                    current_user.total_score += 10;
                    current_user.failed_questions.erase(it);
                    update_user_record();
                    save_users(); 
                }
                userInputText = "";
            } else if (isMouseOver(300, 390, 200, 50)) {
                if (!current_user.failed_questions.empty()) {
                    current_user.failed_questions.pop_front();
                }
                userInputText = "";
            }
        }
    }
    
    if (event.type == Event::TextEntered && !current_user.failed_questions.empty()) {
        if (event.text.unicode == '\b') {
            if (!userInputText.empty()) userInputText.pop_back();
        } else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\t') {
            userInputText += static_cast<char>(event.text.unicode);
        }
    }
}

// Start a new level when player clicks
void handleLevelStartInput(Event& event) {
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
        currentQuestion = generate_random_question(currentLevel + 1);
        gameClock.restart();
        currentState = PLAYING_LEVEL;
    }
}

// Return to menu after level completion
void handleLevelEndInput(Event& event) {
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
        save_users();
        currentState = MAIN_MENU;
    }
}

// Main game loop - keeps everything running
int main() {
    cout << "Starting Math Clash Game..." << endl;
    
    window = new RenderWindow(VideoMode(800, 600), "Math Clash Game");
    window->setFramerateLimit(60);
    
    graphics_mode = loadResources();
    if (!graphics_mode) {
        cout << "Running in console mode only." << endl;
    }
    
    initialize_rng();
    load_users();
    
    while (window->isOpen()) {
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed)
                window->close();
            
            switch (currentState) {
                case AUTH_MENU:
                    handleAuthMenuInput(event);
                    break;
                case MAIN_MENU:
                    handleMainMenuInput(event);
                    break;
                case PLAYING_LEVEL:
                    handleGameLevelInput(event);
                    break;
                case DASHBOARD:
                    handleDashboardInput(event);
                    break;
                case LEADERBOARD:
                    handleLeaderboardInput(event);
                    break;
                case RETRY_FAILED:
                    handleRetryFailedInput(event);
                    break;
                case LEVEL_START:
                    handleLevelStartInput(event);
                    break;
                case LEVEL_END:
                    handleLevelEndInput(event);
                    break;
            }
        }
        
        window->clear(Color(20, 20, 40));
        
        switch (currentState) {
            case AUTH_MENU:
                drawAuthMenu();
                break;
            case MAIN_MENU:
                drawMainMenu();
                break;
            case PLAYING_LEVEL:
                drawGameLevel();
                break;
            case DASHBOARD:
                drawDashboard();
                break;
            case LEADERBOARD:
                drawLeaderboard();
                break;
            case RETRY_FAILED:
                drawRetryFailed();
                break;
            case LEVEL_START:
                drawLevelStart();
                break;
            case LEVEL_END:
                drawLevelEnd();
                break;
        }
        
        window->display();
        
        if (currentState == PLAYING_LEVEL) {
            vector<int> levelTimes = {20, 15, 10};
            int timeLimit = levelTimes[currentLevel];
            float elapsed = gameClock.getElapsedTime().asSeconds();
            
            if (elapsed >= timeLimit && !timeUp) {
                timeUp = true;
                current_user.failed_questions.push_back(currentQuestion);
                current_user.total_score -= 5;
                levelScore -= 5;
                
                this_thread::sleep_for(chrono::seconds(2));
                userInputText = "";
                currentLevel++;
                
                if (currentLevel < 3) {
                    currentState = LEVEL_START;
                } else {
                    if (levelScore > 0) {
                        levelMessage = "LEVEL PASSED! ";
                        current_user.games_won++;
                    } else {
                        levelMessage = "LEVEL FAILED! ";
                        current_user.games_lost++;
                    }
                    current_user.games_played++;
                    currentState = LEVEL_END;
                }
            }
        }
    }
    
    save_users();
    delete window;
    
    return 0;
}
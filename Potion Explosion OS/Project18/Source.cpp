#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <limits>

using namespace std;

enum Ingredient { APPLE, LEMON, CHERRY, WATER, SUGAR, STAR_ANISE, TOADSTOOL };

const int EXPLOSION_THRESHOLD = 3;
const int BOARD_SIZE = 5;

struct Board {
    vector<vector<Ingredient>> grid;
    vector<int> playerPositions;
};

struct Player {
    int score;
    vector<Ingredient> ingredients;
    int position;
};

bool processIngredient(Board& board, Player& player, Ingredient ingredient);
bool checkExplode(Board& board, vector<Player>& players);
void handleExplosion(Board& board, pair<int, int> cell, vector<Player>& players);
void updateScoresAfterExplosion(Board& board, vector<Player>& players, int row, int col);

void initializeGame(Board& board, vector<Player>& players) {
    board.grid.resize(BOARD_SIZE, vector<Ingredient>(BOARD_SIZE, WATER));
    board.playerPositions = { 0, BOARD_SIZE - 1 };

    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board.grid[i][j] = static_cast<Ingredient>(rand() % (TOADSTOOL + 1));
        }
    }

    for (auto& player : players) {
        player.ingredients = { APPLE, LEMON, CHERRY };
        player.score = 0;
    }
}

void displayBoard(const Board& board, const vector<Player>& players) {
    cout << endl;
    cout << "**********BOARD**********" << endl;
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            cout << board.grid[i][j] << " ";
        }
        cout << endl;
    }
    cout << "Player 1's Position: " << board.playerPositions[0] << endl;
    cout << "Player 2's Position: " << board.playerPositions[1] << endl;
    cout << "Player 1's Score: " << players[0].score << endl;
    cout << "Player 2's Score: " << players[1].score << endl;
}

void playerTurn(Board& board, Player& player, vector<Player>& players) {
    cout << "Player " << player.position + 1 << ", It is your turn!" << endl;

    int move;
    do {
        cout << "Enter A Move (1-left, 2-right): ";
        cin >> move;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            move = 0;
        }
    } while (move != 1 && move != 2);

    if (move == 1 || move == 2) {
        int newPosition = board.playerPositions[player.position] + (move == 1 ? -1 : 1);

        if (newPosition >= 0 && newPosition < BOARD_SIZE) {
            board.playerPositions[player.position] = newPosition;

            cout << "Choose The Ingredient For Use:" << endl;
            for (int i = 0; i < player.ingredients.size(); ++i) {
                cout << i + 1 << ": " << player.ingredients[i] << " ";
            }
            cout << endl;

            int ingredientChoice;
            do {
                cout << "Enter The Ingredient Choice: ";
                cin >> ingredientChoice;
                if (cin.fail() || ingredientChoice < 1 || ingredientChoice > player.ingredients.size()) {
                    cout << "Invalid Ingredient Choice! Please Try Again." << endl;
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            } while (cin.fail() || ingredientChoice < 1 || ingredientChoice > player.ingredients.size());

            processIngredient(board, player, player.ingredients[ingredientChoice - 1]);

            if (checkExplode(board, players)) {
                handleExplosion(board, { 0, 0 }, players);
            }
            player.score++;
        }
        else {
            cout << "Invalid move! You Cannot Move Out Of The Board Boundary." << endl;
        }
    }
    else {
        cout << "Invalid Move!" << endl;
    }
}

bool processIngredient(Board& board, Player& player, Ingredient ingredient) {
    if (find(player.ingredients.begin(), player.ingredients.end(), ingredient) == player.ingredients.end()) {
        cout << "Invalid Ingredient!" << endl;
        return false;
    }

    if (board.grid[board.playerPositions[player.position]][board.playerPositions[player.position]] != ingredient) {
        cout << "Ingredient Does Not Match The Cell!" << endl;
        return false;
    }

    player.ingredients.erase(remove(player.ingredients.begin(), player.ingredients.end(), ingredient), player.ingredients.end());

    board.grid[board.playerPositions[player.position]][board.playerPositions[player.position]] = STAR_ANISE;
    return true;
}

bool checkExplode(Board& board, vector<Player>& players) {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board.grid[i][j] == STAR_ANISE) {
                board.grid[i][j] = WATER;
                for (int di = -1; di <= 1; ++di) {
                    for (int dj = -1; dj <= 1; ++dj) {
                        if (di != 0 || dj != 0) {
                            int ni = i + di, nj = j + dj;
                            if (ni >= 0 && ni < BOARD_SIZE && nj >= 0 && nj < BOARD_SIZE) {
                                if (board.grid[ni][nj] == board.grid[i][j]) {
                                    handleExplosion(board, { ni, nj }, players);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board.grid[i][j] == STAR_ANISE) {
                return true;
            }
        }
    }

    return false;
}

void handleExplosion(Board& board, pair<int, int> cell, vector<Player>& players) {
    // Determine the index of the player who caused the explosion
    int explodingPlayerIndex = (players[0].position == players[1].position) ? 1 : 0;

    // Remove the extra turn for the player who caused the explosion
    if (players[explodingPlayerIndex].position == 0) {
        players[explodingPlayerIndex].position = 1;
    }
    else {
        players[explodingPlayerIndex].position = 0;
    }

    // Switch the loop index to the player who caused the explosion
    int currentPlayerIndex = (players[0].position == players[1].position) ? 1 : 0;

    // Continue the loop with the player who caused the explosion
    displayBoard(board, players);
    playerTurn(board, players[currentPlayerIndex], players);
}

void updateScoresAfterExplosion(Board& board, vector<Player>& players, int row, int col) {
    Ingredient explodedIngredient = board.grid[row][col];

    // Update scores based on the explosion
    for (auto& player : players) {
        int count = 0;
        for (int i = 0; i < BOARD_SIZE; ++i) {
            if (board.grid[i][col] == explodedIngredient) {
                count++;
            }
        }
        if (count >= EXPLOSION_THRESHOLD) {
            player.score += count;
        }
    }
}

bool isWinner(const Player& player) {
    const int WINNING_SCORE = 10;

    return player.score >= WINNING_SCORE;
}

int main() {
    Board board;
    vector<Player> players(2);

    initializeGame(board, players);

    while (!isWinner(players[0]) && !isWinner(players[1])) {
        for (auto& player : players) {
            displayBoard(board, players);
            playerTurn(board, player, players);
            if (isWinner(player)) {
                cout << "Player " << player.position + 1 << " Wins!" << endl;
                return 0;
            }
        }
    }

    return 0;
}

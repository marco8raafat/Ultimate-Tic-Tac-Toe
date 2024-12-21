#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QApplication>
#include <QMessageBox>
#include <cstring>
#include <QRandomGenerator>

// Constants for player symbols and board size
const char PLAYER_X = 'X';
const char PLAYER_O = 'O';
const int GRID_SIZE = 9;
const int SUBGRID_SIZE = 3;
char smallBoardStatus[SUBGRID_SIZE][SUBGRID_SIZE] = {{0}};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize game state
    resetGameState();
    initializeCellConnections();
    initializeFinalBoardConnections();

    // Connect UI buttons to slots
    connect(ui->restartButton, &QPushButton::clicked, this, &MainWindow::on_restartButton_clicked);
    connect(ui->playHumanButton, &QPushButton::clicked, this, &MainWindow::on_playHumanButton_clicked);
    connect(ui->playComputerButton, &QPushButton::clicked, this, &MainWindow::on_playComputerButton_clicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::initializeCellConnections() {
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            QString buttonName = QString("cellButton_%1_%2").arg(row).arg(col);
            QPushButton *button = findChild<QPushButton *>(buttonName);
            if (button) {
                connect(button, &QPushButton::clicked, this, [=]() {
                    handleCellClick(row, col);
                });
            }
        }
    }
}

void MainWindow::resetGameState() {
    qDebug() << "Resetting game state...";
    num_Sboards = 0;

    // Reset final board buttons
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            QString buttonName = QString("finalboardbutton_%1_%2").arg(row).arg(col);
            QPushButton *button = findChild<QPushButton *>(buttonName);
            if (button) {
                button->setText("");        // Clear the text
                button->setEnabled(true);   // Enable the button
            }
        }
    }

    // Re-enable the buttons on the main board for input
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            QPushButton *button = getButton(i, j);
            if (button) {
                button->setEnabled(true);
            }
        }
    }

    // Reset button texts on the main board
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            updateButtonText(row, col, '\0');
        }
    }

    // Clear main board and small board states
    memset(mainBoard, 0, sizeof(mainBoard));
    memset(smallBoardStatus, 0, sizeof(smallBoardStatus));

    // Reset game variables
    currentPlayer = PLAYER_X;
    gameInProgress = true;  // Allow input after restart

    qDebug() << "Game state reset completed.";
}


void MainWindow::handleCellClick(int row, int col) {
    if (!gameInProgress) return;

    int smallBoardRow = row / SUBGRID_SIZE;
    int smallBoardCol = col / SUBGRID_SIZE;

    // Prevent playing in a won or drawn small board
    if (smallBoardStatus[smallBoardRow][smallBoardCol] != 0) {
        qDebug() << "This small board is already won or drawn!";
        return;
    }

    QString buttonName = QString("cellButton_%1_%2").arg(row).arg(col);
    QPushButton *button = findChild<QPushButton *>(buttonName);

    if (button && button->text().isEmpty()) {
        // Update button with current player's symbol
        button->setText(currentPlayer == PLAYER_X ? "X" : "O");

        // Check if the move wins the small board
        if (checkSmallBoardWin(row, col)) {
            smallBoardStatus[smallBoardRow][smallBoardCol] = currentPlayer;
            disableSmallBoard(smallBoardRow, smallBoardCol);
            updateMainBoard();
            updateFinalBoard(smallBoardRow, smallBoardCol, currentPlayer);  // Update final board
        } else if (isSmallBoardDraw(smallBoardRow, smallBoardCol)) {
            qDebug() << "The smallboard is a draw!";
            num_Sboards++;
            smallBoardStatus[smallBoardRow][smallBoardCol] = 'D'; // Mark as drawn
            disableSmallBoard(smallBoardRow, smallBoardCol);
        }

        // Check if the game is over
        if (game_is_over()) {
            gameInProgress = false;
            return;
        }

        // Switch to the next player
        currentPlayer = (currentPlayer == PLAYER_X) ? PLAYER_O : PLAYER_X;

        // Trigger computer's move only if playing against the computer
        if (isHumanVsComputer && currentPlayer == PLAYER_O) {
            computerMove();
        }
    }
}



void MainWindow::updateButtonText(int row, int col, char symbol) {
    QString buttonName = QString("cellButton_%1_%2").arg(row).arg(col);
    QPushButton *button = findChild<QPushButton *>(buttonName);
    if (button) {
        button->setText(symbol ? QString(symbol) : "");
    }
}

bool MainWindow::checkSmallBoardWin(int row, int col) {
    int startRow = (row / SUBGRID_SIZE) * SUBGRID_SIZE;
    int startCol = (col / SUBGRID_SIZE) * SUBGRID_SIZE;

    for (int i = 0; i < SUBGRID_SIZE; ++i) {
        if (checkLine(
                getButton(startRow + i, startCol),
                getButton(startRow + i, startCol + 1),
                getButton(startRow + i, startCol + 2)) ||
            checkLine(
                getButton(startRow, startCol + i),
                getButton(startRow + 1, startCol + i),
                getButton(startRow + 2, startCol + i))) {
            return true;
        }
    }

    return checkLine(getButton(startRow, startCol), getButton(startRow + 1, startCol + 1), getButton(startRow + 2, startCol + 2)) ||
           checkLine(getButton(startRow, startCol + 2), getButton(startRow + 1, startCol + 1), getButton(startRow + 2, startCol));
}


bool MainWindow::checkMainBoardWin() {
    for (int i = 0; i < SUBGRID_SIZE; ++i) {
        if ((mainBoard[i][0] == mainBoard[i][1] && mainBoard[i][0] == mainBoard[i][2] && mainBoard[i][0] != 0) ||
            (mainBoard[0][i] == mainBoard[1][i] && mainBoard[0][i] == mainBoard[2][i] && mainBoard[0][i] != 0)) {
            return true;
        }
    }
    return (mainBoard[0][0] == mainBoard[1][1] && mainBoard[0][0] == mainBoard[2][2] && mainBoard[0][0] != 0) ||
           (mainBoard[0][2] == mainBoard[1][1] && mainBoard[0][2] == mainBoard[2][0] && mainBoard[0][2] != 0);
}

QPushButton *MainWindow::getButton(int row, int col) {
    QString buttonName = QString("cellButton_%1_%2").arg(row).arg(col);
    return findChild<QPushButton *>(buttonName);
}

bool MainWindow::checkLine(QPushButton *b1, QPushButton *b2, QPushButton *b3) {
    return b1 && b2 && b3 && !b1->text().isEmpty() &&
           b1->text() == b2->text() && b1->text() == b3->text();
}

void MainWindow::updateMainBoard() {
    for (int i = 0; i < SUBGRID_SIZE; ++i) {
        for (int j = 0; j < SUBGRID_SIZE; ++j) {
            if (mainBoard[i][j] == 0 && checkSmallBoardWin(i * SUBGRID_SIZE, j * SUBGRID_SIZE)) {
                mainBoard[i][j] = currentPlayer;
                qDebug() << "Main board updated at (" << i << "," << j << ")";

                // Update the final 3x3 board
                updateFinalBoard(i, j, currentPlayer);
                num_Sboards++;
            }
        }
    }
}


bool MainWindow::game_is_over() {
    if (checkMainBoardWin()) {
        showMessage(QString("%1 wins the game!").arg(currentPlayer == PLAYER_X ? "X" : "O"));
        gameInProgress = false; // Stop further input
        return true;
    }
    if (is_main_draw()) {
        qDebug() << "The game is a draw!";
        showMessage("The game is a draw!");
        gameInProgress = false; // Stop further input
        return true;
    }
    return false;
}


bool MainWindow::isSmallBoardDraw(int smallBoardRow, int smallBoardCol) {
    int startRow = smallBoardRow * SUBGRID_SIZE;
    int startCol = smallBoardCol * SUBGRID_SIZE;

    for (int i = 0; i < SUBGRID_SIZE; ++i) {
        for (int j = 0; j < SUBGRID_SIZE; ++j) {
            QPushButton *button = getButton(startRow + i, startCol + j);
            if (button && button->text().isEmpty()) {
                return false; // If any cell is empty, not a draw
            }
        }
    }

    // If no winner and no empty cells, it's a draw
    return !checkSmallBoardWin(startRow, startCol);
}


bool MainWindow::is_main_draw() {
    // Check if all small boards are filled
    if (num_Sboards==9){
        return true;
    }
    return false;
}

void MainWindow::showMessage(const QString &message) {
    QMessageBox::information(this, "Game Status", message);
}

void MainWindow::on_restartButton_clicked() {
    resetGameState();
}

void MainWindow::on_playHumanButton_clicked() {
    resetGameState();
    isHumanVsComputer = false; // Set mode to human vs. human
    gameInProgress = true;
    currentPlayer = PLAYER_X;
}

void MainWindow::on_playComputerButton_clicked() {
    resetGameState();
    isHumanVsComputer = true; // Set mode to human vs. computer
    gameInProgress = true;
    currentPlayer = PLAYER_X; // Human starts first
}


void MainWindow::disableSmallBoard(int smallBoardRow, int smallBoardCol) {
    int startRow = smallBoardRow * SUBGRID_SIZE;
    int startCol = smallBoardCol * SUBGRID_SIZE;

    for (int i = 0; i < SUBGRID_SIZE; ++i) {
        for (int j = 0; j < SUBGRID_SIZE; ++j) {
            QPushButton *button = getButton(startRow + i, startCol + j);
            if (button) {
                button->setEnabled(false);
            }
        }
    }
}

void MainWindow::computerMove() {
    if (!gameInProgress) return;

    // Check if the current player (computer) is allowed to play in the small board
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            int smallBoardRow = row / SUBGRID_SIZE;
            int smallBoardCol = col / SUBGRID_SIZE;

            // Skip if the small board is already won or drawn
            if (smallBoardStatus[smallBoardRow][smallBoardCol] != 0) {
                continue; // Skip this cell as the small board is already decided
            }

            QPushButton *button = getButton(row, col);
            if (button && button->text().isEmpty()) {
                button->setText("O"); // Try to place the computer's move
                if (checkSmallBoardWin(row, col)) {
                    smallBoardStatus[smallBoardRow][smallBoardCol] = PLAYER_O;
                    disableSmallBoard(smallBoardRow, smallBoardCol);
                    updateMainBoard(); // Update the main board after computer's win
                    game_is_over(); // Check if the game is over
                    currentPlayer = PLAYER_X; // Switch back to player X after computer's move
                    return;
                }
                button->setText(""); // Undo the move if it didn't win
            }
        }
    }

    // Check if the opponent can win and block
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            int smallBoardRow = row / SUBGRID_SIZE;
            int smallBoardCol = col / SUBGRID_SIZE;

            // Skip if the small board is already won or drawn
            if (smallBoardStatus[smallBoardRow][smallBoardCol] != 0) {
                continue;
            }

            QPushButton *button = getButton(row, col);
            if (button && button->text().isEmpty()) {
                button->setText("X"); // Simulate opponent's move to check for a block
                if (checkSmallBoardWin(row, col)) {
                    button->setText(""); // Undo
                    handleCellClick(row, col); // Block the move
                    currentPlayer = PLAYER_X; // Switch back to player X after blocking
                    return;
                }
                button->setText(""); // Undo
            }
        }
    }

    // Otherwise, make a random move in a valid small board
    QVector<QPair<int, int>> availableMoves;
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            int smallBoardRow = row / SUBGRID_SIZE;
            int smallBoardCol = col / SUBGRID_SIZE;

            // Skip if the small board is already won or drawn
            if (smallBoardStatus[smallBoardRow][smallBoardCol] != 0) {
                continue;
            }

            QPushButton *button = getButton(row, col);
            if (button && button->text().isEmpty()) {
                availableMoves.append(qMakePair(row, col));
            }
        }
    }

    if (availableMoves.isEmpty()) return; // No available moves

    int randomIndex = QRandomGenerator::global()->bounded(availableMoves.size());
    QPair<int, int> move = availableMoves[randomIndex];
    handleCellClick(move.first, move.second); // Make the move

    // After the move, update the main board if necessary
    updateMainBoard();

    // Check if the game is over, either win or draw
    if (game_is_over()) {
        currentPlayer = PLAYER_X; // Switch back to player X after the computer's move
    }
}

void MainWindow::updateFinalBoard(int smallBoardRow, int smallBoardCol, char winner) {
    QString buttonName = QString("finalboardbutton_%1_%2").arg(smallBoardRow).arg(smallBoardCol);
    QPushButton *button = findChild<QPushButton *>(buttonName);
    if (button) {
        button->setText(winner == PLAYER_X ? "X" : "O");
        button->setEnabled(false);  // Disable the button after updating
    }
}


void MainWindow::initializeFinalBoardConnections() {
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            QString buttonName = QString("finalboardbutton_%1_%2").arg(row).arg(col);
            QPushButton *button = findChild<QPushButton *>(buttonName);
            if (button) {
                connect(button, &QPushButton::clicked, this, [=]() {
                    // Handle click event if needed, or do nothing since it's auto-updated
                });
            }
        }
    }
}


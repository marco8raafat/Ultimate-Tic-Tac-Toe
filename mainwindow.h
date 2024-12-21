#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_restartButton_clicked();
    void on_playHumanButton_clicked();
    void on_playComputerButton_clicked();
private:
    Ui::MainWindow *ui;

    // Game state variables
    char mainBoard[3][3];  // Main 3x3 grid to track winner of small boards
    char smallBoards[3][3][3][3];  // 3x3 array of smaller 3x3 Tic Tac Toe boards
    char currentPlayer;  // 'X' or 'O'
    int num_Sboards= 0;


    // Additional flags or helpers
    bool gameInProgress;  // Flag to track if the game is in progress
    bool smallwinflag;

    // Helper methods for game logic
    void resetGameState();  // Reset all game data
    bool checkSmallBoardWin(int row, int col);  // Check if a small board is won
    bool checkMainBoardWin();  // Check if the main board has a winner
    void updateMainBoard();  // Update the main board after each small board win
    void updateButtonText(int row, int col, char symbol);  // Update the button label (X or O)
    void handleCellClick(int row, int col);
    void initializeCellConnections();
    bool checkLine(QPushButton* b1, QPushButton* b2, QPushButton* b3);
    void showMessage(const QString& message);
    bool is_draw();
    bool isSmallBoardDraw(int smallBoardRow, int smallBoardCol);
    bool is_main_draw();
    bool game_is_over();
    void disableSmallBoard(int smallBoardRow, int smallBoardCol);
    void computerMove();
    bool isHumanVsComputer = false; // Default to false (human vs. human)
    void updateFinalBoard(int smallBoardRow, int smallBoardCol, char winner);
    void initializeFinalBoardConnections();


    QPushButton* getButton(int row, int col) ;
    // Store buttons in a 2D array for easier access
    QPushButton* cellButtons[9][9];  // Buttons for the grid
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QMap>
#include <QList>
#include "stage.h"
#include "entities.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum GameState {
    STATE_MENU,
    STATE_STAGE1,
    STATE_STAGE2,
    STATE_GAME_OVER,
    STATE_CLEAR
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void updateGame();

private:
    Ui::MainWindow *ui;
    QTimer *gameTimer;
    GameState currentState;

    Stage* currentStage;
    Kirby* kirby;
    QList<Entity*> projectiles;
    
    double cameraX;
    QMap<int, bool> keysPressed;

    // Game Over Menu Selection: 0 = Continue, 1 = Quit
    int gameOverSelection;

    // HUD icons
    QPixmap hpFullPixmap;
    QPixmap hpEmptyPixmap;
    QPixmap lifeIconPixmap;

    void loadStage(int stageNum);
    void handleInhaleAndCollisions();
    void drawHUD(QPainter& painter);
    void resetGame();
};

#endif // MAINWINDOW_H

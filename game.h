#ifndef GAME_H
#define GAME_H

#include <QGraphicsView>
#include <QTimer>
#include <QSet>
#include <QKeyEvent>
#include "constants.h"

class GameScene;

class Game : public QGraphicsView {
    Q_OBJECT
public:
    explicit Game(QWidget *parent = nullptr);
    ~Game();

    QSet<int> pressedKeys;
    GameState currentState;
    int currentStage;

    void showMenu();
    void startGame();
    void switchStage(int stage);
    void showGameOver();
    void showClear();
    void restartFromMenu();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();

private:
    GameScene *scene;
    QTimer *gameTimer;
};

#endif // GAME_H

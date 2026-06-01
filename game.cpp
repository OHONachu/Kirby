#include "game.h"
#include "gamescene.h"

Game::Game(QWidget *parent) : QGraphicsView(parent) {
    // 建立遊戲場景
    scene = new GameScene(this);
    setScene(scene);

    // 設定視窗
    setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setWindowTitle("Kirby's Adventure");

    // Game Loop Timer
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &Game::gameLoop);

    currentState = STATE_MENU;
    currentStage = 1;

    // 進入主選單
    showMenu();
}

Game::~Game() {}

// ============ 鍵盤事件 ============
void Game::keyPressEvent(QKeyEvent *event) {
    if (!event->isAutoRepeat()) {
        pressedKeys.insert(event->key());
    }

    // 主選單按任意鍵開始
    if (currentState == STATE_MENU) {
        startGame();
        return;
    }

    // Game Over 畫面按鍵處理
    if (currentState == STATE_GAME_OVER) {
        if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
            scene->toggleGameOverSelection();
        }
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_X) {
            scene->confirmGameOverSelection(this);
        }
        return;
    }

    // Clear 畫面按任意鍵回選單
    if (currentState == STATE_CLEAR) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            scene->advanceClearScreen(this);
        }
        return; // 攔截 Enter 與其他所有按鍵，使其在通關狀態下不影響遊戲
    }

    QGraphicsView::keyPressEvent(event);
}

void Game::keyReleaseEvent(QKeyEvent *event) {
    if (!event->isAutoRepeat()) {
        pressedKeys.remove(event->key());
    }
    QGraphicsView::keyReleaseEvent(event);
}

// ============ Game Loop ============
void Game::gameLoop() {
    if (currentState == STATE_PLAYING) {
        scene->updateGame(pressedKeys);
    }
}

// ============ 遊戲流程 ============
void Game::showMenu() {
    currentState = STATE_MENU;
    gameTimer->stop();
    scene->showMenuScreen();
}

void Game::startGame() {
    currentState = STATE_PLAYING;
    currentStage = 1;
    scene->loadStage(1);
    gameTimer->start(GAME_TIMER_INTERVAL);
}

void Game::switchStage(int stage) {
    currentStage = stage;
    scene->loadStage(stage);
}

void Game::showGameOver() {
    currentState = STATE_GAME_OVER;
    gameTimer->stop();
    scene->showGameOverScreen();
}

void Game::showClear() {
    currentState = STATE_CLEAR;
    gameTimer->stop();
    scene->showClearScreen();
}

void Game::restartFromMenu() {
    showMenu();
}

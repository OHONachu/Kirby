#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 設定視窗固定大小為 1620 x 1080
    this->setFixedSize(1620, 1080);

    // 2. 初始狀態設定為選單 (Start Menu)
    currentState = MENU;

    // 3. 設定遊戲迴圈 (Game Loop)
    gameTimer = new QTimer(this);
    // 將計時器的 timeout 訊號連接到我們的 updateGame 函數
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::updateGame);
    // 每 16 毫秒觸發一次 (大約 60 FPS)
    gameTimer->start(16);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 遊戲迴圈：每次計時器時間到，就會執行這裡
void MainWindow::updateGame()
{
    // 呼叫 update() 會觸發下面的 paintEvent 重新畫畫面
    update();
}

// 繪圖事件：負責把東西畫到螢幕上
void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (currentState == MENU) {
        // --- 畫 Start Menu ---
        painter.fillRect(this->rect(), Qt::black); // 黑色背景

        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSize(40);
        painter.setFont(font);

        // 在畫面正中央寫字
        painter.drawText(this->rect(), Qt::AlignCenter, "Kirby's Adventure\n\nPress ENTER to Start");

    } else if (currentState == STAGE1) {
        // --- 畫 Stage 1 ---
        painter.fillRect(this->rect(), QColor(135, 206, 235)); // 天空藍背景

        // 畫一塊綠色的地板 (假裝是地形)
        // x=0, y=800, 寬度=1620, 高度=280
        QRect ground(0, 800, 1620, 280);
        painter.fillRect(ground, Qt::green);
    }
}

// 鍵盤事件：處理玩家的按鍵
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // 如果現在在 MENU 狀態，且玩家按下了 Enter 鍵 (Qt::Key_Return)
    if (currentState == MENU && event->key() == Qt::Key_Return) {
        currentState = STAGE1; // 切換到 Stage 1
    }
}

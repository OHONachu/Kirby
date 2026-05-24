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
    // --- 初始化卡比的數值 ---
    // x=100, y=100(在空中), 寬=80, 高=80
    kirby = QRect(100, 100, 80, 80);
    velocityY = 0;
    moveSpeed = 8;       // 卡比走路的速度
    isMovingLeft = false;
    isMovingRight = false;
    // --- ⬅️ 新增：擺放各種地形方塊 ---
    // 主要的大地板
    platforms.push_back(QRect(0, 800, 1620, 280));
    // 浮空平台 1
    platforms.push_back(QRect(500, 600, 200, 50));
    // 浮空平台 2 (像樓梯一樣變高)
    platforms.push_back(QRect(800, 450, 200, 50));
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
    // 如果進入了 Stage 1，才開始計算物理與移動
    if (currentState == STAGE1) {

        // --- 1. 計算 X 軸 (左右) 移動與碰撞 ---
        int dx = 0;
        if (isMovingLeft) dx -= moveSpeed;
        if (isMovingRight) dx += moveSpeed;

        kirby.translate(dx, 0); // 先試著左右移動

        // 檢查有沒有撞到任何一塊地形
        for (const QRect &plat : platforms) {
            if (kirby.intersects(plat)) {
                if (dx > 0) { // 往右走撞到，就把卡比貼在平台左邊
                    kirby.moveRight(plat.left());
                } else if (dx < 0) { // 往左走撞到，就把卡比貼在平台右邊
                    kirby.moveLeft(plat.right());
                }
            }
        }

        // --- 2. 計算 Y 軸 (重力與跳躍) 移動與碰撞 ---
        velocityY += 1; // 受到地心引力
        kirby.translate(0, velocityY); // 先試著上下移動

        // 檢查有沒有撞到任何一塊地形
        for (const QRect &plat : platforms) {
            if (kirby.intersects(plat)) {
                if (velocityY > 0) {
                    // 往下掉時撞到 (踩到地板了)
                    kirby.moveBottom(plat.top()); // 貼齊地板上方
                    velocityY = 0;                // 重力歸零
                } else if (velocityY < 0) {
                    // 往上跳時撞到 (頭頂撞到天花板)
                    kirby.moveTop(plat.bottom()); // 貼齊天花板下方
                    velocityY = 0;                // 撞到頭，失去向上的力量
                }
            }
        }

        // --- 3. 防止卡比掉出地圖邊界 (死亡判定先簡單處理) ---
        if (kirby.top() > 1080) {
            // 掉下深淵，強制重生在起點
            kirby.moveTo(100, 100);
            velocityY = 0;
        }
    }

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
        for (const QRect &plat : platforms) {
            painter.fillRect(plat, Qt::green);
        }
        // --- 畫出卡比 (紅色的正方形) ---
        painter.fillRect(kirby, Qt::red);
    }
}

// 鍵盤事件：處理玩家的按鍵
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // 如果在 Menu 狀態按下 Enter
    if (currentState == MENU && event->key() == Qt::Key_Return) {
        currentState = STAGE1;
    }

    // 如果在 Stage 1，處理卡比的操作
    if (currentState == STAGE1) {
        if (event->key() == Qt::Key_Left) {
            isMovingLeft = true;
        }
        if (event->key() == Qt::Key_Right) {
            isMovingRight = true;
        }
        // 跳躍邏輯 (按 Up 鍵，且必須踩在地板上才能跳)
        if (event->key() == Qt::Key_Up && velocityY == 0) {
            velocityY = -22; // 給予一個向上的負向速度
        }
    }
}

// 當玩家鬆開鍵盤按鍵時觸發
void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (currentState == STAGE1) {
        if (event->key() == Qt::Key_Left) {
            isMovingLeft = false;
        }
        if (event->key() == Qt::Key_Right) {
            isMovingRight = false;
        }
    }
}

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>       // 加入計時器
#include <QKeyEvent>    // 加入鍵盤事件
#include <QPaintEvent>  // 加入繪圖事件
#include <QPainter>     // 加入畫筆

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 定義遊戲狀態
enum GameState {
    MENU,
    STAGE1
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 覆寫 Qt 內建的事件
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // 我們的遊戲迴圈更新函數
    void updateGame();

private:
    Ui::MainWindow *ui;
    QTimer *gameTimer;       // 遊戲迴圈計時器
    GameState currentState;  // 記錄現在在哪個畫面
};

#endif // MAINWINDOW_H

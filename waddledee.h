#ifndef WADDLEDEE_H
#define WADDLEDEE_H

// ============================================================
// Waddle Dee - 最基本的巡邏型敵人
// 行為：左右來回巡邏，碰到巡邏邊界或方塊就轉向
// ============================================================

#include "enemy.h"
#include <QVector>
#include <QPixmap>

class WaddleDee : public Enemy
{
public:
    // startX, startY: 初始位置
    WaddleDee(double startX, double startY, QGraphicsItem *parent = nullptr);
    ~WaddleDee();

    // 每幀更新邏輯
    void updateEnemy() override;

    // 每幀更新動畫
    void updateAnimation() override;

private:
    // 載入所有精靈圖
    void loadSprites();

    // === 動畫幀 ===
    QVector<QPixmap> walkFrames;  // 7 張行走動畫 (Waddle_Dee_0 ~ Waddle_Dee_6)
};

#endif // WADDLEDEE_H

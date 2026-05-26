#ifndef SPARKY_H
#define SPARKY_H

// ============================================================
// Sparky - 會放電的跳躍型敵人
// 行為：跳躍移動，定期停下來產生電場
// 放電時不可被吸入
// 被吸入後賦予電擊能力 (SPARK)
// ============================================================

#include "enemy.h"
#include <QVector>
#include <QPixmap>

class Sparky : public Enemy
{
public:
    Sparky(double startX, double startY, QGraphicsItem *parent = nullptr);
    ~Sparky();

    // 每幀更新邏輯
    void updateEnemy() override;

    // 每幀更新動畫
    void updateAnimation() override;

    // 是否正在放電 (供碰撞判定使用)
    bool isSparking() const { return isAttacking; }

    // 取得電場碰撞箱 (比本體大)
    QRectF getSparkHitbox() const;

private:
    // 載入所有精靈圖
    void loadSprites();

    // 執行跳躍
    void doHop();

    // === 精靈圖 ===
    QPixmap walkLeft1, walkLeft2;     // 行走動畫 (左)
    QPixmap walkRight1, walkRight2;   // 行走動畫 (右)
    QPixmap attack1, attack2;         // 攻擊動畫 (放電)

    // === 狀態 ===
    bool onGround;        // 是否在地面上
    int hopTimer;         // 跳躍計時器 (決定何時跳)
    int hopInterval;      // 跳躍間隔 (幀)
};

#endif // SPARKY_H

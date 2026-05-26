#ifndef HOTHEAD_H
#define HOTHEAD_H

// ============================================================
// Hot Head - 會噴火的巡邏型敵人
// 行為：左右巡邏，定期停下來朝面對方向射出火球
// 被吸入後賦予火焰能力 (FIRE)
// ============================================================

#include "enemy.h"
#include <QVector>
#include <QPixmap>

// 前向宣告
class Projectile;

class HotHead : public Enemy
{
public:
    HotHead(double startX, double startY, QGraphicsItem *parent = nullptr);
    ~HotHead();

    // 每幀更新邏輯
    void updateEnemy() override;

    // 每幀更新動畫
    void updateAnimation() override;

    // 檢查是否需要生成火球 (由 GameScene 呼叫)
    bool shouldSpawnFireball() const { return spawnFireball; }

    // 取得火球生成資訊
    double getFireballX() const;
    double getFireballY() const;
    double getFireballVX() const;

    // 重置火球生成旗標
    void clearFireballFlag() { spawnFireball = false; }

private:
    // 載入所有精靈圖
    void loadSprites();

    // === 精靈圖 ===
    QPixmap walkLeftSprite;      // 行走 (左)
    QPixmap walkRightSprite;     // 行走 (右)
    QPixmap stopLeftSprite;      // 站立 (左)
    QPixmap stopRightSprite;     // 站立 (右)
    QPixmap attackLeftSprite;    // 攻擊 (左)
    QPixmap attackRightSprite;   // 攻擊 (右)

    // === 狀態 ===
    bool spawnFireball;          // 是否需要生成火球 (通知 GameScene)
};

#endif // HOTHEAD_H

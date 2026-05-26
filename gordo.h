#ifndef GORDO_H
#define GORDO_H

// ============================================================
// Gordo - 無敵的漂浮型敵人
// 行為：在兩個 Y 值之間上下浮動
// 不可被吸入，不可被傷害
// ============================================================

#include "enemy.h"
#include <QVector>
#include <QPixmap>

class Gordo : public Enemy
{
public:
    // startX, startY: 初始位置 (浮動中心點)
    Gordo(double startX, double startY, QGraphicsItem *parent = nullptr);
    ~Gordo();

    // 每幀更新邏輯
    void updateEnemy() override;

    // 每幀更新動畫
    void updateAnimation() override;

    // 設定浮動範圍 (Y 軸)
    void setFloatRange(double minY, double maxY);

private:
    // 載入所有精靈圖
    void loadSprites();

    // === 動畫幀 ===
    QVector<QPixmap> floatFrames;  // 2 張浮動動畫

    // === 浮動參數 ===
    double floatMinY;     // 浮動上限 (最高點)
    double floatMaxY;     // 浮動下限 (最低點)
    double floatCenterY;  // 浮動中心 Y
    double floatAngle;    // 用於正弦浮動的角度
    bool movingDown;      // 是否正在向下移動
};

#endif // GORDO_H

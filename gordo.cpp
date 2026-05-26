// ============================================================
// Gordo 實作
// 無敵的漂浮敵人：在固定路徑上下浮動
// 使用正弦函數產生平滑的上下運動
// 不可被吸入、不可被傷害、不可被殺死
// ============================================================

#include "gordo.h"
#include <QtMath>
#include <QDebug>

// -------------------------------------------------------
// 建構子
// -------------------------------------------------------
Gordo::Gordo(double startX, double startY, QGraphicsItem *parent)
    : Enemy(EnemyType::GORDO, startX, startY, parent)
    , floatCenterY(startY)
    , floatAngle(0.0)
    , movingDown(true)
{
    // Gordo 特殊屬性：完全無敵
    canBeInhaled = false;      // 不可吸入
    canBeDamaged = false;      // 不可傷害
    grantedAbility = AbilityType::NONE;

    // 設定預設浮動範圍
    floatMinY = startY - GORDO_FLOAT_RANGE;
    floatMaxY = startY + GORDO_FLOAT_RANGE;

    // Gordo 不受重力影響，只上下浮動
    vx = 0;
    vy = 0;

    // 載入精靈圖
    loadSprites();

    // 設定初始圖片
    if (!floatFrames.isEmpty()) {
        setPixmap(floatFrames[0]);
    }
}

// -------------------------------------------------------
// 解構子
// -------------------------------------------------------
Gordo::~Gordo()
{
}

// -------------------------------------------------------
// 載入精靈圖
// Gordo 有 2 張動畫幀
// -------------------------------------------------------
void Gordo::loadSprites()
{
    // 載入 Gordo(0).png 和 Gordo(1).png
    for (int i = 0; i <= 1; ++i) {
        QString path = QString("Dataset/Gordo/Gordo(%1).png").arg(i);
        QPixmap frame = loadAndScaleSprite(path);
        floatFrames.append(frame);
    }

    qDebug() << "Gordo: Loaded" << floatFrames.size() << "float frames";
}

// -------------------------------------------------------
// 每幀更新邏輯
// 使用正弦函數計算 Y 位置，產生平滑上下浮動
// -------------------------------------------------------
void Gordo::updateEnemy()
{
    if (!alive) return;

    // === 正弦浮動 ===
    // 使用 sin 函數讓 Gordo 平滑地上下移動
    floatAngle += 0.03;  // 控制浮動速度
    if (floatAngle >= 2 * M_PI) {
        floatAngle -= 2 * M_PI;  // 防止角度溢出
    }

    // 計算新的 Y 位置
    double newY = floatCenterY + qSin(floatAngle) * GORDO_FLOAT_RANGE;

    // X 位置保持不動 (Gordo 不水平移動)
    setPos(x(), newY);

    // 判斷移動方向 (用於可能的視覺效果)
    movingDown = (qCos(floatAngle) > 0);

    // 更新動畫
    updateAnimation();
}

// -------------------------------------------------------
// 更新動畫
// 在 2 張動畫幀之間交替
// -------------------------------------------------------
void Gordo::updateAnimation()
{
    if (!alive || floatFrames.isEmpty()) return;

    animTimer++;

    // 每 GORDO_ANIM_SPEED 幀切換一次動畫
    if (animTimer >= GORDO_ANIM_SPEED) {
        animTimer = 0;
        animFrame = (animFrame + 1) % floatFrames.size();
        setPixmap(floatFrames[animFrame]);
    }
}

// -------------------------------------------------------
// 設定浮動範圍
// -------------------------------------------------------
void Gordo::setFloatRange(double minY, double maxY)
{
    floatMinY = minY;
    floatMaxY = maxY;
    floatCenterY = (minY + maxY) / 2.0;
}

// ============================================================
// Waddle Dee 實作
// 最基本的敵人：左右巡邏，不會攻擊
// 可以被吸入，不賦予任何能力
// ============================================================

#include "waddledee.h"
#include <QDebug>

// -------------------------------------------------------
// 建構子
// -------------------------------------------------------
WaddleDee::WaddleDee(double startX, double startY, QGraphicsItem *parent)
    : Enemy(EnemyType::WADDLE_DEE, startX, startY, parent)
{
    // Waddle Dee 屬性設定
    canBeInhaled = true;       // 可以被吸入
    canBeDamaged = true;       // 可以被傷害
    grantedAbility = AbilityType::NONE;  // 不給予能力

    // 初始速度：向左巡邏
    vx = -ENEMY_SPEED;
    facingRight = false;

    // 載入精靈圖
    loadSprites();

    // 設定初始圖片
    if (!walkFrames.isEmpty()) {
        setPixmap(walkFrames[0]);
    }
}

// -------------------------------------------------------
// 解構子
// -------------------------------------------------------
WaddleDee::~WaddleDee()
{
}

// -------------------------------------------------------
// 載入所有精靈圖
// Waddle Dee 有 7 張行走動畫
// -------------------------------------------------------
void WaddleDee::loadSprites()
{
    // 載入 Waddle_Dee_0.png ~ Waddle_Dee_6.png
    for (int i = 0; i <= 6; ++i) {
        QString path = QString("Dataset/Waddle Dee/Waddle_Dee_%1.png").arg(i);
        QPixmap frame = loadAndScaleSprite(path);
        walkFrames.append(frame);
    }

    qDebug() << "WaddleDee: Loaded" << walkFrames.size() << "walk frames";
}

// -------------------------------------------------------
// 每幀更新邏輯
// 巡邏行為：左右來回走動
// -------------------------------------------------------
void WaddleDee::updateEnemy()
{
    if (!alive) return;

    // === 應用重力 ===
    vy += GRAVITY;
    if (vy > MAX_FALL_SPEED) {
        vy = MAX_FALL_SPEED;
    }

    // === 水平移動 ===
    double newX = x() + vx;
    double newY = y() + vy;

    // === 巡邏邊界檢查 ===
    // 到達巡邏範圍邊緣時轉向
    if (newX <= patrolMinX) {
        newX = patrolMinX;
        vx = ENEMY_SPEED;     // 轉向右
        facingRight = true;
    } else if (newX >= patrolMaxX) {
        newX = patrolMaxX;
        vx = -ENEMY_SPEED;    // 轉向左
        facingRight = false;
    }

    // 更新位置
    setPos(newX, newY);

    // 更新動畫
    updateAnimation();
}

// -------------------------------------------------------
// 更新動畫
// 依照固定間隔切換行走動畫幀
// -------------------------------------------------------
void WaddleDee::updateAnimation()
{
    if (!alive || walkFrames.isEmpty()) return;

    animTimer++;

    // 每 ENEMY_ANIM_SPEED 幀切換一次動畫
    if (animTimer >= ENEMY_ANIM_SPEED) {
        animTimer = 0;
        animFrame = (animFrame + 1) % walkFrames.size();

        QPixmap frame = walkFrames[animFrame];

        // 如果面向右，水平翻轉精靈圖
        // Waddle Dee 的原始圖片面向左
        if (facingRight) {
            frame = frame.transformed(QTransform().scale(-1, 1));
        }

        setPixmap(frame);
    }
}

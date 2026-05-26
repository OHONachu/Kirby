// ============================================================
// Hot Head 實作
// 巡邏型敵人，會定期停下來噴出火球
// 被吸入後賦予火焰能力 (FIRE)
// ============================================================

#include "hothead.h"
#include <QDebug>

// -------------------------------------------------------
// 建構子
// -------------------------------------------------------
HotHead::HotHead(double startX, double startY, QGraphicsItem *parent)
    : Enemy(EnemyType::HOT_HEAD, startX, startY, parent)
    , spawnFireball(false)
{
    // Hot Head 屬性
    canBeInhaled = true;
    canBeDamaged = true;
    grantedAbility = AbilityType::FIRE;  // 賦予火焰能力

    // 初始巡邏速度
    vx = -ENEMY_SPEED;
    facingRight = false;

    // 攻擊冷卻從隨機值開始，避免所有 Hot Head 同時攻擊
    attackCooldown = HOTHEAD_ATTACK_INTERVAL / 2;

    // 載入精靈圖
    loadSprites();

    // 設定初始圖片
    setPixmap(stopLeftSprite);
}

// -------------------------------------------------------
// 解構子
// -------------------------------------------------------
HotHead::~HotHead()
{
}

// -------------------------------------------------------
// 載入所有精靈圖
// -------------------------------------------------------
void HotHead::loadSprites()
{
    // 行走精靈圖
    walkLeftSprite   = loadAndScaleSprite("Dataset/Hot Head/Hot_head_run_L.png");
    walkRightSprite  = loadAndScaleSprite("Dataset/Hot Head/Hot_head_run_R.png");

    // 站立精靈圖
    stopLeftSprite   = loadAndScaleSprite("Dataset/Hot Head/Hot_head_stop_L.png");
    stopRightSprite  = loadAndScaleSprite("Dataset/Hot Head/Hot_head_stop_R.png");

    // 攻擊精靈圖
    attackLeftSprite  = loadAndScaleSprite("Dataset/Hot Head/Hot_head_attack_L.png");
    attackRightSprite = loadAndScaleSprite("Dataset/Hot Head/Hot_head_attack_R.png");

    qDebug() << "HotHead: All sprites loaded";
}

// -------------------------------------------------------
// 每幀更新邏輯
// 行為模式：巡邏 → 停下攻擊 → 繼續巡邏
// -------------------------------------------------------
void HotHead::updateEnemy()
{
    if (!alive) return;

    // 重置火球生成旗標
    spawnFireball = false;

    // === 攻擊狀態 ===
    if (isAttacking) {
        attackTimer++;

        // 攻擊開始時生成火球 (只在攻擊第 10 幀時發射)
        if (attackTimer == 10) {
            spawnFireball = true;
        }

        // 攻擊結束，恢復巡邏
        if (attackTimer >= HOTHEAD_ATTACK_DURATION) {
            isAttacking = false;
            attackTimer = 0;
            attackCooldown = 0;

            // 恢復巡邏速度
            vx = facingRight ? ENEMY_SPEED : -ENEMY_SPEED;
        }

        // 攻擊中不移動
        updateAnimation();
        return;
    }

    // === 攻擊冷卻計時 ===
    attackCooldown++;
    if (attackCooldown >= HOTHEAD_ATTACK_INTERVAL) {
        // 進入攻擊狀態
        isAttacking = true;
        attackTimer = 0;
        vx = 0;  // 停下來攻擊

        updateAnimation();
        return;
    }

    // === 應用重力 ===
    vy += GRAVITY;
    if (vy > MAX_FALL_SPEED) {
        vy = MAX_FALL_SPEED;
    }

    // === 水平移動 (巡邏) ===
    double newX = x() + vx;
    double newY = y() + vy;

    // === 巡邏邊界檢查 ===
    if (newX <= patrolMinX) {
        newX = patrolMinX;
        vx = ENEMY_SPEED;
        facingRight = true;
    } else if (newX >= patrolMaxX) {
        newX = patrolMaxX;
        vx = -ENEMY_SPEED;
        facingRight = false;
    }

    // 更新位置
    setPos(newX, newY);

    // 更新動畫
    updateAnimation();
}

// -------------------------------------------------------
// 更新動畫
// 根據當前狀態選擇正確的精靈圖
// -------------------------------------------------------
void HotHead::updateAnimation()
{
    if (!alive) return;

    animTimer++;

    if (isAttacking) {
        // 攻擊動畫
        setPixmap(facingRight ? attackRightSprite : attackLeftSprite);
    } else if (qAbs(vx) > 0.1) {
        // 行走動畫：在行走和站立之間交替
        if (animTimer >= ENEMY_ANIM_SPEED) {
            animTimer = 0;
            animFrame = (animFrame + 1) % 2;
        }

        if (animFrame == 0) {
            setPixmap(facingRight ? walkRightSprite : walkLeftSprite);
        } else {
            setPixmap(facingRight ? stopRightSprite : stopLeftSprite);
        }
    } else {
        // 站立
        setPixmap(facingRight ? stopRightSprite : stopLeftSprite);
    }
}

// -------------------------------------------------------
// 取得火球生成位置與速度
// 火球從 Hot Head 面前生成
// -------------------------------------------------------
double HotHead::getFireballX() const
{
    if (facingRight) {
        return x() + boundingRect().width();  // 右邊發射
    } else {
        return x() - 30 * SPRITE_SCALE;       // 左邊發射
    }
}

double HotHead::getFireballY() const
{
    // 從身體中央偏下方發射
    return y() + boundingRect().height() * 0.4;
}

double HotHead::getFireballVX() const
{
    return facingRight ? HOTHEAD_FIRE_SPEED : -HOTHEAD_FIRE_SPEED;
}

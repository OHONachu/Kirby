// ============================================================
// Sparky 實作
// 跳躍型敵人，會定期停下來放電
// 放電時不可被吸入 (有電場保護)
// 被吸入後賦予電擊能力 (SPARK)
// ============================================================

#include "sparky.h"
#include <QDebug>
#include <QtMath>

// -------------------------------------------------------
// 建構子
// -------------------------------------------------------
Sparky::Sparky(double startX, double startY, QGraphicsItem *parent)
    : Enemy(EnemyType::SPARKY, startX, startY, parent)
    , onGround(true)
    , hopTimer(0)
    , hopInterval(40)  // 每 40 幀跳一次
{
    // Sparky 屬性
    canBeInhaled = true;       // 可以被吸入 (非放電時)
    canBeDamaged = true;       // 可以被傷害
    grantedAbility = AbilityType::SPARK;  // 賦予電擊能力

    // 初始速度
    vx = -SPARKY_SPEED;
    facingRight = false;

    // 攻擊冷卻
    attackCooldown = 0;

    // 載入精靈圖
    loadSprites();

    // 設定初始圖片
    setPixmap(walkLeft1);
}

// -------------------------------------------------------
// 解構子
// -------------------------------------------------------
Sparky::~Sparky()
{
}

// -------------------------------------------------------
// 載入所有精靈圖
// 注意：原始檔名有 typo "ritht" (應為 right)
// -------------------------------------------------------
void Sparky::loadSprites()
{
    // 行走動畫 - 左
    walkLeft1  = loadAndScaleSprite("Dataset/Sparky/Sparky_left_1.png");
    walkLeft2  = loadAndScaleSprite("Dataset/Sparky/Sparky_left_2.png");

    // 行走動畫 - 右 (注意：原始檔名是 "ritht" 不是 "right")
    walkRight1 = loadAndScaleSprite("Dataset/Sparky/Sparky_ritht_1.png");
    walkRight2 = loadAndScaleSprite("Dataset/Sparky/Sparky_ritht_2.png");

    // 攻擊動畫 (放電)
    attack1 = loadAndScaleSprite("Dataset/Sparky/Sparky_attack_1.png");
    attack2 = loadAndScaleSprite("Dataset/Sparky/Sparky_attack_2.png");

    qDebug() << "Sparky: All sprites loaded";
}

// -------------------------------------------------------
// 每幀更新邏輯
// 行為模式：跳躍移動 → 停下放電 → 繼續跳躍
// -------------------------------------------------------
void Sparky::updateEnemy()
{
    if (!alive) return;

    // === 攻擊狀態 (放電中) ===
    if (isAttacking) {
        attackTimer++;

        // 放電期間不可被吸入
        canBeInhaled = false;

        // 放電結束
        if (attackTimer >= SPARKY_ATTACK_DURATION) {
            isAttacking = false;
            attackTimer = 0;
            attackCooldown = 0;
            canBeInhaled = true;  // 恢復可吸入狀態

            // 恢復移動
            vx = facingRight ? SPARKY_SPEED : -SPARKY_SPEED;
        }

        // 放電中仍受重力影響
        vy += GRAVITY;
        if (vy > MAX_FALL_SPEED) vy = MAX_FALL_SPEED;

        double newY = y() + vy;
        setPos(x(), newY);

        updateAnimation();
        return;
    }

    // === 攻擊冷卻計時 ===
    attackCooldown++;
    if (attackCooldown >= SPARKY_ATTACK_INTERVAL && onGround) {
        // 在地面上時進入攻擊狀態
        isAttacking = true;
        attackTimer = 0;
        vx = 0;  // 停下來放電

        updateAnimation();
        return;
    }

    // === 跳躍計時 ===
    if (onGround) {
        hopTimer++;
        if (hopTimer >= hopInterval) {
            doHop();
            hopTimer = 0;
        }
    }

    // === 應用重力 ===
    vy += GRAVITY;
    if (vy > MAX_FALL_SPEED) {
        vy = MAX_FALL_SPEED;
    }

    // === 移動 ===
    double newX = x() + vx;
    double newY = y() + vy;

    // === 巡邏邊界檢查 ===
    if (newX <= patrolMinX) {
        newX = patrolMinX;
        vx = SPARKY_SPEED;
        facingRight = true;
    } else if (newX >= patrolMaxX) {
        newX = patrolMaxX;
        vx = -SPARKY_SPEED;
        facingRight = false;
    }

    // 更新位置
    setPos(newX, newY);

    // === 簡易地面偵測 ===
    // (實際遊戲中由 GameScene 的碰撞系統處理)
    // 這裡使用 GROUND_Y 作為臨時地面
    if (newY >= GROUND_Y) {
        setPos(newX, GROUND_Y);
        vy = 0;
        onGround = true;
    } else {
        onGround = false;
    }

    // 更新動畫
    updateAnimation();
}

// -------------------------------------------------------
// 跳躍
// -------------------------------------------------------
void Sparky::doHop()
{
    if (!onGround) return;

    vy = SPARKY_HOP_FORCE;
    onGround = false;
}

// -------------------------------------------------------
// 更新動畫
// -------------------------------------------------------
void Sparky::updateAnimation()
{
    if (!alive) return;

    animTimer++;

    if (isAttacking) {
        // 放電動畫：在 attack1 和 attack2 之間交替
        if (animTimer >= SPARKY_ANIM_SPEED) {
            animTimer = 0;
            animFrame = (animFrame + 1) % 2;
        }
        setPixmap(animFrame == 0 ? attack1 : attack2);
    } else {
        // 行走/跳躍動畫
        if (animTimer >= SPARKY_ANIM_SPEED) {
            animTimer = 0;
            animFrame = (animFrame + 1) % 2;
        }

        if (facingRight) {
            setPixmap(animFrame == 0 ? walkRight1 : walkRight2);
        } else {
            setPixmap(animFrame == 0 ? walkLeft1 : walkLeft2);
        }
    }
}

// -------------------------------------------------------
// 取得電場碰撞箱
// 放電時碰撞箱比本體大很多 (電場效果)
// -------------------------------------------------------
QRectF Sparky::getSparkHitbox() const
{
    if (!isAttacking) {
        return getHitbox();  // 非放電時返回普通碰撞箱
    }

    // 放電時碰撞箱向外擴展
    QRectF base = boundingRect();
    double sparkExpand = 20.0 * SPRITE_SCALE;  // 電場擴展範圍

    return QRectF(
        x() - sparkExpand,
        y() - sparkExpand,
        base.width() + 2 * sparkExpand,
        base.height() + 2 * sparkExpand
    );
}

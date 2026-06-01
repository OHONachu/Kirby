#include "enemy.h"

// ============ Enemy 基類 ============
Enemy::Enemy(EnemyType t, double sx, double sy, double pMinX, double pMaxX)
    : type(t), startX(sx), startY(sy), patrolMinX(pMinX), patrolMaxX(pMaxX)//初始化
{
    vx = 0; vy = 0;
    facingRight = true;
    alive = true;
    canBeInhaled = true;
    canBeDamaged = true;
    grantedAbility = ABILITY_NONE;
    isAttacking = false;
    attackTimer = 0;
    attackCooldown = 0;
    animFrame = 0;
    animTimer = 0;
    setPos(sx, sy);
    setZValue(5);
}

QPixmap Enemy::loadAndScale(const QString &path) {//抓圖片
    QPixmap pix(path);
    if (pix.isNull()) {
        pix = QPixmap(50, 50);//破圖處理
        pix.fill(Qt::red);
    }
    return pix.scaled(pix.width() * SPRITE_SCALE, pix.height() * SPRITE_SCALE,
                      Qt::KeepAspectRatio, Qt::FastTransformation);
}

QRectF Enemy::getHitbox() const {
    double w = pixmap().width() * 0.8;
    double h = pixmap().height() * 0.8;
    double offX = (pixmap().width() - w) / 2.0;
    double offY = (pixmap().height() - h) / 2.0;
    return QRectF(x() + offX, y() + offY, w, h);
}

void Enemy::die() {
    alive = false;
    setVisible(false);
}

void Enemy::respawn() {
    alive = true;
    setVisible(true);
    setPos(startX, startY);
    vx = ENEMY_SPEED;
    facingRight = true;
    isAttacking = false;
    attackTimer = 0;
    attackCooldown = 0;
}

// ========================================
// Waddle Dee - 在平台上來回走動
// ========================================
WaddleDee::WaddleDee(double sx, double sy, double pMinX, double pMaxX)
    : Enemy(ENEMY_WADDLE_DEE, sx, sy, pMinX, pMaxX)
{
    canBeInhaled = true;
    canBeDamaged = true;
    grantedAbility = ABILITY_NONE;
    vx = ENEMY_SPEED;

    // 載入 7 個走路動畫 frames
    // 載入 7 個走路動畫 frames (手動逐一加入)
    walkFrames.append(loadAndScale(":/Dataset/Waddle Dee/Waddle_Dee_0.png"));
    walkFrames.append(loadAndScale(":/Dataset/Waddle Dee/Waddle_Dee_1.png"));
    walkFrames.append(loadAndScale(":/Dataset/Waddle Dee/Waddle_Dee_2.png"));
    walkFrames.append(loadAndScale(":/Dataset/Waddle Dee/Waddle_Dee_3.png"));
    walkFrames.append(loadAndScale(":/Dataset/Waddle Dee/Waddle_Dee_4.png"));
    walkFrames.append(loadAndScale(":/Dataset/Waddle Dee/Waddle_Dee_5.png"));
    walkFrames.append(loadAndScale(":/Dataset/Waddle Dee/Waddle_Dee_6.png"));

    if (!walkFrames.isEmpty()) {
        setPixmap(walkFrames[0]);
    }
}

void WaddleDee::updateEnemy() {
    if (!alive) return;

    // 移動
    setPos(x() + vx, y());

    // 碰到巡邏邊界就轉向
    if (x() <= patrolMinX) {
        vx = ENEMY_SPEED;
        facingRight = true;
    }
    if (x() + pixmap().width() >= patrolMaxX) {
        vx = -ENEMY_SPEED;
        facingRight = false;
    }

    // 動畫
    animTimer++;
    if (animTimer >= 6) {
        animTimer = 0;
        animFrame = (animFrame + 1) % walkFrames.size();
        setPixmap(walkFrames[animFrame]);
    }
}

// ========================================
// Gordo - 無敵，上下浮動
// ========================================
Gordo::Gordo(double sx, double sy, double minY, double maxY)
    : Enemy(ENEMY_GORDO, sx, sy, sx, sx), moveMinY(minY), moveMaxY(maxY)
{
    canBeInhaled = false;  // 不可吸入
    canBeDamaged = false;  // 不可攻擊
    grantedAbility = ABILITY_NONE;
    movingDown = true;
    vx = 0;
    vy = 1.5;

    frame0 = loadAndScale(":/Dataset/Gordo/Gordo(0).png");
    frame1 = loadAndScale(":/Dataset/Gordo/Gordo(1).png");
    setPixmap(frame0);
}

void Gordo::updateEnemy() {
    if (!alive) return;

    // 上下移動
    if (movingDown) {
        setPos(x(), y() + vy);
        if (y() >= moveMaxY) movingDown = false;
    } else {
        setPos(x(), y() - vy);
        if (y() <= moveMinY) movingDown = true;
    }

    // 動畫 (兩張交替)
    animTimer++;
    if (animTimer >= 15) {
        animTimer = 0;
        animFrame = 1 - animFrame;
        setPixmap(animFrame == 0 ? frame0 : frame1);
    }
}

// ========================================
// Hot Head - 巡邏並噴火球
// ========================================
HotHead::HotHead(double sx, double sy, double pMinX, double pMaxX)
    : Enemy(ENEMY_HOT_HEAD, sx, sy, pMinX, pMaxX)
{
    canBeInhaled = true;
    canBeDamaged = true;
    grantedAbility = ABILITY_FIRE;
    vx = ENEMY_SPEED;
    wantsToShoot = false;
    attackCooldown = HOTHEAD_FIRE_INTERVAL;

    spr_walk_R  = loadAndScale(":/Dataset/Hot Head/Hot_head_run_R.png");
    spr_walk_L  = loadAndScale(":/Dataset/Hot Head/Hot_head_run_L.png");
    spr_stop_R  = loadAndScale(":/Dataset/Hot Head/Hot_head_stop_R.png");
    spr_stop_L  = loadAndScale(":/Dataset/Hot Head/Hot_head_stop_L.png");
    spr_attack_R = loadAndScale(":/Dataset/Hot Head/Hot_head_attack_R.png");
    spr_attack_L = loadAndScale(":/Dataset/Hot Head/Hot_head_attack_L.png");
    spr_fire_proj = loadAndScale(":/Dataset/Hot Head/Hot_head_fire(1).png");

    setPixmap(spr_walk_R);
}

void HotHead::updateEnemy() {
    if (!alive) return;

    wantsToShoot = false;

    if (isAttacking) {
        // 攻擊動畫
        attackTimer--;
        setPixmap(facingRight ? spr_attack_R : spr_attack_L);
        if (attackTimer <= 0) {
            isAttacking = false;
            wantsToShoot = true; // 通知 GameScene 生成火球
        }
        return;
    }

    // 巡邏移動
    setPos(x() + vx, y());

    if (x() <= patrolMinX) {
        vx = ENEMY_SPEED;
        facingRight = true;
    }
    if (x() + pixmap().width() >= patrolMaxX) {
        vx = -ENEMY_SPEED;
        facingRight = false;
    }

    // 攻擊冷卻
    attackCooldown--;
    if (attackCooldown <= 0) {
        isAttacking = true;
        attackTimer = 30; // 攻擊動畫持續 30 幀
        attackCooldown = HOTHEAD_FIRE_INTERVAL;
        vx = 0;
    }

    // 動畫
    animTimer++;
    if (animTimer >= 10) {
        animTimer = 0;
        animFrame = 1 - animFrame;
    }
    if (vx != 0) {
        setPixmap(facingRight ? spr_walk_R : spr_walk_L);
    } else {
        setPixmap(facingRight ? spr_stop_R : spr_stop_L);
    }
}

bool HotHead::shouldShootFire() {
    bool result = wantsToShoot;
    wantsToShoot = false;
    return result;
}

// ========================================
// Sparky - 跳動 + 電擊
// ========================================
Sparky::Sparky(double sx, double sy, double pMinX, double pMaxX)
    : Enemy(ENEMY_SPARKY, sx, sy, pMinX, pMaxX)
{
    canBeInhaled = true;
    canBeDamaged = true;
    grantedAbility = ABILITY_SPARK;
    vx = ENEMY_SPEED;
    sparkTimer = 0;
    jumpTimer = 60;
    onGround = true;

    // 1. 先載入 attack1 作為「基準尺寸」
    spr_attack1 = loadAndScale(":/Dataset/Sparky/Sparky_attack_1.png");

    // 2. 獲取並儲存 attack1 的尺寸
    QSize targetSize = spr_attack1.size();

    // 3. 載入其他圖片時，直接在後面串接 .scaled() 強制縮放為 targetSize
    spr_attack2 = loadAndScale(":/Dataset/Sparky/Sparky_attack_2.png").scaled(targetSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    spr_walk_L1 = loadAndScale(":/Dataset/Sparky/Sparky_left_1.png").scaled(targetSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_walk_L2 = loadAndScale(":/Dataset/Sparky/Sparky_left_2.png").scaled(targetSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    spr_walk_R1 = loadAndScale(":/Dataset/Sparky/Sparky_ritht_1.png").scaled(targetSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_walk_R2 = loadAndScale(":/Dataset/Sparky/Sparky_ritht_2.png").scaled(targetSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    setPixmap(spr_walk_R1);
}

void Sparky::updateEnemy() {
    if (!alive) return;

    // 電擊狀態
    if (isAttacking) {
        sparkTimer--;
        canBeInhaled = false; // 電擊中不可吸入
        animTimer++;
        if (animTimer >= 8) {
            animTimer = 0;
            animFrame = 1 - animFrame;
        }
        setPixmap(animFrame == 0 ? spr_attack1 : spr_attack2);
        if (sparkTimer <= 0) {
            isAttacking = false;
            canBeInhaled = true;
            attackCooldown = 120;
        }
        return;
    }

    // 重力
    if (!onGround) {
        vy += GRAVITY;
        if (vy > MAX_FALL_SPEED) vy = MAX_FALL_SPEED;
    }

    // 移動
    setPos(x() + vx, y() + vy);

    // 巡邏邊界
    if (x() <= patrolMinX) { vx = ENEMY_SPEED; facingRight = true; }
    if (x() + pixmap().width() >= patrolMaxX) { vx = -ENEMY_SPEED; facingRight = false; }

    // 地板碰撞（簡易）
    if (y() + pixmap().height() >= FLOOR_Y) {
        setY(FLOOR_Y - pixmap().height());
        vy = 0;
        onGround = true;
    }

    // 跳動
    jumpTimer--;
    if (jumpTimer <= 0 && onGround) {
        vy = SPARKY_JUMP_FORCE;
        onGround = false;
        jumpTimer = 90 + (qrand() % 60);
    }

    // 電擊觸發
    attackCooldown--;
    if (attackCooldown <= 0 && onGround) {
        isAttacking = true;
        sparkTimer = SPARKY_SPARK_DURATION;
        vx = 0;
    }

    // 走路動畫
    animTimer++;
    if (animTimer >= 10) {
        animTimer = 0;
        animFrame = 1 - animFrame;
    }
    if (facingRight) {
        setPixmap(animFrame == 0 ? spr_walk_R1 : spr_walk_R2);
    } else {
        setPixmap(animFrame == 0 ? spr_walk_L1 : spr_walk_L2);
    }
}

bool Sparky::isSparking() const {
    return isAttacking;
}

QRectF Sparky::getSparkBox() const {
    if (!isAttacking) return QRectF();
    double expand = 30;
    return QRectF(x() - expand, y() - expand,
                  pixmap().width() + expand * 2,
                  pixmap().height() + expand * 2);
}

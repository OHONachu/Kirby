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
    targetX = sx;

    spr_walk_R  = loadAndScale(":/Dataset/Hot Head/Hot_head_run_R.png");
    spr_walk_L  = loadAndScale(":/Dataset/Hot Head/Hot_head_run_L.png");
    spr_stop_R  = loadAndScale(":/Dataset/Hot Head/Hot_head_stop_R.png");
    spr_stop_L  = loadAndScale(":/Dataset/Hot Head/Hot_head_stop_L.png");
    spr_attack_R = loadAndScale(":/Dataset/Hot Head/Hot_head_attack_R.png");
    spr_attack_L = loadAndScale(":/Dataset/Hot Head/Hot_head_attack_L.png");
    spr_fire_proj = loadAndScale(":/Dataset/Hot Head/Hot_head_fire(1).png");
    spr_breath_R1 = loadAndScale(":/Dataset/Hot Head/Hot_head_fire(2)_R.png");
    spr_breath_L1 = loadAndScale(":/Dataset/Hot Head/Hot_head_fire(2)_L.png");
    spr_breath_R2 = loadAndScale(":/Dataset/Hot Head/Hot_head_fire(3)_R.png");
    spr_breath_L2 = loadAndScale(":/Dataset/Hot Head/Hot_head_fire(3)_L.png");
    breathingFire = false;
    attackMode = 0;
    setPixmap(spr_walk_R);
    fireEffect = new QGraphicsPixmapItem(this);  // this = 跟著 HotHead 移動
    fireEffect->setVisible(false);                // 預設隱藏
    fireEffect->setZValue(6);                     // 顯示在本體前面
}
void HotHead::setTargetX(double kirbyX) {
    targetX = kirbyX;
}
void HotHead::updateEnemy() {
    if (!alive) return;
    wantsToShoot = false;
    breathingFire = false;
    if (isAttacking) {
        attackTimer--;
        if (attackMode == 0) {
            // === Fire Ball ===
            setPixmap(facingRight ? spr_attack_R : spr_attack_L);
            fireEffect->setVisible(false);
            if (attackTimer <= 0) {
                isAttacking = false;
                wantsToShoot = true;
            }
        } else {
            // === Flame Breath ===
            breathingFire = true;
            setPixmap(facingRight ? spr_attack_R : spr_attack_L);
            animTimer++;
            if (animTimer >= 6) {
                animTimer = 0;
                animFrame = 1 - animFrame;
            }
            if (animFrame == 0) {
                fireEffect->setPixmap(facingRight ? spr_breath_R1 : spr_breath_L1);
            } else {
                fireEffect->setPixmap(facingRight ? spr_breath_R2 : spr_breath_L2);
            }
            if (facingRight) {
                fireEffect->setPos(pixmap().width(), 0);
            } else {
                fireEffect->setPos(-fireEffect->pixmap().width(), 0);
            }
            fireEffect->setVisible(true);
            if (attackTimer <= 0) {
                isAttacking = false;
                fireEffect->setVisible(false);
            }
        }
        if (!isAttacking) {
            vx = facingRight ? ENEMY_SPEED : -ENEMY_SPEED;
            fireEffect->setVisible(false);
        }
        return;
    }
    // === 巡邏移動 ===
    setPos(x() + vx, y());
    if (x() <= patrolMinX) { vx = ENEMY_SPEED; facingRight = true; }
    if (x() + pixmap().width() >= patrolMaxX) { vx = -ENEMY_SPEED; facingRight = false; }
    // === 判斷是否攻擊 ===
    attackCooldown--;
    if (attackCooldown <= 0) {
        double dx = targetX - x();
        bool kirbyInFront = (facingRight && dx > 0) || (!facingRight && dx < 0);
        double dist = qAbs(dx);
        if (kirbyInFront && dist <= 150) {
            // 近距離 → Flame Breath
            isAttacking = true;
            attackTimer = 45;
            attackMode = 1;
            attackCooldown = HOTHEAD_FIRE_INTERVAL;
            vx = 0;
            animFrame = 0;
            animTimer = 0;
        } else if (kirbyInFront && dist <= 500) {
            // 中距離 → Fire Ball
            isAttacking = true;
            attackTimer = 30;
            attackMode = 0;
            attackCooldown = HOTHEAD_FIRE_INTERVAL;
            vx = 0;
            animFrame = 0;
            animTimer = 0;
        } else {
            // Kirby 不在前方或太遠 → 不攻擊，短時間後再檢查
            attackCooldown = 30;
        }
    }
    // === 走路動畫 ===
    animTimer++;
    if (animTimer >= 10) { animTimer = 0; animFrame = 1 - animFrame; }
    if (vx != 0) { setPixmap(facingRight ? spr_walk_R : spr_walk_L); }
    else { setPixmap(facingRight ? spr_stop_R : spr_stop_L); }
}

bool HotHead::isBreathingFire() const {
    return breathingFire;
}
QRectF HotHead::getBreathBox() const {
    if (!breathingFire) return QRectF();
    double range = 120;  // 噴火距離
    if (facingRight) {
        return QRectF(x() + pixmap().width(), y(), range, pixmap().height());
    } else {
        return QRectF(x() - range, y(), range, pixmap().height());
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
            vx = facingRight ? ENEMY_SPEED : -ENEMY_SPEED;
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
cutKnight::cutKnight(double sx, double sy, double pMinX, double pMaxX)
    : Enemy(ENEMY_KNIGHT, sx, sy, pMinX, pMaxX)
{
    canBeInhaled = true;
    canBeDamaged = true;
    grantedAbility = ABILITY_CUTTER;
    vx = 0;
    targetX = sx;
    wantsToShoot = false;
    attackCooldown = HOTHEAD_FIRE_INTERVAL;


    spr_stop_R = loadAndScale(":/Dataset/cutter knight/ck_stand_R.png")
                 .scaled(CK_W, CK_H, Qt::KeepAspectRatio, Qt::FastTransformation);
    spr_stop_L = loadAndScale(":/Dataset/cutter knight/ck_stand_L.png")
                 .scaled(CK_W, CK_H, Qt::KeepAspectRatio, Qt::FastTransformation);
    for(int i = 1; i<=4; i++){
        attackR.append(loadAndScale(QString(":/Dataset/cutter knight/ck_attack_%1_R.png").arg(i))
                       .scaled(CK_W, CK_H, Qt::KeepAspectRatio, Qt::FastTransformation));
        attackL.append(loadAndScale(QString(":/Dataset/cutter knight/ck_attack_%1_L.png").arg(i))
                       .scaled(CK_W, CK_H, Qt::KeepAspectRatio, Qt::FastTransformation));
    }
    for (int i = 1; i <= 4; i++) {
        cutterFrames.append(loadAndScale(QString(":/Dataset/cutter knight/ckw%1.png").arg(i))
        .scaled(CKW_W, CKW_H, Qt::KeepAspectRatio, Qt::FastTransformation));
    }
    setPixmap(spr_stop_L);
}

void cutKnight::updateEnemy() {
    if (!alive) return;

    wantsToShoot = false;
    if (targetX>x()){
        facingRight = true;
    }
    else facingRight = false;

    if (isAttacking) {
        animTimer++;
        if (animTimer >= 8) {
            animTimer = 0;
            animFrame++;
        }

        // 攻擊動畫
        if (animFrame >= 4) {
            isAttacking = false;
            wantsToShoot = true; // 通知 GameScene 生成火球
            animFrame = 0;
        }
        else{
            setPixmap(facingRight ? attackR[animFrame] : attackL[animFrame]);
        }
        return;
    }
    setPixmap(facingRight ? spr_stop_R : spr_stop_L);

    // 攻擊冷卻
    attackCooldown--;
    if (attackCooldown <= 0) {
        isAttacking = true;
        animFrame = 0;
        animTimer = 0;
        attackCooldown = 210;
    }

    // 動畫

}
void cutKnight::setTargetX(double kirbyX) {
    targetX = kirbyX;
}
bool cutKnight::shouldShootCutter() {
    bool result = wantsToShoot;
    wantsToShoot = false;
    return result;
}
WaddleDoo::WaddleDoo(double sx, double sy, double pMinX, double pMaxX)
    : Enemy(ENEMY_WADDLE_DOO, sx, sy, pMinX, pMaxX)
{
    canBeInhaled = true;
    canBeDamaged = true;
    grantedAbility = ABILITY_NONE;
    vx = ENEMY_SPEED;
    beaming = false;
    attackCooldown = 180;

    spr_stop_R = loadAndScale(":/Dataset/Waddle Doo/stop_R.png")
                 .scaled(WD_W, WD_H, Qt::KeepAspectRatio, Qt::FastTransformation);
    spr_stop_L = loadAndScale(":/Dataset/Waddle Doo/stop_L.png")
                 .scaled(WD_W, WD_H, Qt::KeepAspectRatio, Qt::FastTransformation);
    for (int i = 1; i <= 6; i++) {
        walkR.append(loadAndScale(QString(":/Dataset/Waddle Doo/run_R_%1.png").arg(i))
                     .scaled(WD_W, WD_H, Qt::KeepAspectRatio, Qt::FastTransformation));
        walkL.append(loadAndScale(QString(":/Dataset/Waddle Doo/run_L_%1.png").arg(i))
                     .scaled(WD_W, WD_H, Qt::KeepAspectRatio, Qt::FastTransformation));
    }
    for (int i = 1; i <= 3; i++) {
        attackR.append(loadAndScale(QString(":/Dataset/Waddle Doo/attack_R_%1.png").arg(i))
                       .scaled(WD_W, WD_H, Qt::KeepAspectRatio, Qt::FastTransformation));
        attackL.append(loadAndScale(QString(":/Dataset/Waddle Doo/attack_L_%1.png").arg(i))
                       .scaled(WD_W, WD_H, Qt::KeepAspectRatio, Qt::FastTransformation));
    }
    // 載入光束掃擊圖片（4 幀，左右各一組）

    for (int i = 1; i <= 4; i++) {
        beamR.append(loadAndScale(QString(":/Dataset/Waddle Doo/beams%1R.png").arg(i))
                     .scaled(BEAM_W, BEAM_H, Qt::KeepAspectRatio, Qt::FastTransformation));
        beamL.append(loadAndScale(QString(":/Dataset/Waddle Doo/beams%1.png").arg(i))
                     .scaled(BEAM_W, BEAM_H, Qt::KeepAspectRatio, Qt::FastTransformation));
    }
    // 建立單一光束特效子物件
    beamEffect = new QGraphicsPixmapItem(this);
    beamEffect->setVisible(false);
    beamEffect->setZValue(6);

    setPixmap(spr_stop_R);
}
void WaddleDoo::updateEnemy() {
    if (!alive) return;
    beaming = false;
    if (isAttacking) {
        attackTimer--;
        if (attackTimer > 30) {
            // === 階段 1：播攻擊動畫（45→31）===
            animTimer++;
            if (animTimer >= 4) {
                animTimer = 0;
                animFrame++;
                if (animFrame >= 3) animFrame = 2;
            }
            setPixmap(facingRight ? attackR[animFrame] : attackL[animFrame]);
            beamEffect->setVisible(false);
        } else {
            // === 階段 2：光束掃擊（30→0，用 beams1~4 播 4 幀）===
            beaming = true;
            setPixmap(facingRight ? attackR[2] : attackL[2]);

            // 根據剩餘時間決定播第幾張（30→0 分成 4 段）
            int frame;
            if (attackTimer > 22)      frame = 0;  // beams1
            else if (attackTimer > 15) frame = 1;  // beams2
            else if (attackTimer > 7)  frame = 2;  // beams3
            else                       frame = 3;  // beams4

            QPixmap beamPix = facingRight ? beamR[frame] : beamL[frame];
            beamEffect->setPixmap(beamPix);

            // 錨點：光束從本體的這個位置射出（相對於本體左上角）
            double anchorX = pixmap().width() * 0.5;   // 本體中心
            double anchorY = pixmap().height() * 0.3+50;   // 眼睛高度

            // 面朝右：光束往右上延伸 → 左下角對齊錨點
            // 面朝左：光束往左上延伸 → 右下角對齊錨點
            if (facingRight) {
                beamEffect->setPos(anchorX, anchorY - beamPix.height());
            } else {
                beamEffect->setPos(anchorX - beamPix.width(), anchorY - beamPix.height());
            }
            beamEffect->setVisible(true);
        }
        if (attackTimer <= 0) {
            isAttacking = false;
            vx = facingRight ? ENEMY_SPEED : -ENEMY_SPEED;
            beamEffect->setVisible(false);
        }
        return;
    }
    // 巡邏移動
    setPos(x() + vx, y());
    if (x() <= patrolMinX) { vx = ENEMY_SPEED; facingRight = true; }
    if (x() + pixmap().width() >= patrolMaxX) { vx = -ENEMY_SPEED; facingRight = false; }
    // 攻擊冷卻
    attackCooldown--;
    if (attackCooldown <= 0) {
        isAttacking = true;
        attackTimer = 45;
        attackCooldown = 180;
        vx = 0;
        animFrame = 0;
        animTimer = 0;
    }
    // 走路動畫
    animTimer++;
    if (animTimer >= 6) {
        animTimer = 0;
        animFrame = (animFrame + 1) % walkR.size();
    }
    setPixmap(facingRight ? walkR[animFrame] : walkL[animFrame]);
}
bool WaddleDoo::isBeaming() const {
    return beaming;
}
QRectF WaddleDoo::getBeamBox() const {
    if (!beaming || !beamEffect->isVisible()) return QRectF();
    // hitbox 就是光束圖片的範圍
    double bx = x() + beamEffect->pos().x();
    double by = y() + beamEffect->pos().y();
    double bw = beamEffect->pixmap().width();
    double bh = beamEffect->pixmap().height();
    return QRectF(bx, by, bw, bh);
}

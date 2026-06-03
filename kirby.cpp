#include "kirby.h"
#include <QDir>

Kirby::Kirby() {
    vx = 0; vy = 0;
    onGround = false;
    facingRight = true;
    state = KIRBY_NORMAL;
    ability = ABILITY_NONE;
    hp = KIRBY_MAX_HP;
    lives = KIRBY_MAX_LIVES;
    invincibleTimer = 0;
    isInvincible = false;
    attackTimer = 0;
    isAttacking = false;
    mouthful = false;
    swallowedEnemy = ENEMY_WADDLE_DEE;
    inhaling = false;
    animFrame = 0;
    animTimer = 0;

    loadSprites();
    setPixmap(spr_stand_R);
    setZValue(10); // Kirby 顯示在最上層

    upKeyReleased = true;
    xKeyReleased = true;
}

// ============ 載入並縮放圖片 ============
QPixmap Kirby::loadAndScale(const QString &path) {
    QPixmap pix(path);
    if (pix.isNull()) {
        // 如果載入失敗，建立一個粉色方塊作為替代
        pix = QPixmap(60, 60);
        pix.fill(Qt::magenta);
    }
    return pix.scaled(pix.width() * SPRITE_SCALE, pix.height() * SPRITE_SCALE,
                      Qt::KeepAspectRatio, Qt::FastTransformation);
}

// ============ 載入所有 Sprites ============
void Kirby::loadSprites() {
    QString base = ":/Dataset/Kirby_normal/";

    // Normal - 站立
    spr_stand_R = loadAndScale(base + "kirby_stop_R.png");
    spr_stand_L = loadAndScale(base + "kirby_stop_L.png");

    // Normal - 走路 (3 frames)
    for (int i = 1; i <= 3; i++) {
        spr_walk_R.append(loadAndScale(base + QString("kirby_run_%1_R.png").arg(i)));
        spr_walk_L.append(loadAndScale(base + QString("kirby_run_%1_L.png").arg(i)));
    }

    // Normal - 跳躍 (3 frames)
    for (int i = 1; i <= 3; i++) {
        spr_jump.append(loadAndScale(base + QString("kirby_jump(%1).png").arg(i)));
    }

    // Normal - 飛行
    spr_fly1_R = loadAndScale(base + "kirby_fly_1_R.png");
    spr_fly1_L = loadAndScale(base + "kirby_fly_1_L.png");
    spr_fly2_R = loadAndScale(base + "kirby_fly_2_R.png");
    spr_fly2_L = loadAndScale(base + "kirby_fly_2_L.png");

    // Normal - 蹲下
    spr_down_R = loadAndScale(base + "kirby_down_R.png");
    spr_down_L = loadAndScale(base + "kirby_down_L.png");

    // Normal - 吸入
    spr_inhale_R = loadAndScale(base + "kirby_attack_R.png");
    spr_inhale_L = loadAndScale(base + "kirby_attack_L.png");

    // Normal - 飽足 (Mouthful)
    spr_mouthful_R = loadAndScale(base + "kirby_attack_star_R(1).png");
    spr_mouthful_L = loadAndScale(base + "kirby_attack_star_L(1).png");

    // 星星彈
    spr_star = loadAndScale(base + "kirby_attack_star(2).png");

    // ====== Fire Ability Sprites ======
    QString fireBase = ":/Dataset/Kirby_fire/";
    spr_fire_stand_R = loadAndScale(fireBase + "kirbyfire_stop_R.png");
    spr_fire_stand_L = loadAndScale(fireBase + "kirbyfire_stop_L.png");

    for (int i = 1; i <= 3; i++) {
        spr_fire_walk_R.append(loadAndScale(fireBase + QString("kirbyfire_run(%1)_R.png").arg(i)));
        spr_fire_walk_L.append(loadAndScale(fireBase + QString("kirbyfire_run(%1)_L.png").arg(i)));
    }

    spr_fire_fly1_R = loadAndScale(fireBase + "kirbyfire_fly(1)_R.png");
    spr_fire_fly1_L = loadAndScale(fireBase + "kirbyfire_fly(1)_L.png");
    spr_fire_fly2_R = loadAndScale(fireBase + "kirbyfire_fly(2)_R.png");
    spr_fire_fly2_L = loadAndScale(fireBase + "kirbyfire_fly(2)_L.png");

    spr_fire_down_R = loadAndScale(fireBase + "kirbyfire_down_R.png");
    spr_fire_down_L = loadAndScale(fireBase + "kirbyfire_down_L.png");

    for (int i = 1; i <= 3; i++) {
        spr_fire_attack_R.append(loadAndScale(fireBase + QString("kirbyfire_fire(%1)_R.png").arg(i)));
        spr_fire_attack_L.append(loadAndScale(fireBase + QString("kirbyfire_fire(%1)_L.png").arg(i)));
    }

    // ====== Spark Ability Sprites ======
    QString sparkBase = ":/Dataset/Kirby_spark/";
    spr_spark_stand_R = loadAndScale(sparkBase + "Kirby_spark_stop_R.png");
    spr_spark_stand_L = loadAndScale(sparkBase + "Kirby_spark_stop_L.png");

    for (int i = 1; i <= 2; i++) {
        spr_spark_walk_R.append(loadAndScale(sparkBase + QString("Kirby_spark_run(%1)_R.png").arg(i)));
        spr_spark_walk_L.append(loadAndScale(sparkBase + QString("Kirby_spark_run(%1)_L.png").arg(i)));
    }

    spr_spark_fly1_R = loadAndScale(sparkBase + "Kirby_spark_fly(1)_R.png");
    spr_spark_fly1_L = loadAndScale(sparkBase + "Kirby_spark_fly(1)_L.png");
    spr_spark_fly2_R = loadAndScale(sparkBase + "Kirby_spark_fly(2)_R.png");
    spr_spark_fly2_L = loadAndScale(sparkBase + "Kirby_spark_fly(2)_L.png");

    spr_spark_down_R = loadAndScale(sparkBase + "Kirby_spark_down_R.png");
    spr_spark_down_L = loadAndScale(sparkBase + "Kirby_spark_down_L.png");

    for (int i = 1; i <= 3; i++) {
        spr_spark_attack.append(loadAndScale(sparkBase + QString("Kirby_spark_attack(%1).png").arg(i)));
    }
}

// ============ 主更新 (每幀呼叫) ============
void Kirby::update(const QSet<int> &keys) {
    if (state == KIRBY_DEAD) return;

    // 處理無敵計時
    if (isInvincible) {
        invincibleTimer--;
        if (invincibleTimer <= 0) {
            isInvincible = false;
            setOpacity(1.0);
        } else {
            // 閃爍效果
            setOpacity((invincibleTimer % 10 < 5) ? 0.3 : 1.0);
        }
    }

    // 處理攻擊計時
    if (isAttacking) {
        attackTimer--;
        if (attackTimer <= 0) {
            isAttacking = false;
            state = KIRBY_NORMAL;
        }
    }

    // 輸入處理（攻擊中不能移動）
    if (!isAttacking) {
        handleInput(keys);
    }

    applyPhysics();
    updateAnimation();
    updateSprite();
}

// ============ 輸入處理 ============
void Kirby::handleInput(const QSet<int> &keys) {
    vx = 0;

    // === Mouthful 狀態：只能吐出或吞下 ===
    if (state == KIRBY_MOUTHFUL) {
        // 左右移動（Mouthful 也能走）
        if (keys.contains(Qt::Key_Left)) { vx = -KIRBY_SPEED; facingRight = false; }
        if (keys.contains(Qt::Key_Right)) { vx = KIRBY_SPEED; facingRight = true; }

        // X = 吐出星星彈
        if (keys.contains(Qt::Key_X)) {
            if (xKeyReleased) {
                doSpit();
                xKeyReleased = false;
                return;
            }
        } else {
            xKeyReleased = true;
        }

        // Down = 吞下
        if (keys.contains(Qt::Key_Down)) {
            doSwallow();
            return;
        }
        return; // Mouthful 狀態不能跳躍或飛行
    }

    // === 有能力時按 X = 能力攻擊 ===
    if (ability != ABILITY_NONE && keys.contains(Qt::Key_X) && !inhaling) {
        startAbilityAttack();
        return;
    }

    // === V = 棄置能力 ===
    if (keys.contains(Qt::Key_V) && ability != ABILITY_NONE) {
        dropAbility();
    }

    // === 左右移動 ===
    if (keys.contains(Qt::Key_Left)) {
        vx = -KIRBY_SPEED;
        facingRight = false;
        if (onGround && state != KIRBY_SQUATTING) state = KIRBY_WALKING;
    }
    if (keys.contains(Qt::Key_Right)) {
        vx = KIRBY_SPEED;
        facingRight = true;
        if (onGround && state != KIRBY_SQUATTING) state = KIRBY_WALKING;
    }

    // 站立
    if (!keys.contains(Qt::Key_Left) && !keys.contains(Qt::Key_Right) && onGround
        && state != KIRBY_SQUATTING && state != KIRBY_INHALING) {
        state = KIRBY_NORMAL;
    }

    // === 蹲下 ===
    if (keys.contains(Qt::Key_Down) && onGround) {
        state = KIRBY_SQUATTING;
        vx = 0;
    }
    if (!keys.contains(Qt::Key_Down) && state == KIRBY_SQUATTING) {
        state = KIRBY_NORMAL;
    }

    // === 跳躍 / 飛行 ===
    if (keys.contains(Qt::Key_Up)) {
        // 只有「前一次已經放開按鍵」才能觸發跳躍或開始飛行（防連發）
        if (upKeyReleased) {
            if (onGround && state != KIRBY_HOVERING) {
                // 第一次按：從地面起跳
                vy = KIRBY_JUMP_FORCE;
                onGround = false;
                state = KIRBY_JUMPING;
                upKeyReleased = false; // 鎖上按鍵，直到玩家放開
            } else if (!onGround && state == KIRBY_JUMPING) {
                // 空中「再按一次」：進入飛行
                state = KIRBY_HOVERING;
                vy = KIRBY_HOVER_FORCE;
                upKeyReleased = false; // 鎖上按鍵
            } else if (state == KIRBY_HOVERING) {
                // (可選) 如果你想讓卡比像原作一樣「連按」才能越飛越高
                // 把給予向上推力的程式碼放在這裡，且一樣鎖上按鍵
                vy = KIRBY_HOVER_FORCE;
                upKeyReleased = false;
            }
        }
    } else {
        // 如果這個 Frame 電腦發現玩家沒有按 Up 鍵，就把鎖解開！
        // 這樣玩家下一次按 Up 鍵時，就能順利通過 `if (upKeyReleased)` 的檢查。
        upKeyReleased = true;
    }

    // === 飛行中按 X = 吐氣（解除飛行）===
    if (state == KIRBY_HOVERING && keys.contains(Qt::Key_X)) {
        state = KIRBY_INHALING; // 借用吸氣狀態來播放動畫
        inhaling = true;        // 標記為正在吸氣(吐氣)
        animFrame = 0;          // 動畫從頭播
        animTimer = 0;          // 計時器歸零
        return;
    }

    // === X = 吸入（普通狀態，沒有能力時）===
    if (keys.contains(Qt::Key_X) && ability == ABILITY_NONE
        && state != KIRBY_HOVERING && state != KIRBY_JUMPING) {
        if (!inhaling) {
            startInhale();
        }
    } else {
        if (inhaling) {
            stopInhale();
        }
    }

    // === 如果在空中且不是其他特殊狀態，強制進入跳躍(下落)狀態 ===
    if (!onGround && state != KIRBY_HOVERING && state != KIRBY_ATTACKING &&
            state != KIRBY_INHALING && state != KIRBY_MOUTHFUL) {
        state = KIRBY_JUMPING;
    }
}

// ============ 物理 ============
void Kirby::applyPhysics() {
    // 重力
    if (!onGround) {
        if (vy > 0) {
            // 下降階段：重力加速度變強（1.5 倍），呈現越掉越快的沉重下墜感
            vy += GRAVITY * 1.5;
        } else {
            // 上升階段：維持原本的重力
            vy += GRAVITY;
        }

        // 放寬最高掉落速度的限制，讓卡比有空間可以「加速」到更快的速度
        if (vy > MAX_FALL_SPEED * 1.8) {
            vy = MAX_FALL_SPEED * 1.8;
        }
    }

    // 更新位置
    setPos(x() + vx, y() + vy);

    // 不能跑出場景左邊界
    if (x() < 0) setX(0);
}

// ============ 動畫更新 ============
void Kirby::updateAnimation() {
    animTimer++;
    if (animTimer >= 8) {
        animTimer = 0;
        animFrame++;

        // 【新增】：如果是在空中播放吸氣(吐氣)動畫，停留 3 個影格的時間後強制掉落
        if (state == KIRBY_INHALING && !onGround && animFrame >= 3) {
            state = KIRBY_JUMPING; // 自動轉為跳躍(下落)狀態
            inhaling = false;      // 關閉吸氣標記
            animFrame = 0;         // 動畫歸零
        }
    }
}

// ============ 根據狀態更新顯示的 Sprite ============
void Kirby::updateSprite() {
    double oldHeight = pixmap().isNull() ? 0 : pixmap().height();
    QPixmap current;
    // 根據能力狀態選擇 sprite set
    if (ability == ABILITY_FIRE) {
        // === Fire Ability Sprites ===
        if (isAttacking) {
            int idx = animFrame % spr_fire_attack_R.size();
            current = facingRight ? spr_fire_attack_R[idx] : spr_fire_attack_L[idx];
        } else if (state == KIRBY_HOVERING) {
            current = (animFrame % 2 == 0)
                ? (facingRight ? spr_fire_fly1_R : spr_fire_fly1_L)
                : (facingRight ? spr_fire_fly2_R : spr_fire_fly2_L);
        } else if (state == KIRBY_SQUATTING) {
            current = facingRight ? spr_fire_down_R : spr_fire_down_L;
        } else if (state == KIRBY_WALKING) {
            int idx = animFrame % spr_fire_walk_R.size();
            current = facingRight ? spr_fire_walk_R[idx] : spr_fire_walk_L[idx];
        } else {
            current = facingRight ? spr_fire_stand_R : spr_fire_stand_L;
        }
    } else if (ability == ABILITY_SPARK) {
        // === Spark Ability Sprites ===
        if (isAttacking) {
            int idx = animFrame % spr_spark_attack.size();
            current = spr_spark_attack[idx];
        } else if (state == KIRBY_HOVERING) {
            current = (animFrame % 2 == 0)
                ? (facingRight ? spr_spark_fly1_R : spr_spark_fly1_L)
                : (facingRight ? spr_spark_fly2_R : spr_spark_fly2_L);
        } else if (state == KIRBY_SQUATTING) {
            current = facingRight ? spr_spark_down_R : spr_spark_down_L;
        } else if (state == KIRBY_WALKING) {
            int idx = animFrame % spr_spark_walk_R.size();
            current = facingRight ? spr_spark_walk_R[idx] : spr_spark_walk_L[idx];
        } else {
            current = facingRight ? spr_spark_stand_R : spr_spark_stand_L;
        }
    } else {
        // === Normal Sprites ===
        if (state == KIRBY_MOUTHFUL) {
            current = facingRight ? spr_mouthful_R : spr_mouthful_L;
        } else if (state == KIRBY_INHALING) {
            current = facingRight ? spr_inhale_R : spr_inhale_L;
        } else if (state == KIRBY_HOVERING) {
            current = (animFrame % 2 == 0)
                ? (facingRight ? spr_fly1_R : spr_fly1_L)
                : (facingRight ? spr_fly2_R : spr_fly2_L);
        } else if (state == KIRBY_JUMPING) {
            // 根據垂直速度 (vy) 來決定播放哪一個跳躍/下落影格，呈現真實物理的下墜過渡效果
            int idx = 0;
            if (vy < -2.0) {
                idx = 0; // 上升中
            } else if (vy >= -2.0 && vy <= 2.0) {
                idx = 1; // 頂點滯空
            } else {
                idx = 2; // 下墜中
            }
            if (idx >= spr_jump.size()) idx = spr_jump.size() - 1;
            current = spr_jump[idx];
        } else if (state == KIRBY_SQUATTING) {
            current = facingRight ? spr_down_R : spr_down_L;
        } else if (state == KIRBY_WALKING) {
            int idx = animFrame % spr_walk_R.size();
            current = facingRight ? spr_walk_R[idx] : spr_walk_L[idx];
        } else {
            current = facingRight ? spr_stand_R : spr_stand_L;
        }
    }

    setPixmap(current);
    double newHeight = pixmap().height();
    if (oldHeight > 0 && newHeight != oldHeight) {
        setY(y() + (oldHeight - newHeight));
    }
}

// ============ 吸入 ============
void Kirby::startInhale() {
    inhaling = true;
    state = KIRBY_INHALING;
}

void Kirby::stopInhale() {
    inhaling = false;
    if (state == KIRBY_INHALING) {
        if (onGround) {
            state = KIRBY_NORMAL;  // 平地放開變回站立
        } else {
            state = KIRBY_JUMPING; // 在空中放開直接掉落
        }
    }
}

// ============ 吸入敵人後進入飽足 ============
void Kirby::swallowEnemy(EnemyType type) {
    mouthful = true;
    swallowedEnemy = type;
    inhaling = false;
    state = KIRBY_MOUTHFUL;
    xKeyReleased = false; // 強制鎖死 X 鍵，必須放開才能再次吐出
}

// ============ 吞下（按 Down）============
void Kirby::doSwallow() {
    if (!mouthful) return;

    // 判斷吞下的敵人是否有複製能力
    if (swallowedEnemy == ENEMY_HOT_HEAD) {
        ability = ABILITY_FIRE;
    } else if (swallowedEnemy == ENEMY_SPARKY) {
        ability = ABILITY_SPARK;
    }
    // Waddle Dee 等無能力敵人 → 回 Normal

    mouthful = false;
    state = KIRBY_NORMAL;
}

// ============ 吐出星星彈（按 X）============
void Kirby::doSpit() {
    // 星星彈的建立由 GameScene 處理
    // 這裡只負責狀態轉換
    mouthful = false;
    state = KIRBY_NORMAL;
}

// ============ 能力攻擊 ============
void Kirby::startAbilityAttack() {
    isAttacking = true;
    state = KIRBY_ATTACKING;
    vx = 0; // 攻擊中不能移動

    if (ability == ABILITY_FIRE) {
        attackTimer = FIRE_DURATION;
    } else if (ability == ABILITY_SPARK) {
        attackTimer = SPARK_DURATION;
    }
    animFrame = 0;
}

// ============ 棄置能力 ============
void Kirby::dropAbility() {
    ability = ABILITY_NONE;
    state = KIRBY_NORMAL;
    isAttacking = false;
}

// ============ 受傷 ============
void Kirby::takeDamage() {
    if (!canBeHurt()) return;

    hp--;
    isInvincible = true;
    invincibleTimer = INVINCIBLE_DURATION;

    // 受傷時失去能力
    if (ability != ABILITY_NONE) {
        dropAbility();
    }
    // 飽足狀態也會解除
    if (mouthful) {
        mouthful = false;
        state = KIRBY_NORMAL;
    }

    if (hp <= 0) {
        die();
    }
}

// ============ 死亡 ============
void Kirby::die() {
    state = KIRBY_DEAD;
    lives--;
}

// ============ 重置 ============
void Kirby::reset(double rx, double ry, bool fullReset) {
    setPos(rx, ry);
    vx = 0; vy = 0;
    onGround = false;
    facingRight = true;
    state = KIRBY_NORMAL;
    if (fullReset) {
        ability = ABILITY_NONE;
        hp = KIRBY_MAX_HP;
    }
    isInvincible = false;
    invincibleTimer = 0;
    isAttacking = false;
    attackTimer = 0;
    mouthful = false;
    inhaling = false;
    animFrame = 0;
    animTimer = 0;
    setOpacity(1.0);
}

// ============ 碰撞框 ============
QRectF Kirby::getHitbox() const {
    // 略小於 sprite 以提升手感
    double w = pixmap().width() * 0.7;
    double h = pixmap().height() * 0.8;
    double offX = (pixmap().width() - w) / 2.0;
    double offY = pixmap().height() - h;
    return QRectF(x() + offX, y() + offY, w, h);
}

QRectF Kirby::getInhaleBox() const {
    double inhaleW = INHALE_RANGE;
    double inhaleH = pixmap().height() * 0.6;
    double iy = y() + pixmap().height() * 0.2;
    if (facingRight) {
        return QRectF(x() + pixmap().width(), iy, inhaleW, inhaleH);
    } else {
        return QRectF(x() - inhaleW, iy, inhaleW, inhaleH);
    }
}

QRectF Kirby::getAttackBox() const {
    if (ability == ABILITY_FIRE) {
        // Fire: 前方噴火範圍
        double aw = 200;
        double ah = pixmap().height() * 0.6;
        double ay = y() + pixmap().height() * 0.2;
        if (facingRight) {
            return QRectF(x() + pixmap().width(), ay, aw, ah);
        } else {
            return QRectF(x() - aw, ay, aw, ah);
        }
    } else if (ability == ABILITY_SPARK) {
        // Spark: 周圍電流
        double expand = 80;
        return QRectF(x() - expand, y() - expand,
                      pixmap().width() + expand * 2,
                      pixmap().height() + expand * 2);
    }
    return QRectF();
}

bool Kirby::canBeHurt() const {
    return !isInvincible && state != KIRBY_DEAD;
}

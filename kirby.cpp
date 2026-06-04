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
    fireEffect = new QGraphicsPixmapItem(this);
    fireEffect->setVisible(false);
    beaming = false;
    sweepStep = 0;
    for (int i = 0; i < 3; i++) {
        beams[i] = new QGraphicsPixmapItem(this);
        beams[i]->setVisible(false);
    }

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
    spr_mouthful_R = loadAndScale(base + "Kirby_mouth_L.png");
    spr_mouthful_R = spr_mouthful_R.scaled(spr_mouthful_R.size() * 0.5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    spr_mouthful_L = loadAndScale(base + "Kirby_mouth_R.png");
    spr_mouthful_L = spr_mouthful_L.scaled(spr_mouthful_L.size() * 0.5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // 星星彈
    spr_star = loadAndScale(base + "kirby_attack_star(2).png");
    QSize targetSizespr_stand_R = spr_stand_R.size();
    QSize targetSizespr_walk_R = spr_walk_R[1].size();
    QSize targetSizespr_fly1_R = spr_fly1_R.size();
    QSize targetSizespr_down_R = spr_down_R.size();
    // ====== Fire Ability Sprites ======
    QString fireBase = ":/Dataset/Kirby_fire/";
    spr_fire_stand_R = loadAndScale(fireBase + "kirbyfire_stop_R.png").scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_fire_stand_L = loadAndScale(fireBase + "kirbyfire_stop_L.png").scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    for (int i = 1; i <= 3; i++) {
        spr_fire_walk_R.append(loadAndScale(fireBase + QString("kirbyfire_run(%1)_R.png").arg(i)).scaled(targetSizespr_walk_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
        spr_fire_walk_L.append(loadAndScale(fireBase + QString("kirbyfire_run(%1)_L.png").arg(i)).scaled(targetSizespr_walk_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    }

    spr_fire_fly1_R = loadAndScale(fireBase + "kirbyfire_fly(1)_R.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_fire_fly1_L = loadAndScale(fireBase + "kirbyfire_fly(1)_L.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_fire_fly2_R = loadAndScale(fireBase + "kirbyfire_fly(2)_R.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_fire_fly2_L = loadAndScale(fireBase + "kirbyfire_fly(2)_L.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    spr_fire_down_R = loadAndScale(fireBase + "kirbyfire_down_R.png").scaled(targetSizespr_down_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_fire_down_L = loadAndScale(fireBase + "kirbyfire_down_L.png").scaled(targetSizespr_down_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_fire_attack_R = loadAndScale(fireBase + "kirbyfire_attack_R.png").scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_fire_attack_L = loadAndScale(fireBase + "kirbyfire_attack_L.png").scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    for (int i = 1; i <= 3; i++) {
        spr_fire_fire_R.append(loadAndScale(fireBase + QString("kirbyfire_fire(%1)_R.png").arg(i)).scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
        spr_fire_fire_L.append(loadAndScale(fireBase + QString("kirbyfire_fire(%1)_L.png").arg(i)).scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    }

    // ====== Spark Ability Sprites ======
    QString sparkBase = ":/Dataset/Kirby_spark/";
    spr_spark_stand_R = loadAndScale(sparkBase + "Kirby_spark_stop_R.png").scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_spark_stand_L = loadAndScale(sparkBase + "Kirby_spark_stop_L.png").scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    for (int i = 1; i <= 2; i++) {
        spr_spark_walk_R.append(loadAndScale(sparkBase + QString("Kirby_spark_run(%1)_R.png").arg(i)).scaled(targetSizespr_walk_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
        spr_spark_walk_L.append(loadAndScale(sparkBase + QString("Kirby_spark_run(%1)_L.png").arg(i)).scaled(targetSizespr_walk_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    }

    spr_spark_fly1_R = loadAndScale(sparkBase + "Kirby_spark_fly(1)_R.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_spark_fly1_L = loadAndScale(sparkBase + "Kirby_spark_fly(1)_L.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_spark_fly2_R = loadAndScale(sparkBase + "Kirby_spark_fly(2)_R.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_spark_fly2_L = loadAndScale(sparkBase + "Kirby_spark_fly(2)_L.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    spr_spark_down_R = loadAndScale(sparkBase + "Kirby_spark_down_R.png").scaled(targetSizespr_down_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_spark_down_L = loadAndScale(sparkBase + "Kirby_spark_down_L.png").scaled(targetSizespr_down_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    for (int i = 1; i <= 3; i++) {
        spr_spark_attack.append(loadAndScale(sparkBase + QString("Kirby_spark_attack(%1).png").arg(i)).scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    }
    // ====== Cutter Ability Sprites ======

    QString cutterBase = ":/Dataset/Kirby_cutter/";
    spr_cutter_stand_R = loadAndScale(cutterBase + "Kirby_cutter_stop_R.png").scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_cutter_stand_L = loadAndScale(cutterBase + "Kirby_cutter_stop_L.png").scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    for (int i = 1; i <= 4; i++) {
        spr_cutter_walk_R.append(loadAndScale(cutterBase + QString("Kirby_cutter_run(%1)_R.png").arg(i)).scaled(targetSizespr_walk_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
        spr_cutter_walk_L.append(loadAndScale(cutterBase + QString("Kirby_cutter_run(%1)_L.png").arg(i)).scaled(targetSizespr_walk_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    }

    spr_cutter_fly1_R = loadAndScale(cutterBase + "Kirby_cutter_fly(1)_R.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_cutter_fly1_L = loadAndScale(cutterBase + "Kirby_cutter_fly(1)_L.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_cutter_fly2_R = loadAndScale(cutterBase + "Kirby_cutter_fly(2)_R.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_cutter_fly2_L = loadAndScale(cutterBase + "Kirby_cutter_fly(2)_L.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    spr_cutter_down_R = loadAndScale(cutterBase + "Kirby_cutter_down_R.png").scaled(targetSizespr_down_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_cutter_down_L = loadAndScale(cutterBase + "Kirby_cutter_down_L.png").scaled(targetSizespr_down_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    for (int i = 1; i <= 2; i++) {
        spr_cutter_attack_R.append(loadAndScale(cutterBase + QString("Kirby_cutter_attack(%1)_R.png").arg(i)).scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
        spr_cutter_attack_L.append(loadAndScale(cutterBase + QString("Kirby_cutter_attack(%1)_L.png").arg(i)).scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    }
    // ====== Doo Ability Sprites ======
    QString dooBase = ":/Dataset/Kirby_beam/";
    spr_doo_stand_R = loadAndScale(dooBase + "Kirby_doo_stop_R.png").scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_doo_stand_L = loadAndScale(dooBase + "Kirby_doo_stop_L.png").scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    for (int i = 1; i <= 4; i++) {
        spr_doo_walk_R.append(loadAndScale(dooBase + QString("Kirby_doo_run(%1)_R.png").arg(i)).scaled(targetSizespr_walk_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
        spr_doo_walk_L.append(loadAndScale(dooBase + QString("Kirby_doo_run(%1)_L.png").arg(i)).scaled(targetSizespr_walk_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    }

    spr_doo_fly1_R = loadAndScale(dooBase + "Kirby_doo_fly(1)_R.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_doo_fly1_L = loadAndScale(dooBase + "Kirby_doo_fly(1)_L.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_doo_fly2_R = loadAndScale(dooBase + "Kirby_doo_fly(2)_R.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_doo_fly2_L = loadAndScale(dooBase + "Kirby_doo_fly(2)_L.png").scaled(targetSizespr_fly1_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    spr_doo_down_R = loadAndScale(dooBase + "Kirby_doo_down_R.png").scaled(targetSizespr_down_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    spr_doo_down_L = loadAndScale(dooBase + "Kirby_doo_down_L.png").scaled(targetSizespr_down_R, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    for (int i = 1; i <= 2; i++) {
        spr_doo_attack_R.append(loadAndScale(dooBase + QString("Kirby_doo_attack(%1)_R.png").arg(i)).scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
        spr_doo_attack_L.append(loadAndScale(dooBase + QString("Kirby_doo_attack(%1)_L.png").arg(i)).scaled(targetSizespr_stand_R, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    }
    spr_doo_beam = loadAndScale(dooBase + "beam.png").scaled(targetSizespr_walk_R*0.25, Qt::IgnoreAspectRatio, Qt::FastTransformation);

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
        animTimer++;
        if (attackTimer <= 0) {
            isAttacking = false;
            state = KIRBY_NORMAL;
            fireEffect->setVisible(false);
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
        if (state == KIRBY_HOVERING) {
            // 飛行中：按住就持續給向上推力
            vy = KIRBY_HOVER_FORCE;
        } else if (upKeyReleased) {
            // 只有「前一次已經放開按鍵」才能觸發跳躍或開始飛行（防連發）
            if (onGround && state != KIRBY_HOVERING) {
                // 第一次按：從地面起跳
                vy = KIRBY_JUMP_FORCE;
                onGround = false;
                state = KIRBY_JUMPING;
                upKeyReleased = false;
            } else if (!onGround && state == KIRBY_JUMPING) {
                // 空中「再按一次」：進入飛行
                state = KIRBY_HOVERING;
                vy = KIRBY_HOVER_FORCE;
                upKeyReleased = false;
            }
        }
    } else {
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
    // === 確保非 Beam 攻擊狀態時隱藏光束 ===
    if (!isAttacking || ability != ABILITY_BEAM) {
        for (int i = 0; i < 3; i++) {
            if (beams[i]) beams[i]->setVisible(false);
        }
        beaming = false;
    }
    // 根據能力狀態選擇 sprite set
    if (ability == ABILITY_FIRE) {
        // === Fire Ability Sprites ===
        if (isAttacking) {
            // 1. 卡比本身的動作：保持噴火姿勢
            current = facingRight ? spr_fire_attack_R : spr_fire_attack_L;

            // 2. 火焰特效動畫處理 (套用 HotHead 的 6 幀切換邏輯)
            // spr_fire_fire_R 有 3 張圖片[cite: 1]，利用 animTimer 來控制切換速度
            int fireFrameIdx = (animTimer / 6) % spr_fire_fire_R.size();
            fireEffect->setPixmap(facingRight ? spr_fire_fire_R[fireFrameIdx] : spr_fire_fire_L[fireFrameIdx]);

            // 3. 火焰特效位置校正
            if (facingRight) {
                // 往右噴：特效放在卡比圖片右側
                fireEffect->setPos(pixmap().width(), 0);
            } else {
                // 往左噴：特效放在卡比圖片左側
                fireEffect->setPos(-fireEffect->pixmap().width(), 0);
            }
            fireEffect->setVisible(true);

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
    } else if(ability == ABILITY_CUTTER){
        // === CUTTER Ability Sprites ===
        if (isAttacking) {
            int idx = animFrame % spr_cutter_attack_R.size();
            current = facingRight ? spr_cutter_attack_R[idx] : spr_cutter_attack_L[idx];
        } else if (state == KIRBY_HOVERING) {
            current = (animFrame % 2 == 0)
                    ? (facingRight ? spr_cutter_fly1_R : spr_cutter_fly1_L)
                    : (facingRight ? spr_cutter_fly2_R : spr_cutter_fly2_L);
        } else if (state == KIRBY_SQUATTING) {
            current = facingRight ? spr_cutter_down_R : spr_cutter_down_L;
        } else if (state == KIRBY_WALKING) {
            int idx = animFrame % spr_cutter_walk_R.size();
            current = facingRight ? spr_cutter_walk_R[idx] : spr_cutter_walk_L[idx];
        } else {
            current = facingRight ? spr_cutter_stand_R : spr_cutter_stand_L;
        }
    } else if(ability == ABILITY_BEAM){
        // === BEAM Ability Sprites ===
        if (isAttacking) {
            if (attackTimer > 30) {
                // === 階段 1：播攻擊動畫（40→31，共 10 幀）===
                // 每 4 幀切換一張圖，最多到第 3 張圖 (index 2)
                int frame = (animTimer / 5);
                if (frame > 1) frame = 1;
                current = facingRight ? spr_doo_attack_R[frame] : spr_doo_attack_L[frame];

                for (int i = 0; i < 3; i++) beams[i]->setVisible(false);
                beaming = false;
            } else {
                // === 階段 2：光束掃擊（30→0）===
                beaming = true;
                current = facingRight ? spr_doo_attack_R[1] : spr_doo_attack_L[1];
                QPixmap beamPix = spr_doo_beam;

                // 計算掃擊階段
                if (attackTimer > 28)      sweepStep = 0;  // 正上方
                else if (attackTimer > 21) sweepStep = 1;  // 斜上方
                else if (attackTimer > 14) sweepStep = 2;  // 斜前方
                else if (attackTimer > 7)  sweepStep = 3;  // 正前方
                else                       sweepStep = 4;  // 斜下方 45°

                // 取得卡比中心點與光束尺寸，以便排版
                double cx = pixmap().width() / 2.0;
                double bw = beamPix.width();
                double bh = beamPix.height();
                double dir = facingRight ? 1.0 : -1.0;

                // WaddleDoo 的偏移量矩陣
                double offsets[5][3][2] = {
                    { {cx, -3*bh+20}, {cx, -2*bh+20}, {cx, -1*bh+20} },
                    { {cx + dir*bw*0.8, -2.5*bh+20}, {cx + dir*bw*0.4, -1.7*bh+20}, {cx, -0.8*bh+20} },
                    { {cx + dir*bw*1.5, -1.5*bh+20}, {cx + dir*bw*0.8, -0.8*bh+20}, {cx + dir*bw*0.2, -0.1*bh+20} },
                    { {cx + dir*bw*2.2, 0+20}, {cx + dir*bw*1.2, 0+20}, {cx + dir*bw*0.2, 0+20} },
                    { {cx + dir*bw*1.5, 1.5*bh+20}, {cx + dir*bw*0.8, 0.8*bh+20}, {cx + dir*bw*0.2, 0.1*bh+20} }
                };

                // 設定光束位置與顯示
                for (int i = 0; i < 3; i++) {
                    beams[i]->setPixmap(beamPix);
                    double bx = offsets[sweepStep][i][0];
                    double by = offsets[sweepStep][i][1];
                    if (!facingRight) {
                        bx = bx - bw * 0.5; // 面朝左的校正
                    }
                    beams[i]->setPos(bx, by);
                    beams[i]->setVisible(true);
                }
            }
        } else if (state == KIRBY_HOVERING) {
            current = (animFrame % 2 == 0)
                    ? (facingRight ? spr_doo_fly1_R : spr_doo_fly1_L)
                    : (facingRight ? spr_doo_fly2_R : spr_doo_fly2_L);
        } else if (state == KIRBY_SQUATTING) {
            current = facingRight ? spr_doo_down_R : spr_doo_down_L;
        } else if (state == KIRBY_WALKING) {
            int idx = animFrame % spr_doo_walk_R.size();
            current = facingRight ? spr_doo_walk_R[idx] : spr_doo_walk_L[idx];
        } else {
            current = facingRight ? spr_doo_stand_R : spr_doo_stand_L;
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
    } else if (swallowedEnemy == ENEMY_KNIGHT) {
        ability = ABILITY_CUTTER;
    } else if(swallowedEnemy == ENEMY_WADDLE_DOO){
        ability = ABILITY_BEAM;
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
    } else if (ability == ABILITY_CUTTER) {
        attackTimer = CUTTER_DURATION;
    }else if (ability == ABILITY_BEAM) {
        attackTimer = 40; // 配合 WaddleDoo 的 40 幀設定
        animFrame = 0;
        animTimer = 0;
        beaming = false;

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
    } else if (ability == ABILITY_CUTTER) {
        // Cutter: 前方的彎刀揮砍或飛行範圍
        double aw = 150; // 距離稍短於火焰 (假設為近戰或剛擲出的範圍)
        double ah = pixmap().height() * 0.4; // 高度較扁，符合刀刃形狀
        double ay = y() + pixmap().height() * 0.3; // 位於卡比的中間偏下位置
        if (facingRight) {
            return QRectF(x() + pixmap().width(), ay, aw, ah);
        } else {
            return QRectF(x() - aw, ay, aw, ah);
        }
    } else if (ability == ABILITY_BEAM) {
        // === 階段 1：如果還在舉起手（未發射光束），沒有攻擊判定 ===
        if (!beaming) {
            return QRectF();
        }

        // === 階段 2：根據光束的揮動階段 (sweepStep) 動態產生碰撞箱 ===
        double kw = pixmap().width();
        double kh = pixmap().height();
        double bx = x();
        double by = y();
        double aw = kw * 1.5; // 預設碰撞箱寬度
        double ah = kh * 1.5; // 預設碰撞箱高度

        // 依照 0~4 的階段，調整碰撞箱的位置與大小
        switch (sweepStep) {
        case 0: // 正上方 (剛揮出)
            bx = x() + (facingRight ? 0 : -kw * 0.5);
            by = y() - kh * 1.2;
            aw = kw * 1.5;
            ah = kh * 1.5;
            break;
        case 1: // 斜上方
            bx = x() + (facingRight ? kw * 0.5 : -kw * 1.5);
            by = y() - kh * 0.8;
            aw = kw * 1.5;
            ah = kh * 1.5;
            break;
        case 2: // 斜前方
            bx = x() + (facingRight ? kw * 0.8 : -kw * 1.8);
            by = y() - kh * 0.2;
            aw = kw * 1.8;
            ah = kh * 1.2;
            break;
        case 3: // 正前方 (最長)
            bx = x() + (facingRight ? kw : -kw * 2.2);
            by = y() + kh * 0.2;
            aw = kw * 2.2;
            ah = kh * 0.8; // 正前方時，碰撞箱較扁長
            break;
        case 4: // 斜下方 45° (收尾)
            bx = x() + (facingRight ? kw * 0.5 : -kw * 1.5);
            by = y() + kh * 0.5;
            aw = kw * 1.8;
            ah = kh * 1.2;
            break;
        default:
            return QRectF();
        }
        return QRectF(bx, by, aw, ah);
    }
    return QRectF();
}

bool Kirby::canBeHurt() const {
    return !isInvincible && state != KIRBY_DEAD;
}

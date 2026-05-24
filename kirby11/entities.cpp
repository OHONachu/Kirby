#include "entities.h"
#include "physics.h"
#include <cmath>
#include <QDebug>

// Helper to load and mirror a pixmap if direction is LEFT
QPixmap getEntityPixmap(const QString& path, Direction dir) {
    QPixmap pixmap(path);
    if (pixmap.isNull()) {
        // Fallback color block if resource path fails
        QPixmap fallback(80, 80);
        fallback.fill(Qt::magenta);
        return fallback;
    }
    if (dir == LEFT) {
        return pixmap.transformed(QTransform().scale(-1, 1));
    }
    return pixmap;
}

// ==========================================
// KIRBY IMPLEMENTATION
// ==========================================

Kirby::Kirby(double x, double y)
    : Entity(x, y, 70, 70) {
    state = K_NORMAL;
    action = A_IDLE;
    hp = 3;
    lives = 3;
    invincibleTicks = 0;
    animFrame = 0;
    animTick = 0;
    isGrounded = false;
    attackTicks = 0;
    isAttacking = false;
    hasSuckedEnemy = false;
    suckedEnemyType = E_WADDLE_DEE;

    // Load HUD boards
    normalBoardPixmap.load(":/Image/item/life.png");
    fireBoardPixmap.load(":/Image/Kirby_fire/kirbyfire_board.png");
    sparkBoardPixmap.load(":/Image/Kirby_spark/Kirby_spark_board.png");
}

void Kirby::handleDamage() {
    if (invincibleTicks <= 0) {
        hp--;
        invincibleTicks = 120; // 2 seconds of invincibility at 60 FPS
        if (hp < 0) hp = 0;
    }
}

void Kirby::startInhale() {
    if (state == K_NORMAL) {
        state = K_INHALING;
        action = A_ATTACKING;
        velocity.setX(0);
    }
}

void Kirby::stopInhale() {
    if (state == K_INHALING) {
        state = K_NORMAL;
        action = A_IDLE;
    }
}

QRectF Kirby::getInhaleRange() const {
    double rangeWidth = 80;
    double rangeHeight = 80;
    if (dir == RIGHT) {
        return QRectF(rect.right(), rect.top() - 5, rangeWidth, rangeHeight);
    } else {
        return QRectF(rect.left() - rangeWidth, rect.top() - 5, rangeWidth, rangeHeight);
    }
}

void Kirby::triggerAction(const QMap<int, bool>& keys) {
    // If in hit stun or attacking/inhaling, some actions are locked
    if (state == K_INHALING) {
        if (!keys.value(Qt::Key_X)) {
            stopInhale();
        }
        return;
    }

    if (action == A_ATTACKING && isAttacking) {
        return; // Complete current attack
    }

    // Horizontal Movement
    double moveX = 0;
    double speed = (state == K_MOUTHFUL) ? 4.0 : 8.0; // Slower when full

    if (keys.value(Qt::Key_Left)) {
        moveX = -speed;
        dir = LEFT;
        if (isGrounded) action = A_RUNNING;
    } else if (keys.value(Qt::Key_Right)) {
        moveX = speed;
        dir = RIGHT;
        if (isGrounded) action = A_RUNNING;
    } else {
        if (isGrounded) action = A_IDLE;
    }
    velocity.setX(moveX);

    // Jump / Fly logic (Up key)
    if (keys.value(Qt::Key_Up)) {
        if (isGrounded && state != K_MOUTHFUL) {
            velocity.setY(-21);
            isGrounded = false;
            action = A_JUMPING;
        } else if (!isGrounded && state != K_MOUTHFUL && velocity.y() > -5) {
            // Initiate/Maintain Hover
            action = A_HOVERING;
            velocity.setY(-7.5); // Upward float push
        }
    }

    // Squat / Swallow (Down key)
    if (keys.value(Qt::Key_Down)) {
        if (isGrounded) {
            action = A_SQUATTING;
            if (state == K_MOUTHFUL) {
                // Swallow!
                if (hasSuckedEnemy) {
                    if (suckedEnemyType == E_HOT_HEAD) {
                        state = K_FIRE;
                    } else if (suckedEnemyType == E_SPARKY) {
                        state = K_SPARK;
                    } else {
                        state = K_NORMAL;
                    }
                } else {
                    state = K_NORMAL;
                }
                hasSuckedEnemy = false;
                action = A_IDLE;
            }
        }
    }

    // Drop Ability (V key)
    if (keys.value(Qt::Key_V)) {
        if (state == K_FIRE || state == K_SPARK) {
            state = K_NORMAL;
            action = A_IDLE;
        }
    }
}

void Kirby::updateAnimation() {
    animTick++;
    if (animTick >= 6) {
        animTick = 0;
        animFrame++;
    }

    // Handle frame bounds based on states
    if (action == A_RUNNING) {
        if (state == K_FIRE) {
            animFrame %= 3;
        } else if (state == K_SPARK) {
            animFrame %= 2;
        } else {
            animFrame %= 4;
        }
    } else if (action == A_HOVERING) {
        animFrame %= 2;
    } else if (state == K_SPARK && action == A_ATTACKING) {
        animFrame %= 3;
    } else {
        animFrame = 0;
    }
}

void Kirby::update(const QList<QRect>& solids, const QList<QRect>& platforms) {
    // 1. Gravity and Hover limits
    if (action == A_HOVERING) {
        velocity.setY(velocity.y() + 0.3); // Gentle gravity when floating
        if (velocity.y() > 3.0) velocity.setY(3.0);
    } else {
        velocity.setY(velocity.y() + 1.2); // Normal fall gravity
        if (velocity.y() > 24.0) velocity.setY(24.0);
    }

    // 2. Physics Movement Resolution
    CollisionResult col = Physics::resolveMovement(rect, velocity, solids, platforms, action == A_HOVERING);
    
    isGrounded = col.onGround;
    if (isGrounded) {
        if (action == A_FALLING || action == A_JUMPING) {
            action = A_IDLE;
        }
    } else {
        if (action != A_HOVERING && action != A_JUMPING && action != A_ATTACKING) {
            action = A_FALLING;
        }
    }

    // Blinking invincibility ticks
    if (invincibleTicks > 0) {
        invincibleTicks--;
    }

    // Handle attack active frame ticks
    if (action == A_ATTACKING && isAttacking) {
        attackTicks--;
        if (attackTicks <= 0) {
            isAttacking = false;
            action = A_IDLE;
        }
    }

    updateAnimation();
}

QPixmap Kirby::getSprite() {
    QString path;
    switch (state) {
        case K_NORMAL:
            if (action == A_RUNNING) {
                path = QString(":/Image/Kirby_normal/kirby_run_%1_R.png").arg(animFrame + 1);
            } else if (action == A_JUMPING) {
                path = ":/Image/Kirby_normal/kirby_jump(1).png";
            } else if (action == A_FALLING) {
                path = ":/Image/Kirby_normal/kirby_jump(3).png";
            } else if (action == A_HOVERING) {
                path = QString(":/Image/Kirby_normal/kirby_fly_%1_R.png").arg(animFrame + 1);
            } else if (action == A_SQUATTING) {
                path = ":/Image/Kirby_normal/kirby_down_R.png";
            } else if (action == A_ATTACKING) {
                path = ":/Image/Kirby_normal/kirby_attack_R.png";
            } else {
                path = ":/Image/Kirby_normal/kirby_stop_R.png";
            }
            break;

        case K_MOUTHFUL:
            if (action == A_SQUATTING) {
                path = ":/Image/Kirby_normal/kirby_down_R.png"; // Squats to swallow
            } else {
                path = ":/Image/Kirby_normal/kirby_attack_star_R(1).png";
            }
            break;

        case K_FIRE:
            if (action == A_RUNNING) {
                path = QString(":/Image/Kirby_fire/kirbyfire_run(%1)_R.png").arg(animFrame + 1);
            } else if (action == A_HOVERING || action == A_JUMPING || action == A_FALLING) {
                path = QString(":/Image/Kirby_fire/kirbyfire_fly(%1)_R.png").arg(animFrame % 2 + 1);
            } else if (action == A_SQUATTING) {
                path = ":/Image/Kirby_fire/kirbyfire_down_R.png";
            } else if (action == A_ATTACKING) {
                path = ":/Image/Kirby_fire/kirbyfire_attack_R.png";
            } else {
                path = ":/Image/Kirby_fire/kirbyfire_stop_R.png";
            }
            break;

        case K_SPARK:
            if (action == A_RUNNING) {
                path = QString(":/Image/Kirby_spark/Kirby_spark_run(%1)_R.png").arg(animFrame + 1);
            } else if (action == A_HOVERING || action == A_JUMPING || action == A_FALLING) {
                path = QString(":/Image/Kirby_spark/Kirby_spark_fly(%1)_R.png").arg(animFrame % 2 + 1);
            } else if (action == A_SQUATTING) {
                path = ":/Image/Kirby_spark/Kirby_spark_down_R.png";
            } else if (action == A_ATTACKING) {
                path = QString(":/Image/Kirby_spark/Kirby_spark_attack(%1).png").arg(animFrame % 3 + 1);
            } else {
                path = ":/Image/Kirby_spark/Kirby_spark_stop_R.png";
            }
            break;

        default:
            path = ":/Image/Kirby_normal/kirby_stop_R.png";
    }

    return getEntityPixmap(path, dir);
}

void Kirby::draw(QPainter& painter, double cameraX) {
    // Blinking effect when invincible
    if (invincibleTicks > 0 && (invincibleTicks / 4) % 2 == 0) {
        return;
    }

    QPixmap currentPixmap = getSprite();
    painter.drawPixmap(rect.x() - cameraX, rect.y(), rect.width(), rect.height(), currentPixmap);
}


// ==========================================
// ENEMIES IMPLEMENTATION
// ==========================================

// Waddle Dee
WaddleDee::WaddleDee(double x, double y)
    : Enemy(x, y, 65, 65, E_WADDLE_DEE) {
    dir = LEFT;
    animFrame = 0;
    animTick = 0;
    patrolStartX = x;
    patrolRange = 250;
    velocity = QPointF(-3.0, 0);
}

void WaddleDee::update(const QList<QRect>& solids, const QList<QRect>& platforms) {
    velocity.setY(velocity.y() + 1.2); // Normal gravity

    // Horizontal patrol logic
    if (dir == LEFT) {
        velocity.setX(-3.0);
        if (rect.x() < patrolStartX - patrolRange) {
            dir = RIGHT;
        }
    } else {
        velocity.setX(3.0);
        if (rect.x() > patrolStartX + patrolRange) {
            dir = LEFT;
        }
    }

    CollisionResult col = Physics::resolveMovement(rect, velocity, solids, platforms);
    if (col.hitWall) {
        dir = (dir == LEFT) ? RIGHT : LEFT;
    }

    animTick++;
    if (animTick >= 7) {
        animTick = 0;
        animFrame = (animFrame + 1) % 6;
    }
}

void WaddleDee::draw(QPainter& painter, double cameraX) {
    QString path = QString(":/Image/Waddle Dee/Waddle_Dee_%1.png").arg(animFrame);
    QPixmap pm = getEntityPixmap(path, dir);
    painter.drawPixmap(rect.x() - cameraX, rect.y(), rect.width(), rect.height(), pm);
}


// Gordo (Spiky Invincible)
Gordo::Gordo(double x, double y, double range)
    : Enemy(x, y, 65, 65, E_GORDO) {
    startY = y;
    movementRange = range;
    speed = 2.5;
    velocity = QPointF(0, speed);
    animFrame = 0;
    animTick = 0;
}

void Gordo::update(const QList<QRect>& solids, const QList<QRect>& platforms) {
    // Moves vertically along fixed path
    rect.translate(0, velocity.y());

    if (rect.y() > startY + movementRange) {
        velocity.setY(-speed);
    } else if (rect.y() < startY - movementRange) {
        velocity.setY(speed);
    }

    animTick++;
    if (animTick >= 12) {
        animTick = 0;
        animFrame = (animFrame + 1) % 2;
    }
}

void Gordo::draw(QPainter& painter, double cameraX) {
    QString path = QString(":/Image/Gordo/Gordo(%1).png").arg(animFrame);
    QPixmap pm = getEntityPixmap(path, dir);
    painter.drawPixmap(rect.x() - cameraX, rect.y(), rect.width(), rect.height(), pm);
}


// Hot Head (Fire Ability)
HotHead::HotHead(double x, double y)
    : Enemy(x, y, 70, 70, E_HOT_HEAD) {
    dir = LEFT;
    animFrame = 0;
    animTick = 0;
    attackCooldown = 0;
    attackActiveTicks = 0;
    breathingFire = false;
    patrolStartX = x;
    patrolRange = 200;
    velocity = QPointF(-2.5, 0);
}

void HotHead::update(const QList<QRect>& solids, const QList<QRect>& platforms) {
    velocity.setY(velocity.y() + 1.2);

    if (attackActiveTicks > 0) {
        attackActiveTicks--;
        velocity.setX(0); // Cannot move during attacks
        if (attackActiveTicks <= 0) {
            breathingFire = false;
        }
    } else {
        // Normal patrol
        if (dir == LEFT) {
            velocity.setX(-2.5);
            if (rect.x() < patrolStartX - patrolRange) {
                dir = RIGHT;
            }
        } else {
            velocity.setX(2.5);
            if (rect.x() > patrolStartX + patrolRange) {
                dir = LEFT;
            }
        }
    }

    CollisionResult col = Physics::resolveMovement(rect, velocity, solids, platforms);
    if (col.hitWall) {
        dir = (dir == LEFT) ? RIGHT : LEFT;
    }

    if (attackCooldown > 0) attackCooldown--;

    animTick++;
    if (animTick >= 8) {
        animTick = 0;
        animFrame = (animFrame + 1) % 2;
    }
}

void HotHead::handleAttackAI(double kirbyX, double kirbyY, QList<Entity*>& projectiles) {
    if (attackCooldown > 0 || attackActiveTicks > 0) return;

    double dist = std::abs(rect.x() - kirbyX);
    double dy = std::abs(rect.y() - kirbyY);

    if (dy < 60 && dist < 500) {
        // Determine whether player is in front of Hot Head
        bool isKirbyInFront = (kirbyX < rect.x() && dir == LEFT) || (kirbyX > rect.x() && dir == RIGHT);

        if (isKirbyInFront) {
            attackCooldown = 180; // 3 seconds cooldown
            if (dist > 150) {
                // Spit Fireball!
                attackActiveTicks = 20;
                projectiles.append(new FireBall(rect.x() + (dir == RIGHT ? 60 : -40), rect.y() + 15, dir));
            } else {
                // Breathe Fire!
                attackActiveTicks = 50;
                breathingFire = true;
            }
        }
    }
}

void HotHead::draw(QPainter& painter, double cameraX) {
    QString path;
    if (attackActiveTicks > 0) {
        if (breathingFire) {
            path = ":/Image/Hot Head/Hot_head_attack_R.png";
        } else {
            path = ":/Image/Hot Head/Hot_head_fire(1).png";
        }
    } else {
        path = QString(":/Image/Hot Head/Hot_head_run_R.png"); // Mirrors nicely
    }

    QPixmap pm = getEntityPixmap(path, dir);
    painter.drawPixmap(rect.x() - cameraX, rect.y(), rect.width(), rect.height(), pm);

    // Draw fire breath particles directly onto stage if breathing fire
    if (breathingFire) {
        QString firePath = QString(":/Image/Hot Head/Hot_head_fire(%1)_R.png").arg(animFrame % 2 + 2);
        QPixmap firePm = getEntityPixmap(firePath, dir);
        double fireX = (dir == RIGHT) ? rect.right() : rect.left() - 65;
        painter.drawPixmap(fireX - cameraX, rect.y() + 5, 65, 60, firePm);
    }
}


// Sparky (Spark Ability)
Sparky::Sparky(double x, double y)
    : Enemy(x, y, 60, 60, E_SPARKY) {
    dir = LEFT;
    animFrame = 0;
    animTick = 0;
    jumpCooldown = 0;
    sparkActiveTicks = 0;
    sparking = false;
    velocity = QPointF(0, 0);
}

void Sparky::update(const QList<QRect>& solids, const QList<QRect>& platforms) {
    // 1. 套用重力
    velocity.setY(velocity.y() + 1.2);

    // 2. 處理當前的水平速度狀態（放電或冷卻時不移動）
    if (sparkActiveTicks > 0) {
        sparkActiveTicks--;
        velocity.setX(0);
        if (sparkActiveTicks <= 0) sparking = false;
    } else {
        if (jumpCooldown > 0) {
            jumpCooldown--;
            velocity.setX(0);
        }
    }

    // 3. 進行物理移動與碰撞偵測
    CollisionResult col = Physics::resolveMovement(rect, velocity, solids, platforms);

    // 4. 撞牆反轉判定
    if (col.hitWall) {
        dir = (dir == LEFT) ? RIGHT : LEFT;
        velocity.setX(-velocity.x());
    }

    // 5. 【修正重點】把 AI 跳躍邏輯移到這裡，直接使用物理運算回傳的 col.onGround
    if (sparkActiveTicks <= 0 && jumpCooldown <= 0) {
        if (col.onGround) {
            velocity.setY(-12.0); // 往上跳
            velocity.setX(dir == LEFT ? -4.5 : 4.5);
            jumpCooldown = 60 + (rand() % 40); // 重置跳躍冷卻時間
        }
    }

    // 6. Spark 放電觸發邏輯
    if (col.onGround && rand() % 250 == 0 && sparkActiveTicks <= 0) {
        sparking = true;
        sparkActiveTicks = 60; // 放電 1 秒
        velocity.setX(0); // 確保放電時停在原地
    }

    // 7. 動畫影格更新
    animTick++;
    if (animTick >= 10) {
        animTick = 0;
        animFrame = (animFrame + 1) % 2;
    }
}

void Sparky::draw(QPainter& painter, double cameraX) {
    QString path;
    if (sparking) {
        path = QString(":/Image/Sparky/Sparky_attack_%1.png").arg(animFrame + 1);
    } else {
        if (dir == LEFT) {
            path = QString(":/Image/Sparky/Sparky_left_%1.png").arg(animFrame + 1);
        } else {
            // Crucial: check spelling in resource files -> Sparky_ritht_1.png!
            path = QString(":/Image/Sparky/Sparky_ritht_%1.png").arg(animFrame + 1);
        }
    }

    QPixmap pm = getEntityPixmap(path, RIGHT); // Do not flip horizontally, textures are already directed!
    painter.drawPixmap(rect.x() - cameraX, rect.y(), rect.width(), rect.height(), pm);
}


// ==========================================
// ITEMS IMPLEMENTATION
// ==========================================

Item::Item(double x, double y, ItemType t)
    : Entity(x, y, 50, 50), type(t) {
    velocity = QPointF(0, 0);
}

void Item::update(const QList<QRect>& solids, const QList<QRect>& platforms) {
    // Items fall gently on land
    velocity.setY(velocity.y() + 1.2);
    Physics::resolveMovement(rect, velocity, solids, platforms);
}

void Item::draw(QPainter& painter, double cameraX) {
    QString path = (type == TOMATO) ? ":/Image/item/Maxim Tomato.png" : ":/Image/item/1UP.png";
    QPixmap pm(path);
    painter.drawPixmap(rect.x() - cameraX, rect.y(), rect.width(), rect.height(), pm);
}


// ==========================================
// PROJECTILES IMPLEMENTATION
// ==========================================

// Spit Star
SpitStar::SpitStar(double x, double y, Direction d)
    : Entity(x, y, 50, 50) {
    dir = d;
    velocity = QPointF(dir == RIGHT ? 18.0 : -18.0, 0);
}

void SpitStar::update(const QList<QRect>& solids, const QList<QRect>& platforms) {
    rect.translate(velocity.x(), 0);

    // Collides with solid brick walls
    for (const QRect& solid : solids) {
        if (rect.intersects(QRectF(solid))) {
            active = false; // Star pops
            break;
        }
    }
}

void SpitStar::draw(QPainter& painter, double cameraX) {
    // Star sprite path in QRC
    QString path = ":/Image/Kirby_normal/kirby_attack_star(2).png";
    QPixmap pm = getEntityPixmap(path, dir);
    painter.drawPixmap(rect.x() - cameraX, rect.y(), rect.width(), rect.height(), pm);
}


// Fireball
FireBall::FireBall(double x, double y, Direction d)
    : Entity(x, y, 40, 30) {
    dir = d;
    velocity = QPointF(dir == RIGHT ? 9.0 : -9.0, 0);
}

void FireBall::update(const QList<QRect>& solids, const QList<QRect>& platforms) {
    rect.translate(velocity.x(), 0);

    for (const QRect& solid : solids) {
        if (rect.intersects(QRectF(solid))) {
            active = false;
            break;
        }
    }
}

void FireBall::draw(QPainter& painter, double cameraX) {
    QString path = ":/Image/Hot Head/Hot_head_fire(1).png";
    QPixmap pm = getEntityPixmap(path, dir);
    painter.drawPixmap(rect.x() - cameraX, rect.y(), rect.width(), rect.height(), pm);
}


// Fire Breath particles (for Kirby Fire)
FireBreath::FireBreath(double x, double y, Direction d)
    : Entity(x, y, 75, 60), ticks(25) {
    dir = d;
    velocity = QPointF(dir == RIGHT ? 10.0 : -10.0, 0);
}

void FireBreath::update(const QList<QRect>& solids, const QList<QRect>& platforms) {
    rect.translate(velocity.x(), 0);
    ticks--;
    if (ticks <= 0) {
        active = false;
    }
}

void FireBreath::draw(QPainter& painter, double cameraX) {
    int fireFrame = (ticks / 6) % 3 + 1; // 1 to 3
    QString path = QString(":/Image/Kirby_fire/kirbyfire_fire(%1)_R.png").arg(fireFrame);
    QPixmap pm = getEntityPixmap(path, dir);
    painter.drawPixmap(rect.x() - cameraX, rect.y(), rect.width(), rect.height(), pm);
}

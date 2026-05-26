#ifndef ENEMY_H
#define ENEMY_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QVector>
#include "constants.h"

class Enemy : public QGraphicsPixmapItem {
public:
    Enemy(EnemyType t, double sx, double sy, double pMinX, double pMaxX);
    virtual ~Enemy() {}

    EnemyType type;
    double vx, vy;
    bool facingRight;
    bool alive;
    bool canBeInhaled;
    bool canBeDamaged;
    AbilityType grantedAbility;

    double startX, startY;
    double patrolMinX, patrolMaxX;

    bool isAttacking;
    int attackTimer;
    int attackCooldown;

    int animFrame, animTimer;

    virtual void updateEnemy() = 0;
    QRectF getHitbox() const;
    void die();
    void respawn();

protected:
    QPixmap loadAndScale(const QString &path);
};

// ============ Waddle Dee ============
class WaddleDee : public Enemy {
public:
    WaddleDee(double sx, double sy, double pMinX, double pMaxX);
    void updateEnemy() override;
private:
    QVector<QPixmap> walkFrames;
};

// ============ Gordo ============
class Gordo : public Enemy {
public:
    Gordo(double sx, double sy, double moveMinY, double moveMaxY);
    void updateEnemy() override;
private:
    QPixmap frame0, frame1;
    double moveMinY, moveMaxY;
    bool movingDown;
};

// ============ Hot Head ============
class HotHead : public Enemy {
public:
    HotHead(double sx, double sy, double pMinX, double pMaxX);
    void updateEnemy() override;
    bool shouldShootFire(); // GameScene 會檢查是否要生成火球

    QPixmap spr_fire_proj; // 火球圖片供 GameScene 使用
private:
    QPixmap spr_walk_R, spr_walk_L;
    QPixmap spr_stop_R, spr_stop_L;
    QPixmap spr_attack_R, spr_attack_L;
    bool wantsToShoot;
};

// ============ Sparky ============
class Sparky : public Enemy {
public:
    Sparky(double sx, double sy, double pMinX, double pMaxX);
    void updateEnemy() override;
    bool isSparking() const;
    QRectF getSparkBox() const;

private:
    QPixmap spr_walk_L1, spr_walk_L2;
    QPixmap spr_walk_R1, spr_walk_R2;
    QPixmap spr_attack1, spr_attack2;
    int sparkTimer;
    int jumpTimer;
    bool onGround;
};

#endif // ENEMY_H

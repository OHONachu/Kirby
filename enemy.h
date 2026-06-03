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
    bool isBreathingFire() const;   // 新：GameScene 用來檢查噴火判定
    QRectF getBreathBox() const;    // 新：噴火傷害範圍
    QPixmap spr_fire_proj; // 火球圖片供 GameScene 使用
    void setTargetX(double kirbyX);   // 新增
private:
    QPixmap spr_walk_R, spr_walk_L;
    QPixmap spr_stop_R, spr_stop_L;
    QPixmap spr_attack_R, spr_attack_L;
    // 新增：噴火圖片
    QPixmap spr_breath_R1, spr_breath_L1;   // fire(2)
    QPixmap spr_breath_R2, spr_breath_L2;   // fire(3)
    bool wantsToShoot;
    bool breathingFire;     // 新：是否正在噴火
    int attackMode;         // 新：0 = fireball, 1 = flame breath
    QGraphicsPixmapItem *fireEffect;   // 新增：火焰特效物件
    double targetX;    // 新增
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
// ============ cutKnight ============
class cutKnight : public Enemy{
public:
    cutKnight(double sx, double sy, double pMinX, double pMaxX);
    void updateEnemy() override;
    bool shouldShootCutter(); // GameScene 會檢查是否要生成
    void setTargetX(double kirbyX);// 讓 GameScene 告訴他 Kirby 在哪

    QVector<QPixmap> cutterFrames; // 火球圖片供 GameScene 使用
private:
    double targetX;
    QPixmap spr_stop_R, spr_stop_L;
    QVector<QPixmap> attackR, attackL;
    bool wantsToShoot;

};
// ============ Waddle Doo ============
class WaddleDoo : public Enemy {
public:
    WaddleDoo(double sx, double sy, double pMinX, double pMaxX);
    void updateEnemy() override;
    bool isBeaming() const;
    QRectF getBeamBox() const;
private:
    QPixmap spr_stop_R, spr_stop_L;
    QVector<QPixmap> walkR, walkL;
    QVector<QPixmap> attackR, attackL;
    QPixmap spr_beam1, spr_beam2;
    QGraphicsPixmapItem *beams[3];    // 3 顆光束
    bool beaming;
    int sweepStep;                     // 掃擊階段 0~3
};
#endif // ENEMY_H

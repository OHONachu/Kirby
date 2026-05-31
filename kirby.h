#ifndef KIRBY_H
#define KIRBY_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QVector>
#include <QSet>
#include "constants.h"

class Kirby : public QGraphicsPixmapItem {
public:
    Kirby();

    // 物理屬性
    double vx, vy;
    bool onGround;
    bool facingRight;

    // 狀態
    KirbyState state;
    AbilityType ability;
    int hp, lives;

    // 無敵
    int invincibleTimer;
    bool isInvincible;

    // 攻擊
    int attackTimer;
    bool isAttacking;

    // 吸入/飽足
    bool mouthful;
    EnemyType swallowedEnemy;
    bool inhaling;

    // 動畫
    int animFrame;
    int animTimer;

    // 方法
    void loadSprites();
    void update(const QSet<int> &keys);
    void handleInput(const QSet<int> &keys);
    void applyPhysics();
    void updateAnimation();
    void updateSprite();

    // 動作
    void startInhale();
    void stopInhale();
    void swallowEnemy(EnemyType type);
    void doSwallow();
    void doSpit();
    void startAbilityAttack();
    void dropAbility();

    // 生命
    void takeDamage();
    void die();
    void reset(double x, double y);

    // 碰撞框
    QRectF getHitbox() const;
    QRectF getInhaleBox() const;
    QRectF getAttackBox() const;
    bool canBeHurt() const;

    bool upKeyReleased;
private:
    // ====== Sprites ======
    // Normal
    QPixmap spr_stand_R, spr_stand_L;
    QVector<QPixmap> spr_walk_R, spr_walk_L;
    QVector<QPixmap> spr_jump;
    QPixmap spr_fly1_R, spr_fly1_L, spr_fly2_R, spr_fly2_L;
    QPixmap spr_down_R, spr_down_L;
    QPixmap spr_inhale_R, spr_inhale_L;
    QPixmap spr_mouthful_R, spr_mouthful_L;

    // Fire ability
    QPixmap spr_fire_stand_R, spr_fire_stand_L;
    QVector<QPixmap> spr_fire_walk_R, spr_fire_walk_L;
    QPixmap spr_fire_fly1_R, spr_fire_fly1_L, spr_fire_fly2_R, spr_fire_fly2_L;
    QPixmap spr_fire_down_R, spr_fire_down_L;
    QVector<QPixmap> spr_fire_attack_R, spr_fire_attack_L;

    // Spark ability
    QPixmap spr_spark_stand_R, spr_spark_stand_L;
    QVector<QPixmap> spr_spark_walk_R, spr_spark_walk_L;
    QPixmap spr_spark_fly1_R, spr_spark_fly1_L, spr_spark_fly2_R, spr_spark_fly2_L;
    QPixmap spr_spark_down_R, spr_spark_down_L;
    QVector<QPixmap> spr_spark_attack;

    // 星星彈
    QPixmap spr_star;

    QPixmap loadAndScale(const QString &path);
};

#endif // KIRBY_H

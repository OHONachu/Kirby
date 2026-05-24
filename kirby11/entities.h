#ifndef ENTITIES_H
#define ENTITIES_H

#include <QRectF>
#include <QPointF>
#include <QPainter>
#include <QPixmap>
#include <QList>
#include <QString>
#include <QTime>

enum Direction {
    LEFT,
    RIGHT
};

enum KirbyState {
    K_NORMAL,
    K_INHALING,
    K_MOUTHFUL,
    K_FIRE,
    K_SPARK
};

enum ActionState {
    A_IDLE,
    A_RUNNING,
    A_JUMPING,
    A_FALLING,
    A_HOVERING,
    A_SQUATTING,
    A_ATTACKING
};

enum EnemyType {
    E_WADDLE_DEE,
    E_GORDO,
    E_HOT_HEAD,
    E_SPARKY
};

class Entity {
public:
    QRectF rect;
    QPointF velocity;
    Direction dir;
    bool active;

    Entity(double x, double y, double w, double h)
        : rect(x, y, w, h), velocity(0, 0), dir(RIGHT), active(true) {}
    virtual ~Entity() {}

    virtual void update(const QList<QRect>& solids, const QList<QRect>& platforms) = 0;
    virtual void draw(QPainter& painter, double cameraX) = 0;
    virtual QRectF getCollisionBox() const { return rect; }
};

// Forward declaration
class SpitStar;
class FireBreath;
class SparkShield;

class Kirby : public Entity {
public:
    KirbyState state;
    ActionState action;
    
    int hp;
    int lives;
    int invincibleTicks;
    int animFrame;
    int animTick;
    bool isGrounded;
    
    // Attack duration
    int attackTicks;
    bool isAttacking;

    // Sucking logic
    bool hasSuckedEnemy;
    EnemyType suckedEnemyType;

    // Abilities HUD display
    QPixmap normalBoardPixmap;
    QPixmap fireBoardPixmap;
    QPixmap sparkBoardPixmap;

    Kirby(double x, double y);

    void update(const QList<QRect>& solids, const QList<QRect>& platforms) override;
    void draw(QPainter& painter, double cameraX) override;
    void handleDamage();
    void startInhale();
    void stopInhale();
    void triggerAction(const QMap<int, bool>& keys);

    QRectF getInhaleRange() const;

private:
    QPixmap getSprite();
    void updateAnimation();
};

class Enemy : public Entity {
public:
    EnemyType type;
    Enemy(double x, double y, double w, double h, EnemyType t)
        : Entity(x, y, w, h), type(t) {}
    virtual bool isSuckable() const = 0;
};

class WaddleDee : public Enemy {
private:
    int animFrame;
    int animTick;
    double patrolStartX;
    double patrolRange;

public:
    WaddleDee(double x, double y);
    void update(const QList<QRect>& solids, const QList<QRect>& platforms) override;
    void draw(QPainter& painter, double cameraX) override;
    bool isSuckable() const override { return true; }
};

class Gordo : public Enemy {
private:
    int animFrame;
    int animTick;
    double startY;
    double movementRange;
    double speed;

public:
    Gordo(double x, double y, double range = 150);
    void update(const QList<QRect>& solids, const QList<QRect>& platforms) override;
    void draw(QPainter& painter, double cameraX) override;
    bool isSuckable() const override { return false; }
};

class HotHead : public Enemy {
private:
    int animFrame;
    int animTick;
    int attackCooldown;
    int attackActiveTicks;
    bool breathingFire;
    double patrolStartX;
    double patrolRange;

public:
    HotHead(double x, double y);
    void update(const QList<QRect>& solids, const QList<QRect>& platforms) override;
    void draw(QPainter& painter, double cameraX) override;
    bool isSuckable() const override { return true; }
    
    // Triggers fire attacks relative to Kirby
    void handleAttackAI(double kirbyX, double kirbyY, QList<Entity*>& projectiles);
};

class Sparky : public Enemy {
private:
    int animFrame;
    int animTick;
    int jumpCooldown;
    int sparkActiveTicks;
    bool sparking;

public:
    Sparky(double x, double y);
    void update(const QList<QRect>& solids, const QList<QRect>& platforms) override;
    void draw(QPainter& painter, double cameraX) override;
    bool isSuckable() const override { return true; }
    void handleAttackAI();
};

class Item : public Entity {
public:
    enum ItemType {
        TOMATO,
        ONE_UP
    };
    ItemType type;

    Item(double x, double y, ItemType t);
    void update(const QList<QRect>& solids, const QList<QRect>& platforms) override;
    void draw(QPainter& painter, double cameraX) override;
};

// Projectile entities
class SpitStar : public Entity {
public:
    SpitStar(double x, double y, Direction dir);
    void update(const QList<QRect>& solids, const QList<QRect>& platforms) override;
    void draw(QPainter& painter, double cameraX) override;
};

class FireBall : public Entity {
public:
    FireBall(double x, double y, Direction dir);
    void update(const QList<QRect>& solids, const QList<QRect>& platforms) override;
    void draw(QPainter& painter, double cameraX) override;
};

class FireBreath : public Entity {
private:
    int ticks;
public:
    FireBreath(double x, double y, Direction dir);
    void update(const QList<QRect>& solids, const QList<QRect>& platforms) override;
    void draw(QPainter& painter, double cameraX) override;
};

#endif // ENTITIES_H

#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include "constants.h"

class Projectile : public QGraphicsPixmapItem {
public:
    Projectile(ProjectileType t, double sx, double sy, double velX, double velY,
               bool fromPlayer, const QPixmap &sprite);

    ProjectileType type;
    double vx, vy;
    bool fromPlayer;
    int lifetime;
    bool active;

    void updateProjectile();
    QRectF getHitbox() const;
};

#endif // PROJECTILE_H

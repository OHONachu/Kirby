#include "projectile.h"

Projectile::Projectile(ProjectileType t, double sx, double sy, double velX, double velY,
                       bool fp, const QPixmap &sprite)
    : type(t), vx(velX), vy(velY), fromPlayer(fp), active(true)
{
    setPixmap(sprite);
    setPos(sx, sy);
    setZValue(8);

    // 根據類型設定生命週期
    switch (type) {
    case PROJ_SPIT_STAR:  lifetime = 120; break;
    case PROJ_FIREBALL:   lifetime = 150; break;
    case PROJ_FIRE_BREATH: lifetime = FIRE_DURATION; break;
    case PROJ_SPARK_FIELD: lifetime = SPARK_DURATION; break;
    }
}

void Projectile::updateProjectile() {
    if (!active) return;

    setPos(x() + vx, y() + vy);
    lifetime--;
    if (lifetime <= 0) {
        active = false;
        setVisible(false);
    }
}

QRectF Projectile::getHitbox() const {
    return QRectF(x(), y(), pixmap().width(), pixmap().height());
}

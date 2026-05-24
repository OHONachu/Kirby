#ifndef PHYSICS_H
#define PHYSICS_H

#include <QRect>
#include <QList>
#include <QPointF>
#include <QRectF>

struct CollisionResult {
    bool onGround = false;
    bool hitCeiling = false;
    bool hitWall = false;
};

class Physics {
public:
    // Basic rectangle overlap check
    static bool checkOverlap(const QRect& r1, const QRect& r2);
    static bool checkOverlapF(const QRectF& r1, const QRectF& r2);

    // Platformer physics resolver:
    // Moves actorBox by velocity, handles solid collisions (blocks) and top-only platform collisions.
    // Modifies actorBox position and velocity (e.g. setting vertical speed to 0 on floor/ceiling).
    static CollisionResult resolveMovement(QRectF& actorBox, QPointF& velocity,
                                           const QList<QRect>& solids,
                                           const QList<QRect>& platforms,
                                           bool isFlying = false);
};

#endif // PHYSICS_H

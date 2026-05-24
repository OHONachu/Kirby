#include "physics.h"
#include <cmath>
#include <algorithm>

bool Physics::checkOverlap(const QRect& r1, const QRect& r2) {
    return r1.intersects(r2);
}

bool Physics::checkOverlapF(const QRectF& r1, const QRectF& r2) {
    return r1.intersects(r2);
}

CollisionResult Physics::resolveMovement(QRectF& actorBox, QPointF& velocity,
                                       const QList<QRect>& solids,
                                       const QList<QRect>& platforms,
                                       bool isFlying) {
    CollisionResult result;

    // --- 1. Resolve Horizontal (X-axis) Movement ---
    if (std::abs(velocity.x()) > 0.0001) {
        actorBox.translate(velocity.x(), 0);

        for (const QRect& solid : solids) {
            QRectF solidF(solid);
            if (actorBox.intersects(solidF)) {
                result.hitWall = true;
                if (velocity.x() > 0) {
                    actorBox.setRight(solidF.left() - 0.01);
                } else if (velocity.x() < 0) {
                    actorBox.setLeft(solidF.right() + 0.01);
                }
                velocity.setX(0);
                break; // Break on first collision to avoid double adjustments
            }
        }
    }

    // --- 2. Resolve Vertical (Y-axis) Movement ---
    // Save previous bottom position to check one-way platform logic
    double previousBottom = actorBox.bottom();
    
    if (std::abs(velocity.y()) > 0.0001) {
        actorBox.translate(0, velocity.y());

        // Check solid blocks (full 4-way collision)
        for (const QRect& solid : solids) {
            QRectF solidF(solid);
            if (actorBox.intersects(solidF)) {
                if (velocity.y() > 0) {
                    actorBox.setBottom(solidF.top() - 0.01);
                    result.onGround = true;
                } else if (velocity.y() < 0) {
                    actorBox.setTop(solidF.bottom() + 0.01);
                    result.hitCeiling = true;
                }
                velocity.setY(0);
                break;
            }
        }

        // Check one-way platforms (only collide when falling down and from above)
        if (velocity.y() > 0 && !result.onGround) {
            for (const QRect& plat : platforms) {
                QRectF platF(plat);
                // Actor bottom must intersect the platform, and previous bottom must be above the platform top
                if (actorBox.intersects(platF) && previousBottom <= platF.top() + 15.0) {
                    actorBox.setBottom(platF.top() - 0.01);
                    velocity.setY(0);
                    result.onGround = true;
                    break;
                }
            }
        }
    }

    return result;
}

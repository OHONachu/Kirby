#ifndef STAGE_H
#define STAGE_H

#include <QRect>
#include <QList>
#include <QPainter>
#include <QPixmap>
#include "entities.h"

class Stage {
public:
    int stageNum;
    double totalWidth;
    
    QList<QRect> solids;
    QList<QRect> platforms;
    QList<Enemy*> enemies;
    QList<Item*> items;
    
    QRect portalRect; // Stage 1 end portal
    QRect goalRect;   // Stage 2 end goal
    
    // Textures
    QPixmap bgImages[5];
    QPixmap brickPixmap;
    QPixmap platformPixmap;
    QPixmap floorPixmap;
    QPixmap doorPixmap;
    QPixmap goalDoorPixmap;

    Stage(int num);
    ~Stage();

    void loadLayout();
    void draw(QPainter& painter, double cameraX);
    void clear();
};

#endif // STAGE_H

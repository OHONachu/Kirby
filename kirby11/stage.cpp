#include "stage.h"
#include <QDebug>

Stage::Stage(int num) : stageNum(num) {
    totalWidth = (stageNum == 1) ? 4860.0 : 8100.0;

    // Load static textures
    brickPixmap.load(":/Image/item/brick.png");
    platformPixmap.load(":/Image/item/floor.png");
    floorPixmap.load(":/Image/item/floor.png");
    doorPixmap.load(":/Image/item/door.png");
    goalDoorPixmap.load(":/Image/item/goal_door.png");

    // Load corresponding background frames
    if (stageNum == 1) {
        bgImages[0].load(":/Image/background/Stage1(1).png");
        bgImages[1].load(":/Image/background/Stage1(2).png");
        bgImages[2].load(":/Image/background/Stage1(3).png");
    } else {
        bgImages[0].load(":/Image/background/Stage2(1).png");
        bgImages[1].load(":/Image/background/Stage2(2).png");
        bgImages[2].load(":/Image/background/Stage2(1).png");
        bgImages[3].load(":/Image/background/Stage2(2).png");
        bgImages[4].load(":/Image/background/Stage2(1).png");
    }
}

Stage::~Stage() {
    clear();
}

void Stage::clear() {
    solids.clear();
    platforms.clear();
    qDeleteAll(enemies);
    enemies.clear();
    qDeleteAll(items);
    items.clear();
}

void Stage::loadLayout() {
    clear();

    if (stageNum == 1) {
        // ==========================================
        // STAGE 1 LAYOUT (3 Frames, Width = 4860)
        // ==========================================
        
        // --- Frame 1 (x: 0 - 1620) ---
        solids.append(QRect(0, 900, 1620, 180)); // Ground
        platforms.append(QRect(400, 700, 300, 30));
        platforms.append(QRect(900, 530, 300, 30));
        solids.append(QRect(750, 720, 80, 80)); // Solid block
        enemies.append(new WaddleDee(500, 800));

        // --- Frame 2 (x: 1620 - 3240) ---
        // Includes a Hole gap in the floor (x: 2220 to 2620)
        solids.append(QRect(1620, 900, 600, 180)); // Ground before hole
        solids.append(QRect(2620, 900, 620, 180)); // Ground after hole
        platforms.append(QRect(1800, 700, 300, 30));
        platforms.append(QRect(2220, 500, 400, 30)); // Floating platform above hole
        solids.append(QRect(2800, 750, 80, 80)); // Solid block
        
        enemies.append(new WaddleDee(1850, 800));
        enemies.append(new Gordo(2400, 250, 120)); // Vertically patrolling Gordo guards the hole

        // --- Frame 3 (x: 3240 - 4860) ---
        solids.append(QRect(3240, 900, 1620, 180)); // Ground
        platforms.append(QRect(3500, 700, 300, 30));
        platforms.append(QRect(4000, 520, 300, 30));
        solids.append(QRect(3850, 620, 80, 80)); // Solid block
        
        enemies.append(new WaddleDee(3600, 800));
        enemies.append(new Gordo(4150, 650, 0)); // Static Gordo obstacle

        // Anywhere Door (Portal)
        portalRect = QRect(4500, 720, 100, 180);

    } else {
        // ==========================================
        // STAGE 2 LAYOUT (5 Frames, Width = 8100)
        // ==========================================

        // --- Frame 1 (x: 0 - 1620) ---
        solids.append(QRect(0, 900, 1620, 180)); // Ground
        platforms.append(QRect(300, 700, 250, 30));
        platforms.append(QRect(900, 520, 250, 30));
        solids.append(QRect(600, 650, 80, 80)); // Solid block
        
        enemies.append(new HotHead(400, 800)); // Fire enemy
        enemies.append(new WaddleDee(1000, 800));

        // --- Frame 2 (x: 1620 - 3240) ---
        // Floor gap/hole (x: 2320 to 2720)
        solids.append(QRect(1620, 900, 700, 180)); // Ground Left
        solids.append(QRect(2720, 900, 520, 180)); // Ground Right
        platforms.append(QRect(1800, 700, 250, 30));
        platforms.append(QRect(2250, 530, 350, 30)); // Floating platform above hole
        solids.append(QRect(2850, 720, 80, 80)); // Solid block
        
        enemies.append(new Sparky(1900, 800)); // Spark enemy
        enemies.append(new Gordo(2420, 300, 150));

        // --- Frame 3 (x: 3240 - 4860) ---
        solids.append(QRect(3240, 900, 1620, 180)); // Ground
        platforms.append(QRect(3400, 720, 250, 30));
        platforms.append(QRect(4100, 550, 250, 30));
        solids.append(QRect(3800, 650, 80, 80)); // Solid block
        
        // Item Spawning (Maxim Tomato exactly once!)
        items.append(new Item(3815, 600, Item::TOMATO));
        
        enemies.append(new HotHead(3550, 800));
        enemies.append(new WaddleDee(4300, 800));

        // --- Frame 4 (x: 4860 - 6480) ---
        // Floor gap/hole (x: 5560 to 5960)
        solids.append(QRect(4860, 900, 700, 180)); // Ground Left
        solids.append(QRect(5960, 900, 520, 180)); // Ground Right
        platforms.append(QRect(5050, 700, 250, 30));
        platforms.append(QRect(5500, 520, 320, 30)); // Platform above hole
        solids.append(QRect(6050, 700, 80, 80)); // Solid block
        
        // Item Spawning (1UP exactly once!)
        items.append(new Item(5630, 450, Item::ONE_UP));
        
        enemies.append(new Sparky(5100, 800));
        enemies.append(new Gordo(5660, 280, 140));

        // --- Frame 5 (x: 6480 - 8100) ---
        solids.append(QRect(6480, 900, 1620, 180)); // Ground
        platforms.append(QRect(6700, 700, 250, 30));
        platforms.append(QRect(7200, 520, 250, 30));
        solids.append(QRect(7000, 650, 80, 80)); // Solid block
        
        enemies.append(new WaddleDee(6800, 800));
        enemies.append(new HotHead(7300, 800));

        // Goal Door
        goalRect = QRect(7700, 720, 120, 180);
    }
}

void Stage::draw(QPainter& painter, double cameraX) {
    // 1. Draw backgrounds frame by frame (smooth parallax / tiling)
    int totalFrames = (stageNum == 1) ? 3 : 5;
    for (int i = 0; i < totalFrames; i++) {
        double frameX = i * 1620.0;
        // Only draw backgrounds visible in active camera view
        if (frameX + 1620.0 >= cameraX && frameX <= cameraX + 1620.0) {
            painter.drawPixmap(frameX - cameraX, 0, 1620, 1080, bgImages[i]);
        }
    }

    // 2. Draw solids (Brick blocks & Ground floors)
    for (const QRect& solid : solids) {
        if (solid.x() + solid.width() < cameraX || solid.x() > cameraX + 1620.0) {
            continue; // Cull invisible geometry
        }

        // Check if it is a simple solid block or the main ground floors
        if (solid.height() == 80 && solid.width() == 80) {
            // Draw brick texture
            painter.drawPixmap(solid.x() - cameraX, solid.y(), solid.width(), solid.height(), brickPixmap);
        } else {
            // It is a floor block. Draw tiled ground texture!
            int tileWidth = 80;
            int tileHeight = 80;
            // Draw tiled texture across floor bounds
            for (int tx = solid.x(); tx < solid.x() + solid.width(); tx += tileWidth) {
                for (int ty = solid.y(); ty < solid.y() + solid.height(); ty += tileHeight) {
                    painter.drawPixmap(tx - cameraX, ty, tileWidth, tileHeight, floorPixmap);
                }
            }
        }
    }

    // 3. Draw platforms
    for (const QRect& plat : platforms) {
        if (plat.x() + plat.width() < cameraX || plat.x() > cameraX + 1620.0) {
            continue;
        }
        
        // Draw platform texture (stretched/tiled)
        // Since platform is narrow, draw floor texture repeating horizontally once
        painter.drawPixmap(plat.x() - cameraX, plat.y(), plat.width(), plat.height(), platformPixmap);
    }

    // 4. Draw Door / Goal Door
    if (stageNum == 1) {
        if (portalRect.x() + portalRect.width() >= cameraX && portalRect.x() <= cameraX + 1620.0) {
            painter.drawPixmap(portalRect.x() - cameraX, portalRect.y(), portalRect.width(), portalRect.height(), doorPixmap);
        }
    } else {
        if (goalRect.x() + goalRect.width() >= cameraX && goalRect.x() <= cameraX + 1620.0) {
            painter.drawPixmap(goalRect.x() - cameraX, goalRect.y(), goalRect.width(), goalRect.height(), goalDoorPixmap);
        }
    }

    // 5. Draw items
    for (Item* item : items) {
        if (item->active) {
            item->draw(painter, cameraX);
        }
    }
}

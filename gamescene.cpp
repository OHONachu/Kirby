#include "gamescene.h"
#include "game.h"
#include "kirby.h"
#include "enemy.h"
#include "projectile.h"
#include <QApplication>
#include <QGraphicsView>
#include <QBrush>
#include <QColor>
#include <QPen>

// ============ 建構/解構 ============
GameScene::GameScene(Game *g) : QGraphicsScene(g), game(g) {
    currentStage = 0;
    kirby = nullptr;
    portal = nullptr;
    goalDoor = nullptr;
    maximTomato = nullptr;
    oneUpItem = nullptr;
    abilityIcon = nullptr;
    menuBg = nullptr;
    gameOverBg = nullptr;
    clearBg = nullptr;
    hasPortal = false;
    hasGoal = false;
    tomatoCollected = false;
    oneUpCollected = false;
    gameOverSelection = 0;
    deathTimer = 0;

    // 載入 HUD 素材
    hpFullPix  = loadAndScale("Dataset/item/HP_1.png");
    hpEmptyPix = loadAndScale("Dataset/item/HP_0.png");
    lifePix    = loadAndScale("Dataset/item/life.png");
    fireBoard  = loadAndScale("Dataset/Kirby_fire/kirbyfire_board.png");
    sparkBoard = loadAndScale("Dataset/Kirby_spark/Kirby_spark_board.png");

    // Game Over 畫面
    gameOverContinuePix = loadPix("Dataset/background/game_over_continue.png");
    gameOverQuitPix     = loadPix("Dataset/background/game_over_quit.png");
}

GameScene::~GameScene() {}

QPixmap GameScene::loadPix(const QString &path) {
    QPixmap pix(path);
    if (pix.isNull()) {
        pix = QPixmap(100, 100);
        pix.fill(Qt::darkGray);
    }
    return pix;
}

QPixmap GameScene::loadAndScale(const QString &path) {
    QPixmap pix(path);
    if (pix.isNull()) {
        pix = QPixmap(50, 50);
        pix.fill(Qt::darkGray);
    }
    return pix.scaled(pix.width() * SPRITE_SCALE, pix.height() * SPRITE_SCALE,
                      Qt::KeepAspectRatio, Qt::FastTransformation);
}

// ============ 清除場景 ============
void GameScene::clearStage() {
    // 移除所有物件
    for (auto e : enemies) { removeItem(e); delete e; }
    enemies.clear();
    for (auto p : projectiles) { removeItem(p); delete p; }
    projectiles.clear();
    for (auto &b : blocks) { removeItem(b.item); delete b.item; }
    blocks.clear();
    for (auto &p : platforms) { removeItem(p.item); delete p.item; }
    platforms.clear();
    for (auto f : floorTiles) { removeItem(f); delete f; }
    floorTiles.clear();
    floorRects.clear();
    for (auto bg : bgItems) { removeItem(bg); delete bg; }
    bgItems.clear();
    holes.clear();

    if (portal) { removeItem(portal); delete portal; portal = nullptr; }
    if (goalDoor) { removeItem(goalDoor); delete goalDoor; goalDoor = nullptr; }
    if (maximTomato) { removeItem(maximTomato); delete maximTomato; maximTomato = nullptr; }
    if (oneUpItem) { removeItem(oneUpItem); delete oneUpItem; oneUpItem = nullptr; }

    // 清除 HUD
    for (auto h : hpIcons) { removeItem(h); delete h; }
    hpIcons.clear();
    for (auto l : lifeIcons) { removeItem(l); delete l; }
    lifeIcons.clear();
    if (abilityIcon) { removeItem(abilityIcon); delete abilityIcon; abilityIcon = nullptr; }

    // 清除選單背景
    if (menuBg) { removeItem(menuBg); delete menuBg; menuBg = nullptr; }
    if (gameOverBg) { removeItem(gameOverBg); delete gameOverBg; gameOverBg = nullptr; }
    if (clearBg) { removeItem(clearBg); delete clearBg; clearBg = nullptr; }

    hasPortal = false;
    hasGoal = false;
}

// ============ 載入關卡 ============
void GameScene::loadStage(int stageNum) {
    clearStage();
    currentStage = stageNum;

    int totalWidth = (stageNum == 1) ? FRAME_WIDTH * STAGE1_FRAMES
                                     : FRAME_WIDTH * STAGE2_FRAMES;
    setSceneRect(0, 0, totalWidth, WINDOW_HEIGHT);
    setBackgroundBrush(QBrush(QColor(135, 206, 235))); // 天空藍

    // 建立 Kirby
    if (!kirby) {
        kirby = new Kirby();
        addItem(kirby);
    }
    kirby->reset(100, FLOOR_Y - 300);

    // 根據 Stage 配置
    if (stageNum == 1) setupStage1();
    else setupStage2();

    // 設置 HUD
    setupHUD();

    deathTimer = 0;
}

// ============ 添加地板磚塊 ============
void GameScene::addFloor(double fx, double fy, int count) {
    QPixmap floorPix = loadAndScale("Dataset/item/floor.png");
    double tileW = floorPix.width();
    double tileH = floorPix.height();
    for (int i = 0; i < count; i++) {
        QGraphicsPixmapItem *tile = new QGraphicsPixmapItem(floorPix);
        tile->setPos(fx + i * tileW, fy);
        tile->setZValue(2);
        addItem(tile);
        floorTiles.append(tile);
        floorRects.append(QRectF(fx + i * tileW, fy, tileW, tileH));
    }
}

// ============ 添加磚頭 ============
void GameScene::addBlock(double bx, double by) {
    QPixmap brickPix = loadAndScale("Dataset/item/brick.png");
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(brickPix);
    item->setPos(bx, by);
    item->setZValue(3);
    addItem(item);
    blocks.append({item, QRectF(bx, by, brickPix.width(), brickPix.height())});
}

// ============ 添加平台 ============
void GameScene::addPlatform(double px, double py, int width) {
    QPixmap floorPix = loadAndScale("Dataset/item/floor.png");
    double tileW = floorPix.width();
    double tileH = floorPix.height();
    // 平台由多個 floor tile 組成
    double totalW = tileW * width;
    for (int i = 0; i < width; i++) {
        QGraphicsPixmapItem *tile = new QGraphicsPixmapItem(floorPix);
        tile->setPos(px + i * tileW, py);
        tile->setZValue(2);
        addItem(tile);
        floorTiles.append(tile);
    }
    platforms.append({nullptr, QRectF(px, py, totalW, tileH)});
}

// ============ 添加 Hole ============
void GameScene::addHole(double hx, double hy, double hw, double hh) {
    holes.append(QRectF(hx, hy, hw, hh));
}

// ============ 添加敵人 ============
Enemy* GameScene::addEnemy(EnemyType type, double ex, double ey, double pMin, double pMax) {
    Enemy *e = nullptr;
    switch (type) {
    case ENEMY_WADDLE_DEE:
        e = new WaddleDee(ex, ey, pMin, pMax);
        break;
    case ENEMY_GORDO:
        e = new Gordo(ex, ey, ey - 150, ey + 100);
        break;
    case ENEMY_HOT_HEAD:
        e = new HotHead(ex, ey, pMin, pMax);
        break;
    case ENEMY_SPARKY:
        e = new Sparky(ex, ey, pMin, pMax);
        break;
    }
    if (e) {
        addItem(e);
        enemies.append(e);
    }
    return e;
}

// ============ Stage 1 配置 (3 frames) ============
void GameScene::setupStage1() {
    double floorTileW = loadAndScale("Dataset/item/floor.png").width();
    int tilesPerFrame = (int)(FRAME_WIDTH / floorTileW) + 1;

    // === 載入背景 ===
    // Stage1(1) 是地面紋理，Stage1(2) 是裝飾
    QPixmap bgSky = loadPix("Dataset/background/supplement(1).jpg");
    bgSky = bgSky.scaled(FRAME_WIDTH, WINDOW_HEIGHT, Qt::IgnoreAspectRatio);
    for (int f = 0; f < STAGE1_FRAMES; f++) {
        QGraphicsPixmapItem *bg = new QGraphicsPixmapItem(bgSky);
        bg->setPos(f * FRAME_WIDTH, 0);
        bg->setZValue(0);
        addItem(bg);
        bgItems.append(bg);
    }

    // === Frame 1-1: 基礎教學 ===
    // 地板（整段，但中間留 Hole 給 Frame 1-2）
    addFloor(0, FLOOR_Y, tilesPerFrame);

    // 平台
    addPlatform(300, FLOOR_Y - 250, 3);
    addPlatform(800, FLOOR_Y - 400, 3);
    addPlatform(1200, FLOOR_Y - 200, 2);

    // 磚頭
    addBlock(600, FLOOR_Y - 214);
    addBlock(1000, FLOOR_Y - 214);

    // 敵人 (至少 1 個)
    addEnemy(ENEMY_WADDLE_DEE, 500, FLOOR_Y - 260, 200, 800);

    // === Frame 1-2: 學會迴避 ===
    double f2x = FRAME_WIDTH;
    addFloor(f2x, FLOOR_Y, 6); // 左半段地板
    // Hole
    double holeStart = f2x + 6 * floorTileW;
    double holeWidth = 3 * floorTileW;
    addHole(holeStart, FLOOR_Y, holeWidth, 200);
    addFloor(holeStart + holeWidth, FLOOR_Y, tilesPerFrame - 9);

    addPlatform(f2x + 200, FLOOR_Y - 300, 3);
    addPlatform(f2x + 900, FLOOR_Y - 350, 3);

    addBlock(f2x + 500, FLOOR_Y - 214);

    addEnemy(ENEMY_WADDLE_DEE, f2x + 300, FLOOR_Y - 260, f2x + 100, f2x + 700);
    addEnemy(ENEMY_GORDO, f2x + 1100, FLOOR_Y - 400, 0, 0);

    // === Frame 1-3: 吸入吐出 + Portal ===
    double f3x = FRAME_WIDTH * 2;
    addFloor(f3x, FLOOR_Y, tilesPerFrame);

    addPlatform(f3x + 200, FLOOR_Y - 280, 3);
    addPlatform(f3x + 800, FLOOR_Y - 350, 2);

    addBlock(f3x + 500, FLOOR_Y - 214);

    addEnemy(ENEMY_WADDLE_DEE, f3x + 300, FLOOR_Y - 260, f3x + 100, f3x + 600);
    addEnemy(ENEMY_WADDLE_DEE, f3x + 700, FLOOR_Y - 260, f3x + 600, f3x + 1100);

    // 道具 - Maxim Tomato（Stage 1 放置）
    if (!tomatoCollected) {
        QPixmap tomatoPix = loadAndScale("Dataset/item/Maxim Tomato.png");
        maximTomato = new QGraphicsPixmapItem(tomatoPix);
        maximTomato->setPos(f2x + 950, FLOOR_Y - 400);
        maximTomato->setZValue(5);
        addItem(maximTomato);
        tomatoRect = QRectF(f2x + 950, FLOOR_Y - 400, tomatoPix.width(), tomatoPix.height());
    }

    // Portal (任意門)
    QPixmap doorPix = loadAndScale("Dataset/item/door.png");
    portal = new QGraphicsPixmapItem(doorPix);
    portal->setPos(f3x + FRAME_WIDTH - 400, FLOOR_Y - doorPix.height());
    portal->setZValue(4);
    addItem(portal);
    portalRect = QRectF(portal->x(), portal->y(), doorPix.width(), doorPix.height());
    hasPortal = true;
}

// ============ Stage 2 配置 (5 frames) ============
void GameScene::setupStage2() {
    double floorTileW = loadAndScale("Dataset/item/floor.png").width();
    int tilesPerFrame = (int)(FRAME_WIDTH / floorTileW) + 1;

    // 背景
    QPixmap bgPix1 = loadPix("Dataset/background/Stage2(1).png");
    bgPix1 = bgPix1.scaled(FRAME_WIDTH, 600, Qt::IgnoreAspectRatio);
    QPixmap bgPix2 = loadPix("Dataset/background/Stage2(2).png");
    bgPix2 = bgPix2.scaled(FRAME_WIDTH, 600, Qt::IgnoreAspectRatio);
    for (int f = 0; f < STAGE2_FRAMES; f++) {
        QGraphicsPixmapItem *bg = new QGraphicsPixmapItem(f % 2 == 0 ? bgPix1 : bgPix2);
        bg->setPos(f * FRAME_WIDTH, 0);
        bg->setZValue(0);
        addItem(bg);
        bgItems.append(bg);
    }

    // === Frame 2-1: 引入 Fire 能力 ===
    addFloor(0, FLOOR_Y, tilesPerFrame);
    addPlatform(300, FLOOR_Y - 250, 3);
    addPlatform(900, FLOOR_Y - 400, 3);
    addBlock(600, FLOOR_Y - 214);
    addBlock(1200, FLOOR_Y - 214);
    addEnemy(ENEMY_WADDLE_DEE, 400, FLOOR_Y - 260, 200, 800);
    addEnemy(ENEMY_HOT_HEAD, 1000, FLOOR_Y - 260, 800, 1400);

    // === Frame 2-2: 引入 Spark 能力 ===
    double f2x = FRAME_WIDTH;
    addFloor(f2x, FLOOR_Y, tilesPerFrame);
    addPlatform(f2x + 200, FLOOR_Y - 300, 3);
    addPlatform(f2x + 700, FLOOR_Y - 250, 3);
    addPlatform(f2x + 1100, FLOOR_Y - 400, 2);
    addBlock(f2x + 500, FLOOR_Y - 214);
    addEnemy(ENEMY_GORDO, f2x + 600, FLOOR_Y - 350, 0, 0);
    addEnemy(ENEMY_SPARKY, f2x + 900, FLOOR_Y - 260, f2x + 700, f2x + 1300);

    // === Frame 2-3: 複雜配置 + Maxim Tomato ===
    double f3x = FRAME_WIDTH * 2;
    addFloor(f3x, FLOOR_Y, tilesPerFrame);
    addPlatform(f3x + 150, FLOOR_Y - 250, 3);
    addPlatform(f3x + 800, FLOOR_Y - 380, 3);
    addBlock(f3x + 400, FLOOR_Y - 214);
    addBlock(f3x + 1100, FLOOR_Y - 320);
    addEnemy(ENEMY_WADDLE_DEE, f3x + 300, FLOOR_Y - 260, f3x + 100, f3x + 600);
    addEnemy(ENEMY_HOT_HEAD, f3x + 900, FLOOR_Y - 260, f3x + 700, f3x + 1300);

    // === Frame 2-4: 高難度 + 1UP ===
    double f4x = FRAME_WIDTH * 3;
    addFloor(f4x, FLOOR_Y, 6);
    double holeStart = f4x + 6 * floorTileW;
    double holeWidth = 2 * floorTileW;
    addHole(holeStart, FLOOR_Y, holeWidth, 200);
    addFloor(holeStart + holeWidth, FLOOR_Y, tilesPerFrame - 8);

    addPlatform(f4x + 200, FLOOR_Y - 350, 2);
    addPlatform(f4x + 700, FLOOR_Y - 300, 3);
    addBlock(f4x + 400, FLOOR_Y - 214);
    addEnemy(ENEMY_GORDO, f4x + 500, FLOOR_Y - 400, 0, 0);
    addEnemy(ENEMY_SPARKY, f4x + 900, FLOOR_Y - 260, f4x + 700, f4x + 1300);

    // 1UP
    if (!oneUpCollected) {
        QPixmap oneUpPix = loadAndScale("Dataset/item/1UP.png");
        oneUpItem = new QGraphicsPixmapItem(oneUpPix);
        oneUpItem->setPos(f4x + 750, FLOOR_Y - 420);
        oneUpItem->setZValue(5);
        addItem(oneUpItem);
        oneUpRect = QRectF(f4x + 750, FLOOR_Y - 420, oneUpPix.width(), oneUpPix.height());
    }

    // === Frame 2-5: 終點 ===
    double f5x = FRAME_WIDTH * 4;
    addFloor(f5x, FLOOR_Y, tilesPerFrame);
    addPlatform(f5x + 200, FLOOR_Y - 280, 3);
    addPlatform(f5x + 800, FLOOR_Y - 350, 2);
    addBlock(f5x + 500, FLOOR_Y - 214);
    addEnemy(ENEMY_WADDLE_DEE, f5x + 300, FLOOR_Y - 260, f5x + 100, f5x + 600);
    addEnemy(ENEMY_HOT_HEAD, f5x + 700, FLOOR_Y - 260, f5x + 500, f5x + 1100);

    // Goal (終點門)
    QPixmap goalPix = loadAndScale("Dataset/item/goal_door.png");
    goalDoor = new QGraphicsPixmapItem(goalPix);
    goalDoor->setPos(f5x + FRAME_WIDTH - 400, FLOOR_Y - goalPix.height());
    goalDoor->setZValue(4);
    addItem(goalDoor);
    goalRect = QRectF(goalDoor->x(), goalDoor->y(), goalPix.width(), goalPix.height());
    hasGoal = true;
}

// ============ HUD 設置 ============
void GameScene::setupHUD() {
    // HP 顯示 (畫面下方)
    for (int i = 0; i < KIRBY_MAX_HP; i++) {
        QGraphicsPixmapItem *hp = new QGraphicsPixmapItem(hpFullPix);
        hp->setZValue(100);
        addItem(hp);
        hpIcons.append(hp);
    }

    // Lives 顯示
    QGraphicsPixmapItem *lifeIcon = new QGraphicsPixmapItem(lifePix);
    lifeIcon->setZValue(100);
    addItem(lifeIcon);
    lifeIcons.append(lifeIcon);

    // 能力圖示
    abilityIcon = new QGraphicsPixmapItem();
    abilityIcon->setZValue(100);
    abilityIcon->setVisible(false);
    addItem(abilityIcon);
}

// ============ HUD 更新 ============
void GameScene::updateHUD() {
    if (!kirby) return;

    // 取得當前視口左上角（用於固定 HUD 位置）
    QPointF viewTopLeft = game->mapToScene(0, 0);
    double hudX = viewTopLeft.x() + 20;
    double hudY = viewTopLeft.y() + WINDOW_HEIGHT - 120;

    // HP
    for (int i = 0; i < hpIcons.size(); i++) {
        hpIcons[i]->setPos(hudX + 140 + i * (hpFullPix.width() + 5), hudY + 20);
        hpIcons[i]->setPixmap(i < kirby->hp ? hpFullPix : hpEmptyPix);
    }

    // Lives
    if (!lifeIcons.isEmpty()) {
        lifeIcons[0]->setPos(hudX, hudY);
    }

    // 能力圖示
    if (kirby->ability == ABILITY_FIRE) {
        abilityIcon->setPixmap(fireBoard);
        abilityIcon->setPos(viewTopLeft.x() + WINDOW_WIDTH - fireBoard.width() - 20,
                           hudY);
        abilityIcon->setVisible(true);
    } else if (kirby->ability == ABILITY_SPARK) {
        abilityIcon->setPixmap(sparkBoard);
        abilityIcon->setPos(viewTopLeft.x() + WINDOW_WIDTH - sparkBoard.width() - 20,
                           hudY);
        abilityIcon->setVisible(true);
    } else {
        abilityIcon->setVisible(false);
    }
}

// ============ 主更新循環 ============
void GameScene::updateGame(const QSet<int> &keys) {
    if (!kirby) return;

    // 死亡處理
    if (kirby->state == KIRBY_DEAD) {
        deathTimer++;
        if (deathTimer > 60) { // 1 秒後重生或 Game Over
            if (kirby->lives <= 0) {
                game->showGameOver();
            } else {
                kirby->reset(100, FLOOR_Y - 300);
                // 重新載入當前 stage
                loadStage(currentStage);
            }
        }
        return;
    }

    updateKirby(keys);
    updateEnemies();
    updateProjectiles();
    checkCollisions();
    scrollCamera();
    updateHUD();
}

// ============ 更新 Kirby ============
void GameScene::updateKirby(const QSet<int> &keys) {
    // 儲存舊的 mouthful 狀態（用來偵測吐出動作）
    bool wasMouthful = kirby->mouthful;

    kirby->update(keys);

    // 偵測 Kirby 吐出星星彈
    if (wasMouthful && !kirby->mouthful && kirby->state == KIRBY_NORMAL
        && keys.contains(Qt::Key_X)) {
        // 生成星星彈
        QPixmap starPix = loadAndScale("Dataset/Kirby_normal/kirby_attack_star(2).png");
        double sx = kirby->facingRight ? kirby->x() + kirby->pixmap().width()
                                       : kirby->x() - starPix.width();
        double sy = kirby->y() + kirby->pixmap().height() / 3.0;
        double svx = kirby->facingRight ? SPIT_STAR_SPEED : -SPIT_STAR_SPEED;
        Projectile *star = new Projectile(PROJ_SPIT_STAR, sx, sy, svx, 0, true, starPix);
        addItem(star);
        projectiles.append(star);
    }
}

// ============ 更新敵人 ============
void GameScene::updateEnemies() {
    for (auto e : enemies) {
        if (!e->alive) continue;
        e->updateEnemy();

        // Hot Head 火球生成
        if (e->type == ENEMY_HOT_HEAD) {
            HotHead *hh = dynamic_cast<HotHead*>(e);
            if (hh && hh->shouldShootFire()) {
                double fx = hh->facingRight ? hh->x() + hh->pixmap().width()
                                            : hh->x() - hh->spr_fire_proj.width();
                double fy = hh->y() + hh->pixmap().height() / 3.0;
                double fvx = hh->facingRight ? HOTHEAD_FIRE_SPEED : -HOTHEAD_FIRE_SPEED;
                Projectile *fb = new Projectile(PROJ_FIREBALL, fx, fy, fvx, 0,
                                                false, hh->spr_fire_proj);
                addItem(fb);
                projectiles.append(fb);
            }
        }

        // 檢查敵人地形碰撞
        checkEnemyTerrainCollision(e);
    }
}

// ============ 更新投射物 ============
void GameScene::updateProjectiles() {
    for (int i = projectiles.size() - 1; i >= 0; i--) {
        projectiles[i]->updateProjectile();
        if (!projectiles[i]->active) {
            removeItem(projectiles[i]);
            delete projectiles[i];
            projectiles.removeAt(i);
        }
    }
}

// ============ 碰撞檢測 ============
void GameScene::checkCollisions() {
    checkKirbyTerrainCollision();
    checkKirbyEnemyCollision();
    checkKirbyItemCollision();
    checkKirbyPortalGoal(game->pressedKeys);
    checkProjectileCollisions();
    checkKirbyHole();
}

// ============ Kirby 地形碰撞 ============
void GameScene::checkKirbyTerrainCollision() {
    QRectF kb = kirby->getHitbox();
    kirby->onGround = false;

    // 地板碰撞
    for (const QRectF &fr : floorRects) {
        if (kb.intersects(fr)) {
            // 從上方落下
            if (kirby->vy >= 0 && kb.bottom() >= fr.top() && kb.bottom() - kirby->vy <= fr.top() + 10) {
                kirby->setY(fr.top() - kirby->pixmap().height());
                kirby->vy = 0;
                kirby->onGround = true;
                if (kirby->state == KIRBY_JUMPING) kirby->state = KIRBY_NORMAL;
            }
        }
    }

    // 平台碰撞（可從下方穿越）
    for (const Platform &pf : platforms) {
        if (kb.intersects(pf.rect)) {
            if (kirby->vy >= 0 && kb.bottom() >= pf.rect.top()
                && kb.bottom() - kirby->vy <= pf.rect.top() + 10) {
                kirby->setY(pf.rect.top() - kirby->pixmap().height());
                kirby->vy = 0;
                kirby->onGround = true;
                if (kirby->state == KIRBY_JUMPING) kirby->state = KIRBY_NORMAL;
            }
        }
    }

    // 磚頭碰撞（四個方向阻擋）
    for (const Block &bl : blocks) {
        if (kb.intersects(bl.rect)) {
            double overlapLeft   = kb.right() - bl.rect.left();
            double overlapRight  = bl.rect.right() - kb.left();
            double overlapTop    = kb.bottom() - bl.rect.top();
            double overlapBottom = bl.rect.bottom() - kb.top();

            double minOverlap = qMin(qMin(overlapLeft, overlapRight),
                                     qMin(overlapTop, overlapBottom));

            if (minOverlap == overlapTop && kirby->vy >= 0) {
                kirby->setY(bl.rect.top() - kirby->pixmap().height());
                kirby->vy = 0;
                kirby->onGround = true;
                if (kirby->state == KIRBY_JUMPING) kirby->state = KIRBY_NORMAL;
            } else if (minOverlap == overlapBottom && kirby->vy < 0) {
                kirby->setY(bl.rect.bottom());
                kirby->vy = 0;
            } else if (minOverlap == overlapLeft) {
                kirby->setX(bl.rect.left() - kirby->pixmap().width() +
                            (kirby->pixmap().width() - kb.width()) / 2.0);
            } else if (minOverlap == overlapRight) {
                kirby->setX(bl.rect.right() -
                            (kirby->pixmap().width() - kb.width()) / 2.0);
            }
        }
    }

    // 防止掉出畫面底部（非 Hole 區域）
    if (kirby->y() + kirby->pixmap().height() >= FLOOR_Y && !kirby->onGround) {
        // 檢查是否在 Hole 上方
        bool overHole = false;
        QRectF kRect(kirby->x(), FLOOR_Y, kirby->pixmap().width(), 10);
        for (const QRectF &h : holes) {
            if (kRect.intersects(h)) { overHole = true; break; }
        }
        if (!overHole) {
            kirby->setY(FLOOR_Y - kirby->pixmap().height());
            kirby->vy = 0;
            kirby->onGround = true;
            if (kirby->state == KIRBY_JUMPING) kirby->state = KIRBY_NORMAL;
        }
    }

    // 不超出場景邊界
    if (kirby->x() < 0) kirby->setX(0);
    double maxX = sceneRect().width() - kirby->pixmap().width();
    if (kirby->x() > maxX) kirby->setX(maxX);
}

// ============ 敵人地形碰撞 ============
void GameScene::checkEnemyTerrainCollision(Enemy *e) {
    if (!e->alive) return;

    // 簡易地板碰撞（Gordo 不需要地板碰撞）
    if (e->type == ENEMY_GORDO) return;

    double bottom = e->y() + e->pixmap().height();
    if (bottom >= FLOOR_Y) {
        e->setY(FLOOR_Y - e->pixmap().height());
        e->vy = 0;
    }
}

// ============ Kirby vs 敵人碰撞 ============
void GameScene::checkKirbyEnemyCollision() {
    QRectF kb = kirby->getHitbox();
    QRectF inhaleBox = kirby->inhaling ? kirby->getInhaleBox() : QRectF();
    QRectF attackBox = kirby->isAttacking ? kirby->getAttackBox() : QRectF();

    for (auto e : enemies) {
        if (!e->alive) continue;
        QRectF eb = e->getHitbox();

        // === 吸入判定 ===
        if (kirby->inhaling && e->canBeInhaled && inhaleBox.intersects(eb)) {
            // Sparky 在電擊中不可吸入
            if (e->type == ENEMY_SPARKY) {
                Sparky *sp = dynamic_cast<Sparky*>(e);
                if (sp && sp->isSparking()) continue;
            }
            kirby->swallowEnemy(e->type);
            e->die();
            continue;
        }

        // === 能力攻擊判定 ===
        if (kirby->isAttacking && e->canBeDamaged && attackBox.intersects(eb)) {
            e->die();
            continue;
        }

        // === Sparky 電擊範圍判定 ===
        if (e->type == ENEMY_SPARKY) {
            Sparky *sp = dynamic_cast<Sparky*>(e);
            if (sp && sp->isSparking()) {
                QRectF sparkBox = sp->getSparkBox();
                if (sparkBox.intersects(kb)) {
                    kirby->takeDamage();
                    continue;
                }
            }
        }

        // === 接觸傷害 ===
        if (kb.intersects(eb)) {
            kirby->takeDamage();
        }
    }
}

// ============ Kirby vs 道具碰撞 ============
void GameScene::checkKirbyItemCollision() {
    QRectF kb = kirby->getHitbox();

    // Maxim Tomato
    if (maximTomato && !tomatoCollected && kb.intersects(tomatoRect)) {
        kirby->hp = KIRBY_MAX_HP;
        tomatoCollected = true;
        removeItem(maximTomato);
        delete maximTomato;
        maximTomato = nullptr;
    }

    // 1UP
    if (oneUpItem && !oneUpCollected && kb.intersects(oneUpRect)) {
        if (kirby->lives < KIRBY_MAX_LIVES) {
            kirby->lives++;
        }
        oneUpCollected = true;
        removeItem(oneUpItem);
        delete oneUpItem;
        oneUpItem = nullptr;
    }
}

// ============ Portal / Goal 判定 ============
void GameScene::checkKirbyPortalGoal(const QSet<int> &keys) {
    QRectF kb = kirby->getHitbox();

    // Portal (Stage 1 → Stage 2)
    if (hasPortal && portal && kb.intersects(portalRect)) {
        if (keys.contains(Qt::Key_Up)) {
            game->switchStage(2);
        }
    }

    // Goal (Stage 2 → Clear)
    if (hasGoal && goalDoor && kb.intersects(goalRect)) {
        if (keys.contains(Qt::Key_Up)) {
            game->showClear();
        }
    }
}

// ============ 投射物碰撞 ============
void GameScene::checkProjectileCollisions() {
    for (int i = projectiles.size() - 1; i >= 0; i--) {
        Projectile *p = projectiles[i];
        if (!p->active) continue;
        QRectF pb = p->getHitbox();

        // 投射物 vs 磚頭
        for (const Block &bl : blocks) {
            if (pb.intersects(bl.rect)) {
                p->active = false;
                p->setVisible(false);
                break;
            }
        }
        if (!p->active) continue;

        if (p->fromPlayer) {
            // 玩家投射物 vs 敵人
            for (auto e : enemies) {
                if (!e->alive || !e->canBeDamaged) continue;
                if (pb.intersects(e->getHitbox())) {
                    e->die();
                    p->active = false;
                    p->setVisible(false);
                    break;
                }
            }
        } else {
            // 敵人投射物 vs Kirby
            QRectF kb = kirby->getHitbox();
            if (pb.intersects(kb)) {
                kirby->takeDamage();
                p->active = false;
                p->setVisible(false);
            }
        }
    }
}

// ============ Kirby 掉入 Hole ============
void GameScene::checkKirbyHole() {
    // 如果 Kirby 掉出畫面底部
    if (kirby->y() > WINDOW_HEIGHT) {
        kirby->lives--;
        if (kirby->lives <= 0) {
            kirby->state = KIRBY_DEAD;
            kirby->lives = 0;
        } else {
            kirby->reset(100, FLOOR_Y - 300);
            loadStage(currentStage);
        }
    }
}

// ============ 捲軸攝影機 ============
void GameScene::scrollCamera() {
    if (!kirby) return;

    // 將 Kirby 置中
    double viewX = kirby->x() - WINDOW_WIDTH / 2.0;
    double viewY = 0; // 固定垂直位置

    // 限制不超出場景邊界
    if (viewX < 0) viewX = 0;
    if (viewX > sceneRect().width() - WINDOW_WIDTH)
        viewX = sceneRect().width() - WINDOW_WIDTH;

    game->setSceneRect(viewX, viewY, WINDOW_WIDTH, WINDOW_HEIGHT);
    game->centerOn(viewX + WINDOW_WIDTH / 2.0, WINDOW_HEIGHT / 2.0);
}

// ============ 選單畫面 ============
void GameScene::showMenuScreen() {
    clearStage();
    setSceneRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    QPixmap menuPix = loadPix("Dataset/background/start.png");
    menuPix = menuPix.scaled(WINDOW_WIDTH, WINDOW_HEIGHT, Qt::IgnoreAspectRatio);
    menuBg = new QGraphicsPixmapItem(menuPix);
    menuBg->setPos(0, 0);
    menuBg->setZValue(0);
    addItem(menuBg);
}

// ============ Game Over 畫面 ============
void GameScene::showGameOverScreen() {
    clearStage();
    setSceneRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    gameOverSelection = 0; // 預設選 Continue

    QPixmap goPix = gameOverContinuePix.scaled(WINDOW_WIDTH, WINDOW_HEIGHT, Qt::IgnoreAspectRatio);
    gameOverBg = new QGraphicsPixmapItem(goPix);
    gameOverBg->setPos(0, 0);
    gameOverBg->setZValue(0);
    addItem(gameOverBg);
}

void GameScene::toggleGameOverSelection() {
    gameOverSelection = 1 - gameOverSelection;
    if (gameOverBg) {
        QPixmap pix = (gameOverSelection == 0) ? gameOverContinuePix : gameOverQuitPix;
        gameOverBg->setPixmap(pix.scaled(WINDOW_WIDTH, WINDOW_HEIGHT, Qt::IgnoreAspectRatio));
    }
}

void GameScene::confirmGameOverSelection(Game *g) {
    if (gameOverSelection == 0) {
        // Continue → 回到 Start Menu
        tomatoCollected = false;
        oneUpCollected = false;
        g->restartFromMenu();
    } else {
        // Quit → 關閉遊戲
        qApp->quit();
    }
}

// ============ Clear 畫面 ============
void GameScene::showClearScreen() {
    clearStage();
    setSceneRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    setBackgroundBrush(QBrush(QColor(255, 223, 128)));

    // 簡單的通關畫面
    QGraphicsTextItem *text = new QGraphicsTextItem("STAGE CLEAR!");
    text->setDefaultTextColor(Qt::white);
    QFont font("Arial", 72, QFont::Bold);
    text->setFont(font);
    text->setPos(WINDOW_WIDTH / 2 - 300, WINDOW_HEIGHT / 2 - 80);
    text->setZValue(10);
    addItem(text);

    QGraphicsTextItem *sub = new QGraphicsTextItem("Press any key to continue...");
    sub->setDefaultTextColor(Qt::white);
    sub->setFont(QFont("Arial", 24));
    sub->setPos(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 + 80);
    sub->setZValue(10);
    addItem(sub);
}

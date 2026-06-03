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
    clearStep = 0;

    // 載入 HUD 素材
    hpFullPix  = loadAndScale(":/Dataset/item/HP_1.png");
    hpEmptyPix = loadAndScale(":/Dataset/item/HP_0.png");

    // 🌟 改為載入 live0, live1, live2 三張生命標示圖片
    live0Pix   = loadAndScale(":/Dataset/item/live0.png");
    live1Pix   = loadAndScale(":/Dataset/item/live1.png");
    live2Pix   = loadAndScale(":/Dataset/item/live2.png");
    lifeIconItem = nullptr; // 初始設為空指標
    fireBoard  = loadAndScale(":/Dataset/Kirby_fire/kirbyfire_board.png");
    sparkBoard = loadAndScale(":/Dataset/Kirby_spark/Kirby_spark_board.png");
    cutterBoard = loadAndScale(":/Dataset/Kirby_cutter/cs.png");


    // Game Over 畫面 (對應你的新圖檔)
    gameOverContinuePix = loadPix(":/Dataset/background/gameover(1).png");
    gameOverQuitPix     = loadPix(":/Dataset/background/gameover(2).png");
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
    if (lifeIconItem) {
        removeItem(lifeIconItem);
        delete lifeIconItem;
        lifeIconItem = nullptr;
    }
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

    // 🌟 修改：根據關卡數決定場景總寬度 (Stage 3 有 3 個 frames)
    int totalWidth = FRAME_WIDTH;
    if (stageNum == 1)      totalWidth = FRAME_WIDTH * STAGE1_FRAMES;
    else if (stageNum == 2) totalWidth = FRAME_WIDTH * STAGE2_FRAMES;
    else if (stageNum == 3) totalWidth = FRAME_WIDTH * STAGE3_FRAMES;
    else if (stageNum == 4) totalWidth = FRAME_WIDTH * STAGE4_FRAMES;

    setSceneRect(0, 0, totalWidth, WINDOW_HEIGHT);
    setBackgroundBrush(QBrush(QColor(135, 206, 235))); // 天空藍

    // 建立 Kirby
    if (!kirby) {
        kirby = new Kirby();
        addItem(kirby);
    }
    kirby->reset(100, FLOOR_Y - 300);

    // 🌟 修改：加上 Stage 3 的判斷
    if (stageNum == 1)      setupStage1();
    else if (stageNum == 2) setupStage2();
    else if (stageNum == 3) setupStage3();
    else if (stageNum == 4) setupStage4();
    // 設置 HUD
    setupHUD();

    deathTimer = 0;
}
// ============ 添加地板磚塊 ============
void GameScene::addFloor(double fx, double fy, int count) {
    QPixmap floorPix = loadAndScale(":/Dataset/item/floor.png");
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
    QPixmap brickPix = loadAndScale(":/Dataset/item/brick.png");
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(brickPix);
    item->setPos(bx, by);
    item->setZValue(3);
    addItem(item);
    blocks.append({item, QRectF(bx, by, brickPix.width(), brickPix.height())});
}

// ============ 添加平台 ============
void GameScene::addPlatform(double px, double py, int width) {
    QPixmap floorPix = loadAndScale(":/Dataset/item/floor.png");//載入圖片
    double tileW = floorPix.width();//寬度
    double tileH = floorPix.height();//高度
    // 平台由多個 floor tile 組成
    double totalW = tileW * width;
    for (int i = 0; i < width; i++) {
        QGraphicsPixmapItem *tile = new QGraphicsPixmapItem(floorPix);
        tile->setPos(px + i * tileW, py);
        tile->setZValue(2);//圖層
        addItem(tile);//呈現在螢幕上
        floorTiles.append(tile);//加入陣列
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
    case ENEMY_KNIGHT:
        e = new cutKnight(ex, ey, pMin, pMax);
        break;
    }
    if (e) {
        // === 新增：自動貼齊地板的邏輯 ===
        // 取得這個敵人建構好之後，圖片的實際高度
        double enemyHeight = e->boundingRect().height();

        // 刺球(Gordo)通常是飄浮的，我們排除牠；其他的敵人都強制把腳對齊傳入的 ey
        if (type != ENEMY_GORDO) {
            e->setY(ey - enemyHeight);
        }
        addItem(e);
        enemies.append(e);
    }
    return e;
}

// ============ Stage 1 配置 (3 frames) ============
void GameScene::setupStage1() {
    double floorTileW = loadAndScale(":/Dataset/item/floor.png").width();
    int tilesPerFrame = (int)(FRAME_WIDTH / floorTileW) + 1;

    // === 載入背景 ===
    for (int f = 0; f < STAGE1_FRAMES; f++) {
        // 動態產生檔名：當 f=0 時載入 Stage1(1).png，f=1 時載入 Stage1(2).png...
        QString imagePath = QString(":/Dataset/background/Stage1(%1).png").arg(f + 1);

        // 載入當下對應的背景圖
        QPixmap bgSky = loadPix(imagePath);

        // 強制將圖片寬度縮放為 1620，高度按比例縮放
        bgSky = bgSky.scaledToWidth(1620, Qt::SmoothTransformation);

        // 計算貼齊底部的 Y 座標 (以防每張圖高度有些微落差，我們在迴圈內計算)
        double bottomY = WINDOW_HEIGHT - bgSky.height();

        // 建立圖形元件
        QGraphicsPixmapItem *bg = new QGraphicsPixmapItem(bgSky);

        // X 座標：依序放在 0, 1620, 3240 的位置
        // Y 座標：貼齊視窗底部
        bg->setPos(f * 1620, bottomY);

        bg->setZValue(0); // 最底層
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
    addEnemy(ENEMY_WADDLE_DEE, 500, FLOOR_Y, 200, 800);

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

    // 瓦豆魯迪：直接傳入平台的高度 (FLOOR_Y - 300)，讓 addEnemy 自動算好並貼齊平台
    addEnemy(ENEMY_WADDLE_DEE, f2x + 300, FLOOR_Y - 300, f2x + 100, f2x + 700);

    // 刺球(Gordo)：維持原樣，因為 addEnemy 裡的 if 判斷已經排除了牠，讓牠維持在半空飄浮
    addEnemy(ENEMY_GORDO, f2x + 1100, FLOOR_Y - 400, 0, 0);

    // === Frame 1-3: 吸入吐出 + Portal ===
    double f3x = FRAME_WIDTH * 2;
    addFloor(f3x, FLOOR_Y, tilesPerFrame);

    addPlatform(f3x + 200, FLOOR_Y - 280, 3);
    addPlatform(f3x + 800, FLOOR_Y - 350, 2);

    addBlock(f3x + 500, FLOOR_Y - 214);

    // 兩隻瓦豆魯迪都直接傳入主地板高度 FLOOR_Y，讓程式自動算出貼齊地面的高度
    addEnemy(ENEMY_WADDLE_DEE, f3x + 300, FLOOR_Y, f3x + 100, f3x + 600);
    addEnemy(ENEMY_WADDLE_DEE, f3x + 700, FLOOR_Y, f3x + 600, f3x + 1100);
    // 道具 - Maxim Tomato（Stage 1 放置）
    // 道具 - Maxim Tomato（Stage 1 放置）
    if (!tomatoCollected) {
        QPixmap tomatoPix = loadAndScale(":/Dataset/item/Maxim Tomato.png");
        maximTomato = new QGraphicsPixmapItem(tomatoPix);

        // 要放置的目標高度 (對齊底下的平台 FLOOR_Y - 350)
        double targetY = FLOOR_Y - 350;

        // 實際的 Y 座標 = 目標高度 - 番茄圖片的高度
        double tomatoY = targetY - tomatoPix.height();

        maximTomato->setPos(f2x + 950, tomatoY);
        maximTomato->setZValue(5);
        addItem(maximTomato);

        // 碰撞矩形也要使用算好貼齊的 tomatoY
        tomatoRect = QRectF(f2x + 950, tomatoY, tomatoPix.width(), tomatoPix.height());
    }

    // Portal (任意門)
    QPixmap doorPix = loadAndScale(":/Dataset/item/door.png");
    portal = new QGraphicsPixmapItem(doorPix);
    portal->setPos(f3x + FRAME_WIDTH - 400, FLOOR_Y - doorPix.height());
    portal->setZValue(4);
    addItem(portal);
    portalRect = QRectF(portal->x(), portal->y(), doorPix.width(), doorPix.height());
    hasPortal = true;
}

// ============ Stage 2 配置 (5 frames) ============
void GameScene::setupStage2() {
    double floorTileW = loadAndScale(":/Dataset/item/floor.png").width();
    int tilesPerFrame = (int)(FRAME_WIDTH / floorTileW) + 1;

    // === 載入背景 (Stage 2) ===
    // 1. 預先載入兩張圖片，並強制縮放寬度為 1620 (高度自動等比例縮放)
    QPixmap bgPix1 = loadPix(":/Dataset/background/Stage2(1).png").scaledToWidth(1620, Qt::SmoothTransformation);
    QPixmap bgPix2 = loadPix(":/Dataset/background/Stage2(2).png").scaledToWidth(1620, Qt::SmoothTransformation);

    // 2. 預先計算兩張圖片貼齊底部的 Y 座標 (以防兩張圖的高度有些微不同)
    double bottomY1 = WINDOW_HEIGHT - bgPix1.height();
    double bottomY2 = WINDOW_HEIGHT - bgPix2.height();

    // 3. 鋪設 5 個 Frame 的背景
    for (int f = 0; f < STAGE2_FRAMES; f++) {
        // 利用 f % 2 == 0 判斷現在是偶數(0,2,4)還是奇數(1,3)，來交替選擇圖片與對應的 Y 座標
        QPixmap currentPix = (f % 2 == 0) ? bgPix1 : bgPix2;
        double currentBottomY = (f % 2 == 0) ? bottomY1 : bottomY2;

        // 建立圖形元件
        QGraphicsPixmapItem *bg = new QGraphicsPixmapItem(currentPix);

        // X 座標：依序放在 0, 1620, 3240, 4860, 6480 的位置
        // Y 座標：使用剛才算好貼齊底部的 Y 座標
        bg->setPos(f * 1620, currentBottomY);

        bg->setZValue(0); // 設定為最底層
        addItem(bg);
        bgItems.append(bg);
    }

    // === Frame 2-1: 引入 Fire 能力 ===
    addFloor(0, FLOOR_Y, tilesPerFrame);
    addPlatform(300, FLOOR_Y - 250, 3);
    addPlatform(900, FLOOR_Y - 400, 3);
    addBlock(600, FLOOR_Y - 214);
    addBlock(1200, FLOOR_Y - 214);
    // 瓦豆魯迪與熱氣頭都直接傳入主地板高度 FLOOR_Y，讓程式自動計算並完美貼地
    addEnemy(ENEMY_WADDLE_DEE, 400, FLOOR_Y, 200, 800);
    addEnemy(ENEMY_HOT_HEAD, 1000, FLOOR_Y, 800, 1400);

    // === Frame 2-2: 引入 Spark 能力 ===
    double f2x = FRAME_WIDTH;
    addFloor(f2x, FLOOR_Y, tilesPerFrame);
    addPlatform(f2x + 200, FLOOR_Y - 300, 3);
    addPlatform(f2x + 700, FLOOR_Y - 250, 3);
    addPlatform(f2x + 1100, FLOOR_Y - 400, 2);
    addBlock(f2x + 500, FLOOR_Y - 214);
    addEnemy(ENEMY_GORDO, f2x + 600, FLOOR_Y - 350, 0, 0); // Gordo 維持飄浮
    addEnemy(ENEMY_SPARKY, f2x + 900, FLOOR_Y, f2x + 700, f2x + 1300); // 拿掉 -260
    // === Frame 2-3: 複雜配置 + Maxim Tomato ===
    double f3x = FRAME_WIDTH * 2;
    addFloor(f3x, FLOOR_Y, tilesPerFrame);
    addPlatform(f3x + 150, FLOOR_Y - 250, 3);
    addPlatform(f3x + 800, FLOOR_Y - 380, 3);
    addBlock(f3x + 400, FLOOR_Y - 214);
    addBlock(f3x + 1100, FLOOR_Y - 320);
    addEnemy(ENEMY_WADDLE_DEE, f3x + 300, FLOOR_Y, f3x + 100, f3x + 600);
    addEnemy(ENEMY_HOT_HEAD, f3x + 900, FLOOR_Y, f3x + 700, f3x + 1300);

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
    addEnemy(ENEMY_GORDO, f4x + 500, FLOOR_Y - 400, 0, 0); // Gordo 維持飄浮
    addEnemy(ENEMY_SPARKY, f4x + 900, FLOOR_Y, f4x + 700, f4x + 1300);
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
    addEnemy(ENEMY_WADDLE_DEE, f5x + 300, FLOOR_Y, f5x + 100, f5x + 600);
    addEnemy(ENEMY_HOT_HEAD, f5x + 700, FLOOR_Y, f5x + 500, f5x + 1100);

    // Goal (終點門)
    QPixmap goalPix = loadAndScale(":/Dataset/item/goal_door.png");
    goalDoor = new QGraphicsPixmapItem(goalPix);
    goalDoor->setPos(f5x + FRAME_WIDTH - 400, FLOOR_Y - goalPix.height());
    goalDoor->setZValue(4);
    addItem(goalDoor);
    goalRect = QRectF(goalDoor->x(), goalDoor->y(), goalPix.width(), goalPix.height());
    hasGoal = true;
}

// ============ 🌟 新增：Stage 3 配置 (3 frames) ============
void GameScene::setupStage3() {
    double floorTileW = loadAndScale(":/Dataset/item/floor.png").width();
    int tilesPerFrame = (int)(FRAME_WIDTH / floorTileW) + 1;

    // === 載入 Stage 3 背景圖片 ===
    for (int f = 0; f < STAGE3_FRAMES; f++) {
        // 動態產生檔名：Stage3(1).png, Stage3(2).png, Stage3(3).png
        QString imagePath = QString(":/Dataset/background/stage3(%1).png").arg(f + 1);
        QPixmap bgSky = loadPix(imagePath);
        bgSky = bgSky.scaledToWidth(1620, Qt::SmoothTransformation);

        double bottomY = WINDOW_HEIGHT - bgSky.height();
        QGraphicsPixmapItem *bg = new QGraphicsPixmapItem(bgSky);
        bg->setPos(f * 1620, bottomY);
        bg->setZValue(0);
        addItem(bg);
        bgItems.append(bg);
    }

    // === Frame 3-1: 雙重懸崖與空中刺球 ===
    // 建立分段地板，中間留兩個洞 (Hole) 考驗跳躍
    addFloor(0, FLOOR_Y, 5);
    addHole(5 * floorTileW, FLOOR_Y, 2 * floorTileW, 200); // 第一個洞

    addFloor(7 * floorTileW, FLOOR_Y, 6);
    addHole(13 * floorTileW, FLOOR_Y, 2 * floorTileW, 200); // 第二個洞

    addFloor(15 * floorTileW, FLOOR_Y, tilesPerFrame - 15);

    // 空中安全平台，幫助玩家安全越過陷阱
    addPlatform(400, FLOOR_Y - 260, 3);
    addPlatform(950, FLOOR_Y - 320, 2);

    // 放一隻在平台上巡邏的 Waddle Dee 阻礙玩家
    addEnemy(ENEMY_WADDLE_DEE, 500, FLOOR_Y - 260, 400, 600);
    // 空中漂浮無法被吸入的 Gordo 刺球
    addEnemy(ENEMY_GORDO, 1100, FLOOR_Y - 450, 0, 0);


    // === Frame 3-2: 元素夾擊戰 + 補血番茄 ===
    double f2x = FRAME_WIDTH;
    addFloor(f2x, FLOOR_Y, tilesPerFrame); // 這一層是穩固的平地

    // 置中的金字塔型高台
    addBlock(f2x + 400, FLOOR_Y - 214);
    addPlatform(f2x + 500, FLOOR_Y - 350, 4);
    addBlock(f2x + 950, FLOOR_Y - 214);

    // 左右兩邊分別派出一隻 Hot Head (火) 與 Sparky (電) 夾擊玩家
    addEnemy(ENEMY_KNIGHT, f2x + 250, FLOOR_Y, f2x + 100, f2x + 450);
    addEnemy(ENEMY_SPARKY, f2x + 1100, FLOOR_Y, f2x + 900, f2x + 1300);

    // 在高台中央放置一個補血大番茄 (Maxim Tomato)
    if (!tomatoCollected) {
        QPixmap tomatoPix = loadAndScale(":/Dataset/item/Maxim Tomato.png");
        maximTomato = new QGraphicsPixmapItem(tomatoPix);
        double tomatoY = (FLOOR_Y - 350) - tomatoPix.height();
        maximTomato->setPos(f2x + 650, tomatoY);
        maximTomato->setZValue(5);
        addItem(maximTomato);
        tomatoRect = QRectF(f2x + 650, tomatoY, tomatoPix.width(), tomatoPix.height());
    }


    // === Frame 3-3: 終點前哨衝刺 ===
    double f3x = FRAME_WIDTH * 2;
    addFloor(f3x, FLOOR_Y, tilesPerFrame);

    // 連續的階梯平台
    addPlatform(f3x + 150, FLOOR_Y - 200, 2);
    addPlatform(f3x + 400, FLOOR_Y - 320, 2);
    addPlatform(f3x + 650, FLOOR_Y - 440, 2);

    // 高空平台的防守敵人
    addEnemy(ENEMY_WADDLE_DEE, f3x + 450, FLOOR_Y - 320, f3x + 400, f3x + 550);
    // 地面上也有一隻 Hot Head 攔路
    addEnemy(ENEMY_KNIGHT, f3x + 800, FLOOR_Y, f3x + 600, f3x + 1000);

    // 設置關卡終點門 (Goal Door)
    QPixmap goalPix = loadAndScale(":/Dataset/item/door.png");
    goalDoor = new QGraphicsPixmapItem(goalPix);
    goalDoor->setPos(f3x + FRAME_WIDTH - 350, FLOOR_Y - goalPix.height());
    goalDoor->setZValue(4);
    addItem(goalDoor);
    goalRect = QRectF(goalDoor->x(), goalDoor->y(), goalPix.width(), goalPix.height());
    hasGoal = true;
}
// ============ Stage 4 配置 (3 frames) ============
void GameScene::setupStage4() {
    double floorTileW = loadAndScale(":/Dataset/item/floor.png").width();
    int tilesPerFrame = (int)(FRAME_WIDTH / floorTileW) + 1;
    // === 載入 Stage 4 背景圖片 ===
    for (int f = 0; f < 3; f++) {
        QString imagePath = QString(":/Dataset/background/stage4(%1).png").arg(f + 1);
        QPixmap bgSky = loadPix(imagePath);
        bgSky = bgSky.scaledToWidth(1620, Qt::SmoothTransformation);
        double bottomY = WINDOW_HEIGHT - bgSky.height();
        QGraphicsPixmapItem *bg = new QGraphicsPixmapItem(bgSky);
        bg->setPos(f * 1620, bottomY);
        bg->setZValue(0);
        addItem(bg);
        bgItems.append(bg);
    }
    // === Frame 4-1: 開場深淵與精準跳躍 ===
    addFloor(0, FLOOR_Y, 4);
    addHole(4 * floorTileW, FLOOR_Y, 2 * floorTileW, 200);
    addFloor(6 * floorTileW, FLOOR_Y, 2);
    addHole(8 * floorTileW, FLOOR_Y, 3 * floorTileW, 200);
    addFloor(11 * floorTileW, FLOOR_Y, tilesPerFrame - 11);
    // 磚頭作為高空落腳點
    addBlock(5 * floorTileW, FLOOR_Y - 250);
    addBlock(9 * floorTileW, FLOOR_Y - 300);
    // 在深淵處放置 Gordo 增加跳躍壓力
    addEnemy(ENEMY_GORDO, 5 * floorTileW, FLOOR_Y - 150, 0, 0);
    addEnemy(ENEMY_GORDO, 9 * floorTileW, FLOOR_Y - 450, 0, 0);
    // === Frame 4-2: 元素夾擊戰 ===
    double f2x = FRAME_WIDTH;
    addFloor(f2x, FLOOR_Y, tilesPerFrame);
    addPlatform(f2x + 300, FLOOR_Y - 200, 3);
    addPlatform(f2x + 800, FLOOR_Y - 350, 3);
    addPlatform(f2x + 1300, FLOOR_Y - 200, 3);
    // 安排熱氣頭與電擊怪
    addEnemy(ENEMY_HOT_HEAD, f2x + 400, FLOOR_Y, f2x + 200, f2x + 600);
    addEnemy(ENEMY_SPARKY, f2x + 850, FLOOR_Y - 350, f2x + 800, f2x + 1000);
    addEnemy(ENEMY_HOT_HEAD, f2x + 1200, FLOOR_Y, f2x + 1100, f2x + 1400);
    // === Frame 4-3: 通往勝利的防守線 ===
    double f3x = FRAME_WIDTH * 2;
    addFloor(f3x, FLOOR_Y, tilesPerFrame);
    addPlatform(f3x + 200, FLOOR_Y - 250, 2);
    addBlock(f3x + 500, FLOOR_Y - 300);
    addPlatform(f3x + 700, FLOOR_Y - 350, 2);
    addEnemy(ENEMY_WADDLE_DEE, f3x + 200, FLOOR_Y - 250, f3x + 150, f3x + 350);
    addEnemy(ENEMY_WADDLE_DEE, f3x + 700, FLOOR_Y - 350, f3x + 650, f3x + 850);
    addEnemy(ENEMY_WADDLE_DEE, f3x + 900, FLOOR_Y, f3x + 800, f3x + 1200);
    // 設置關卡終點門 (Goal Door)
    QPixmap goalPix = loadAndScale(":/Dataset/item/goal_door.png");
    goalDoor = new QGraphicsPixmapItem(goalPix);
    goalDoor->setPos(f3x + FRAME_WIDTH - 350, FLOOR_Y - goalPix.height());
    goalDoor->setZValue(4);
    addItem(goalDoor);
    goalRect = QRectF(goalDoor->x(), goalDoor->y(), goalPix.width(), goalPix.height());
    hasGoal = true;
}
// ============ HUD 設置 ============
void GameScene::setupHUD() {
    // HP 顯示 (畫面下方，根據你的設定已改為右上角排版)
    for (int i = 0; i < KIRBY_MAX_HP; i++) {
        QGraphicsPixmapItem *hp = new QGraphicsPixmapItem(hpFullPix);
        hp->setZValue(100);
        addItem(hp);
        hpIcons.append(hp);
    }

    // 🌟 移除了原本 lifeIcons 的初始化，改由 updateHUD 動態生成

    // 能力圖示
    abilityIcon = new QGraphicsPixmapItem();
    abilityIcon->setZValue(100);
    abilityIcon->setVisible(false);
    addItem(abilityIcon);
}


// ============ HUD 更新 ============
void GameScene::updateHUD() {
    if (!kirby || !game) return;

    // 取得當前視口左上角（用於固定 HUD 位置）
    QPointF viewTopLeft = game->mapToScene(0, 0);

    // === 自訂排版參數 (畫面的右上角) ===
    double startX = viewTopLeft.x() + WINDOW_WIDTH - 30; // 距離畫面右邊界 30 像素
    double startY = viewTopLeft.y() + 30;                // 距離畫面頂端 30 像素
    double hpSpacing = 5;                                // 血量圖示之間的間距

    // 1. HP 顯示 (從右往左排列)
    for (int i = 0; i < hpIcons.size(); i++) {
        // 計算每個血量圖示的位置 (第0滴血在最右邊，依序往左排)
        double hpX = startX - (i + 1) * (hpFullPix.width() + hpSpacing);
        hpIcons[i]->setPos(hpX, startY);
        // 根據當前血量切換滿血或空血圖片
        hpIcons[i]->setPixmap(i < kirby->hp ? hpFullPix : hpEmptyPix);
    }

    // 2. 🌟 新版 Lives 顯示 (放在 HP 的下方，靠右對齊)
    // 先移除上一幀的生命標示
    if (lifeIconItem) {
        removeItem(lifeIconItem);
        delete lifeIconItem;
        lifeIconItem = nullptr;
    }

    // 根據目前的命數選擇對應的圖片
    QPixmap currentLivePix;
    if (kirby->lives == 2) {
        currentLivePix = live2Pix;
    } else if (kirby->lives == 1) {
        currentLivePix = live1Pix;
    } else {
        currentLivePix = live0Pix;
    }

    // 建立新圖片並設定座標
    lifeIconItem = new QGraphicsPixmapItem(currentLivePix);
    double lifeX = startX - currentLivePix.width();
    double lifeY = startY + hpFullPix.height() + 15; // 距離上一排(HP) 15 像素

    lifeIconItem->setPos(lifeX, lifeY);
    lifeIconItem->setZValue(100);
    addItem(lifeIconItem);

    // 3. 能力圖示 (放在 Lives 的左邊，保留 20 像素的間距)
    if (kirby->ability == ABILITY_FIRE) {
        abilityIcon->setPixmap(fireBoard);
        abilityIcon->setPos(lifeX - fireBoard.width() - 20, lifeY);
        abilityIcon->setVisible(true);
    } else if (kirby->ability == ABILITY_SPARK) {
        abilityIcon->setPixmap(sparkBoard);
        abilityIcon->setPos(lifeX - sparkBoard.width() - 20, lifeY);
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
        QPixmap starPix = loadAndScale(":/Dataset/Kirby_normal/kirby_attack_star(2).png");
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
            if (hh && hh->isBreathingFire()){
                QRectF breathBox = hh->getBreathBox();
                if (breathBox.intersects(kirby->getHitbox())) {
                    kirby->takeDamage();
                }
            }
        }
        if (e->type == ENEMY_KNIGHT) {
            cutKnight *ck = dynamic_cast<cutKnight*>(e);
            if (ck) {
                // 每幀告訴 cutKnight Kirby 在哪
                ck->setTargetX(kirby->x());

                // 檢查是否要發射 cutter
                if (ck->shouldShootCutter()) {
                    double cx = ck->facingRight ? ck->x() + ck->pixmap().width()
                                                : ck->x() - ck->cutterFrames[0].width();
                    double cy = ck->y() + ck->pixmap().height() / 3.0;
                    double cvx = ck->facingRight ? 6.0 : -6.0;

                    Projectile *cutter = new Projectile(PROJ_CUTTER, cx, cy, cvx, 0,
                                                        false, ck->cutterFrames[0]);
                    // 設定旋轉動畫的兩張圖
                    cutter->animFrames = ck->cutterFrames;

                    addItem(cutter);
                    projectiles.append(cutter);
                }
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
// ============ Kirby 地形碰撞 (完美貼地穩定版) ============
void GameScene::checkKirbyTerrainCollision() {
    QRectF kb = kirby->getHitbox();
    kirby->onGround = false;

    // 建立一個向下延伸 2 像素的檢測矩形，用於穩定著地判定
    // 避免因為剛好貼地導致 intersects() 回傳 false 引起角色抖動
    QRectF footCheck = kb;
    footCheck.setBottom(footCheck.bottom() + 2);

    // 地板碰撞
    for (const QRectF &fr : floorRects) {
        if (footCheck.intersects(fr)) {
            // 從上方落下，或者原本就完美站在地面上
            if (kirby->vy >= 0 && kb.bottom() - kirby->vy <= fr.top() + 10) {
                kirby->setY(fr.top() - kirby->pixmap().height());
                kirby->vy = 0;
                kirby->onGround = true;
                if (kirby->state == KIRBY_JUMPING) kirby->state = KIRBY_NORMAL;
                break; // 踩到主地板了，直接跳出
            }
        }
    }

    // 平台碰撞（可從下方穿越）
    if (!kirby->onGround) {
        for (const Platform &pf : platforms) {
            if (footCheck.intersects(pf.rect)) {
                if (kirby->vy >= 0 && kb.bottom() - kirby->vy <= pf.rect.top() + 10) {
                    kirby->setY(pf.rect.top() - kirby->pixmap().height());
                    kirby->vy = 0;
                    kirby->onGround = true;
                    if (kirby->state == KIRBY_JUMPING) kirby->state = KIRBY_NORMAL;
                    break;
                }
            }
        }
    }

    // 磚頭碰撞（四個方向阻擋）
    for (const Block &bl : blocks) {
        // 為了精確的左右推擠與頂磚頭判定，多方向碰撞依然使用原本的緊密包圍盒 kb
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

        // 額外著地穩定機制：如果卡比正好站在磚頭頂端，用 footCheck 來確保著地狀態不閃爍
        if (!kirby->onGround && kirby->vy >= 0 && footCheck.intersects(bl.rect)) {
            if (kb.bottom() <= bl.rect.top() + 5) {
                kirby->setY(bl.rect.top() - kirby->pixmap().height());
                kirby->vy = 0;
                kirby->onGround = true;
                if (kirby->state == KIRBY_JUMPING) kirby->state = KIRBY_NORMAL;
            }
        }
    }

    // 防止掉出畫面底部（非 Hole 區域的保底主草皮 Y 軸判定）
    if (!kirby->onGround) {
        // 這裡同樣引入 2 像素的緩衝檢測
        if (kirby->y() + kirby->pixmap().height() >= FLOOR_Y - 2) {
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

    // Stage 1 的傳送門 (進 Stage 2)
    if (hasPortal && portal && kb.intersects(portalRect)) {
        if (keys.contains(Qt::Key_Up)) {
            game->switchStage(2);
        }
    }

    // 終點門判定
    if (hasGoal && goalDoor && kb.intersects(goalRect)) {
        if (keys.contains(Qt::Key_Up)) {
            if (currentStage == 2) {
                // 🌟 修改：如果原本在 Stage 2，按下 Up 就進入 Stage 3！
                game->switchStage(3);
            } else if (currentStage == 3) {
                game->switchStage(4);
            }else {
                game->showClear();
            }
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

    QPixmap menuPix = loadPix(":/Dataset/background/start.png");
    menuPix = menuPix.scaled(WINDOW_WIDTH, WINDOW_HEIGHT, Qt::IgnoreAspectRatio);
    menuBg = new QGraphicsPixmapItem(menuPix);
    menuBg->setPos(0, 0);
    menuBg->setZValue(0);
    addItem(menuBg);
}

// ============ Game Over 畫面 ============
void GameScene::showGameOverScreen() {
    clearStage();

    // 1. 限制場景的物理範圍
    setSceneRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // 2. 徹底重置相機鏡頭 (解決跑圖的關鍵)
    if (!views().isEmpty()) {
        QGraphicsView *view = views().first();
        view->setSceneRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT); // 強制 View 也套用此範圍
        view->resetTransform(); // 重置任何可能的變形或位移
        view->centerOn(WINDOW_WIDTH / 2.0, WINDOW_HEIGHT / 2.0); // 鏡頭精準對準畫面的正中央
    }

    gameOverSelection = 0; // 預設為 gameover(1)

    // 3. 載入並拉伸圖片
    QPixmap goPix = loadPix(":/Dataset/background/gameover(1).png");
    goPix = goPix.scaled(WINDOW_WIDTH, WINDOW_HEIGHT, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    gameOverBg = new QGraphicsPixmapItem(goPix);
    gameOverBg->setPos(0, 0); // 絕對綁定在 0,0
    gameOverBg->setZValue(999); // 設為 999 確保圖層在最上方，不會被遮擋
    addItem(gameOverBg);
}

void GameScene::changeGameOverSelection(int key) {
    // 判斷按鍵並切換索引
    if (key == Qt::Key_Up) {
        gameOverSelection = 0;
    } else if (key == Qt::Key_Down) {
        gameOverSelection = 1;
    }

    // 更新圖片
    if (gameOverBg) {
        QPixmap pix = (gameOverSelection == 0) ? gameOverContinuePix : gameOverQuitPix;
        gameOverBg->setPixmap(pix.scaled(WINDOW_WIDTH, WINDOW_HEIGHT, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

        // 💡 強制確保位置依舊鎖死在 0,0
        gameOverBg->setPos(0, 0);
    }
}
void GameScene::confirmGameOverSelection(Game *g) {
    if (gameOverSelection == 0) {
        // 選擇 gameover(1) 按 Enter -> 回到 stage1 且卡比恢復滿血滿命狀態
        tomatoCollected = false;
        oneUpCollected = false;

        // 💡 呼叫 startGame()。因為上面執行過 clearStage()，
        // startGame 內部重新 loadStage(1) 時，會幫你自動生成一隻全新、滿血滿命的卡比！
        g->startGame();
    }
    else {
        // 選擇 gameover(2) 按 Enter -> 回到 start 主選單畫面
        tomatoCollected = false;
        oneUpCollected = false;
        g->restartFromMenu();
    }
}

// ============ Clear 畫面 ============
void GameScene::showClearScreen() {
    clearStage();
    setSceneRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    clearStep = 0; // 剛進通關畫面，設為步數 0

    // 載入第一張通關圖片 CLEAR.jpg
    QPixmap clearPix = loadPix(":/Dataset/background/CLEAR.png");
    clearPix = clearPix.scaled(WINDOW_WIDTH, WINDOW_HEIGHT, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    clearBg = new QGraphicsPixmapItem(clearPix);
    clearBg->setPos(0, 0);
    clearBg->setZValue(0);
    addItem(clearBg);
}

// 🌟 新增：Enter 推進通關畫面的核心處理函數
void GameScene::advanceClearScreen(Game *g) {
    if (clearStep == 0) {
        // 目前是 CLEAR.jpg -> 切換成 last1.jpg
        QPixmap pix1 = loadPix(":/Dataset/background/last1.png");
        pix1 = pix1.scaled(WINDOW_WIDTH, WINDOW_HEIGHT, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if (clearBg) {
            clearBg->setPixmap(pix1);
        }
        clearStep = 1;
    }
    else if (clearStep == 1) {
        // 目前是 last1.jpg -> 切換成 last2.jpg
        QPixmap pix2 = loadPix(":/Dataset/background/last2.png");
        pix2 = pix2.scaled(WINDOW_WIDTH, WINDOW_HEIGHT, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if (clearBg) {
            clearBg->setPixmap(pix2);
        }
        clearStep = 2;
    }
    else if (clearStep == 2) {
        // 目前是 last2.jpg -> 再按一次 Enter 直接關閉視窗結束遊戲
        qApp->quit();
    }
}

#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QSet>
#include <QList>
#include "constants.h"

class Game;
class Kirby;
class Enemy;
class Projectile;

// 場景物件結構
struct Block {
    QGraphicsPixmapItem *item;
    QRectF rect;
};

struct Platform {
    QGraphicsPixmapItem *item;
    QRectF rect;
};

class GameScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit GameScene(Game *game);
    ~GameScene();

    void loadStage(int stageNum, bool isNewGame = false);
    void clearStage();
    void updateGame(const QSet<int> &keys);

    // 畫面切換
    void showMenuScreen();
    void showGameOverScreen();
    void showClearScreen();
    void advanceClearScreen(Game *game);
    void changeGameOverSelection(int key);
    void confirmGameOverSelection(Game *game);

private:
    Game *game;
    int currentStage;

    // 遊戲物件
    Kirby *kirby;
    QList<Enemy*> enemies;
    QList<Projectile*> projectiles;
    QList<Block> blocks;
    QList<Platform> platforms;

    // 場景物件
    QGraphicsPixmapItem *portal;
    QGraphicsPixmapItem *goalDoor;
    QRectF portalRect, goalRect;
    bool hasPortal, hasGoal;

    // 道具
    QGraphicsPixmapItem *maximTomato;
    QGraphicsPixmapItem *oneUpItem;
    QRectF tomatoRect, oneUpRect;
    bool tomatoCollected, oneUpCollected;

    // Hole 區域
    QList<QRectF> holes;

    // 背景
    QList<QGraphicsPixmapItem*> bgItems;

    // 地板
    QList<QGraphicsPixmapItem*> floorTiles;
    QList<QRectF> floorRects;

    // HUD
    QList<QGraphicsPixmapItem*> hpIcons;
    QList<QGraphicsPixmapItem*> lifeIcons;
    QGraphicsPixmapItem *abilityIcon;
    QPixmap hpFullPix, hpEmptyPix;
    QGraphicsPixmapItem *lifeIconItem;
    QPixmap live0Pix, live1Pix, live2Pix; // 🌟 新增：生命狀態圖片
    QPixmap fireBoard, sparkBoard, cutterBoard,BeamBoard;

    // 選單/結束畫面
    QGraphicsPixmapItem *menuBg;
    QGraphicsPixmapItem *gameOverBg;
    QGraphicsPixmapItem *clearBg;
    int gameOverSelection; // 0=Continue, 1=Quit
    QPixmap gameOverContinuePix, gameOverQuitPix;

    // 死亡/重生計時
    int deathTimer;
    int clearStep;

    // 圖片載入
    QPixmap loadPix(const QString &path);
    QPixmap loadAndScale(const QString &path);

    // 更新子系統
    void updateKirby(const QSet<int> &keys);
    void updateEnemies();
    void updateProjectiles();
    void checkCollisions();
    void scrollCamera();
    void updateHUD();

    // 碰撞輔助
    void checkKirbyTerrainCollision();
    void checkEnemyTerrainCollision(Enemy *e);
    void checkKirbyEnemyCollision();
    void checkKirbyItemCollision();
    void checkKirbyPortalGoal(const QSet<int> &keys);
    void checkProjectileCollisions();
    void checkKirbyHole();

    // 關卡配置
    void setupStage1();
    void setupStage2();
    void setupStage3();
    void setupStage4();
    void addFloor(double x, double y, int count);
    void addBlock(double x, double y);
    void addPlatform(double x, double y, int width);
    void addHole(double x, double y, double w, double h);
    Enemy* addEnemy(EnemyType type, double x, double y, double pMin, double pMax);
    void setupHUD();
};

#endif // GAMESCENE_H

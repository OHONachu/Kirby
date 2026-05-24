#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "physics.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // 1. Set window size to exactly 1620 x 1080
    this->setFixedSize(1620, 1080);
    this->setWindowTitle("Kirby's Adventure");

    // 2. Load HUD assets
    hpFullPixmap.load(":/Image/item/HP_1.png");
    hpEmptyPixmap.load(":/Image/item/HP_0.png");
    lifeIconPixmap.load(":/Image/item/life.png");

    // 3. Initialize state machines
    currentState = STATE_MENU;
    currentStage = nullptr;
    kirby = nullptr;
    cameraX = 0;
    gameOverSelection = 0;

    // 4. Start high-performance Game Loop Timer (approx. 60 FPS)
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::updateGame);
    gameTimer->start(16);
}

MainWindow::~MainWindow() {
    delete ui;
    if (currentStage) delete currentStage;
    if (kirby) delete kirby;
    qDeleteAll(projectiles);
}

void MainWindow::resetGame() {
    if (currentStage) {
        delete currentStage;
        currentStage = nullptr;
    }
    if (kirby) {
        delete kirby;
        kirby = nullptr;
    }
    qDeleteAll(projectiles);
    projectiles.clear();

    // Spawn Kirby with 3 HP and 3 Lives
    kirby = new Kirby(100, 830);
    kirby->lives = 3;
    kirby->hp = 3;
    kirby->state = K_NORMAL;

    loadStage(1);
}

void MainWindow::loadStage(int stageNum) {
    // Keep old stats
    int oldLives = 3;
    int oldHp = 3;
    KirbyState oldState = K_NORMAL;

    if (kirby) {
        oldLives = kirby->lives;
        oldHp = kirby->hp;
        oldState = kirby->state;
    }

    if (currentStage) {
        delete currentStage;
    }
    qDeleteAll(projectiles);
    projectiles.clear();

    cameraX = 0;
    currentStage = new Stage(stageNum);
    currentStage->loadLayout();

    // Respawn Kirby at stage start (ground x=100, y=830)
    if (kirby) delete kirby;
    kirby = new Kirby(100, 830);
    kirby->lives = oldLives;
    kirby->hp = oldHp;
    kirby->state = oldState;

    if (stageNum == 1) {
        currentState = STATE_STAGE1;
    } else {
        currentState = STATE_STAGE2;
    }
}

void MainWindow::handleInhaleAndCollisions() {
    if (!kirby || !currentStage) return;

    // --- 1. SUCKING / INHALING LOGIC ---
    if (kirby->state == K_INHALING) {
        QRectF inhaleBox = kirby->getInhaleRange();
        for (Enemy* enemy : currentStage->enemies) {
            if (enemy->active && enemy->isSuckable()) {
                if (Physics::checkOverlapF(inhaleBox, enemy->rect)) {
                    // Sucked!
                    enemy->active = false;
                    kirby->state = K_MOUTHFUL;
                    kirby->action = A_IDLE;
                    kirby->hasSuckedEnemy = true;
                    kirby->suckedEnemyType = enemy->type;
                    kirby->stopInhale();
                    break;
                }
            }
        }
    }

    // --- 2. PROJECTILE & MELEE ATTACKS LOGIC ---
    // If Kirby Spark is actively shocking, test melee collision box
    if (kirby->state == K_SPARK && kirby->action == A_ATTACKING) {
        // Shock barrier box: surrounding Kirby
        QRectF shockBox(kirby->rect.x() - 50, kirby->rect.y() - 50, kirby->rect.width() + 100, kirby->rect.height() + 100);
        for (Enemy* enemy : currentStage->enemies) {
            if (enemy->active && enemy->type != E_GORDO) {
                if (Physics::checkOverlapF(shockBox, enemy->rect)) {
                    enemy->active = false; // Defeated
                }
            }
        }
    }

    // Handle projectile collisions (e.g. Star Spit, Fire breath)
    for (Entity* proj : projectiles) {
        if (!proj->active) continue;

        // SpitStar and FireBreath damage normal enemies
        bool isKirbyProjectile = (dynamic_cast<SpitStar*>(proj) != nullptr) || (dynamic_cast<FireBreath*>(proj) != nullptr);
        if (isKirbyProjectile) {
            for (Enemy* enemy : currentStage->enemies) {
                if (enemy->active && enemy->type != E_GORDO) {
                    if (Physics::checkOverlapF(proj->rect, enemy->rect)) {
                        enemy->active = false; // Enemy defeated!
                        if (dynamic_cast<SpitStar*>(proj)) {
                            proj->active = false; // Star pops
                        }
                    }
                }
            }
        }

        // FireBall (from Hot Head) damages Kirby
        if (dynamic_cast<FireBall*>(proj)) {
            if (Physics::checkOverlapF(proj->rect, kirby->rect)) {
                if (kirby->invincibleTicks <= 0) {
                    kirby->handleDamage();
                    proj->active = false;
                }
            }
        }
    }

    // --- 3. ITEM COLLECTIONS ---
    for (Item* item : currentStage->items) {
        if (item->active) {
            if (Physics::checkOverlapF(kirby->rect, item->rect)) {
                item->active = false;
                if (item->type == Item::TOMATO) {
                    kirby->hp = 3; // Refills full HP
                } else if (item->type == Item::ONE_UP) {
                    kirby->lives = qMin(3, kirby->lives + 1); // 1UP
                }
            }
        }
    }

    // --- 4. KIRBY-ENEMY DIRECT DAMAGE ---
    for (Enemy* enemy : currentStage->enemies) {
        if (enemy->active) {
            if (Physics::checkOverlapF(kirby->rect, enemy->rect)) {
                // Gordo is invincible and hurts always, normal enemies only hurt if not invulnerable
                if (kirby->invincibleTicks <= 0) {
                    kirby->handleDamage();
                }
            }
        }
    }
}

void MainWindow::updateGame() {
    if (currentState == STATE_STAGE1 || currentState == STATE_STAGE2) {
        if (!kirby || !currentStage) return;

        // 1. Update Kirby using input keys
        kirby->triggerAction(keysPressed);
        
        // Handle Fire Breath attack triggers
        if (kirby->state == K_FIRE && keysPressed.value(Qt::Key_X) && !kirby->isAttacking) {
            kirby->action = A_ATTACKING;
            kirby->isAttacking = true;
            kirby->attackTicks = 45; // Fires for 45 ticks
            
            // Spawn Fire breath particles
            projectiles.append(new FireBreath(kirby->rect.x() + (kirby->dir == RIGHT ? 60 : -60), kirby->rect.y() + 10, kirby->dir));
        }

        // Handle Spit Star trigger
        if (kirby->state == K_MOUTHFUL && keysPressed.value(Qt::Key_X)) {
            projectiles.append(new SpitStar(kirby->rect.x() + (kirby->dir == RIGHT ? 60 : -40), kirby->rect.y() + 10, kirby->dir));
            kirby->state = K_NORMAL;
            kirby->action = A_IDLE;
        }

        // Update Kirby physics
        kirby->update(currentStage->solids, currentStage->platforms);

        // 2. Camera Tracking
        if (kirby->rect.center().x() > cameraX + 810.0) {
            cameraX = kirby->rect.center().x() - 810.0;
        } else if (kirby->rect.center().x() < cameraX + 810.0) {
            cameraX = kirby->rect.center().x() - 810.0;
        }
        
        // Clamp Camera Offset
        cameraX = qBound(0.0, cameraX, currentStage->totalWidth - 1620.0);

        // Keep Kirby inside viewport limits
        if (kirby->rect.left() < cameraX) {
            kirby->rect.moveLeft(cameraX);
        }
        if (kirby->rect.right() > currentStage->totalWidth) {
            kirby->rect.moveRight(currentStage->totalWidth);
        }

        // 3. Update active enemies
        for (Enemy* enemy : currentStage->enemies) {
            if (enemy->active) {
                enemy->update(currentStage->solids, currentStage->platforms);
                
                // Let Hot Head run active attack AI
                if (enemy->type == E_HOT_HEAD) {
                    static_cast<HotHead*>(enemy)->handleAttackAI(kirby->rect.x(), kirby->rect.y(), projectiles);
                }
            }
        }

        // 4. Update items
        for (Item* item : currentStage->items) {
            if (item->active) {
                item->update(currentStage->solids, currentStage->platforms);
            }
        }

        // 5. Update projectiles
        for (int i = projectiles.size() - 1; i >= 0; i--) {
            Entity* proj = projectiles[i];
            if (proj->active) {
                proj->update(currentStage->solids, currentStage->platforms);
            } else {
                delete proj;
                projectiles.removeAt(i);
            }
        }

        // 6. Handle collisions and sucking mechanics
        handleInhaleAndCollisions();

        // 7. Check death conditions
        if (kirby->hp <= 0 || kirby->rect.top() > 1080) {
            kirby->lives--;
            if (kirby->lives <= 0) {
                currentState = STATE_GAME_OVER;
                gameOverSelection = 0;
            } else {
                // Respawn at current stage start
                loadStage(currentStage->stageNum);
            }
        }

        // 8. Stage transitions
        if (currentState == STATE_STAGE1) {
            if (Physics::checkOverlapF(kirby->rect, QRectF(currentStage->portalRect)) && keysPressed.value(Qt::Key_Up)) {
                loadStage(2); // Cutscene/Teleport to Stage 2
            }
        } else if (currentState == STATE_STAGE2) {
            if (Physics::checkOverlapF(kirby->rect, QRectF(currentStage->goalRect)) && keysPressed.value(Qt::Key_Up)) {
                currentState = STATE_CLEAR; // Beat the game!
            }
        }
    }

    update();
}

void MainWindow::drawHUD(QPainter& painter) {
    if (!kirby) return;

    // --- 1. Draw HUD Background plate ---
    // Drawn at y = 920 to 1080 (HUD overlay bar height = 160px)
    painter.fillRect(0, 920, 1620, 160, QColor(0, 0, 50)); // Dark Indigo plate

    // Draw the copying ability board texture
    QPixmap boardPixmap;
    if (kirby->state == K_FIRE) {
        boardPixmap = kirby->fireBoardPixmap;
    } else if (kirby->state == K_SPARK) {
        boardPixmap = kirby->sparkBoardPixmap;
    } else {
        boardPixmap = kirby->normalBoardPixmap;
    }

    if (!boardPixmap.isNull()) {
        painter.drawPixmap(60, 940, 300, 120, boardPixmap); // Left aligned board
    }

    // --- 2. Draw HP Bars ---
    // Draw 3 heart containers. Filled or empty hearts.
    painter.setPen(Qt::white);
    QFont hudFont = painter.font();
    hudFont.setPointSize(24);
    hudFont.setBold(true);
    painter.setFont(hudFont);
    painter.drawText(500, 1000, "HP:");

    int heartStartX = 580;
    for (int i = 0; i < 3; i++) {
        QPixmap hpHeart = (i < kirby->hp) ? hpFullPixmap : hpEmptyPixmap;
        painter.drawPixmap(heartStartX + (i * 70), 965, 60, 60, hpHeart);
    }

    // --- 3. Draw Lives ---
    painter.drawText(1000, 1000, "LIVES:");
    painter.drawPixmap(1130, 960, 60, 60, lifeIconPixmap);
    painter.drawText(1210, 1000, QString("x %1").arg(kirby->lives));
}

void MainWindow::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (currentState == STATE_MENU) {
        // Draw standard menu background
        QPixmap startBg(":/Image/background/start.png");
        if (!startBg.isNull()) {
            painter.drawPixmap(0, 0, 1620, 1080, startBg);
        } else {
            painter.fillRect(this->rect(), Qt::black);
        }

        // Draw blinking blinking text
        static int menuTick = 0;
        menuTick++;
        if ((menuTick / 30) % 2 == 0) {
            painter.setPen(Qt::white);
            QFont font = painter.font();
            font.setPointSize(36);
            font.setBold(true);
            painter.setFont(font);
            painter.drawText(this->rect(), Qt::AlignCenter, "\n\n\n\n\n\nPress ENTER to Start");
        }

    } else if (currentState == STATE_STAGE1 || currentState == STATE_STAGE2) {
        if (!currentStage || !kirby) return;

        // 1. Draw tiled backgrounds & scenery floors
        currentStage->draw(painter, cameraX);

        // 2. Draw projectiles
        for (Entity* proj : projectiles) {
            if (proj->active) {
                proj->draw(painter, cameraX);
            }
        }

        // 3. Draw Kirby
        kirby->draw(painter, cameraX);

        // 4. Draw HUD overlays
        drawHUD(painter);

    } else if (currentState == STATE_GAME_OVER) {
        // Draw selection-specific Game Over plates
        QPixmap gameOverBg;
        if (gameOverSelection == 0) {
            gameOverBg.load(":/Image/background/game_over_continue.png");
        } else {
            gameOverBg.load(":/Image/background/game_over_quit.png");
        }

        if (!gameOverBg.isNull()) {
            painter.drawPixmap(0, 0, 1620, 1080, gameOverBg);
        } else {
            painter.fillRect(this->rect(), Qt::black);
            painter.setPen(Qt::red);
            QFont font = painter.font();
            font.setPointSize(50);
            painter.setFont(font);
            painter.drawText(this->rect(), Qt::AlignCenter, "GAME OVER\n\nContinue    Quit");
        }

    } else if (currentState == STATE_CLEAR) {
        // Draw visual premium Stage Clear celebrate screen
        painter.fillRect(this->rect(), QColor(255, 182, 193)); // Light pink canvas

        painter.setPen(Qt::darkMagenta);
        QFont font = painter.font();
        font.setPointSize(60);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(this->rect(), Qt::AlignCenter, "STAGE CLEAR!\n\nCONGRATULATIONS!\n\nPress ENTER for Menu");
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    keysPressed[event->key()] = true;

    if (currentState == STATE_MENU) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            resetGame();
        }
    } else if (currentState == STATE_GAME_OVER) {
        // Toggle continue vs quit options using arrow keys
        if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Up) {
            gameOverSelection = 0; // Continue
        }
        if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Down) {
            gameOverSelection = 1; // Quit
        }

        // Press ENTER to choose option
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_X) {
            if (gameOverSelection == 0) {
                currentState = STATE_MENU; // Go to start menu
            } else {
                this->close(); // Exit application
            }
        }
    } else if (currentState == STATE_CLEAR) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            currentState = STATE_MENU;
        }
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    keysPressed[event->key()] = false;
}

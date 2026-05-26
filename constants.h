#ifndef CONSTANTS_H
#define CONSTANTS_H

// ============ 視窗與畫面 ============
const int WINDOW_WIDTH  = 1620;
const int WINDOW_HEIGHT = 1080;
const int FRAME_WIDTH   = 1620;
const int STAGE1_FRAMES = 3;
const int STAGE2_FRAMES = 5;

// ============ 圖片縮放 ============
const int SPRITE_SCALE = 2;

// ============ 物理參數 ============
const double GRAVITY         = 0.5;
const double MAX_FALL_SPEED  = 10.0;
const double KIRBY_SPEED     = 5.0;
const double KIRBY_JUMP_FORCE = -12.0;
const double KIRBY_HOVER_FORCE = -3.0;

// ============ 地板位置 ============
const int FLOOR_Y = 950;

// ============ 生命系統 ============
const int KIRBY_MAX_HP    = 3;
const int KIRBY_MAX_LIVES = 3;
const int INVINCIBLE_DURATION = 120; // 2秒 (60fps × 2)

// ============ 攻擊參數 ============
const double INHALE_RANGE     = 135.0 * SPRITE_SCALE;
const double SPIT_STAR_SPEED  = 10.0;
const int    FIRE_DURATION    = 45;
const int    SPARK_DURATION   = 45;

// ============ 敵人參數 ============
const double ENEMY_SPEED          = 2.0;
const double HOTHEAD_FIRE_SPEED   = 6.0;
const int    HOTHEAD_FIRE_INTERVAL = 180;
const double SPARKY_JUMP_FORCE    = -10.0;
const int    SPARKY_SPARK_DURATION = 60;

// ============ Game Loop ============
const int GAME_TIMER_INTERVAL = 16; // ~60fps

// ============ 遊戲狀態 ============
enum GameState {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_CLEAR
};

// ============ Kirby 狀態 ============
enum KirbyState {
    KIRBY_NORMAL,
    KIRBY_WALKING,
    KIRBY_JUMPING,
    KIRBY_HOVERING,
    KIRBY_SQUATTING,
    KIRBY_INHALING,
    KIRBY_MOUTHFUL,
    KIRBY_ATTACKING,
    KIRBY_DAMAGED,
    KIRBY_DEAD
};

// ============ 能力類型 ============
enum AbilityType {
    ABILITY_NONE,
    ABILITY_FIRE,
    ABILITY_SPARK
};

// ============ 敵人類型 ============
enum EnemyType {
    ENEMY_WADDLE_DEE,
    ENEMY_GORDO,
    ENEMY_HOT_HEAD,
    ENEMY_SPARKY
};

// ============ 投射物類型 ============
enum ProjectileType {
    PROJ_SPIT_STAR,
    PROJ_FIREBALL,
    PROJ_FIRE_BREATH,
    PROJ_SPARK_FIELD
};

#endif // CONSTANTS_H

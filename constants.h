#ifndef CONSTANTS_H
#define CONSTANTS_H

// ============ 視窗與畫面 ============
const int WINDOW_WIDTH  = 1620;
const int WINDOW_HEIGHT = 1080;
const int FRAME_WIDTH   = 1620;
const int STAGE1_FRAMES = 3;
const int STAGE2_FRAMES = 5;
const int STAGE3_FRAMES = 3;
const int STAGE4_FRAMES = 3;

// ============ 圖片縮放 ============
const int SPRITE_SCALE = 1;
const int CK_W = 150;
const int CK_H = 150;
const int CKW_W = 150;  // 回力鏢小一點
const int CKW_H = 150;
const int WD_W = 150;
const int WD_H = 150;
const int BEAM_W = 230;
const int BEAM_H = 230;


// ============ 物理參數 ============
const double GRAVITY         = 0.25;//重力
const double MAX_FALL_SPEED  = 10.0;//終端速度
const double KIRBY_SPEED     = 5.0;//水平速度
const double KIRBY_JUMP_FORCE = -12.0;//往上跳
const double KIRBY_HOVER_FORCE = -6.0;//飛行

// ============ 地板位置 ============
const int FLOOR_Y = 950;//地板

// ============ 生命系統 ============
const int KIRBY_MAX_HP    = 3;//最大血量
const int KIRBY_MAX_LIVES = 3;//最大生命
const int INVINCIBLE_DURATION = 120; // 2秒 (60fps × 2)

// ============ 攻擊參數 ============
const double INHALE_RANGE     = 135.0 * SPRITE_SCALE;
const double SPIT_STAR_SPEED  = 10.0;
const int    FIRE_DURATION    = 45;
const int    SPARK_DURATION   = 45;
const int    CUTTER_DURATION   = 300;
const int    BEAM_DURATION   = 300;

// ============ 敵人參數 ============
const double ENEMY_SPEED          = 2.0;
const double HOTHEAD_FIRE_SPEED   = 6.0;
const int    HOTHEAD_FIRE_INTERVAL = 210;
const double SPARKY_JUMP_FORCE    = -10.0;
const int    SPARKY_SPARK_DURATION = 60;

// ============ Game Loop ============
const int GAME_TIMER_INTERVAL = 16; // ~60fps

// ============ 遊戲狀態 ============
enum GameState {
    STATE_MENU,// 主選單
    STATE_PLAYING,// 遊玩中
    STATE_GAME_OVER,// 遊戲結束
    STATE_CLEAR// 過關畫面
};

// ============ Kirby 狀態 ============
enum KirbyState {
    KIRBY_NORMAL,// 普通狀態
    KIRBY_WALKING,// 走路
    KIRBY_JUMPING,// 跳躍
    KIRBY_HOVERING,// 飛行 (充氣)
    KIRBY_SQUATTING,// 蹲下
    KIRBY_INHALING,// 正在吸氣
    KIRBY_MOUTHFUL,// 嘴裡含著敵人 (飽足狀態)
    KIRBY_ATTACKING,// 使用能力攻擊中
    KIRBY_DAMAGED,// 受傷無敵狀態
    KIRBY_DEAD// 死亡
};

// ============ 能力類型 ============
enum AbilityType {
    ABILITY_NONE,// 沒有能力
    ABILITY_FIRE,// 火焰能力
    ABILITY_SPARK,// 電擊能力
    ABILITY_CUTTER,// Cutter Knight
    ABILITY_BEAM
};

// ============ 敵人類型 ============
enum EnemyType {
    ENEMY_WADDLE_DEE,
    ENEMY_GORDO,// 海膽怪 (刺刺的那個)
    ENEMY_HOT_HEAD,// 火炎頭
    ENEMY_SPARKY,// 電擊怪
    ENEMY_KNIGHT,
    ENEMY_WADDLE_DOO
};

// ============ 投射物類型 ============
enum ProjectileType {
    PROJ_SPIT_STAR,// 吐出的星星
    PROJ_FIREBALL,// 敵人吐的火球
    PROJ_FIRE_BREATH,// 卡比的噴火攻擊
    PROJ_SPARK_FIELD,// 卡比/敵人的電擊場
    PROJ_CUTTER
};

#endif // CONSTANTS_H

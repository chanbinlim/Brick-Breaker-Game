#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <iostream>
#include <string>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 20;
const int BALL_SIZE = 15;
const int BRICK_WIDTH = 60;
const int BRICK_HEIGHT = 20;
const int BRICK_ROWS = 5;
const int BRICK_COLUMNS = 10;

struct Brick {
    SDL_Rect rect;
    bool destroyed;
};

class Game {
public:
    Game();
    ~Game();
    bool init();
    void run();
    void close();

private:
    void handleEvents();
    void update();
    void render();
    void resetBall();
    void gameOver();
    void renderText(const std::string& text, int x, int y, SDL_Color color);

    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Rect paddle;
    SDL_Rect ball;
    int ballVelX, ballVelY;
    std::vector<Brick> bricks;
    bool quit;
    int score;
    bool isGameOver;
};

Game::Game() : window(nullptr), renderer(nullptr), font(nullptr), ballVelX(5), ballVelY(-5), quit(false), score(0), isGameOver(false) {
    paddle = {WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2, WINDOW_HEIGHT - 40, PADDLE_WIDTH, PADDLE_HEIGHT};
    ball = {WINDOW_WIDTH / 2 - BALL_SIZE / 2, WINDOW_HEIGHT / 2 - BALL_SIZE / 2, BALL_SIZE, BALL_SIZE};
}

Game::~Game() {
    close();
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Brick Breaker", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return false;
    }

    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24); // 실제 폰트 파일 경로로 변경
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return false;
    }

    for (int i = 0; i < BRICK_ROWS; ++i) {
        for (int j = 0; j < BRICK_COLUMNS; ++j) {
            Brick brick = {{j * (BRICK_WIDTH + 10) + 35, i * (BRICK_HEIGHT + 10) + 50, BRICK_WIDTH, BRICK_HEIGHT}, false};
            bricks.push_back(brick);
        }
    }

    return true;
}

void Game::run() {
    while (!quit) {
        handleEvents();
        update();
        render();
        SDL_Delay(16); // ~60 FPS
    }
}

void Game::close() {
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_LEFT:
                    paddle.x -= 20;
                    if (paddle.x < 0) paddle.x = 0;
                    break;
                case SDLK_RIGHT:
                    paddle.x += 20;
                    if (paddle.x + PADDLE_WIDTH > WINDOW_WIDTH) paddle.x = WINDOW_WIDTH - PADDLE_WIDTH;
                    break;
            }
        }
    }
}

void Game::update() {
    if (isGameOver) return;

    ball.x += ballVelX;
    ball.y += ballVelY;

    if (ball.x <= 0 || ball.x + BALL_SIZE >= WINDOW_WIDTH) {
        ballVelX = -ballVelX;
    }
    if (ball.y <= 0) {
        ballVelY = -ballVelY;
    }
    if (ball.y + BALL_SIZE >= WINDOW_HEIGHT) {
        gameOver();
    }

    if (SDL_HasIntersection(&ball, &paddle)) {
        ballVelY = -ballVelY;
        ball.y = paddle.y - BALL_SIZE;
    }

    for (auto& brick : bricks) {
        if (!brick.destroyed && SDL_HasIntersection(&ball, &brick.rect)) {
            brick.destroyed = true;
            ballVelY = -ballVelY;
            score++;
            break;
        }
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(renderer, &paddle);

    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderFillRect(renderer, &ball);

    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
    for (const auto& brick : bricks) {
        if (!brick.destroyed) {
            SDL_RenderFillRect(renderer, &brick.rect);
        }
    }

    SDL_Color white = {255, 255, 255, 255};
    renderText("Score: " + std::to_string(score), 10, 10, white);

    if (isGameOver) {
        renderText("Game Over", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 20, white);
    }

    SDL_RenderPresent(renderer);
}

void Game::resetBall() {
    ball.x = WINDOW_WIDTH / 2 - BALL_SIZE / 2;
    ball.y = WINDOW_HEIGHT / 2 - BALL_SIZE / 2;
    ballVelX = 5;
    ballVelY = -5;
}

void Game::gameOver() {
    isGameOver = true;
}

void Game::renderText(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dstrect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

int main(int argc, char* argv[]) {
    Game game;
    if (!game.init()) {
        std::cerr << "Failed to initialize game." << std::endl;
        return 1;
    }

    game.run();
    return 0;
}

#include "Game.hpp"
#include "EntityComponentSystem/Components/ColliderComponent.hpp"
#include "EntityComponentSystem/Components/KeyboardInput.hpp"
#include "EntityComponentSystem/Components/TextureComponent.hpp"
#include "EntityComponentSystem/Components/TransformComponent.hpp"
#include "EntityComponentSystem/ECS.hpp"
#include "SDL_image.h"
#include "TileMap.hpp"

#include <cassert>
#include <iostream>

TileMap *tileMap;

ECS ecs;
Entity &rock = ecs.addEntity();
Entity &player = ecs.addEntity();

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;

Game::Game() {
    this->isRunning = false;
    this->window = nullptr;
}

Game::~Game() {
    this->clean();
}

void Game::init(const std::string &title, int x_pos, int y_pos, int width, int height, bool fullscreen) {
    int sdlWindowFlags = SDL_WINDOW_SHOWN;
    if (fullscreen) {
        sdlWindowFlags |= SDL_WINDOW_FULLSCREEN;
    }

    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        std::cerr << "[INFO] Subsystems initialized!" << '\n';
    } else {
        std::cerr << "[ERROR] There was a problem initializing subsystems!" << '\n' << SDL_GetError();
        exit(1);
    }

    if (IMG_Init(IMG_INIT_PNG) != 0) {
        std::cerr << "[INFO] SDL_image initialized!" << '\n';
    } else {
        std::cerr << "[ERROR] There was a problem initializing SDL_image!" << '\n' << SDL_GetError();
        exit(1);
    }

    this->window = SDL_CreateWindow(title.c_str(), x_pos, y_pos, width, height, sdlWindowFlags);
    if (this->window != nullptr) {
        std::cerr << "[INFO] Window created!" << '\n';
    } else {
        std::cerr << "[ERROR] There was a problem creating window!" << '\n' << SDL_GetError();
        exit(1);
    }

    Game::renderer = SDL_CreateRenderer(window, -1, 0);
    if (Game::renderer != nullptr) {
        std::cerr << "[INFO] Renderer created!" << '\n';
    } else {
        std::cerr << "[ERROR] There was a problem creating renderer!" << '\n' << SDL_GetError();
        exit(1);
    }

    isRunning = true;

    // SDL_RenderSetLogicalSize(Game::renderer, width / 2, height / 2);
    SDL_SetRenderDrawColor(Game::renderer, 0, 0, 0, 255);

    auto &playerTransform = player.addComponent<TransformComponent>(500, 250, 24, 24);
    playerTransform.scale = 2;
    player.addComponent<TextureComponent>("assets/player/IDLES_5_frames.png", playerTransform);
    player.addComponent<KeyboardInput>(playerTransform);
    player.addComponent<ColliderComponent>(playerTransform, 12, 20, 6, 2);

    auto &rockTransform = rock.addComponent<TransformComponent>(544, 288, 16, 16);
    rockTransform.scale = 2;
    rock.addComponent<TextureComponent>("assets/tiles/seasonal sample (autumn).png", rockTransform, 64, 128);
    rock.addComponent<ColliderComponent>(rockTransform);

    if (player.hasComponents<TransformComponent, TextureComponent, KeyboardInput, ColliderComponent>()) {
        std::cout << "[INFO] Player has been initialized." << '\n';
    } else {
        std::cout << "[ERROR] Player doesn't have all the components." << '\n';
        exit(1);
    }

    tileMap = new TileMap(16);
}

void Game::handleEvents() {
    SDL_PollEvent(&Game::event);

    switch (Game::event.type) {
    case SDL_QUIT:
        isRunning = false;
        break;
    default:
        break;
    }
}

void Game::update() {
    ecs.update();
    if (player.getComponent<TransformComponent>() != nullptr) {
        if (player.getComponent<TransformComponent>()->position.y > 300)
            player.getComponent<TextureComponent>()->setSrcX(24 * 10); // 11th sprite in the current spritesheet
        else
            player.getComponent<TextureComponent>()->setSrcX(0); // 1st sprite in the current spritesheet
    }

    if (player.getComponent<ColliderComponent>() != nullptr && rock.getComponent<ColliderComponent>() != nullptr) {
        if (ColliderComponent::Collision(&player.getComponent<ColliderComponent>()->collider,
                                         &rock.getComponent<ColliderComponent>()->collider)) {
            player.getComponent<TransformComponent>()->scale = 1;
            std::cout << "Colliding with rock!" << '\n';
        }
    }
}

void Game::render() {
    SDL_RenderClear(Game::renderer);

    tileMap->render();
    ecs.render();
    player.getComponent<ColliderComponent>()->debug();
    rock.getComponent<ColliderComponent>()->debug();

    SDL_RenderPresent(Game::renderer);
}

void Game::clean() {
    SDL_DestroyWindow(this->window);
    SDL_DestroyRenderer(Game::renderer);
    SDL_Quit();
    std::cerr << "[INFO] Game cleaned!" << '\n';
}

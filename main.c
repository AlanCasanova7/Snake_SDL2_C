#define SDL_MAIN_HANDLED
#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

typedef struct apple{
    int x;
    int y;
} Apple_t;

typedef struct snake_segment{
    int x;
    int y;
    struct snake_segment *next_segment;
} Snake_Segment_t;

const SDL_Color BLACK = { .r = 0, .g = 0, .b = 0, .a = 255 };
const SDL_Color WHITE = { .r = 255, .g = 255, .b = 255, .a = 255 };
const SDL_Color RED = { .r = 255, .g = 0, .b = 0, .a = 255 };
const SDL_Color MAROON =  { .r = 128, .g = 0, .b = 0, .a = 255 };
const SDL_Color BLUE = { .r = 0, .g = 0, .b = 255, .a = 255 };
const SDL_Color NAVY = { .r = 0, .g = 0, .b = 128, .a = 255 };
const SDL_Color LIME = { .r = 0, .g = 255, .b = 0, .a = 255 };
const SDL_Color GREEN = { .r = 0, .g = 128, .b = 0, .a = 255 };

const int screen_width = 640;
const int screen_height = 480;
const int cell_size = 16;

Apple_t *magic_apple = NULL;

Snake_Segment_t *snake_head = NULL;
int snake_lenght;
char snake_horizontal;
char snake_vertical;

#define ERROR_CHECK(string, error, returnValue) \
        SDL_Log(string, error); \
        return returnValue;
#define SDL_KEY_PRESSED event.key.keysym.sym

void init(){
    magic_apple = (Apple_t *)malloc(sizeof(Apple_t));
    magic_apple->x = 0;
    magic_apple->y = 0;

    snake_head = (Snake_Segment_t *)malloc(sizeof(Snake_Segment_t));
    snake_horizontal = 1;
    snake_vertical = 0;
    snake_lenght = 5;
    snake_head->x = 320;
    snake_head->y = 240;
    snake_head->next_segment = (Snake_Segment_t *)malloc(sizeof(Snake_Segment_t));
    void *next = (void *)snake_head->next_segment;
    memset(next, 0, sizeof(Snake_Segment_t));
    Snake_Segment_t *last_segment = snake_head;

    for(int i = 1; i < snake_lenght; i++){
        Snake_Segment_t *new_segment = (Snake_Segment_t *)malloc(sizeof(Snake_Segment_t));
        new_segment->x = 320;
        new_segment->y = 240 + (i * cell_size);
        last_segment->next_segment = new_segment;
        last_segment = new_segment;
    }

    last_segment->next_segment = NULL;
}

void draw_snake_segments(SDL_Renderer *renderer){
    Snake_Segment_t *current_segment = snake_head;
    while (current_segment)
    {
        SDL_Rect segment = { current_segment->x, current_segment->y, cell_size, cell_size };
        SDL_SetRenderDrawColor(renderer, LIME.r, LIME.g, LIME.b, LIME.a);
        SDL_RenderFillRect(renderer, &segment);
        current_segment = current_segment->next_segment;
    }
}

void addSnakeSegment(){
    Snake_Segment_t *current_segment = snake_head;
    Snake_Segment_t *tail;
    while (current_segment)
    {
        tail = current_segment;
        current_segment = current_segment->next_segment;
    }

    Snake_Segment_t *new_segment = (Snake_Segment_t *)malloc(sizeof(Snake_Segment_t));
    new_segment->x = tail->x;
    new_segment->y = tail->y;
    tail->next_segment = new_segment;
    new_segment->next_segment = NULL;
}

void check_apple_collision(){
    if(snake_head->x == magic_apple->x && snake_head->y == magic_apple->y){
        magic_apple->x = rand() % (screen_width / cell_size);
        magic_apple->y = rand() % (screen_height / cell_size);

        magic_apple->x *= 16;
        magic_apple->y *= 16;

        addSnakeSegment();
    }
}

void check_out_of_bounds(){
    if(snake_head->x >= screen_width){
        snake_head->x = 0;
    } else if(snake_head->x < 0){
        snake_head->x = screen_width - 16;
    }

    if(snake_head->y >= screen_height){
        snake_head->y = 0;
    } else if(snake_head->y < 0){
        snake_head->y = screen_height - 16;
    }
}

int check_self_collision(){
    Snake_Segment_t *current_segment = snake_head->next_segment;
    while (current_segment)
    {
        if(snake_head->x == current_segment->x && snake_head->y == current_segment->y){
            return -1;
        }
        current_segment = current_segment->next_segment;
    }
    return 0;
}

int move_snake(){
    //Starting from the head we find the tail through the next-segment, once we find it we brake the link with his predecessor, then we move our tail
    //in front of the head then we update his next_segment to point at our head. Our head pointer also needs to be update to this new segment.
    Snake_Segment_t *current_segment = snake_head;
    Snake_Segment_t *pre_tail;
    Snake_Segment_t *tail;

    while (current_segment->next_segment)
    {
        pre_tail = current_segment;
        current_segment = current_segment->next_segment;
    }

    tail = pre_tail->next_segment;
    pre_tail->next_segment = NULL;

    tail->x = snake_head->x + (16 * snake_horizontal);
    tail->y = snake_head->y + (16 * snake_vertical);
    tail->next_segment = snake_head;
    snake_head = tail;
    check_apple_collision();
    check_out_of_bounds();
    int collided = 0;
    collided = check_self_collision();
    return collided;
}

int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        ERROR_CHECK("Unable to initialize SDL: %s", SDL_GetError(), 1);
    }
    SDL_Window *window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    // make the scaled rendering look smoother.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, screen_width, screen_height);
    
    int quit = 0;
    int counter = 0;
    init();
    while (!quit) {
        counter++;
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            if(event.type == SDL_QUIT){
                quit = 1;
            }
            if(event.type == SDL_KEYDOWN){
                if(SDL_KEY_PRESSED == SDLK_UP){
                    snake_horizontal = 0;
                    if(snake_vertical != 1){
                        snake_vertical = -1;
                    }
                }
                else if(SDL_KEY_PRESSED == SDLK_DOWN){
                    snake_horizontal = 0;
                    if(snake_vertical != -1){
                        snake_vertical = 1;
                    }
                }
                else if(SDL_KEY_PRESSED == SDLK_RIGHT){
                    snake_vertical = 0;
                    if(snake_horizontal != -1){
                        snake_horizontal = 1;
                    }
                }
                else if(SDL_KEY_PRESSED == SDLK_LEFT){
                    snake_vertical = 0;
                    if(snake_horizontal != 1){
                        snake_horizontal = -1;
                    }
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, NAVY.r, NAVY.g, NAVY.b, NAVY.a); //draw background
        SDL_RenderClear(renderer);

        SDL_Rect apple_rect = { magic_apple->x, magic_apple->y, cell_size, cell_size };
        SDL_SetRenderDrawColor(renderer, RED.r, RED.g, RED.b, RED.a);
        SDL_RenderFillRect(renderer, &apple_rect);

        draw_snake_segments(renderer);

        if(counter > 8){
            quit = move_snake();
            counter = 0;
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
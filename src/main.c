#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>

#define GAME_WIDTH 128
#define GAME_HEIGHT 128
#define OBJECT_NUMBER 128

#define max(a, b) ((a)>(b)? (a) : (b))
#define min(a, b) ((a)<(b)? (a) : (b))
#define sign(a) ((a > 0) - (a < 0))
#define deg(a) (a * (3.1416 / 180))

const Color grayPalette[4] = {
    (Color){0.0f, 0.0f, 0.0f, 255.0f},
    (Color){68.0f, 68.0f, 68.0f, 255.0f},
    (Color){190.0f, 190.0f, 190.0f, 255.0f},
    (Color){255.0f, 255.0f, 255.0f, 255.0f},
};

typedef struct Sprite {
        Vector2 position;
        Vector2 velocity;
        int pal;
        int layer;
        int spr;
} Sprite;

float dist(Vector3 v1, Vector3 v2);
int sortPalettes(const void *a, const void *b);
int sortLayers(const void *a, const void *b);

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    float scale = 5.0f;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(GAME_WIDTH * scale, GAME_HEIGHT * scale, "Gamewame");
    SetWindowMinSize(GAME_WIDTH, GAME_HEIGHT);

    Camera2D camera = { 0 };
    camera.offset = (Vector2) {0.0f, 0.0f};
    camera.target = (Vector2) {0.0f, 0.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    float timer = 0.0f;

    SetExitKey(KEY_ESCAPE);

    RenderTexture2D renderTarget = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
    RenderTexture2D finalTarget = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);

    Shader shader = LoadShader("shaders/main.vs", "shaders/main.fs");

// SHAYDAH
//-------------------------------------------------------------------------
    //Palette image
    Image paletteImage = LoadImage("resources/palette.png");

    Color *colors = (Color *) malloc((paletteImage.height*paletteImage.width)*sizeof(Color));
    colors = GetImageData(paletteImage);

    // Dynamically initialize palettes array because
    // i don't want to edit this every time I resize the palette     
    Vector3 PALETTE[paletteImage.height][paletteImage.width];

    for (int y = 0; y < paletteImage.height; y++)
    {
        for (int x = 0; x < paletteImage.width; x++)
        {
            Color color = colors[x + y * paletteImage.width];
            PALETTE[y][x].x = color.r/255.0f;
            PALETTE[y][x].y = color.g/255.0f;
            PALETTE[y][x].z = color.b/255.0f;
        }
    }

    int paletteLoc = GetShaderLocation(shader, "palette");
    SetShaderValueV(shader, paletteLoc, &PALETTE[0], UNIFORM_VEC3, paletteImage.width);

    // Free stuffe
    free(colors);
    UnloadImage(paletteImage);
//-------------------------------------------------------------------------


    Texture2D palette = LoadTexture("resources/palette.png");
    Texture2D spriteSheet = LoadTexture("resources/sprites.png");


    SetTargetFPS(60);

    Sprite sprites[OBJECT_NUMBER];

    for (int i = 0; i < OBJECT_NUMBER; i++)
    {
        sprites[i].position = (Vector2) {
            GetRandomValue(0 + 16, GAME_WIDTH - 32),
            GetRandomValue(0 + 16, GAME_HEIGHT - 32)
        };

        sprites[i].velocity = (Vector2) {
            GetRandomValue(-256, 256)/16.0f,
            GetRandomValue(-256, 256)/16.0f
        };

        sprites[i].pal = GetRandomValue(0, 7);
        sprites[i].layer = GetRandomValue(0, 7);
        sprites[i].spr = GetRandomValue(0, 3);
    }
    int lastPalette;

    qsort(sprites, OBJECT_NUMBER, sizeof(Sprite), sortPalettes);
    qsort(sprites, OBJECT_NUMBER, sizeof(Sprite), sortLayers);


    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------

        SetShaderValueV(shader, paletteLoc, &PALETTE[0], UNIFORM_VEC3, paletteImage.width);

        float dt = GetFrameTime();

        timer += dt;

        scale = min( (float)GetScreenWidth()/GAME_WIDTH, (float)GetScreenHeight()/GAME_HEIGHT );

        qsort(sprites, OBJECT_NUMBER, sizeof(Sprite), sortPalettes);
        qsort(sprites, OBJECT_NUMBER, sizeof(Sprite), sortLayers);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();


            ClearBackground(BLACK);

            BeginTextureMode(renderTarget);
            BeginShaderMode(shader);

            ClearBackground(WHITE);

            BeginMode2D(camera);
            SetShaderValueV(shader, paletteLoc, &PALETTE[0], UNIFORM_VEC3, paletteImage.width);
            DrawRectangle(0, 0, GAME_WIDTH, GAME_HEIGHT, WHITE);



            for (int i = 0; i < OBJECT_NUMBER; i++)
            {
                Sprite *sprite = &sprites[i];

                if (sprite->pal != lastPalette)
                {
                    lastPalette = sprite->pal;
                    SetShaderValueV(shader, paletteLoc, &PALETTE[sprite->pal], UNIFORM_VEC3, paletteImage.width);
                }

                DrawTextureRec(
                    spriteSheet,
                    (Rectangle){(sprite->spr % 2) * 16, (sprite->spr / 2) * 16.0f, 16.0f, 16.0f},
                    sprite->position,
                    WHITE);

                //sprite->position = Vector2Add(sprite->position, Vector2Scale(sprite->velocity, dt));
            }


            EndShaderMode();
            EndMode2D();
            
            EndTextureMode();

            // Render Texture by scanlains
            BeginTextureMode(finalTarget);
            ClearBackground(BLACK);

            float offset = 0;
            for (int i = GAME_HEIGHT; i >= 0; i--)
            {
                offset = (i % 2 == 0) ? 1 : -1;
                offset *= sin((timer * 2.0f) + (i / 16.0f)) * 16;
                DrawTexturePro(
                renderTarget.texture, 
                (Rectangle){ 0.0f + offset, (float)i + (timer * 16.0f), (float)GAME_WIDTH, 1.0f },
                (Rectangle){ 0.0f, (float)(GAME_HEIGHT - (i+1)), (float)GAME_WIDTH, 1 },
                (Vector2){ 0, 0 }, 
                0.0f, 
                WHITE);
            }
            EndTextureMode();
            ////////////////////////////////////////////////////////////////

            //Finally draw to screen
            DrawTexturePro(
                finalTarget.texture, 
                (Rectangle){ 0.0f, 0.0f, (float)renderTarget.texture.width, (float)-renderTarget.texture.height },
                (Rectangle){ (GetScreenWidth() - ((float)GAME_WIDTH*scale))*0.5, (GetScreenHeight() - ((float)GAME_HEIGHT*scale))*0.5, (float)GAME_WIDTH*scale, (float)GAME_HEIGHT*scale }, 
                (Vector2){ 0, 0 }, 
                0.0f, 
                WHITE);

            /* DrawRectangle(0, 0, 160, 80, WHITE);
            DrawFPS(10, 10);
            DrawText(TextFormat("Objects: %i", OBJECT_NUMBER), 10, 40, 20, LIME);
            DrawText(TextFormat("Time: %f", timer), 10, 60, 20, LIME); */

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}


int sortPalettes(const void *a, const void *b)
{
    return ( ((Sprite*)a)->pal - ((Sprite*)b)->pal );
}

int sortLayers(const void *a, const void *b)
{
    return ( ((Sprite*)a)->layer - ((Sprite*)b)->layer );
}

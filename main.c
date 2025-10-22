// Include Graphics Libraries
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdebug.h>
#include <pspctrl.h>

//Include Revel-Lib
#include "Revel-Lib/revel-lib.h"



// Define PSP Width / Height
#define PSP_BUF_WIDTH (512)
#define PSP_SCR_WIDTH (480)
#define PSP_SCR_HEIGHT (272)

// PSP Module Info
PSP_MODULE_INFO("Revel Lib R-Pong", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// sceGuobal variables
int running = 1;
static unsigned int __attribute__((aligned(16))) list[262144];

// Initialize Graphics
void initGraphics() {
    void* fbp0 = getStaticVramBuffer(PSP_BUF_WIDTH,PSP_SCR_HEIGHT,GU_PSM_8888);
	void* fbp1 = getStaticVramBuffer(PSP_BUF_WIDTH,PSP_SCR_HEIGHT,GU_PSM_8888);
	void* zbp = getStaticVramBuffer(PSP_BUF_WIDTH,PSP_SCR_HEIGHT,GU_PSM_4444);

	sceGuInit();

	sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_8888,fbp0,PSP_BUF_WIDTH);
	sceGuDispBuffer(PSP_SCR_WIDTH,PSP_SCR_HEIGHT,fbp1,PSP_BUF_WIDTH);
	sceGuDepthBuffer(zbp,PSP_BUF_WIDTH);
	sceGuOffset(2048 - (PSP_SCR_WIDTH/2),2048 - (PSP_SCR_HEIGHT/2));
	sceGuViewport(2048,2048,PSP_SCR_WIDTH,PSP_SCR_HEIGHT);
	sceGuDepthRange(65535,0);
	sceGuScissor(0,0,PSP_SCR_WIDTH,PSP_SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
}

// Start Frame
void startFrame() {
    sceGuStart(GU_DIRECT, list);
}

// End Frame
void endFrame() {
    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
}

// End Graphics
void termGraphics() {
    sceGuTerm();
}

int exit_callback(int arg1, int arg2, void* common){
	sceKernelExitGame();
	return 0;
}
 
int CallbackThread(SceSize args, void* argp) {
	int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
 
	return 0;
}
 
int SetupCallbacks(void) {
	int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if (thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}
	return thid;
}



int main() {
    // Boilerplate
    SetupCallbacks();

    // Initialize Graphics
    initGraphics();
    pspDebugScreenInit();
    
    // Initialize Matrices
    sceGumMatrixMode(GU_PROJECTION);
    sceGumLoadIdentity();
    sceGumOrtho(-16.0f / 9.0f, 16.0f / 9.0f, -1.0f, 1.0f, -10.0f, 10.0f);

    sceGumMatrixMode(GU_VIEW);
    sceGumLoadIdentity();

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();
    pspDebugScreenSetXY(0, 0);

    //Setup Controllers
    SceCtrlData pad;
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    //Setup Images
    Texture* PaddleTexture = load_texture("Assets/Sprites/RPong-Paddles.png", GU_TRUE); 
    if(PaddleTexture == NULL){
        goto cleanup;
    }
    Texture* NetTexture = load_texture("Assets/Sprites/RPong-Net.png", GU_TRUE); 
    if(NetTexture == NULL){
        goto cleanup;
    }
    Texture* BallTexture = load_texture("Assets/Sprites/RPong-Ball.png", GU_TRUE); 
    if(NetTexture == NULL){
        goto cleanup;
    }

    //Setup Sprites
    int walloffset = 64;
    unsigned int Color_Mods[2] = {0xFFb5aa2d,0xFF613df2};

    ScePspFVector2 paddleoffsets = {12,48};
    Sprite2D* PaddleA = create_sprite2d(PaddleTexture, Color_Mods[0], (ScePspFVector2){0,0}, (ScePspFVector2){paddleoffsets.x,paddleoffsets.y}, paddleoffsets);
    Sprite2D* PaddleB = create_sprite2d(PaddleTexture, Color_Mods[1], (ScePspFVector2){0,0}, (ScePspFVector2){paddleoffsets.x,paddleoffsets.y}, paddleoffsets);
    PaddleA->core->position.y = (PSP_SCR_HEIGHT/ 2) - (paddleoffsets.y / 2);
    PaddleB->core->position.y = (PSP_SCR_HEIGHT/ 2) - (paddleoffsets.y / 2);

    ScePspFVector2 netoffsets = {2,16};
    Sprite2D* NetA = create_sprite2d(NetTexture, Color_Mods[0], (ScePspFVector2){0,0}, (ScePspFVector2){netoffsets.x,PSP_SCR_HEIGHT}, (ScePspFVector2){2,PSP_SCR_HEIGHT});
    Sprite2D* NetB = create_sprite2d(NetTexture, Color_Mods[1], (ScePspFVector2){0,0}, (ScePspFVector2){netoffsets.x,PSP_SCR_HEIGHT}, (ScePspFVector2){2,PSP_SCR_HEIGHT});
    
    Sprite2D* Ball = create_sprite2d(BallTexture, 0xFFFFFFFF, (ScePspFVector2){0,0}, (ScePspFVector2){8,8}, (ScePspFVector2){8,8});
    Ball->core->position.x = (PSP_SCR_WIDTH / 2) - 4;
    Ball->core->position.y = (PSP_SCR_HEIGHT/ 2) - 4;


    ScePspFVector2 BallVelocity = {2.0f,1};
    float PaddleAVelocity = 0;
    float PaddleBVelocity = 0;
    float PaddleSpeed = 0.1f;
    while(running){
        pspDebugScreenSetXY(0, 0);
        ScePspFVector2 paddleoffsetA = {0,0};
        ScePspFVector2 paddleoffsetB = {0,0};
        startFrame();
        sceCtrlReadBufferPositive(&pad, 1);
        // Blending
        sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
        sceGuEnable(GU_BLEND);

        //Clear background to Bjack
        sceGuClearColor(0xFF000000);
        sceGuClearDepth(0);
        sceGuClear(GU_COLOR_BUFFER_BIT | GU_STENCIL_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
        
        //** Put Everything Here **//
        PaddleA->core->position.x = walloffset;
        draw_sprite2d(PaddleA);
        PaddleB->core->position.x = (PSP_SCR_WIDTH - 12) - walloffset;
        draw_sprite2d(PaddleB);

        NetA->core->position.x = (PSP_SCR_WIDTH / 2) - 1 - 2;
        draw_sprite2d(NetA);
        NetB->core->position.x = (PSP_SCR_WIDTH / 2) - 1 + 2;
        draw_sprite2d(NetB);

        
        draw_sprite2d(Ball);

        //** End Everything Here **//

        //Controller Processing
        if (pad.Buttons != 0)
        {
            if (pad.Buttons & PSP_CTRL_SQUARE){
            }
            if (pad.Buttons & PSP_CTRL_TRIANGLE){
                paddleoffsetB.x -= 1;
            }
            if (pad.Buttons & PSP_CTRL_CIRCLE){
            }
            if (pad.Buttons & PSP_CTRL_CROSS){
                paddleoffsetB.x += 1;
            }
            if (pad.Buttons & PSP_CTRL_UP){
                paddleoffsetA.x -= 1;
            }
            if (pad.Buttons & PSP_CTRL_DOWN){
                paddleoffsetA.x += 1;
            }
            if (pad.Buttons & PSP_CTRL_LEFT){
            }
            if (pad.Buttons & PSP_CTRL_RIGHT){
            }
        }
        PaddleAVelocity += paddleoffsetA.x * PaddleSpeed;
        PaddleA->core->position.y += PaddleAVelocity;
        if(paddleoffsetA.x == 0){
            PaddleAVelocity =  lerp_float(PaddleAVelocity, 0.0f, 0.1);
        } 
        PaddleA->core->position.y = clamp_float(PaddleA->core->position.y, 0, PSP_SCR_HEIGHT-paddleoffsets.y);
        if(PaddleA->core->position.y == 0 || PaddleA->core->position.y == PSP_SCR_HEIGHT-paddleoffsets.y){
            PaddleAVelocity = 0;
        }

        PaddleBVelocity += paddleoffsetB.x * PaddleSpeed;
        PaddleB->core->position.y += PaddleBVelocity;
        if(paddleoffsetB.x == 0){
            PaddleBVelocity =  lerp_float(PaddleBVelocity, 0.0f, 0.1);
        } 
        PaddleB->core->position.y = clamp_float(PaddleB->core->position.y, 0, PSP_SCR_HEIGHT-paddleoffsets.y);
        if(PaddleB->core->position.y == 0 || PaddleB->core->position.y == PSP_SCR_HEIGHT-paddleoffsets.y){
            PaddleBVelocity = 0;
        }
        Ball->core->position.x += BallVelocity.x;
        Ball->core->position.x = clamp_float(Ball->core->position.x, 0, PSP_SCR_WIDTH-8);
        if(Ball->core->position.x == 0 || Ball->core->position.x == PSP_SCR_WIDTH-8){
            BallVelocity.x *= -1;
            Ball->core->position.x = (PSP_SCR_WIDTH / 2) - 4;
            Ball->core->position.y = (PSP_SCR_HEIGHT/ 2) - 4;
        }

        Ball->core->position.y += BallVelocity.y;
        Ball->core->position.y = clamp_float(Ball->core->position.y, 0, PSP_SCR_HEIGHT-8);
        if(Ball->core->position.y == 0 || Ball->core->position.y == PSP_SCR_HEIGHT-8){
            BallVelocity.y *= -1;
        }

        //pspDebugScreenPrintf("%f, %f", Ball->core->position.y, PaddleA->core->position.y);
        if(Ball->core->position.x >= (PSP_SCR_WIDTH - paddleoffsets.x) - walloffset - 8 && Ball->core->position.x <= (PSP_SCR_WIDTH) - walloffset - 8 && BallVelocity.x > 0) {
            if(Ball->core->position.y >= PaddleB->core->position.y && Ball->core->position.y <= PaddleB->core->position.y + paddleoffsets.y){
                BallVelocity.x *= -1;
            }

        } else if (Ball->core->position.x >= PaddleA->core->position.x && Ball->core->position.x <= PaddleA->core->position.x + paddleoffsets.x  && BallVelocity.x < 0){
            if(Ball->core->position.y >= PaddleA->core->position.y && Ball->core->position.y <= PaddleA->core->position.y + paddleoffsets.y){
                BallVelocity.x *= -1;
            }
        }

        endFrame();
    }

cleanup:

    // Terminate Graphics
    termGraphics();

    // Exit Game
    sceKernelExitGame();
    return 0;
}
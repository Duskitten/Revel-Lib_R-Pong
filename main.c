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
    Texture* PaddleSprite = load_texture("Assets/Sprites/RPong-Paddles.png", GU_TRUE); 
    if(PaddleSprite == NULL){
        goto cleanup;
    }

    //Setup Sprites
    int walloffset = 64;
    int paddlespeed = 4;
    ScePspFVector2 offsets = {12,48};
    Sprite2D* PaddleA = create_sprite2d(PaddleSprite, 0xFFFFFFFF, (ScePspFVector2){offsets.x * 0,0}, (ScePspFVector2){offsets.x * 1,offsets.y}, (ScePspFVector2){12,48});
    Sprite2D* PaddleB = create_sprite2d(PaddleSprite, 0xFFFFFFFF, (ScePspFVector2){offsets.x * 1,0}, (ScePspFVector2){offsets.x * 2,offsets.y}, (ScePspFVector2){12,48});
    while(running){
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

        //** End Everything Here **//

        //Controller Processing
        if (pad.Buttons != 0)
        {
            if (pad.Buttons & PSP_CTRL_SQUARE){
            }
            if (pad.Buttons & PSP_CTRL_TRIANGLE){
                if (PaddleB->core->position.y - paddlespeed > 0){
                    PaddleB->core->position.y -= paddlespeed;
                } else{
                    PaddleB->core->position.y = 0;
                }
            }
            if (pad.Buttons & PSP_CTRL_CIRCLE){
            }
            if (pad.Buttons & PSP_CTRL_CROSS){
                if ((PaddleB->core->position.y + offsets.y) + paddlespeed < PSP_SCR_HEIGHT){
                    PaddleB->core->position.y += paddlespeed;
                } else{
                    PaddleB->core->position.y = PSP_SCR_HEIGHT - offsets.y;
                }
            }
            if (pad.Buttons & PSP_CTRL_UP){
                if (PaddleA->core->position.y - paddlespeed > 0){
                    PaddleA->core->position.y -= paddlespeed;
                } else{
                    PaddleA->core->position.y = 0;
                }
            }
            if (pad.Buttons & PSP_CTRL_DOWN){
                if ((PaddleA->core->position.y + offsets.y) + paddlespeed < PSP_SCR_HEIGHT){
                    PaddleA->core->position.y += paddlespeed;
                } else{
                    PaddleA->core->position.y = PSP_SCR_HEIGHT - offsets.y;
                }
            }
            if (pad.Buttons & PSP_CTRL_LEFT){
            }
            if (pad.Buttons & PSP_CTRL_RIGHT){
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
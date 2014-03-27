
#include "libretro.h"

//#include "emumain.h"
#include "cps2.h"

volatile int Loop;
volatile int Sleep;
char launchDir[MAX_PATH];
char screenshotDir[MAX_PATH];

int psp_cpuclock = PSPCLOCK_333;
int devkit_version = 0x03050210;
void set_cpu_clock(int value)
{
   switch (value)
   {
   case PSPCLOCK_266: scePowerSetClockFrequency(266, 266, 133); break;
   case PSPCLOCK_300: scePowerSetClockFrequency(300, 300, 150); break;
   case PSPCLOCK_333: scePowerSetClockFrequency(333, 333, 166); break;
   default: scePowerSetClockFrequency(222, 222, 111); break;
   }
}

void initSystemButtons(int devkit_version){}
unsigned int readSystemButtons(void){return 0;}
unsigned int readHomeButton(void){return 0;}
unsigned int readVolumeButtons(void){return 0;}
unsigned int readVolUpButton(void){return 0;}
unsigned int readVolDownButton(void){return 0;}
unsigned int readNoteButton(void){return 0;}
unsigned int readScreenButton(void){return 0;}
unsigned int readHoldSwitch(void){return 0;}
unsigned int readWLANSwitch(void){return 0;}
int readMainVolume(void){return 0;}

#include <stdio.h>
#include <string.h>

static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;

void retro_get_system_info(struct retro_system_info *info)
{
   info->library_name = "NJEMU";
   info->library_version = "v0.0.1";
   info->need_fullpath = true;
   info->block_extract = true;
   info->valid_extensions = "zip";
}

static struct retro_system_timing g_timing;

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   g_timing.fps = 60.0;
   g_timing.sample_rate = 44100;

   struct retro_game_geometry geom = { 160, 144, 160, 144 };
   info->geometry = geom;
   info->timing   = g_timing;
}

//void msg_printf(const char *text, ...)
//{
//   if(!log_cb)
//      return;
//   va_list arg;
//   va_start(arg, text);
//   log_cb(0,text, arg);
//   va_end(arg);
//}
//void ui_init(void){}
//int messagebox(int number){return 0;}
void cps2_main(void){}
static int cps2_init(void)
{
   if (!cps2_driver_init())
      return 0;

   msg_printf(TEXT(DONE2));
   msg_screen_clear();

   video_clear_screen();
   return cps2_video_init();
}

static void cps2_reset(void)
{
   video_clear_screen();

   Loop = LOOP_EXEC;

   autoframeskip_reset();

   cps2_driver_reset();
   cps2_video_reset();

   timer_reset();
   input_reset();
   sound_reset();

   blit_clear_all_sprite();
}

static void cps2_exit(void)
{
   video_set_mode(32);
   video_clear_screen();

   ui_popup_reset();

   video_clear_screen();
   msg_screen_init(WP_LOGO, ICON_SYSTEM, TEXT(EXIT_EMULATION2));

   msg_printf(TEXT(PLEASE_WAIT2));

   cps2_video_exit();
   cps2_driver_exit();

   msg_printf(TEXT(DONE2));

   show_exit_screen();
}




void retro_init()
{
   char prx_path[MAX_PATH];

   getcwd(launchDir, MAX_PATH - 1);
   strcat(launchDir, "/");
   sprintf(screenshotDir,"%s",launchDir);
   printf("\n%s\n",launchDir);
   ui_text_init();
   pad_init();
   video_init();
   show_frame = (void *)(0x200000 - FRAMESIZE * 3);
   draw_frame = (void *)(0x200000 - FRAMESIZE * 3);
   work_frame = (void *)(0x200000 - FRAMESIZE * 2);
   tex_frame  = (void *)(0x200000 - FRAMESIZE * 1);

   sceGuStart(GU_DIRECT, gulist);
   sceGuDrawBuffer(GU_PSM_5551, draw_frame, BUF_WIDTH);
   sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, show_frame, BUF_WIDTH);
   sceGuFinish();
   sceGuSync(0, GU_SYNC_FINISH);

   video_clear_frame(show_frame);
   video_clear_frame(draw_frame);
   video_clear_frame(work_frame);

   sceDisplayWaitVblankStart();
//   strcpy(game_dir, curr_dir);


   ui_popup_reset();
   video_clear_screen();



   struct retro_log_callback log;

   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = NULL;


   if (environ_cb)
   {
      g_timing.fps = 60.0;
      g_timing.sample_rate = 44100;
   }

//   video_init();
//   file_browser();
}

void retro_deinit()
{
}

void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

void retro_set_controller_port_device(unsigned port, unsigned device) {}

void retro_reset()
{
}


size_t retro_serialize_size()
{
   return 0;
}

bool retro_serialize(void *data, size_t size)
{
   return true;
}

bool retro_unserialize(const void *data, size_t size)
{
   return true;
}

void retro_cheat_reset() {}
void retro_cheat_set(unsigned index, bool enabled, const char *code) {}


bool retro_load_game(const struct retro_game_info *info)
{
   strcpy(game_name, "sfa");
   memory_init();
   sound_init();
   input_init();
   cps2_init();
   cps2_reset();


   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      if (log_cb)
         log_cb(RETRO_LOG_INFO, "[NJEMU]: RGB565 is not supported.\n");
      return false;
   }

//   if (!load(info->data, info->size)))
//      return false;


   return true;
}


bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{ return false; }

void retro_unload_game()
{}

unsigned retro_get_region() { return RETRO_REGION_NTSC; }

void *retro_get_memory_data(unsigned id)
{

   return 0;
}

size_t retro_get_memory_size(unsigned id)
{

   return 0;
}


void retro_run()
{
   static unsigned int __attribute__((aligned(16))) d_list[32];
   void* const texture_vram_p = (void*) ((u32)work_frame|0x04000000);

   timer_update_cpu();
//   update_screen();
   sceGuStart(GU_DIRECT, d_list);
   sceGuTexImage(0, 512, 272, 512, texture_vram_p);
   sceGuTexMode(GU_PSM_5551, 0, 0, GU_FALSE);
   sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
   sceGuDisable(GU_BLEND);
   sceGuFinish();

   video_cb(texture_vram_p, 512, 272, 512);
   sceGuSync(0,0);

   update_inputport();

   static int frames=0;
   printf("frame = %i\n", frames++);

}

unsigned retro_api_version() { return RETRO_API_VERSION; }


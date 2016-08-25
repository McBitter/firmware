/* Copyright (C) 2011 The Android Open Source Project
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/

#include <SDL.h>
#include <SDL_syswm.h>

#include "android/avd/util.h"
#include "android/globals.h"
#include "android/main-common.h"
#include "android/qemulator.h"
#include "android/display.h"
#include "android/resource.h"
#include "android/skin/image.h"
#include "android/skin/trackball.h"
#include "android/skin/keyboard.h"
#include "android/skin/file.h"
#include "android/skin/window.h"
#include "android/user-config.h"
#include "android/utils/bufprint.h"
#include "android/utils/debug.h"
#include "android/utils/eintr_wrapper.h"
#include "android/utils/path.h"

#include "ui/console.h"

#include <stdlib.h>

#define  D(...)  do {  if (VERBOSE_CHECK(init)) dprint(__VA_ARGS__); } while (0)

/***  CONFIGURATION
 ***/

static AUserConfig*  userConfig;

void
user_config_init( void )
{
    userConfig = auserConfig_new( android_avdInfo );
}

/* only call this function on normal exits, so that ^C doesn't save the configuration */
void
user_config_done( void )
{
    int  win_x, win_y;

    if (!userConfig) {
        D("no user configuration?");
        return;
    }

    SDL_WM_GetPos( &win_x, &win_y );
    auserConfig_setWindowPos(userConfig, win_x, win_y);
    auserConfig_save(userConfig);
}

void
user_config_get_window_pos( int *window_x, int *window_y )
{
    *window_x = *window_y = 10;

    if (userConfig)
        auserConfig_getWindowPos(userConfig, window_x, window_y);
}

/***********************************************************************/
/***********************************************************************/
/*****                                                             *****/
/*****            K E Y S E T   R O U T I N E S                    *****/
/*****                                                             *****/
/***********************************************************************/
/***********************************************************************/

#define  KEYSET_FILE    "default.keyset"

SkinKeyset*  android_keyset = NULL;

static int
load_keyset(const char*  path)
{
    if (path_can_read(path)) {
        AConfig*  root = aconfig_node("","");
        if (!aconfig_load_file(root, path)) {
            android_keyset = skin_keyset_new(root);
            if (android_keyset != NULL) {
                D( "keyset loaded from: %s", path);
                return 0;
            }
        }
    }
    return -1;
}

void
parse_keyset(const char*  keyset, AndroidOptions*  opts)
{
    char   kname[MAX_PATH];
    char   temp[MAX_PATH];
    char*  p;
    char*  end;

    /* append .keyset suffix if needed */
    if (strchr(keyset, '.') == NULL) {
        p   =  kname;
        end = p + sizeof(kname);
        p   = bufprint(p, end, "%s.keyset", keyset);
        if (p >= end) {
            derror( "keyset name too long: '%s'\n", keyset);
            exit(1);
        }
        keyset = kname;
    }

    /* look for a the keyset file */
    p   = temp;
    end = p + sizeof(temp);
    p = bufprint_config_file(p, end, keyset);
    if (p < end && load_keyset(temp) == 0)
        return;

    p = temp;
    p = bufprint(p, end, "%s" PATH_SEP "keysets" PATH_SEP "%s", opts->sysdir, keyset);
    if (p < end && load_keyset(temp) == 0)
        return;

    p = temp;
    p = bufprint_app_dir(p, end);
    p = bufprint(p, end, PATH_SEP "keysets" PATH_SEP "%s", keyset);
    if (p < end && load_keyset(temp) == 0)
        return;

    return;
}

void
write_default_keyset( void )
{
    char   path[MAX_PATH];

    bufprint_config_file( path, path+sizeof(path), KEYSET_FILE );

    /* only write if there is no file here */
    if (!path_exists(path)) {
        int          fd = open( path, O_WRONLY | O_CREAT, 0666 );
        const char*  ks = skin_keyset_get_default();


        D( "writing default keyset file to %s", path );

        if (fd < 0) {
            D( "%s: could not create file: %s", __FUNCTION__, strerror(errno) );
            return;
        }
        HANDLE_EINTR(write(fd, ks, strlen(ks)));
        IGNORE_EINTR(close(fd));
    }
}



/***********************************************************************/
/***********************************************************************/
/*****                                                             *****/
/*****            S D L   S U P P O R T                            *****/
/*****                                                             *****/
/***********************************************************************/
/***********************************************************************/

void *readpng(const unsigned char*  base, size_t  size, unsigned *_width, unsigned *_height);

#ifdef CONFIG_DARWIN
#  define  ANDROID_ICON_PNG  "android_icon_256.png"
#else
#  define  ANDROID_ICON_PNG  "android_icon_16.png"
#endif

static void
sdl_set_window_icon( void )
{
    static int  window_icon_set;

    if (!window_icon_set)
    {
#ifdef _WIN32
        HANDLE         handle = GetModuleHandle( NULL );
        HICON          icon   = LoadIcon( handle, MAKEINTRESOURCE(1) );
        SDL_SysWMinfo  wminfo;

        SDL_GetWMInfo(&wminfo);

        SetClassLongPtr( wminfo.window, GCLP_HICON, (LONG)icon );
#else  /* !_WIN32 */
        unsigned              icon_w, icon_h;
        size_t                icon_bytes;
        const unsigned char*  icon_data;
        void*                 icon_pixels;

        window_icon_set = 1;

        icon_data = android_icon_find( ANDROID_ICON_PNG, &icon_bytes );
        if ( !icon_data )
            return;

        icon_pixels = readpng( icon_data, icon_bytes, &icon_w, &icon_h );
        if ( !icon_pixels )
            return;

       /* the data is loaded into memory as RGBA bytes by libpng. we want to manage
        * the values as 32-bit ARGB pixels, so swap the bytes accordingly depending
        * on our CPU endianess
        */
        {
            unsigned*  d     = icon_pixels;
            unsigned*  d_end = d + icon_w*icon_h;

            for ( ; d < d_end; d++ ) {
                unsigned  pix = d[0];
#if HOST_WORDS_BIGENDIAN
                /* R,G,B,A read as RGBA => ARGB */
                pix = ((pix >> 8) & 0xffffff) | (pix << 24);
#else
                /* R,G,B,A read as ABGR => ARGB */
                pix = (pix & 0xff00ff00) | ((pix >> 16) & 0xff) | ((pix & 0xff) << 16);
#endif
                d[0] = pix;
            }
        }

        SDL_Surface* icon = sdl_surface_from_argb32( icon_pixels, icon_w, icon_h );
        if (icon != NULL) {
            SDL_WM_SetIcon(icon, NULL);
            SDL_FreeSurface(icon);
            free( icon_pixels );
        }
#endif  /* !_WIN32 */
    }
}

/***********************************************************************/
/***********************************************************************/
/*****                                                             *****/
/*****            S K I N   S U P P O R T                          *****/
/*****                                                             *****/
/***********************************************************************/
/***********************************************************************/

const char*  skin_network_speed = NULL;
const char*  skin_network_delay = NULL;


static void sdl_at_exit(void)
{
    user_config_done();
    qemulator_done(qemulator_get());
    SDL_Quit();
}


void sdl_display_init(DisplayState *ds, int full_screen, int  no_frame)
{
    QEmulator*    emulator = qemulator_get();
    SkinDisplay*  disp     = skin_layout_get_display(emulator->layout);
    int           width, height;
    char          buf[128];

    if (disp->rotation & 1) {
        width  = disp->rect.size.h;
        height = disp->rect.size.w;
    } else {
        width  = disp->rect.size.w;
        height = disp->rect.size.h;
    }

    snprintf(buf, sizeof buf, "width=%d,height=%d", width, height);
    android_display_init(ds, qframebuffer_fifo_get());
}

typedef struct part_properties part_properties;
struct part_properties {
    const char*      name;
    int              width;
    int              height;
    part_properties* next;
};

part_properties*
read_all_part_properties(AConfig* parts)
{
    part_properties* head = NULL;
    part_properties* prev = NULL;

    AConfig *node = parts->first_child;
    while (node) {
        part_properties* t = calloc(1, sizeof(part_properties));
        t->name = node->name;

        AConfig* bg = aconfig_find(node, "background");
        if (bg != NULL) {
            t->width = aconfig_int(bg, "width", 0);
            t->height = aconfig_int(bg, "height", 0);
        }

        if (prev == NULL) {
            head = t;
        } else {
            prev->next = t;
        }
        prev = t;
        node = node->next;
    }

    return head;
}

void
free_all_part_properties(part_properties* head)
{
    part_properties* prev = head;
    while (head) {
        prev = head;
        head = head->next;
        free(prev);
    }
}

part_properties*
get_part_properties(part_properties* allparts, char *partname)
{
    part_properties* p;
    for (p = allparts; p != NULL; p = p->next) {
        if (!strcmp(partname, p->name))
            return p;
    }

    return NULL;
}

void
add_parts_to_layout(AConfig* layout,
                    char* parts[],
                    int n_parts,
                    part_properties *props,
                    int xoffset,
                    int x_margin,
                    int y_margin)
{
    int     i;
    int     y = 10;
    char    tmp[512];
    for (i = 0; i < n_parts; i++) {
        part_properties *p = get_part_properties(props, parts[i]);
        snprintf(tmp, sizeof tmp,
            "part%d {\n \
                name %s\n \
                x %d\n \
                y %d\n \
            }",
            i + 2,  // layout already has the device part as part1, so start from part2
            p->name,
            xoffset + x_margin,
            y
            );
        y += p->height + y_margin;
        aconfig_load(layout, strdup(tmp));
    }
}

int
load_dynamic_skin(AndroidHwConfig* hwConfig,
                  char**           skinDirPath,
                  int              width,
                  int              height,
                  AConfig*         root)
{
    char      tmp[1024];
    AConfig*  node;
    int       i;
    int       max_part_width;

    *skinDirPath = avdInfo_getDynamicSkinPath(android_avdInfo);
    if (*skinDirPath == NULL) {
        dwarning("Unable to locate dynamic skin directory. Will not use dynamic skin.");
        return 0;
    }

    snprintf(tmp, sizeof(tmp), "%s/layout", *skinDirPath);
    D("trying to load skin file '%s'", tmp);

    if(aconfig_load_file(root, tmp) < 0) {
        dwarning("could not load skin file '%s', won't use a skin\n", tmp);
        return 0;
    }

    /* Fix the width and height specified for the "device" part in the layout */
    node = aconfig_find(root, "parts");
    if (node != NULL) {
        node = aconfig_find(node, "device");
        if (node != NULL) {
            node = aconfig_find(node, "display");
            if (node != NULL) {
                snprintf(tmp, sizeof tmp, "%d", width);
                aconfig_set(node, "width", strdup(tmp));
                snprintf(tmp, sizeof tmp, "%d", height);
                aconfig_set(node, "height", strdup(tmp));
            }
        }
    }

    /* The dynamic layout declares all the parts that are available statically
       in the layout file. Now we need to dynamically generate the
       appropriate layout based on the hardware config */

    part_properties* props = read_all_part_properties(aconfig_find(root, "parts"));

    const int N_PARTS = 4;
    char* parts[N_PARTS];
    parts[0] = "basic_controls";
    parts[1] = hwConfig->hw_mainKeys ? "hwkeys_on" : "hwkeys_off";
    parts[2] = hwConfig->hw_dPad ? "dpad_on" : "dpad_off";
    parts[3] = hwConfig->hw_keyboard ? "keyboard_on" : "keyboard_off";

    for (i = 0, max_part_width = 0; i < N_PARTS; i++) {
        part_properties *p = get_part_properties(props, parts[i]);
        if (p != NULL && p->width > max_part_width)
                max_part_width = p->width;
    }

    int x_margin = 10;
    int y_margin = 10;
    snprintf(tmp, sizeof tmp,
            "layouts {\n \
                portrait {\n \
                    width %d\n \
                    height %d\n \
                    color 0x404040\n \
                    event EV_SW:0:1\n \
                    part1 {\n name device\n x 0\n y 0\n}\n \
                }\n \
                landscape {\n \
                    width %d\n \
                    height %d\n \
                    color 0x404040\n \
                    event EV_SW:0:0\n \
                    dpad-rotation 3\n \
                    part1 {\n name device\n x 0\n y %d\n rotation 3\n }\n \
                    }\n \
                }\n \
             }\n",
            width  + max_part_width + 2 * x_margin,
            height,
            height + max_part_width + 2 * x_margin,
            width,
            width);
    aconfig_load(root, strdup(tmp));

    /* Add parts to portrait orientation */
    node = aconfig_find(root, "layouts");
    if (node != NULL) {
        node = aconfig_find(node, "portrait");
        if (node != NULL) {
            add_parts_to_layout(node, parts, N_PARTS, props, width, x_margin, y_margin);
        }
    }

    /* Add parts to landscape orientation */
    node = aconfig_find(root, "layouts");
    if (node != NULL) {
        node = aconfig_find(node, "landscape");
        if (node != NULL) {
            add_parts_to_layout(node, parts, N_PARTS, props, height, x_margin, y_margin);
        }
    }

    free_all_part_properties(props);

    return 1;
}

/* list of skin aliases */
static const struct {
    const char*  name;
    const char*  alias;
} skin_aliases[] = {
    { "QVGA-L", "320x240" },
    { "QVGA-P", "240x320" },
    { "HVGA-L", "480x320" },
    { "HVGA-P", "320x480" },
    { "QVGA", "320x240" },
    { "HVGA", "320x480" },
    { NULL, NULL }
};

void
parse_skin_files(const char*      skinDirPath,
                 const char*      skinName,
                 AndroidOptions*  opts,
                 AndroidHwConfig* hwConfig,
                 AConfig*        *skinConfig,
                 char*           *skinPath)
{
    char      tmp[1024];
    AConfig*  root;
    const char* path = NULL;
    AConfig*  n;

    root = aconfig_node("", "");

    if (skinName == NULL)
        goto DEFAULT_SKIN;

    /* Support skin aliases like QVGA-H QVGA-P, etc...
       But first we check if it's a directory that exist before applying
       the alias */
    int  checkAlias = 1;

    if (skinDirPath != NULL) {
        bufprint(tmp, tmp+sizeof(tmp), "%s/%s", skinDirPath, skinName);
        if (path_exists(tmp)) {
            checkAlias = 0;
        } else {
            D("there is no '%s' skin in '%s'", skinName, skinDirPath);
        }
    }

    if (checkAlias) {
        int  nn;

        for (nn = 0; ; nn++ ) {
            const char*  skin_name  = skin_aliases[nn].name;
            const char*  skin_alias = skin_aliases[nn].alias;

            if (!skin_name)
                break;

            if (!strcasecmp( skin_name, skinName )) {
                D("skin name '%s' aliased to '%s'", skinName, skin_alias);
                skinName = skin_alias;
                break;
            }
        }
    }

    /* Magically support skins like "320x240" or "320x240x16" */
    if(isdigit(skinName[0])) {
        char *x = strchr(skinName, 'x');
        if(x && isdigit(x[1])) {
            int width = atoi(skinName);
            int height = atoi(x+1);
            int bpp   = 16;
            char* y = strchr(x+1, 'x');
            if (y && isdigit(y[1])) {
                bpp = atoi(y+1);
            }

            if (opts->dynamic_skin) {
                char *dynamicSkinDirPath;
                if (load_dynamic_skin(hwConfig, &dynamicSkinDirPath, width, height, root)) {
                    path = dynamicSkinDirPath;
                    D("loaded dynamic skin width=%d height=%d bpp=%d\n", width, height, bpp);
                    goto FOUND_SKIN;
                }
            }

            snprintf(tmp, sizeof tmp,
                    "display {\n  width %d\n  height %d\n bpp %d}\n",
                    width, height,bpp);
            aconfig_load(root, strdup(tmp));
            path = ":";
            D("found magic skin width=%d height=%d bpp=%d\n", width, height, bpp);
            goto FOUND_SKIN;
        }
    }

    if (skinDirPath == NULL) {
        derror("unknown skin name '%s'", skinName);
        exit(1);
    }

    snprintf(tmp, sizeof tmp, "%s/%s/layout", skinDirPath, skinName);
    D("trying to load skin file '%s'", tmp);

    if(aconfig_load_file(root, tmp) < 0) {
        dwarning("could not load skin file '%s', using built-in one\n",
                 tmp);
        goto DEFAULT_SKIN;
    }

    snprintf(tmp, sizeof tmp, "%s/%s/", skinDirPath, skinName);
    path = tmp;
    goto FOUND_SKIN;

FOUND_SKIN:
    /* the default network speed and latency can now be specified by the device skin */
    n = aconfig_find(root, "network");
    if (n != NULL) {
        skin_network_speed = aconfig_str(n, "speed", 0);
        skin_network_delay = aconfig_str(n, "delay", 0);
    }

    /* extract framebuffer information from the skin.
     *
     * for version 1 of the skin format, they are in the top-level
     * 'display' element.
     *
     * for version 2 of the skin format, they are under parts.device.display
     */
    n = aconfig_find(root, "display");
    if (n == NULL) {
        n = aconfig_find(root, "parts");
        if (n != NULL) {
            n = aconfig_find(n, "device");
            if (n != NULL) {
                n = aconfig_find(n, "display");
            }
        }
    }

    if (n != NULL) {
        int  width  = aconfig_int(n, "width", hwConfig->hw_lcd_width);
        int  height = aconfig_int(n, "height", hwConfig->hw_lcd_height);
        int  depth  = aconfig_int(n, "bpp", hwConfig->hw_lcd_depth);

        if (width > 0 && height > 0) {
            /* The emulated framebuffer wants sizes that are multiples of 4 */
            if (((width|height) & 3) != 0) {
                width  = (width+3) & ~3;
                height = (height+3) & ~3;
                D("adjusting LCD dimensions to (%dx%dx)", width, height);
            }

            /* only depth values of 16 and 32 are correct. 16 is the default. */
            if (depth != 32 && depth != 16) {
                depth = 16;
                D("adjusting LCD bit depth to %d", depth);
            }

            hwConfig->hw_lcd_width  = width;
            hwConfig->hw_lcd_height = height;
            hwConfig->hw_lcd_depth  = depth;
        }
        else {
            D("ignoring invalid skin LCD dimensions (%dx%dx%d)",
              width, height, depth);
        }
    }

    *skinConfig = root;
    *skinPath   = strdup(path);
    return;

DEFAULT_SKIN:
    {
        const unsigned char*  layout_base;
        size_t                layout_size;
        char*                 base;

        skinName = "<builtin>";

        layout_base = android_resource_find( "layout", &layout_size );
        if (layout_base == NULL) {
            fprintf(stderr, "Couldn't load builtin skin\n");
            exit(1);
        }
        base = malloc( layout_size+1 );
        memcpy( base, layout_base, layout_size );
        base[layout_size] = 0;

        D("parsing built-in skin layout file (%d bytes)", (int)layout_size);
        aconfig_load(root, base);
        path = ":";
    }
    goto FOUND_SKIN;
}


void
init_sdl_ui(AConfig*         skinConfig,
            const char*      skinPath,
            AndroidOptions*  opts)
{
    int  win_x, win_y, flags;

    signal(SIGINT, SIG_DFL);
#ifndef _WIN32
    signal(SIGQUIT, SIG_DFL);
#endif

    /* we're not a game, so allow the screensaver to run */
    setenv("SDL_VIDEO_ALLOW_SCREENSAVER","1",1);

    flags = SDL_INIT_NOPARACHUTE;
    if (!opts->no_window)
        flags |= SDL_INIT_VIDEO;

    if(SDL_Init(flags)){
        fprintf(stderr, "SDL init failure, reason is: %s\n", SDL_GetError() );
        exit(1);
    }

    if (!opts->no_window) {
        SDL_EnableUNICODE(!opts->raw_keys);
        SDL_EnableKeyRepeat(0,0);

        sdl_set_window_icon();
    }
    else
    {
#ifndef _WIN32
       /* prevent SIGTTIN and SIGTTOUT from stopping us. this is necessary to be
        * able to run the emulator in the background (e.g. "emulator &").
        * despite the fact that the emulator should not grab input or try to
        * write to the output in normal cases, we're stopped on some systems
        * (e.g. OS X)
        */
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
#endif
    }
    atexit(sdl_at_exit);

    user_config_get_window_pos(&win_x, &win_y);

    if ( qemulator_init(qemulator_get(), skinConfig, skinPath, win_x, win_y, opts) < 0 ) {
        fprintf(stderr, "### Error: could not load emulator skin from '%s'\n", skinPath);
        exit(1);
    }

    /* add an onion overlay image if needed */
    if (opts->onion) {
        SkinImage*  onion = skin_image_find_simple( opts->onion );
        int         alpha, rotate;

        if ( opts->onion_alpha && 1 == sscanf( opts->onion_alpha, "%d", &alpha ) ) {
            alpha = (256*alpha)/100;
        } else
            alpha = 128;

        if ( opts->onion_rotation && 1 == sscanf( opts->onion_rotation, "%d", &rotate ) ) {
            rotate &= 3;
        } else
            rotate = SKIN_ROTATION_0;

        qemulator_get()->onion          = onion;
        qemulator_get()->onion_alpha    = alpha;
        qemulator_get()->onion_rotation = rotate;
    }
}

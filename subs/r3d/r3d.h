/*  3D Software Rasterizer
 *  
 *  r3d.h
 *  
 *  Copyright (c) Dylan Muller, 2019
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *  
 *  Compiling under linux:
 *  gcc -o prog prog.cpp -lSDL2
 *  
 *  Version: 2019-11-23T13:07:51
 */

#ifndef _RD_H
#define _RD_H

#define NULL 0
#define SWAP(x, y)       \
    do                   \
    {                    \
        (x) = (x) ^ (y); \
        (y) = (x) ^ (y); \
        (x) = (x) ^ (y); \
    } while (0)

#define DSP_STR(x) x
#define BOOL int
#define STR_MAX 225

#include <SDL2/SDL.h>

typedef enum
{
    IDLE, // Client is idle, no initialization
    INIT, // Client has initialized
    FAIL, // Failure return code
    SYNC_BLANK,
    SYNC_UNDEFINED,
    OK // Success return code

} RD_STATE;

typedef enum
{
    RD_VIDEO = 0xFE,
    RD_TIMER,
    RD_AUDIO

} RD_FLAGS;

typedef struct
{
    int w;  // Screen width
    int h;  // Screen height
    int wp; // Pixel width
    int hp; // Pixel height
    char *str;
} RD_Client;

typedef struct
{
    Uint8 b;
    Uint8 g;
    Uint8 r;
    Uint8 a;
} RD_Cell;

typedef struct
{
    RD_Client client;
    RD_Cell *mem; // video memory
    int sync;
    int d_nt; // video blank period
    unsigned int d_s;

} RD_Video_Subsystem;

typedef struct
{
    int nt_begin;   // video blank period
    int nt_present; // video blank period

} RD_Timer_Subsystem;

typedef struct
{
    RD_Timer_Subsystem *timer;
    RD_Video_Subsystem *video;
} RD_Subsystem;

typedef struct
{
    RD_Subsystem *sys; // SDL2 sys manager
    int state;         // Context state
    int flags;
    SDL_Window *window;     // SDL2 window object
    SDL_Renderer *renderer; // SDL2 render object
    SDL_Texture *disp;      // SDL2 texture object (draw blit)
    SDL_RendererInfo nfo;
    SDL_Event event; // SDL2 event object

} RD_Context;

void RD_LogMessage(char *message)
{
    puts(message);
}

int RD_PollEvents(RD_Context *context)
{
    SDL_PollEvent(&context->event);

    switch (context->event.type)
    {
    case SDL_QUIT:
        return 0;
    }

    return OK;
}
void *RD_Begin(RD_Context *context)
{
    int pitch = 0x0;

    if (context != NULL &&
        context->disp != NULL &&
        context->sys->video->sync == SYNC_UNDEFINED)
    {
        SDL_LockTexture(context->disp, NULL,
                        (void **)&context->sys->video->mem, &pitch);
        context->sys->video->sync = SYNC_BLANK;
        if (context->flags & RD_TIMER)
        {
            RD_Timer_Subsystem *timer = (RD_Timer_Subsystem *)
                                            context->sys->timer;
            timer->nt_begin = SDL_GetTicks();
        }
    }

    return context->sys->video->mem;
}

RD_Cell RD_CreateColor(int r, int g, int b)
{
    RD_Cell pixel;
    pixel.a = 255;
    pixel.r = r;
    pixel.g = g;
    pixel.b = b;
    return pixel;
}
void *RD_Present(RD_Context *context)
{
    int pitch = 0x0;
    RD_Video_Subsystem *video = (RD_Video_Subsystem *)
                                    context->sys->video;
    if (context != NULL &&
        context->disp != NULL &&
        video->sync == SYNC_BLANK)
    {
        SDL_UnlockTexture(context->disp);
        SDL_RenderCopy(context->renderer, context->disp, NULL, NULL);
        SDL_RenderPresent(context->renderer);
        video->mem = NULL;
        video->sync = SYNC_UNDEFINED;
        // should stay at end
        if (context->flags & RD_TIMER)
        {
            RD_Timer_Subsystem *timer = (RD_Timer_Subsystem *)
                                            context->sys->timer;
            timer->nt_present = SDL_GetTicks();
            video->d_nt = timer->nt_present - timer->nt_begin;
            if (video->d_nt != NULL)
            {
                int dt_s = 1000 / video->d_nt;

                video->d_s = dt_s;
            }
        }
    }

    return video->mem;
}

int RD_CreateClient(int w, int h, int wp, int hp, char *str, RD_Client *client)
{
    client->w = w;
    client->h = h;
    client->hp = hp;
    client->wp = wp;
    client->str = str;

    return OK;
}

int RD_DrawLine(int x0, int y0, int x1, int y1,
                RD_Cell *pixel, RD_Context *context)
{
    RD_Video_Subsystem *video = (RD_Video_Subsystem *)
                                    context->sys->video;

    if (y1 > video->client.h)
        y1 = video->client.h;
    if (y0 > video->client.h)
        y0 = video->client.h;
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;

    for (;;)
    {
        RD_Cell *base = video->mem +
                        (y0 * video->client.w) + x0;
        *base = *pixel;

        if (x0 == x1 && y0 == y1)
            break;
        e2 = err;
        if (e2 > -dx)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy)
        {
            err += dx;
            y0 += sy;
        }
    }

    return OK;
}

void RD_DrawTriangle(int x0, int y0, int x1, int y1,
                     int x2, int y2, RD_Cell *pixel, RD_Context *context)
{
    RD_DrawLine(x0, y0, x1, y1, pixel, context);
    RD_DrawLine(x1, y1, x2, y2, pixel, context);
    RD_DrawLine(x2, y2, x0, y0, pixel, context);
}

void RD_FillTriangle(int x1, int y1, int x2, int y2,
                     int x3, int y3, RD_Cell *pixel, RD_Context *context)
{

    RD_Video_Subsystem *video = (RD_Video_Subsystem *)
                                    context->sys->video;

    RD_Cell *base = 0x0;

    int t1x, t2x, y, minx, maxx, t1xp, t2xp;
    int changed1 = 0;
    int changed2 = 0;
    int signx1, signx2, dx1, dy1, dx2, dy2;
    int e1, e2;

    if (y1 > y2)
    {
        SWAP(y1, y2);
        SWAP(x1, x2);
    }
    if (y1 > y3)
    {
        SWAP(y1, y3);
        SWAP(x1, x3);
    }
    if (y2 > y3)
    {
        SWAP(y2, y3);
        SWAP(x2, x3);
    }

    t1x = t2x = x1;
    y = y1;
    dx1 = (int)(x2 - x1);
    if (dx1 < 0)
    {
        dx1 = -dx1;
        signx1 = -1;
    }
    else
        signx1 = 1;
    dy1 = (int)(y2 - y1);

    dx2 = (int)(x3 - x1);
    if (dx2 < 0)
    {
        dx2 = -dx2;
        signx2 = -1;
    }
    else
        signx2 = 1;
    dy2 = (int)(y3 - y1);

    if (dy1 > dx1)
    {
        SWAP(dx1, dy1);
        changed1 = 1;
    }
    if (dy2 > dx2)
    {
        SWAP(dy2, dx2);
        changed2 = 1;
    }

    e2 = (int)(dx2 >> 1);

    if (y1 == y2)
        goto next;
    e1 = (int)(dx1 >> 1);

    for (int i = 0; i < dx1;)
    {
        t1xp = 0;
        t2xp = 0;
        if (t1x < t2x)
        {
            minx = t1x;
            maxx = t2x;
        }
        else
        {
            minx = t2x;
            maxx = t1x;
        }

        while (i < dx1)
        {
            i++;
            e1 += dy1;
            while (e1 >= dx1)
            {
                e1 -= dx1;
                if (changed1)
                    t1xp = signx1;
                else
                    goto next1;
            }
            if (changed1)
                break;
            else
                t1x += signx1;
        }

    next1:
        while (1)
        {
            e2 += dy2;
            while (e2 >= dx2)
            {
                e2 -= dx2;
                if (changed2)
                    t2xp = signx2;
                else
                    goto next2;
            }
            if (changed2)
                break;
            else
                t2x += signx2;
        }
    next2:
        if (minx > t1x)
            minx = t1x;
        if (minx > t2x)
            minx = t2x;
        if (maxx < t1x)
            maxx = t1x;
        if (maxx < t2x)
            maxx = t2x;
        for (int i = minx; i <= maxx; i++)
        {

            base = video->mem + ((y * video->client.w) + i);
            *base = *pixel;
        };
        if (!changed1)
            t1x += signx1;
        t1x += t1xp;
        if (!changed2)
            t2x += signx2;
        t2x += t2xp;
        y += 1;
        if (y == y2)
            break;
    }

next:
    dx1 = (int)(x3 - x2);
    if (dx1 < 0)
    {
        dx1 = -dx1;
        signx1 = -1;
    }
    else
        signx1 = 1;
    dy1 = (int)(y3 - y2);
    t1x = x2;

    if (dy1 > dx1)
    {
        SWAP(dy1, dx1);
        changed1 = 1;
    }
    else
        changed1 = 0;

    e1 = (int)(dx1 >> 1);

    for (int i = 0; i <= dx1; i++)
    {
        t1xp = 0;
        t2xp = 0;
        if (t1x < t2x)
        {
            minx = t1x;
            maxx = t2x;
        }
        else
        {
            minx = t2x;
            maxx = t1x;
        }

        while (i < dx1)
        {
            e1 += dy1;
            while (e1 >= dx1)
            {
                e1 -= dx1;
                if (changed1)
                {
                    t1xp = signx1;
                    break;
                }
                else
                    goto next3;
            }
            if (changed1)
                break;
            else
                t1x += signx1;
            if (i < dx1)
                i++;
        }
    next3:

        while (t2x != x3)
        {
            e2 += dy2;
            while (e2 >= dx2)
            {
                e2 -= dx2;
                if (changed2)
                    t2xp = signx2;
                else
                    goto next4;
            }
            if (changed2)
                break;
            else
                t2x += signx2;
        }
    next4:

        if (minx > t1x)
            minx = t1x;
        if (minx > t2x)
            minx = t2x;
        if (maxx < t1x)
            maxx = t1x;
        if (maxx < t2x)
            maxx = t2x;
        for (int i = minx; i <= maxx; i++)
        {
            base = video->mem + ((y * video->client.w) + i);
            *base = *pixel;
        };
        if (!changed1)
            t1x += signx1;
        t1x += t1xp;
        if (!changed2)
            t2x += signx2;
        t2x += t2xp;
        y += 1;
        if (y > y3)
            return;
    }
}

int RD_FillRect(int x0, int y0, int w, int h,
                RD_Cell *pixel, RD_Context *context)
{
    RD_Video_Subsystem *video = (RD_Video_Subsystem *)
                                    context->sys->video;

    for (int y = y0; y < (y0 + h); y++)
    {
        for (int x = x0; x < (x0 + w); x++)
        {
            RD_Cell *base = (video->mem) +
                            (((y * video->client.w) + x));
            *base = *pixel;
        }
    }

    return OK;
}

void RD_CreateContext(RD_Client client,
                     int flags,
                     RD_Context *context)
{

    if (context != NULL)
    {
        Uint32 in_flags = 0x0;
        int ret = 0x0;
        context->flags = flags;
        context->disp = 0x0;

        context->sys = (RD_Subsystem *)
            malloc(sizeof(RD_Subsystem));

        // Current support timer, video, and audio

        in_flags |= SDL_INIT_EVENTS;

        if (flags & RD_TIMER)
            in_flags |= SDL_INIT_TIMER;
        if (flags & RD_VIDEO)
            in_flags |= SDL_INIT_VIDEO;
        if (flags & RD_AUDIO)
            in_flags |= SDL_INIT_AUDIO;

        SDL_Init(in_flags);
        // Initialize video subsystem
        if (flags & RD_VIDEO)
        {
            // Alloc sys->video
            context->sys->video = (RD_Video_Subsystem *)
                malloc(sizeof(RD_Video_Subsystem));
            context->sys->video->sync = SYNC_UNDEFINED;

            memcpy(&context->sys->video->client,
                   &client, sizeof(RD_Client));
            context->sys->video->d_s = 0xFFFFFFFF;
            RD_Client *client = (RD_Client *)&context->sys->video->client;
            context->window = SDL_CreateWindow(client->str, SDL_WINDOWPOS_CENTERED,
                                               SDL_WINDOWPOS_CENTERED,
                                               client->w * client->wp,
                                               client->h * client->hp, NULL);
            if (context->window)
            {
                context->renderer = SDL_CreateRenderer(context->window, -1,
                                                       SDL_RENDERER_ACCELERATED);
                SDL_GetRendererInfo(context->renderer, &context->nfo);

                context->disp = SDL_CreateTexture(
                    context->renderer,
                    SDL_PIXELFORMAT_ARGB8888,
                    SDL_TEXTUREACCESS_STREAMING,
                    client->w,
                    client->h);
                if (!context->disp)
                {
                    printf(DSP_STR("RD_CreateContext: FAIL"));
                    // Critical exit
                }
                else
                {
                    printf(DSP_STR("RD_CreateContext: OK\n"));
                }
            }
        }

        if (flags & RD_TIMER)
        {
            context->sys->timer = (RD_Timer_Subsystem *)
                malloc(sizeof(RD_Timer_Subsystem));
        }
    }
}

void RD_FreeContext(RD_Context *context)
{
    if (context != NULL)
    {
        SDL_DestroyRenderer(context->renderer);
        SDL_DestroyWindow(context->window);
        SDL_Quit();

        printf(DSP_STR("RD_FreeContext: OK\n"));
    }
}
#endif

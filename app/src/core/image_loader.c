#include "image_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stb_image.h"

static uint16_t read_u16_le(const unsigned char *data)
{
    return (uint16_t)(data[0] | (data[1] << 8));
}

static uint32_t read_u32_le(const unsigned char *data)
{
    return (uint32_t)(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

SDL_Surface *image_loader_load_bmp_rgba(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        SDL_Log("image_loader: failed to open %s", path);
        return NULL;
    }

    unsigned char header[14];
    if (fread(header, 1, sizeof(header), file) != sizeof(header)) {
        SDL_Log("image_loader: failed to read bmp header");
        fclose(file);
        return NULL;
    }

    if (header[0] != 'B' || header[1] != 'M') {
        SDL_Log("image_loader: not a bmp %s", path);
        fclose(file);
        return NULL;
    }

    uint32_t pixel_offset = read_u32_le(header + 10);

    unsigned char info_header[40];
    if (fread(info_header, 1, sizeof(info_header), file) != sizeof(info_header)) {
        SDL_Log("image_loader: failed to read bmp info header");
        fclose(file);
        return NULL;
    }

    uint32_t info_size = read_u32_le(info_header);
    if (info_size < 40) {
        SDL_Log("image_loader: unsupported bmp info header");
        fclose(file);
        return NULL;
    }

    int32_t width = (int32_t)read_u32_le(info_header + 4);
    int32_t height = (int32_t)read_u32_le(info_header + 8);
    uint16_t planes = read_u16_le(info_header + 12);
    uint16_t bpp = read_u16_le(info_header + 14);
    uint32_t compression = read_u32_le(info_header + 16);

    if (planes != 1 || (bpp != 24 && bpp != 32) || compression != 0) {
        SDL_Log("image_loader: unsupported bmp format (%d bpp, comp=%u)", bpp, compression);
        fclose(file);
        return NULL;
    }

    int flip = 1;
    int abs_height = height;
    if (height < 0) {
        flip = 0;
        abs_height = -height;
    }

    if (width <= 0 || abs_height <= 0) {
        SDL_Log("image_loader: invalid bmp size");
        fclose(file);
        return NULL;
    }

    size_t row_bytes = ((size_t)bpp * (size_t)width + 7) / 8;
    size_t padded_row_bytes = (row_bytes + 3) & ~((size_t)3);

    if (fseek(file, (long)pixel_offset, SEEK_SET) != 0) {
        SDL_Log("image_loader: failed to seek bmp data");
        fclose(file);
        return NULL;
    }

    size_t pixel_count = (size_t)width * (size_t)abs_height;
    size_t out_bytes = pixel_count * 4;
    unsigned char *out_pixels = (unsigned char *)SDL_malloc(out_bytes);
    if (out_pixels == NULL) {
        SDL_Log("image_loader: alloc failed");
        fclose(file);
        return NULL;
    }

    unsigned char *row_buffer = (unsigned char *)SDL_malloc(padded_row_bytes);
    if (row_buffer == NULL) {
        SDL_Log("image_loader: row alloc failed");
        SDL_free(out_pixels);
        fclose(file);
        return NULL;
    }

    for (int y = 0; y < abs_height; ++y) {
        if (fread(row_buffer, 1, padded_row_bytes, file) != padded_row_bytes) {
            SDL_Log("image_loader: failed to read bmp row");
            SDL_free(row_buffer);
            SDL_free(out_pixels);
            fclose(file);
            return NULL;
        }

        int out_y = flip ? (abs_height - 1 - y) : y;
        unsigned char *dst = out_pixels + ((size_t)out_y * (size_t)width * 4);
        const unsigned char *src = row_buffer;
        for (int x = 0; x < width; ++x) {
            unsigned char b = src[0];
            unsigned char g = src[1];
            unsigned char r = src[2];
            unsigned char a = (bpp == 32) ? src[3] : 255;
            dst[0] = r;
            dst[1] = g;
            dst[2] = b;
            dst[3] = a;
            dst += 4;
            src += (bpp == 32) ? 4 : 3;
        }
    }

    SDL_free(row_buffer);
    fclose(file);

    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(
        out_pixels,
        width,
        abs_height,
        32,
        width * 4,
        SDL_PIXELFORMAT_RGBA32
    );
    if (surface == NULL) {
        SDL_Log("image_loader: SDL_CreateRGBSurfaceWithFormatFrom failed: %s", SDL_GetError());
        SDL_free(out_pixels);
        return NULL;
    }

    SDL_Surface *converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    SDL_free(out_pixels);

    if (converted == NULL) {
        SDL_Log("image_loader: SDL_ConvertSurfaceFormat failed: %s", SDL_GetError());
    }
    return converted;
}

SDL_Surface *image_loader_load_rgba(const char *path)
{
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char *pixels = stbi_load(path, &width, &height, &channels, 4);
    if (pixels == NULL) {
        SDL_Log("image_loader: stbi_load failed for %s", path);
        return image_loader_load_bmp_rgba(path);
    }

    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(
        pixels,
        width,
        height,
        32,
        width * 4,
        SDL_PIXELFORMAT_RGBA32
    );
    if (surface == NULL) {
        SDL_Log("image_loader: SDL_CreateRGBSurfaceWithFormatFrom failed: %s", SDL_GetError());
        stbi_image_free(pixels);
        return NULL;
    }

    SDL_Surface *converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    stbi_image_free(pixels);

    if (converted == NULL) {
        SDL_Log("image_loader: SDL_ConvertSurfaceFormat failed: %s", SDL_GetError());
    }
    return converted;
}

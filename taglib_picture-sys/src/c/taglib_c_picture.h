#ifndef TAGLIB_PICTURE_LIB
#define TAGLIB_PICTURE_LIB

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { int dummy; } TagLib_File;

typedef struct {
    char* data;
    char* mimetype;
} Picture;

TagLib_File *taglib_picture_open_file(const char *filename);
void taglib_picture_free_file(TagLib_File *file);
Picture *taglib_picture_read_cover(TagLib_File *file);
void taglib_picture_write_cover(TagLib_File *file, Picture *picture);

#ifdef __cplusplus
}
#endif

#endif


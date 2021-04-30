#ifndef TAGLIB_PICTURE_LIB
#define TAGLIB_PICTURE_LIB

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { int dummy; } TagLib_File;

typedef struct {
    int status_code;
    const char *data;
    unsigned int data_len;
    const char *mimetype;
} Picture;

typedef struct {
    int width;
    int height;
    int depth;
} PictureMeta;

TagLib_File *taglib_picture_open_file(const char *filename);
void taglib_picture_free_file(TagLib_File *file);
void taglib_picture_free_vecs();
void taglib_picture_free_strs();
Picture taglib_picture_read_cover(TagLib_File *file);
void taglib_picture_write_cover(TagLib_File *file, Picture picture, PictureMeta meta);

#ifdef __cplusplus
}
#endif

#endif


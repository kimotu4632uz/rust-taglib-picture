#include <vector>
#include <tuple>
#include <string.h>

#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/tbytevector.h>

#include <taglib/flacfile.h>
#include <taglib/mpegfile.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/id3v2tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/mp4file.h>
#include <taglib/mp4coverart.h>
#include <taglib/asffile.h>
#include <taglib/asfpicture.h>

#include "tag_c_picture.h"

namespace {
    const std::tuple<const char *, TagLib::MP4::CoverArt::Format> mimedict[] = {
        {"image/jpeg", TagLib::MP4::CoverArt::Format::JPEG},
        {"image/png", TagLib::MP4::CoverArt::Format::PNG},
        {"image/bmp", TagLib::MP4::CoverArt::Format::BMP},
        {"image/gif", TagLib::MP4::CoverArt::Format::GIF}
    };

    const char *getmime(TagLib::MP4::CoverArt::Format f0) {
        for ([[maybe_unused]] auto [mime, f] : mimedict) {
            if (f == f0) { return mime; }
        }
        return "";
    }

    TagLib::MP4::CoverArt::Format getfmt(const char *mime0) {
        for ([[maybe_unused]] auto [mime, f] : mimedict) {
            if (::strcmp(mime, mime0)) { return f; }
        }
        return TagLib::MP4::CoverArt::Format::Unknown;
    }
}

namespace {
    std::vector<TagLib::ByteVector> savedByteVectors;
    std::vector<TagLib::String> savedStrings;
}

TagLib_File *taglib_picture_open_file(const char *filename) {
    return reinterpret_cast<TagLib_File *>(TagLib::FileRef::create(filename));
}

void taglib_picture_free_file(TagLib_File *file) {
    delete reinterpret_cast<TagLib::File *>(file);
}

void taglib_picture_free_vecs() {
    savedByteVectors.clear();
}

void taglib_picture_free_strs() {
    savedStrings.clear();
}

Picture taglib_picture_read_cover(TagLib_File *file) {
    auto pFileR = reinterpret_cast<TagLib::File *>(file);
    auto pTagR = pFileR->tag();

    TagLib::ID3v2::Tag       *id3v2 = nullptr;
    TagLib::Ogg::XiphComment *xc    = nullptr;

    if (auto pFile = dynamic_cast<TagLib::MPEG::File*>(pFileR); pFile != nullptr) {
        if (!pFile->hasID3v2Tag()) { return Picture{1, NULL, 0, NULL}; }
        id3v2 = pFile->ID3v2Tag();

    } else if (auto pFile = dynamic_cast<TagLib::MP4::File*>(pFileR); pFile != nullptr) {
        auto tag = pFile->tag();
        if (!tag->contains("covr")) { return Picture{11, NULL, 0, NULL}; }
        auto coverL = tag->item("covr").toCoverArtList();

        if (coverL.isEmpty()) { return Picture{12, NULL, 0, NULL}; }
        auto pic = coverL[0];

        auto data = pic.data();
        auto data_ptr = data.data();
        auto data_len = data.size();
        savedByteVectors.push_back(data);

        auto mime = TagLib::String(getmime(pic.format()), TagLib::String::UTF8);
        auto mime_ptr = mime.toCString(true);
        savedStrings.push_back(mime);

        return Picture{0, data_ptr, data_len, mime_ptr};


    } else if (auto pFile = dynamic_cast<TagLib::FLAC::File*>(pFileR); pFile != nullptr) {
        if (pFile->hasID3v2Tag()) { id3v2 = pFile->ID3v2Tag(); }
        else if (!pFile->pictureList().isEmpty()) {
            TagLib::FLAC::Picture *target_pic = nullptr;

            for (const auto pic : pFile->pictureList()) {
                if (pic != nullptr) {
                    if (pic->type() == TagLib::FLAC::Picture::Type::FrontCover) {
                        target_pic = pic;
                    }
                }
            }

            if (target_pic == nullptr) { target_pic = pFile->pictureList()[0]; }

            auto data = target_pic->data();
            auto data_ptr = data.data();
            auto data_len = data.size();
            savedByteVectors.push_back(data);

            auto mime = target_pic->mimeType();
            auto mime_ptr = mime.toCString(true);
            savedStrings.push_back(mime);

            return Picture{0, data_ptr, data_len, mime_ptr};

        } else {
            xc = pFile->xiphComment(false);
        }

    } else if (auto pFile = dynamic_cast<TagLib::ASF::File*>(pFileR); pFile != nullptr) {
        auto attrL = pFile->tag()->attribute("WM/Picture");
        if (attrL.isEmpty()) { return Picture{21, NULL, 0, NULL}; }
        auto pic = attrL[0].toPicture();

        auto data = pic.picture();
        auto data_ptr = data.data();
        auto data_len = data.size();
        savedByteVectors.push_back(data);

        auto mime = pic.mimeType();
        auto mime_ptr = mime.toCString(true);
        savedStrings.push_back(mime);

        return Picture{0, data_ptr, data_len, mime_ptr};

    } else {
        return Picture{91, NULL, 0, NULL};
    }

    if (id3v2 == nullptr) { id3v2 = dynamic_cast<TagLib::ID3v2::Tag*>(pTagR); }
    if (xc == nullptr) { xc = dynamic_cast<TagLib::Ogg::XiphComment*>(pTagR); }

    if (id3v2 != nullptr) {
        auto frameL = id3v2->frameList("APIC");
        if(frameL.isEmpty()) { return Picture{31, NULL, 0, NULL}; }
        auto pic = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameL[0]);

        auto data = pic->picture();
        auto data_ptr = data.data();
        auto data_len = data.size();
        savedByteVectors.push_back(data);

        auto mime = pic->mimeType();
        auto mime_ptr = mime.toCString(true);
        savedStrings.push_back(mime);

        return Picture{0, data_ptr, data_len, mime_ptr};

    } else if (xc != nullptr) {
        if(xc->pictureList().isEmpty()) { return Picture{41, NULL, 0, NULL}; }
        TagLib::FLAC::Picture *target_pic = nullptr;

        for (const auto pic : xc->pictureList()) {
            if (pic != nullptr) {
                if (pic->type() == TagLib::FLAC::Picture::Type::FrontCover) {
                    target_pic = pic;
                }
            }
        }

        if (target_pic == nullptr) { target_pic = xc->pictureList()[0]; }

        auto data = target_pic->data();
        auto data_ptr = data.data();
        auto data_len = data.size();
        savedByteVectors.push_back(data);

        auto mime = target_pic->mimeType();
        auto mime_ptr = mime.toCString(true);
        savedStrings.push_back(mime);

        return Picture{0, data_ptr, data_len, mime_ptr};
    }

    return Picture{92, NULL, 0, NULL};
}

void taglib_picture_write_cover(TagLib_File *file, Picture picture, PictureMeta meta) {
    auto pFileR = reinterpret_cast<TagLib::File *>(file);
    auto pic_vec = TagLib::ByteVector(picture.data, picture.data_len);
    auto mime_str = TagLib::String(picture.mimetype, TagLib::String::UTF8);

    TagLib::ID3v2::Tag       *id3v2 = nullptr;
    TagLib::Ogg::XiphComment *xc    = nullptr;

    if (auto pFile = dynamic_cast<TagLib::MPEG::File*>(pFileR); pFile != nullptr) {
        id3v2 = pFile->ID3v2Tag(true);

    } else if (auto pFile = dynamic_cast<TagLib::MP4::File*>(pFileR); pFile != nullptr) {
        TagLib::MP4::CoverArt     cover(getfmt(picture.mimetype), pic_vec);
        TagLib::MP4::CoverArtList coverL; coverL.append(cover);
        TagLib::MP4::Item         coverI(coverL);
        pFile->tag()->setItem("covr", coverI);

    } else if (auto pFile = dynamic_cast<TagLib::FLAC::File*>(pFileR); pFile != nullptr) {
        if (pFile->hasID3v2Tag()) { id3v2 = pFile->ID3v2Tag(); }
        else {
            auto pic = new TagLib::FLAC::Picture();
            pic->setData(pic_vec);
            pic->setMimeType(mime_str);
            pic->setWidth(meta.width);
            pic->setHeight(meta.height);
            pic->setColorDepth(meta.depth);
            pic->setType(TagLib::FLAC::Picture::Type::FrontCover);
            pic->setDescription("Front Cover");

            pFile->addPicture(pic);
        }

    } else if (auto pFile = dynamic_cast<TagLib::ASF::File*>(pFileR); pFile != nullptr) {
        TagLib::ASF::Picture pic;
        pic.setPicture(pic_vec);
        pic.setMimeType(mime_str);
        pic.setType(TagLib::ASF::Picture::Type::FrontCover);
        pFile->tag()->addAttribute("WM/Picture", TagLib::ASF::Attribute(pic));

    } else { return; }

    if (id3v2 != nullptr) {
        auto frame = new TagLib::ID3v2::AttachedPictureFrame();
        frame->setPicture(pic_vec);
        frame->setMimeType(mime_str);
        id3v2->addFrame(frame);

    } else if (xc != nullptr) {
        auto pic = new TagLib::FLAC::Picture();
        pic->setData(pic_vec);
        pic->setMimeType(mime_str);
        pic->setType(TagLib::FLAC::Picture::Type::FrontCover);
        pic->setDescription("Front Cover");

        xc->addPicture(pic);
    }

    pFileR->save();
}

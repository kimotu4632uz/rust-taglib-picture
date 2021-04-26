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

#include "lib.h"

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
    std::vector<char *> savedStrings;

    char *stringToCharArray(const TagLib::String &s) {
        const std::string str = s.to8Bit(true);
        return ::strdup(str.c_str());
    }

    TagLib::String charArrayToString(const char *s) {
        return TagLib::String(s, TagLib::String::UTF8);
    }
}

TagLib_File *taglib_picture_open_file(const char *filename) {
    return reinterpret_cast<TagLib_File *>(TagLib::FileRef::create(filename));
}

void taglib_picture_free_file(TagLib_File *file) {
    delete reinterpret_cast<TagLib::File *>(file);
}

Picture *taglib_picture_read_cover(TagLib_File *file) {
    auto pFileR = reinterpret_cast<TagLib::File *>(file);
    auto pTagR = pFileR->tag();
//    auto pTagR  = audio.tag();

    TagLib::ID3v2::Tag*       id3v2 = nullptr;
    TagLib::Ogg::XiphComment* xc    = nullptr;

    if (auto pFile = dynamic_cast<TagLib::MPEG::File*>(pFileR); pFile != nullptr) {
        if (!pFile->hasID3v2Tag()) { return NULL; }
        id3v2 = pFile->ID3v2Tag();

    } else if (auto pFile = dynamic_cast<TagLib::MP4::File*>(pFileR); pFile != nullptr) {
        auto tag = pFile->tag();
        if (!tag->contains("covr")) { return NULL; }
        auto coverL = tag->item("covr").toCoverArtList();

        if (coverL.isEmpty()) { return NULL; }
        auto data = coverL[0].data();
        auto mime = getmime(coverL[0].format());

        auto result = (Picture *)malloc(sizeof(Picture));
        if (result == NULL) {
            return NULL;
        } else {
            result->data = data.data();
            savedByteVectors.push_back(data);
            result->mimetype = mime;
            return result;
        }

    } else if (auto pFile = dynamic_cast<TagLib::FLAC::File*>(pFileR); pFile != nullptr) {
        if (pFile->hasID3v2Tag()) { id3v2 = pFile->ID3v2Tag(); }
        else { xc = pFile->xiphComment(true); }

    } else if (auto pFile = dynamic_cast<TagLib::ASF::File*>(pFileR); pFile != nullptr) {
        auto attrL = pFile->tag()->attribute("WM/Picture");
        if (attrL.isEmpty()) { return NULL; }
        auto pic = attrL[0].toPicture();

        data = pic.picture();
        mime = pic.mimeType();
        auto result = (Picture *)malloc(sizeof(Picture));
        if (result == NULL) {
            return NULL;
        } else {
            result->data = data.data();
            savedByteVectors.push_back(data);
            result->mimetype = mime;
            return result;
        }

    } else { return NULL; }

    if (id3v2 == nullptr) { id3v2 = dynamic_cast<TagLib::ID3v2::Tag*>(pTagR); }
    if (xc == nullptr) { xc = dynamic_cast<TagLib::Ogg::XiphComment*>(pTagR); }

    if (id3v2 != nullptr) {
        auto frameL = id3v2->frameList("APIC");
        if(frameL.isEmpty()) { return 2; }
        auto cover = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameL[0]);

        data = cover->picture();
        mime = cover->mimeType();
        auto result = (Picture *)malloc(sizeof(Picture));
        if (result == NULL) {
            return NULL;
        } else {
            result->data = data.data();
            savedByteVectors.push_back(data);
            result->mimetype = mime;
            return result;
        }

    } else if (xc != nullptr) {
        if(xc->pictureList().isEmpty()) { return 1; }

        auto pic = xc->pictureList()[0];
        data = pic->data();
        mime = pic->mimeType();
        auto result = (Picture *)malloc(sizeof(Picture));
        if (result == NULL) {
            return NULL;
        } else {
            result->data = data.data();
            savedByteVectors.push_back(data);
            result->mimetype = mime;
            return result;
        }

    }
    return NULL;
}

void taglib_picture_write_cover(TagLib_File *file, Picture *picture) {
    auto pFileR = reinterpret_cast<TagLib::File *>(file);

    TagLib::ID3v2::Tag*       id3v2 = nullptr;
    TagLib::Ogg::XiphComment* xc    = nullptr;

    if (auto pFile = dynamic_cast<TagLib::MPEG::File*>(pFileR); pFile != nullptr) {
        id3v2 = pFile->ID3v2Tag(true);

    } else if (auto pFile = dynamic_cast<TagLib::MP4::File*>(pFileR); pFile != nullptr) {
        TagLib::MP4::CoverArt     cover(getfmt(picture->mimetype), picture->data);
        TagLib::MP4::CoverArtList coverL; coverL.append(cover);
        TagLib::MP4::Item         coverI(coverL);
        pFile->tag()->setItem("covr", coverI);

    } else if (auto pFile = dynamic_cast<TagLib::FLAC::File*>(pFileR); pFile != nullptr) {
        if (pFile->hasID3v2Tag()) { id3v2 = pFile->ID3v2Tag(); }
        else { xc = pFile->xiphComment(true); }

    } else if (auto pFile = dynamic_cast<TagLib::ASF::File*>(pFileR); pFile != nullptr) {
        TagLib::ASF::Picture pic;
        pic.setPicture(picture->data);
        pic.setMimeType(picture->mimetype);
        pic.setType(TagLib::ASF::Picture::Type::FrontCover);
        pFile->tag()->addAttribute("WM/Picture", TagLib::ASF::Attribute(pic));

    } else { return; }

    if (id3v2 != nullptr) {
        auto frame = new TagLib::ID3v2::AttachedPictureFrame();
        frame->setPicture(picture->data);
        frame->setMimeType(picture->mimetype);
        id3v2->addFrame(frame);

    } else if (xc != nullptr) {
        auto pic = new TagLib::FLAC::Picture();
        pic->setData(picture->mimetype);
        pic->setMimeType(picture->mimetype);
        pic->setType(TagLib::FLAC::Picture::Type::FrontCover);
        pic->setDescription("Front Cover");

        xc->addPicture(pic);
    }

    pFileR->save();
}

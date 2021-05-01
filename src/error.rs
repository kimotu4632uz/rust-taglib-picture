use std::str::Utf8Error;
use thiserror::Error;
use image::error::ImageError;

#[derive(Error, Debug)]
pub enum TagLibError {
    #[error("mp3 file do not include id3v2 tag")]
    Mp3fileTag,

    #[error("mp4 file do not include covr frame")]
    Mp4fileCovrFrame,

    #[error("covr frame in mp4 file is empty")]
    Mp4fileCovrList,

    #[error("asf file not include cover art")]
    AsffileTag,

    #[error("id3v2 tag not include cover art")]
    Id3v2Tag,

    #[error("xiph tag not include cover art")]
    XiphTag,

    #[error("unsupported file format")]
    UnsupportedFile,

    #[error("both id3v2 and xiph tag not included")]
    Id3andXiphNotFound,

    #[error("unknown internal error")]
    Unknown,
}

impl TagLibError {
    pub fn from_status_code(code: i32) -> TagLibError {
        match code {
            1 => TagLibError::Mp3fileTag,
            11 => TagLibError::Mp4fileCovrFrame,
            12 => TagLibError::Mp4fileCovrList,
            21 => TagLibError::AsffileTag,
            31 => TagLibError::Id3v2Tag,
            41 => TagLibError::XiphTag,
            91 => TagLibError::UnsupportedFile,
            92 => TagLibError::Id3andXiphNotFound,
            _ => TagLibError::Unknown,
        }
    }
}

#[derive(Error, Debug)]
pub enum Error {
    #[error("invalid file name")]
    InvalidFileName,

    #[error("an error occured while parsing string: {0}")]
    Utf8Error(Utf8Error),

    #[error("{0}")]
    TagLibError(TagLibError),

    #[error("an error occured while loading image: {0}")]
    ImageError(ImageError),
}

impl From<Utf8Error> for Error {
    fn from(e: Utf8Error) -> Error {
        Error::Utf8Error(e)
    }
}

impl From<ImageError> for Error {
    fn from(i: ImageError) -> Error {
        Error::ImageError(i)
    }
}
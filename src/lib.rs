use std::path::Path;
use std::ffi::{CString, CStr};
use std::os::raw::c_char;

use image::GenericImageView;

use taglib_picture_sys as taglib;
pub mod error;
use error::{Error, TagLibError};

use std::result::Result as StdResult;
type Result<T> = StdResult<T, Error>;

pub struct File {
    inner: *mut taglib::TagLib_File,
}

impl Drop for File {
    fn drop(&mut self) {
        unsafe { taglib::taglib_picture_free_file(self.inner) }
    }
}

impl File {
    pub fn new<P: AsRef<Path>>(filename: P) -> Result<File> {
        let filename = filename.as_ref().to_str().unwrap();
        let c_str = CString::new(filename).unwrap();
        let c_str_ptr = c_str.as_ptr();

        let inner = unsafe { taglib::taglib_picture_open_file(c_str_ptr) };
        if inner.is_null() {
            Err(Error::InvalidFileName)
        } else {
            Ok(File { inner: inner })
        }
    }

    pub fn read_cover(&self) -> Result<(Vec<u8>, String)> {
        let result = unsafe { taglib::taglib_picture_read_cover(self.inner) };
        if result.status_code != 0 {
            Err(Error::TagLibError(TagLibError::from_status_code(result.status_code)))
        } else {
            let data = unsafe {
                let u8_ptr = result.data as *const u8;
                std::slice::from_raw_parts(u8_ptr, result.data_len as usize)
            };

            let mut data_copy = Vec::with_capacity(data.len());
            data_copy.resize(data.len(), 0);
            data_copy.clone_from_slice(data);
            drop(data);
            unsafe { taglib::taglib_picture_free_vecs() };

            let mime = unsafe { CStr::from_ptr(result.mimetype) };
            let mime_slice = mime.to_bytes();
            let mut mime_vec = Vec::with_capacity(mime_slice.len());
            mime_vec.resize(mime_slice.len(), 0);
            mime_vec.clone_from_slice(mime_slice);
            let mime_string = String::from_utf8(mime_vec).map_err(|e| e.utf8_error())?;
            unsafe { taglib::taglib_picture_free_strs() };

            Ok((data_copy, mime_string))
        }
    }

    pub fn write_cover<U: AsRef<[u8]>, S: Into<Vec<u8>>>(&self, data: U, mimetype: S) -> Result<()> {
        let data = data.as_ref();

        let image = image::load_from_memory(data)?;

        let (width, height) = image.dimensions();
        let depth = image.color().bits_per_pixel() as i32;

        let data_len = data.len() as u32;
        let data_u8_ptr = data.as_ptr();
        let data_ptr = data_u8_ptr as *const c_char;

        let mime_c_string = CString::new(mimetype).unwrap();
        let mime_ptr = mime_c_string.as_ptr();

        unsafe { taglib::taglib_picture_write_cover(
            self.inner,
            taglib::Picture{
                data: data_ptr,
                data_len: data_len,
                mimetype: mime_ptr,
                status_code: 0
            },
            taglib::PictureMeta{
                width: width as i32,
                height: height as i32,
                depth: depth
            }) };

        Ok(())
    }
}

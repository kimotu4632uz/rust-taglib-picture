use std::path::Path;
use std::ffi::{CString, CStr};
use std::os::raw::c_char;

use image::GenericImageView;

use taglib_picture_sys as sys;

pub mod error;
use error::{Error, TagLibError};

use std::result::Result as StdResult;
type Result<T> = StdResult<T, Error>;

pub struct File {
    inner: *mut sys::TagLib_File,
}

impl Drop for File {
    fn drop(&mut self) {
        unsafe { sys::taglib_picture_free_file(self.inner) }
    }
}

impl File {
    pub fn new<P: AsRef<Path>>(filename: P) -> Result<File> {
        let filename = filename.as_ref().to_str().unwrap();
        let c_str = CString::new(filename).unwrap();
        let c_str_ptr = c_str.as_ptr();

        let inner = unsafe { sys::taglib_picture_open_file(c_str_ptr) };
        if inner.is_null() {
            Err(Error::InvalidFileName)
        } else {
            Ok(File { inner: inner })
        }
    }

    pub fn read_cover(&self) -> Result<(Vec<u8>, String)> {
        let result = unsafe { sys::taglib_picture_read_cover(self.inner) };
        if result.status_code != 0 {
            Err(Error::TagLibError(TagLibError::from_status_code(result.status_code)))
        } else {
            let data = unsafe {
                let u8_ptr = result.data as *const u8;
                std::slice::from_raw_parts(u8_ptr, result.data_len as usize)
            };

            let data_copy = data.to_vec();
            drop(data);
            unsafe { sys::taglib_picture_free_vecs() };

            let mime = unsafe { CStr::from_ptr(result.mimetype) };
            let mime_str = mime.to_str().unwrap();
            let mime_string = mime_str.to_string();

            unsafe { sys::taglib_picture_free_strs() };

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

        unsafe { sys::taglib_picture_write_cover(
            self.inner,
            sys::Picture{
                data: data_ptr,
                data_len: data_len,
                mimetype: mime_ptr,
                status_code: 0
            },
            sys::PictureMeta{
                width: width as i32,
                height: height as i32,
                depth: depth
            }) };

        Ok(())
    }
}

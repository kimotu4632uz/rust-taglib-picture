use std::path::Path;
use std::ffi::CString;
use taglib_picture_sys as taglib;
use libc;

pub struct File {
    inner: *mut taglib::TagLib_File,
}

impl File {
    pub fn new<P: AsRef<Path>>(filename: P) -> File {
        let filename = filename.as_ref().to_str().unwrap();
        let c_str = CString::new(filename).unwrap();
        let c_str_ptr = c_str.as_ptr();

        File { inner: unsafe { taglib::taglib_picture_open_file(c_str_ptr) } }
    }

    pub fn read_cover(&self) -> Result<&[u8], ()> {
        let result = unsafe {taglib::taglib_picture_read_cover(self.inner)};
        if result.is_null() {
            Err(())
        } else {

        }
    }
}
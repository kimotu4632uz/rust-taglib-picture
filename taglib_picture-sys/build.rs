use std::path::PathBuf;

fn main() {
    cc::Build::new()
        .cpp(true)
        .warnings(true)
        .flag("-Wall")
        .flag("-std=c++17")
        .file("src/c/taglib_c_picture.cpp")
        .include("src/c")
        .compile("libtaglib_c_picture.a");
    
    bindgen::Builder::default()
        .header("src/c/taglib_c_picture.h")
        .generate()
        .expect("unable to generate binding.")
        .write_to_file(PathBuf::from(std::env::var("OUT_DIR").unwrap()).join("binding.rs"))
        .expect("couldn't write binding file.");
}
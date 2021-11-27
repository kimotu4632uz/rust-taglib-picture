use std::path::PathBuf;

fn main() {
    let lib = pkg_config::probe_library("taglib").expect("Unable to find taglib.");

    cc::Build::new()
        .cpp(true)
        .warnings(true)
        .flag("-Wall")
        .flag("-std=c++17")
        .file("src/c/tag_c_picture.cpp")
        .includes(lib.include_paths)
        .include("src/c")
        .compile("libtag_c_picture.a");

    bindgen::Builder::default()
        .header("src/c/tag_c_picture.h")
        .generate()
        .expect("unable to generate binding.")
        .write_to_file(PathBuf::from(std::env::var("OUT_DIR").unwrap()).join("binding.rs"))
        .expect("couldn't write binding file.");
    
    println!("cargo:rerun-if-changed=src/c/tag_c_picture.cpp")
}
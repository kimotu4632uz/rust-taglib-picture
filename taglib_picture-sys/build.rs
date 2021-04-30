use std::path::PathBuf;

fn main() {
    let lib_res = pkg_config::probe_library("taglib");
    if let Err(e) = lib_res {
        println!("{}", e);
    } else {
        let lib = lib_res.unwrap();

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
    }
}
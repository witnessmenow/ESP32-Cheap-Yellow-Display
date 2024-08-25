fn main() {
    println!("cargo:rustc-link-arg-bins=-Tlinkall.x");

    println!("cargo:rustc-link-arg-bins=-Trom_functions.x");
}

/*
prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/arm-linux-androideabi/bin/ld: error: version script assignment of LIBC to symbol dummy_dlsym failed: symbol not defined
prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/arm-linux-androideabi/bin/ld: error: version script assignment of LIBC_N to symbol dummy_dlvsym failed: symbol not defined
prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/arm-linux-androideabi/bin/ld: error: version script assignment of LIBC_OMR1 to symbol dummy___cfi_shadow_size failed: symbol not defined
*/

void dummy_dlsym(void){}
void dummy_dlvsym(void){}
void dummy___cfi_shadow_size(void){}
